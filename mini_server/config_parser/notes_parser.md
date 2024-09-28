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
		multiple server blocks;
		possible blocks:
			http --> how many is valid in one conf file?
				server --> does it always belong in a http block? is there a minimum of one for server block?
					location --> does it always belong in a server block? is it required for valid conf file?
			? only these three?
			CGIBlock, too?
				

		possible directives:
			-listen
			-server_name
			--root
			error_page --> must check whether in both blocks
			index --> must check whether in both blocks
			-allowed_methods
			-client_max_body_size

		possible location directives:
			allowed_methods
			root
			alias (maybe not mandatory)
			error_page
			index
			autoindex

		possible server directives:
			listen port (listening to)
			host IP-Address (listening to)
			server_name
			root
			index --> purpose unclear
			error_page
			allowed_methods
			location
			client_max_body_size

		all directives:
			listen
			server


# syntax rules overview

	There must be a http block to define the protocol in use for the server(s) inside.

	the syntax is not strict:
	* everything (all directives and blocks) be in just one line.
	* whitespace is only required for the directive keys and values to be separated.
	* the rest (such as {, }, ;) can, BUT doesn’t need any whitespace separation form anything.
	* newlines are treated similar to whitespaces
	* so newlines are allowed before a ';' -> a directive doesn't need to be followed by a ';' on the same line, but can also at the next line
	
	* no dublicates of location block with the same path allowed (2 times --> location /hello)
	* in case multiple servers with the same host (server_name) and port (listen), nginx will only give a warning and take the first as default, the others will NOT be used
	* in case of no location in a server block, it takes the root from the server
		* if no root in server: nginx default page shows up
			* how to interpret that?
	* in case no server_name in server block
		* it also only shows the nginx default page


	* alias --> don't think we need to handle alias
		* acts a bit weird
			* sometimes (location /) without '/' at the end, works, sometimes (location /hello) not
			* with '/' at the end it always works
	
	* check order




## this is an example of a valid conf file syntax:

	http{server{listen
	8090   ;server_name 127.0.0.1;location /hello{alias /home/aismaili/webSite;index index.html;}}
	}events{
		worker_connections
		1024
		;
	}


# TASKS
## TODO
	[] Figure out which directives are used in the server block
	[] Figure out which directives are used in the location block
	[] Which are used in both
	[] _indepthErrors()
	[] _fillBlocks()

## DONE
	[x] Basic Structure for Parsing
	[x] _generallErrors()