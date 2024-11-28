# PARSER NOTES
	Here I will take notes for anything important regarding the parsing of the .conf file
## possible approach
### Idea
	We will have a "Parser" class, that will implicitly represent the http Block
		--> it holds a vector of "ServerBlocks"
			--> those will each hold a vector of "LocationBlocks" & their attributes, partly inherited from "Block" class
				--> those will have their attributes, also partly inherited from "Block" class

#### Inheritence
	We will have a "Block" class as Base for the Server and Location Blocks - those will be the Derived classes

## structure of blocks and directives

	one config file:
		multiple server blocks in one http block;
		possible blocks:
			http --> how many is valid in one conf file? only one is valid, nginx gives a dublicate error otherwise
				server --> does it always belong in a http block? is there a minimum of one for server block? yes (server always inside a http block), no (no minimum, also possible w/o any server block)!
					location --> does it always belong in a server block? is it required for valid conf file?
						location (nesting possible)
			? only these three?
			CGIBlock, too?


		possible directives:
			-listen
			-server_name
			--root
			-alias (maybe not mandatory), only location
			-allowed_methods (limit_except in nginx)
			-client_max_body_size
			--error_page
			--index --> must check whether in both blocks, and purpose
			--autoindex --> must check whether in both blocks, and purpose
			--return
			--try_files



		possible location directives:
			allowed_methods
			root
			alias (maybe not mandatory)
			error_page
			index --> specifies the file(s) to server for current location (overrides definitions from server block)
			autoindex
			try_files

		possible server directives:
			listen port (listening to)
			server_name (host IP-Address)
			root
			index --> specifies the file(s) to server for current server block
			autoindex
			error_page
			allowed_methods
			location
			client_max_body_size
			return 307 / --> nginx accepts 307 to redirect, other codes would not redirect
			try_files



# syntax rules overview

	There must be a http block to define the protocol in use for the server(s) inside.

	the syntax is not strict:
	* everything (all directives and blocks) can be in just one line.
	* whitespace is only required for the directive keys and values to be separated.
	* the rest (such as {, }, ;) can, BUT doesn’t need any whitespace separation form anything.
	* newlines are treated similar to whitespaces, directive key and value can be separated by newline(s)
	* so newlines are allowed before a ';' -> a directive doesn't need to be followed by a ';' on the same line, but can also at the next line



	Dublicates:
	* multiple http blocks --> duplicate error
	* no dublicates of location block with the same path allowed (2 times --> location /hello)
	* in case multiple servers with the same host (server_name) and port (listen), nginx will only give a warning and take the first as default, the others will NOT be used
	* multiple root in server/location block --> duplicate error
	* multiple listen in server block --> duplicate error
	* 

	Not Existing:
	* in case of no location in a server block, it takes the root from the server
		* if also no root in server: nginx default page shows up
			* how to interpret that?
	* in case no server_name in server block
		* it also only shows the nginx default page


	* alias --> don't think we need to handle alias
		* acts a bit weird
			* sometimes (location /) without '/' at the end, works, sometimes (location /hello) not
			* with '/' at the end it always works
		* only allowed in the location block

	* check order
		* order of directives doesn't matter

	* what is allowed in http block, except for server block?
		* allowed: root, index, autoindex
		* not allowed: location block, server_name, listen, return, 




## this is an example of a valid conf file syntax:

	http{server{listen 8090   ;server_name 127.0.0.1;location /hello{alias /home/aismaili/webSite;index index.html;}}}

## subject
### default server
	The first server for a host:port will be the default for this host:port (that means
	it will answer to all the requests that don’t belong to an other server).

# Defaults of all implemented directives
## those with no default
### error_page
### return

## those with defaults
### listen
Default: listen *:80 | *:8000;

### server_name
Default: server_name "";

### root
Default: root html; that is a relative path from the nginx excecutable file -> Q: what if someone deletes it or it doesn't exist? A: server just give the error 404 Not Found

### index
Default: index index.html;

### autoindex
Default: autoindex off;

### error_page
Syntax:	error_page code ... [=[response]] uri;
Default:	—
Context:	http, server, location, if in location

### allowed_methods
Default: 


### location
Default: 


### client_max_body_size
Syntax:		client_max_body_size size;  note: Setting size to 0 disables checking of client request body size.
Default:	client_max_body_size 1m; If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client.
Context:	http, server, location

### return
Syntax:	return code [text];
		return code URL;
		return URL;
