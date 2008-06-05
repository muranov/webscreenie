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

#include <QtDebug>
#include <QtGui>

#include <QImage>
#include <QString>
#include <QWebFrame>
#include <QWebPage>
#include <QWebView>

QApplication * app;

/*
*
*/
void WebScreenie::usage(FILE * fd) 
{
	fprintf(fd,
"Usage: webscreenie [OPTIONS]... [<input url> [<output file>]]\n"
"\n"
"Converts a html page to an image\n"
"Use without options for the graphical user interface\n"
"\n"
"Example:\n"
"webscreenie --url http://www.example.com --file example.png --target_size 640x480\n"
"\n"
"Options:\n"
"  --help                 show this help / usage message\n"
"  --url <url>            url to open\n"
"  --file <url>           use url as output\n"
"\n"
"Image Options:\n"
"  --offset <WxY>         offset from upper-top to crop\n"
"  --size <WxY>           dimension of the web page capture\n"
"  --target_size <WxY>    target dimension of the output image\n"
"\n"
"Advanced Options:\n"
"  --script <script>      custom javascript to execute before render\n"
"  --script_file <file>   custom javascript file to execute before render\n"
"  --css <css>            custom CSS to change render style\n"
"  --css_file <file>      custom CSS\n"
"  --delay <ms>           delay, in microseconds, before render\n"
"  --options <file>       configuration file\n" 
	);
}

/*
*
*/
void WebScreenie::parseArgs(int argc, const char ** argv)
{
	for(int i=1; i < argc; ++i) {

		if(argv[i][1] == '-') { //Long style arguments
			if(!strcmp(argv[i],"--help")) {
				usage(stdout); 
				exit(0);
			}
			else if(!strcmp(argv[i],"--url")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				url = argv[++i];
			} 
			else if(!strcmp(argv[i],"--file")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				file = argv[++i];
			}
			else if(!strcmp(argv[i],"--offset")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				QString arg = argv[++i];
				QRegExp regex("(\\d+)x(\\d+)");
				int pos = regex.indexIn(arg);
				if(pos > -1) {
					options.offset.setX(regex.cap(1).toInt());
					options.offset.setY(regex.cap(2).toInt());
				}
				else {
					fprintf(stderr, "Invalid offset parameter: %s", arg.toAscii().constData());
					usage(stderr);
					exit(1);
				}
			}
			else if(!strcmp(argv[i],"--size")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				;
				
			}
			else if(!strcmp(argv[i],"--target_size")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
			}
			else {
				usage(stderr);
				exit(1);
			}
			continue;
		}
	}
}

/*
*
*/
void WebScreenie::run(int argc, const char ** argv)
{
	// Set the default options
	options.offset.setX(0);
	options.offset.setY(0);
	
	options.size.setWidth(0);
	options.size.setHeight(0);
	
	options.targetSize.setWidth(0);
	options.targetSize.setHeight(0);
	
	options.delay = 0;
	
	url = "http://www.example.com";
	file = "webscreenie.png";

	parseArgs(argc,argv);
	
	connect(&webview, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
	connect(&webview, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
	
	load(url);
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
	
	render();
	
	app->quit();
}
/*
*
*/
void WebScreenie::load(QUrl url)
{
	webview.load(url);
}

/*
*
*/
void WebScreenie::render()
{
	// Maximize content
	webview.page()->setViewportSize(webview.page()->mainFrame()->contentsSize());
	
	// Hide scrollbars
	webview.page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	webview.page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
	
	// TODO: Read from command-line / UI options
	options.script = ""
	"var links = document.getElementsByTagName('a');"
	"for(var i = 0; i < links.length; i++) {"
	"	links[i].style.border = '3px solid blue';"
	"}";
	
	if(!options.script.isEmpty()) {
		QVariant result = webview.page()->mainFrame()->evaluateJavaScript(options.script);
		qDebug() << result.toString();
	}
	
	if(!options.css.isEmpty()) {
		
	}
	
	int x = options.offset.x();
	int y = options.offset.y();
	int w = webview.page()->viewportSize().width() - x;
	int h = webview.page()->viewportSize().height() - y;

	QImage image(webview.page()->viewportSize(), QImage::Format_ARGB32);
	QPainter painter(&image);
	
	webview.page()->mainFrame()->render(&painter);
	painter.end();
	
	// Crop
	QImage cropped = image.copy(x,y,w,h);
	
	// Resize
	//QImage resized = image.scaled();
	
	cropped.save(file);
	
	
	
	printf("Image successfully saved to \"%s\"\n", file.toAscii().constData());
}

/*
*
*/
void WebScreenie::loadProgress(int progress)
{
	//Print out the load status
	printf("Loading page \"%s\": %d%%   \r", url.toString().toAscii().constData(), progress);
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
