# PARSER NOTES
## possible approach

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


# syntax overview

There must be a http block to define the protocol in use for the server(s) inside.

the syntax is not strict:
* everything (all directives and blocks) be in just one line.
* whitespace is only required for the directive keys and values to be separated.
* the rest (such as {, }, ;) can, BUT doesn’t need any whitespace sepatation form anything.
* newlines are treated similar to whitespaces
* so newlines are allowed before a ';' -> a directive doesn't need to be followed by a ';' on the same line, but can also at the next line

## this is an example of a valid conf file syntax:

http{server{listen
8090   ;server_name 127.0.0.1;location /hello{alias /home/aismaili/webSite;index index.html;}}
}events{
	worker_connections
	 1024
	;
}