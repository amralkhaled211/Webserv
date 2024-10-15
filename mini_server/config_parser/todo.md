# TASKS

## TODO
### QUESTIONS
- [ ] regarding location, should we handle also the modifiers [ = | ~ | ~* | ^~ ]


### TESTING
- [ ] Test case with return 307 /path; play with the space and newline
- [ ] Test index with wrong extension
- [ ] Test the difference between '/' and '\'


### LEARNING
- [ ] Figure out meaning/use case of each directive
- [ ] Figure out CGI


### CODING
- [ ] throw error in case of duplicate location _prefix
- [ ] give dafaults to all the directives
  - [ ] if a Location Block's directive does not have any Value, it must take the global one
    - [ ] need to know about all the directive defaults
- [ ] Edge Cases Parsing
  	[ ] Test and Handle nested locations
- [ ] Figure out valid and invalid cases
- [ ] Accept Quotes as Directive Key
- [ ] Deny Quotes as Directive Value


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