Default:	—
Context:	server, location, if


### try_files
Syntax:	try_files file ... uri;
		try_files file ... =code;
Default:	—
Context:	server, location



# INDIVIDUAL DIRECTIVES
## error_page
### Explanation:
	The first part (404, 500, etc.) indicates the error codes for which the directive applies.
	The second part (/custom_404.html, /50x.html, /error_handler.php) indicates the URI or file to be served when that error occurs.
	The = prefix can be used to change the response code when serving the error page. For example, error_page 404 =200 /error_handler.php; will serve /error_handler.php and return an HTTP 200 status instead of 404.

	* can be used globally in server block
		--> would apply to all location blocks, unless overridden there
	* if used in a location block, it only applies there 

### Example 1: Serve a custom 404 page
	error_page 404 /custom_404.html;

### Example 2: Handle multiple error codes
	error_page 500 502 503 504 /50x.html;

### Example 3: Redirect errors to another location
	error_page 404 = /error_handler.php;


## client-max-body-size
### Explanation
	Syntax:	client_max_body_size size;
	Default:	
	client_max_body_size 1m;
	Context:	http, server, location
	Sets the maximum allowed size of the client request body. If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client. Please be aware that browsers cannot correctly display this error. Setting size to 0 disables checking of client request body size.


## location
### Explanation
**Attributes**

	Syntax:	location [ = | ~ | ~* | ^~ ] uri { ... }
	location @name { ... }
	Default:	—
	Context:	server, location

	Sets configuration depending on a request URI.

**Matching**

	The matching is performed against a normalized URI, after decoding the text encoded in the “%XX” form, resolving references to relative path components “.” and “..”, and possible compression of two or more adjacent slashes into a single slash.

## listen
https://nginx.org/en/docs/http/ngx_http_core_module.html#listen

## server_name
https://nginx.org/en/docs/http/ngx_http_core_module.html#server_name

## root
https://nginx.org/en/docs/http/ngx_http_core_module.html#root

## index
https://nginx.org/en/docs/http/ngx_http_index_module.html#index

(only) last one can be specified via an absolute path!

## autoindex
https://nginx.org/en/docs/http/ngx_http_autoindex_module.html#autoindex

## error_page
https://nginx.org/en/docs/http/ngx_http_core_module.html#error_page

## try_files
https://nginx.org/en/docs/http/ngx_http_core_module.html#try_files

## client_max_body_size
https://nginx.org/en/docs/http/ngx_http_core_module.html#client_max_body_size

## return
https://nginx.org/en/docs/http/ngx_http_rewrite_module.html#return

### Example with ~* 
**location ~* /.(gif|jpg|png)$ { ... }**

	The ~* indicates a case-insensitive regular expression.
	This location block handles requests for image files with .gif, .jpg, or .png extensions.
	expires 30d; sets an expiration header, telling the browser to cache these files for 30 days.

### Example with ~
**location ~ /.php$ { ... } (CGI Part)**

	The ~ indicates a case-sensitive regular expression. This block matches any URL ending with .php.
	This block is where the CGI (or more specifically, FastCGI) process comes into play.



# PLAN FOR HANDLING ERRORS
## ERROR_PAGE
prepare response on the occured error
	- get the right status code

void SendData::prepErrorResponse( code, locationBlock ):
	create response header corresponding to the code
	if code has an error_page defined for it:
		serve the error_page with the right response header
	else
		create a custom response body, displaying the msg



error 4xx (client)
https://datatracker.ietf.org/doc/html/rfc2616#section-10.4


cases to check with nginx:

error happens with error_page directive in conf file,

	but the specified file does not OPEN
		---->	new error 403 will be thrown

	but the specified file does not EXIST
		---->	new error 404 will be thrown

	but the specified file extension is not supported (has permissions)
		---->	



where should the error_page file be appended to?
	-->		take root from requested location, but add to it also the location's prefix


   403    vs   404
Forbidden vs Not Found
403: when location found, but file not mentioned in index
404: when location not found


sending/responding in chunks
steps:
procedure for chunked response
1. send only the header
2. send the body in a certain format
	length\r\n
	body_chunk\r\n
1. avoid killing and closing the client fd for now
2. epoll_wait() should report on this client again
3. send the next body_chunk in the spcified format
4. continue till the all body_chunks are sent
5. signal to client all chunks were send, with this msg
	0\r\n
	\r\n