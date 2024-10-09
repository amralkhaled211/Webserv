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


# TASKS

## TODO
- [] Test case with return 307 /path; play with the space and newline
- [] Test with '{', '}' and ';' followed by each other, probably error, if so --> add to _generallErrors
	[] ’;’, '{', '}' in series maybe
	[] invalid '{' and '}' --> opened but not closed, vice versa
- [] Figure out meaning/use case of each directive
- [] Figure out which directives are used in the server block
- [] Figure out which directives are used in the location block
- [] Which are used in both
- [] Figure out CGI
- [] Approach to parse
-	[x] delete all comments (#)
-	[x] put everything in one string
-	[x] replace the '\n' with ' '
-	[x] tokenize
-	[] Figure out valid and invalid cases
- [] _fillBlocks()

## DONE
- [x] Basic Structure for Parsing
- [x] _configToContent() & _removeExcessSpace()
- [x] _generallErrors() --> more stuff to add, but only if it is easier there




# INDIVIDUAL DIRECTIVES
## error_page
### explanation:
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

##