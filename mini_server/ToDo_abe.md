# TASKS
- [ ] Try passing the interpreters from cgi_path to execve
## LEARNING

- [ ] Read CGI book
- [ ] Research how chunked responses need to behave

### TESTS

- [ ] Test with php scripts

#### DONE

- [x] CGI Enviroment setup with correct values
- [x] Rewrite the findCGIIndex in isDie
- [x] Handle different status codes coming form CGI, set error pages accordingly -> 508 for inf loop, 500 for execve error, 403 for forbidden, 404 for not found
- [x] Setup error pages 
- [x] Fix child leak -> dont pass location and serverblock to CGI maybe
- [x] Add CGI to isDir in SendData at GET method
- [x] Handle CGI based on extensions, not path
- [x] Fix error 1
- [x] Implement POST handler for CGI
- [x] Research new approach for CGI response instead of the cgi_output.html method
- [x] Return an error when file extension is not allowed, error page here or not found page??
- [x] Implement own response builder for CGI
- [x] Setup signals for interrupting infinite loops
- [x] Research how signals should work inside
- [x] Return an error when execve cannot open file, use the notfound page there as well
- [x] Check for allowed extensions -> compare request path extension with allowed ones
- [x] Check for a CGI request a better way
- [x] Test with images and vids

##### ERRORS

------    1    -------


./Server config_parser/conf_files/example.conf
Port: http://localhost:8080
Port: http://localhost:8282
Failed to execute CGI script: Exec format error
^CStatus set
Content type set
Content length set
CGI Status: HTTP/1.1 HTTP/1.1 404 Not Found

;GI Content type: Content-Type: text/html;

CGI Content length: Content-Length: 155

Response Status: HTTP/1.1 HTTP/1.1 404 Not Found

;ontent Type: Content-Type: text/html;

Content Length: Content-Length: 155

Server shutting down


-------    2    -------