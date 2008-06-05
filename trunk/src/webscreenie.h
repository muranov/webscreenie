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

#ifndef __WEBSCREENIE_H__
#define __WEBSCREENIE_H__

//#include <stdlib.h>

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QUrl>
#include <QWebView>

/**
*
*/
class PreprocOptions : public QObject
{
	Q_OBJECT
public:

	/**
	* Distance, in pixels, from upper-left corner of the web page
	*/
	QPoint offset;
	
	/**
	* Size of the screen to grab. If 0x0, grab everything
	*/
	QSize size;
	
	/**
	* Target size of the final image
	*/
	QSize targetSize;
	
	/**
	* Javascript to apply before rendering
	*/
	QString script;
	
	/**
	* CSS to apply before rendering
	*/
	QString css;
	
	/**
	* Delay, in microseconds, before rendering
	*/
	int delay;
};

/**
*
*/
class WebScreenie : public QObject 
{
	Q_OBJECT
public:
	/**
	* The QWebView will be used to load & render the web page
	*/
	QWebView webview;
	
	/**
	* Options
	*/
	PreprocOptions options;
	
	/**
	* Input URL
	*/
	QUrl url;
	/**
	* Output file
	*/
	QString file;
	
	/**
	* Print basic usage information
	*/
	void usage(FILE * fd);
	
	/**
	* Get the options from the command line
	*/
	void parseArgs(int argc, const char** argv);
	
	/**
	* Go.
	*/
	void run(int argc, const char** argv);
	
public slots:

	/**
	* Called when the page has finished loading
	*
	* This is where the screenshot should be saved
	*
	* @param bool	Success / Failure
	*/
	void loadFinished(bool success);
	
	/**
	* Called while the page is being loaded.
	* Display feedback about progress status
	*
	* @param int	Progress so far, in percentage
	*/
	void loadProgress(int progress);
	
	void load(const QUrl url);
	
	void interactiveMode();
	
	void render();
	
	void save() { };

};

#endif //__WEBSCREENIE_H__
