# TASKS

## TODO
### QUESTIONS


### TESTING
- [ ] Test the difference between '/' and '\'
- [ ] Test return syntax checks
- [ ] Test running siege tester and uploading something in chuncks
      - what if there is a new request in between chunks?


### LEARNING
- [ ] Figure out meaning/use case of each directive
- [ ] Figure out CGI


### CODING
- [x] REMOVE TRY_FILES FROM PARSING


- [ ] PARSER ERROR HANDLING FOR
  - [x] ERROR_PAGE - see below for details
  - [x] INDEX - only last can be absolute path
  - [x] client_max_body_size -> accepted k,m,g as postfix, 0 means do not check size
  - [ ] cgi-ext and cgi-path
  - [ ] check range for ports

- [ ] autoindex better design

- [X] decide on listen directive
- [X] defualt allowed_methods
- [x] set 302 as default code for return directive / redirection
- [x] throw error in case of duplicate location _prefix
- [x] give dafaults to all the directives
  - [x] if a Location Block's directive does not have any Value, it must take the global one
    - [x] need to know about all the directive defaults
- [ ] Edge Cases Parsing
  - [ ] Test and Handle nested locations
- [ ] QUOTES
  - [ ] ask around how others implement quotes
    - [ ] Accept/Deny Quotes as Directive Key
    - [ ] Accept/Deny Quotes as Directive Value
        - [ ] adjust removal of comments '#'
- [ ] error_page
  - [ ] there must be always at least one code between 300 and 599, followed by a path (which will be appended to the root)
  - [ ] accept multiple directives of error_page
- [ ] return
  - [ ] accepts multiple, but only uses the first one
  - [ ] what is the accepted range? 0 - 999; note: client interprets 0 as 'Finished'
  - [x] if another location is redirected to, there must be a status code
  - [ ] if the same location is redirected to, a 404 error is thrown


## DONE
- [x] Basic Structure for Parsing
- [x] _configToContent() & _removeExcessSpace()
- [x] _generallErrors()
- [x] Figure out which directives are used in the server block
- [x] Figure out which directives are used in the location block
- [x] Which are used in both
- [x] Approach to parse
-	[x] delete all comments (#)
-	[x] put everything in one string
-	[x] replace the '\n' with ' '
-	[x] tokenize
-	[X] _fillBlocks()
	- [X] _serverBlock()
	- [X] _locationBlock()
- [x] regarding location, should we handle also the modifiers [ = | ~ | ~* | ^~ ]
		A: No, according to subject