# TASKS
- [ ] Implement POST handler for CGI
- [ ] Integrate LocationBlock & ServerBlock attributes into CGI header and code
- [ ] CGI Enviroment setup with correct values

## LEARNING

- [ ] Read CGI book
- [ ] Research how chunked responses need to behave

### TESTS

- [ ] Test with sh scripts

#### DONE

- [x] Research new approach for CGI response instead of the cgi_output.html method
- [x] Return an error when file extension is not allowed, error page here or not found page??
- [x] Implement own response builder for CGI
- [x] Setup signals for interrupting infinite loops
- [x] Research how signals should work inside
- [x] Return an error when execve cannot open file, use the notfound page there as well
- [x] Check for allowed extensions -> compare request path extension with allowed ones
- [x] Check for a CGI request a better way
- [x] Test with images and vids
