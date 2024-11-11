# TASKS

- [ ] Integrate LocationBlock & ServerBlock attributes into CGI header and code
- [ ] CGI Enviroment setup with correct values
- [ ] Setup signals for interrupting infinite loops
- [ ] Return an error when execve cannot open file, use the notfound page there as well
- [ ] Return an error when file extension is not allowed, error page here or not found page??

## LEARNING

- [ ] Read CGI book
- [ ] Research new approach for CGI response instead of the cgi_output.html method
- [ ] Research how chunked responses need to behave
- [ ] Research how signals should work inside

### TESTS

- [ ] Test with sh scripts

#### DONE

- [x] Check for allowed extensions -> compare request path extension with allowed ones
- [x] Check for a CGI request a better way
