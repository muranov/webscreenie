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

#include <iostream> 
#include <string>

#include <QtDebug>
#include <QtGui>
#include <QtScript>

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
"\n"
"Scripting Options:\n"
"  --interactive          run as an interactive QtScript shell\n"
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
					fprintf(stderr, "Invalid offset parameter: %s. Must be in format \"100x100\"", arg.toAscii().constData());
					usage(stderr);
					exit(1);
				}
			}
			else if(!strcmp(argv[i],"--size")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				QString arg = argv[++i];
				QRegExp regex("(\\d+)x(\\d+)");
				int pos = regex.indexIn(arg);
				if(pos > -1) {
					options.size.setWidth(regex.cap(1).toInt());
					options.size.setHeight(regex.cap(2).toInt());
				}
				else {
					fprintf(stderr, "Invalid size parameter: %s.  Must be in format \"100x100\"", arg.toAscii().constData());
					usage(stderr);
					exit(1);
				}
				
			}
			else if(!strcmp(argv[i],"--target_size")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				QString arg = argv[++i];
				QRegExp regex("(\\d+)x(\\d+)");
				int pos = regex.indexIn(arg);
				if(pos > -1) {
					options.targetSize.setWidth(regex.cap(1).toInt());
					options.targetSize.setHeight(regex.cap(2).toInt());
				}
				else {
					fprintf(stderr, "Invalid size parameter: %s. Must be in format \"100x100\"", arg.toAscii().constData());
					usage(stderr);
					exit(1);
				}
			}
			else if(!strcmp(argv[i], "--interactive")) {
				interactiveMode();
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
void WebScreenie::interactiveMode()
{
	printf("WebScreenie Interactive Mode. Type \"help\" for a list of commands\n");
	
	QScriptEngine engine(this);
	QScriptValue main = engine.newQObject(this);
     	engine.globalObject().setProperty("main", main);

	while(1) {
		
		printf("> ");
		
		char line[80];
		scanf("%s", line);
		QString cmd;
		
		cmd = line;
		
		if(cmd == "help") {
			printf("Type \"quit\" to exit\n");
			continue;
		}
		
		if(cmd == "quit" || cmd == "exit") {
			exit(0);
		}
		
		QScriptValue result = engine.evaluate(cmd);

		QString resultString = result.toString();
		
		qDebug() << resultString;
		
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
	
	url = "http://code.google.com/p/webscreenie/";
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
	// Hide scrollbars
	webview.page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	webview.page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
	
	// TODO: Read from command-line / UI options
	options.script = ""
	"var links = document.getElementsByTagName('a');"
	"for(var i = 0; i < links.lgth; i++) {"
	"	links[i].style.border = '3px solid blue';"
	"}";
	
	if(!options.script.isEmpty()) {
		QVariant result = webview.page()->mainFrame()->evaluateJavaScript(options.script);
		qDebug() << result.toString();
	}
	
	if(!options.css.isEmpty()) {
		
	}
	
	// Calculate page dimension
	int pageWidth = options.size.width() ? options.size.width() : webview.page()->mainFrame()->contentsSize().width();
	int pageHeight = options.size.height() ? options.size.height() : webview.page()->mainFrame()->contentsSize().height();
	pageWidth += options.offset.x();
	pageHeight += options.offset.y();
	
	// Set the dimension
	webview.page()->setViewportSize(QSize(pageWidth, pageHeight));
	
	// Create the target image
	QImage image(webview.page()->viewportSize(), QImage::Format_ARGB32);
	QPainter painter(&image);
	
	webview.page()->mainFrame()->render(&painter);
	painter.end();
	
	// Crop
	int w = webview.page()->viewportSize().width() - options.offset.x();
	int h = webview.page()->viewportSize().height() - options.offset.y();
	QImage cropped = image.copy(options.offset.x(), options.offset.y(), w, h);
	
	// Resize
	QImage resized;
	if(options.targetSize.width() && options.targetSize.height()) {
		resized = cropped.scaled(options.targetSize.width(), options.targetSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	else if(options.targetSize.width()) {
		resized = cropped.scaledToWidth(options.targetSize.width(), Qt::SmoothTransformation);
	}
	else if(options.targetSize.height()) {
		resized = cropped.scaledToHeight(options.targetSize.height(), Qt::SmoothTransformation);
	}
	else {
		resized = cropped.copy();
	}
	//QImage resized = image.scaled();
	
	resized.save(file);
	
	
	// Success!
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
	WebScreenie *x = new WebScreenie();
	x->run(argc,(const char **)argv);
	delete x;
	return a.exec();
}