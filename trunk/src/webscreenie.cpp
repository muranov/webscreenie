/*
Copyright 2008 Mathieu Ducharme <ducharme.mathieu@gmail.com>

This file is part of WebScreenie.

WebScreenie is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

WebScreenie is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with WebScreenie.  If not, see <http://www.gnu.org/licenses/>.

This file was based on "wkhtmltopdf"
http://code.google.com/p/wkhtmltopdf/
*/

#include "webscreenie.h"

#include <string.h>

#include <QtGui>
#include <QString>
#include <QImage>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>

QApplication * app;

/*
*
*/
void WebScreenie::usage(FILE * fd) 
{
	fprintf(fd,
"Usage: webscreenie [OPTIONS]... [<input file> [<output file>]]\n"
"\n"
"Converts a html page to an image\n"
"Use without options for the graphical user interface\n"
"\n"
"Options:\n"
"  -?, --help                 show this help / usage message\n"
"  -u, --url <url>            url to open\n"
"  -f, --file <url>         use url as output\n"
"  -o, --options <file>       configuration file\n" 
"\n"
"Image Options:\n"
"  -x, --offset_x <pixels>    offset from top to crop\n"
"  -y, --offset_y <pixels>    offset from left to crop\n"
"  -w, --width <pixels>       width of the image to grab\n"
"  -h, --height <pixels>      height of the image to grab\n"
"  -r, --ratio <percentage>   scale ratio (ex: 0.5)\n"
"\n"
"Advanced Options:\n"
"  -s, --script <script>      custom javascript to execute before render\n"
"      --script_file <file>   custom javascript file to execute before render\n"
"  -c, --css <css>            custom CSS to change render style\n"
"      --css_file <file>      custom CSS\n"
"  -d, --delay <seconds>      delay, in seconds, before render\n"
	);
}

/*
*
*/
void WebScreenie::parseArgs(int argc, const char ** argv)
{
	
	//Load default configuration
	in = "http://www.example.com";
	out = "example.png";
	
	int x=0;
	for(int i=1; i < argc; ++i) {

		if(argv[i][0] != '-') {
			//Default arguments
			++x;
			if(x==1) {
				in = argv[i];
			}
			else if(x == 2) {
				out = argv[i];
			}
			else {
				usage(stderr);
				exit(1);
			}
			continue;
		}
		
		if(argv[i][1] == '-') { //Long style arguments
			if(!strcmp(argv[i],"--help")) {
				usage(stdout); 
				exit(0);
			}
			else if(!strcmp(argv[i],"--input")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				in = argv[++i];
			} 
			else if(!strcmp(argv[i],"--output")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				out = argv[++i];
			} 
			else {
				usage(stderr);
				exit(1);
			}
			continue;
		}

		//Short style arguments
		int c=i;
		for(int j=1; argv[c][j] != '\0'; ++j) {
			switch(argv[c][j]) {
				case 'h':
					usage(stdout); 
					exit(0);
				break;
	
				case 'i':
					if(i+1>= argc) {
						usage(stderr);
						exit(1);
					}
					in = argv[++i];
				break;
	
				case 'o':
					if(i+1>= argc) {
						usage(stderr);
						exit(1);
					}
					out = argv[++i];
				break;
	
				default:
					usage(stderr);
					exit(1);
				break;
			}
		}
	}
}

/*
*
*/
void WebScreenie::run(int argc, const char ** argv)
{
	//Parse the arguments
	parseArgs(argc,argv);
	//Make a url of the input file
	QUrl url(in);
	
	//When loading is progressing we want loadProgress to be called
	connect(&webview, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
	//Once the loading is done we want loadFinished to be called
	connect(&webview, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
	
	//Tell the vebview to load for the newly created url
	webview.load(url);
}

/*
*
*/
void WebScreenie::loadFinished(bool success)
{
	printf("\n");
	fflush(stdout);

	if(!success) {
		//It went bad, return with 1
		printf("Failed loading page\n");
		app->exit(1);
		return;
	}
	
	// Maximize content
	webview.page()->setViewportSize(webview.page()->mainFrame()->contentsSize());
	QImage image(webview.page()->viewportSize(), QImage::Format_ARGB32);
	QPainter painter(&image);
	
	// TODO: Read from command-line / UI options
	QString script;
	script = "var links = document.getElementsByTagName('a');for(var i = 0; i < links.length; i++) {links[i].style.border = '2px solid red';}";
	
	if(!script.isEmpty()) {
		QVariant result;
		result = webview.page()->mainFrame()->evaluateJavaScript(script);
		//qDebug() << result;
	}
	
	webview.page()->mainFrame()->render(&painter);
	painter.end();
	
	image.save(out);
	
	printf("Image successfully saved to \"%s\"\n", out);
	
	app->quit();
}

/*
*
*/
void WebScreenie::loadProgress(int progress)
{
	//Print out the load status
	printf("Loading page \"%s\": %d%%   \r", in, progress);
	fflush(stdout);
}

/*
*
*/
int main(int argc, char * argv[]) 
{
	QApplication a(argc,argv);
	app = &a;
	WebScreenie x;
	x.run(argc,(const char **)argv);
	return a.exec();
}
