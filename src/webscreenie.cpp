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
#include <QTimer>
#include <QWebFrame>
#include <QWebPage>
#include <QWebView>

QApplication * app;

WebScreenie::WebScreenie()
{
	feedbackTimer = new QTimer(this);
}

WebScreenie::~WebScreenie()
{
	delete(feedbackTimer);
}

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
"  --offset <WxH>         offset from upper-top to crop\n"
"  --size <WxH>           dimension of the web page capture\n"
"  --target_size <WxH>    target dimension of the output image\n"
"  --zoom <factor>        zoom factor (float) for the whole page\n"
"  --textzoom <factor>    text zoom factor (float)\n"
"\n"
"Advanced Options:\n"
"  --script <script>      custom javascript to execute before render\n"
"  --script_file <file>   custom javascript file to execute before render\n"
"  --css <css>            custom CSS to change render style\n"
"  --css_file <file>      custom CSS\n"
"  --delay <ms>           delay, in milliseconds, before render\n"
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
			else if(!strcmp(argv[i], "--zoom")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				QString arg = argv[++i];
				options.zoom = arg.toFloat();
			}
			else if(!strcmp(argv[i], "--textzoom")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				QString arg = argv[++i];
				options.textzoom = arg.toFloat();
			}
			else if(!strcmp(argv[i], "--script")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				QString arg = argv[++i];
				options.script = arg;
			}
			else if(!strcmp(argv[i], "--script_file")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				QString arg = argv[++i];
				QFile file(arg);
				if (!file.open(QIODevice::ReadOnly)) {
					fprintf(stderr, "Could not open file \"%s\".", arg.toAscii().constData());
					usage(stderr);
					exit(1);
				}
				QTextStream stream(&file);
				while(!(stream.atEnd())) {
					options.script += (stream.readLine() + "\n");
				}
				file.close();
				
			}
			else if(!strcmp(argv[i], "--css")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				QString arg = argv[++i];
				options.css = arg;
			}
			else if(!strcmp(argv[i], "--css_file")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				QString arg = argv[++i];
				QFile file(arg);
				if (!file.open(QIODevice::ReadOnly)) {
					fprintf(stderr, "Could not open file \"%s\".", arg.toAscii().constData());
					usage(stderr);
					exit(1);
				}
				QTextStream stream(&file);
				while(!(stream.atEnd())) {
					options.css += (stream.readLine() + "\n");
				}
				file.close();
				
			}
			else if(!strcmp(argv[i], "--delay")) {
				if(i+1>= argc) {
					usage(stderr);
					exit(1);
				}
				QString arg = argv[++i];
				options.delay = arg.toInt();
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
/*
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
*/
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
	
	options.zoom = 1.0;
	options.textzoom = 1.0;
	
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
	
	if(options.delay > 0) {
	    printf("Waiting %d milliseconds before snapshot..", options.delay);
	    QTimer::singleShot(options.delay, this, SLOT(render()));
	    
	    // User feedback in case delay is longer than 2 seconds
	    if(options.delay > 2000) {
		
		connect(feedbackTimer, SIGNAL(timeout()), this, SLOT(delayFeedback()));
		feedbackTimer->start(1000);
	    }
	    
	}
	else {
	    render();
	}	
	
}

void WebScreenie::delayFeedback()
{
    printf(".");
    fflush(stdout);
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
	printf("\n");
	fflush(stdout);
	
	// Stop feedback
	feedbackTimer->stop();
	
	// Hide scrollbars
	webview.page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	webview.page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
	
	// Set the zoom factor & text zoom
	if(options.zoom != 1.0) {
		webview.page()->mainFrame()->setZoomFactor(options.zoom);
	}
	if(options.textzoom != 1.0) {
		webview.page()->mainFrame()->setTextSizeMultiplier(options.textzoom);
	}
	
	// Execute custom javascript
	if(!options.script.isEmpty()) {
		QVariant jsResult = webview.page()->mainFrame()->evaluateJavaScript(options.script);
	}
	
	// Add custom CSS
	if(!options.css.isEmpty()) {
		// TODO: What's the proper way of adding a style element in the page with QtWebKit?
		// Handle this with javascript for now...
		
		// Will fail if options.css contains "'" (?)
		
		QString cssJs;
		cssJs.append("var style = document.createElement('style');var txt = document.createTextNode('");
		cssJs.append(options.css);
		cssJs.append("'); style.appendChild(txt);document.getElementsByTagName('head')[0].appendChild(style);");

 		QVariant cssResult = webview.page()->mainFrame()->evaluateJavaScript(cssJs);
		
	}
	
	// Calculate page dimension, or try to use best possible size
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
	
	app->quit();
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
	WebScreenie webscreenie;
	webscreenie.run(argc,(const char **)argv);
	return a.exec();
}
