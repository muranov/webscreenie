## Usage: ##
webscreenie [OPTIONS](OPTIONS.md)... [<input url> [<output file>]]

Converts a html page to an image
Use without options for the graphical user interface

## Example: ##
webscreenie --url http://www.example.com --file example.png --target\_size 640x480

### Options: ###
  * --help                 show this help / usage message
  * --url 

&lt;url&gt;

            url to open
  * --file 

&lt;url&gt;

           use url as output

### Image Options: ###
  * --offset 

&lt;WxH&gt;

         offset from upper-top to crop
  * --size 

&lt;WxH&gt;

           dimension of the web page capture
  * --target\_size 

&lt;WxH&gt;

    target dimension of the output image

### Advanced Options: ###
  * --script 

&lt;script&gt;

      custom javascript to execute before render
  * --script\_file 

&lt;file&gt;

   custom javascript file to execute before render
  * --css 

&lt;css&gt;

            custom CSS to change render style
  * --css\_file 

&lt;file&gt;

      custom CSS
  * --delay 

&lt;ms&gt;

           delay, in milliseconds, before render
  * --options 

&lt;file&gt;

       configuration file (Not yet implemented)