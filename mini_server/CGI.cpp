#include "CGI.hpp"
#include <signal.h>

CGI::CGI() {}

CGI::CGI(const std::string &scriptPath, const parser &request) : _scriptPath(scriptPath) , _request(request), _typeSet(false), _statusSet(false), _interpreter("")
{}

CGI::~CGI() {}

std::string CGI::getResponse() const
{
	return _responseBody;
}

std::string CGI::getContentType() const
{
	return _contentType;
}

std::string CGI::getContentLength() const
{
	return _contentLength;
}

std::string CGI::getResponseStatus() const
{
	return _responseStatus;
}

bool CGI::getTypeSet() const
{
	return _typeSet;
}

/* bool CGI::getLengthSet() const
{
	return _lengthSet;
} */

bool CGI::getStatusSet() const
{
	return _statusSet;
}

std::vector<char*> CGI::setUpEnvp()
{
    std::vector<char*> envp;
    for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); ++it)
    {
        if (!it->first.empty() && !it->second.empty())
        {
            std::string envVar = it->first + "=" + it->second;
			char* envVarCStr = new char[envVar.size() + 1];
            std::strcpy(envVarCStr, envVar.c_str());
            envp.push_back(envVarCStr);
        }
    }
    envp.push_back(NULL); // Null-terminate the array
    return envp;
}

std::string join(const std::vector<std::string>& vec, const std::string& delimiter)
{
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (i != 0)
            oss << delimiter;
        oss << vec[i];
    }
    return oss.str();
}

void CGI::setEnv(ServerBlock server)
{
	_env["REQUEST_METHOD"] = _request.method;
	_env["QUERY_STRING"] = _request.queryString;
	_env["SCRIPT_NAME"] = _scriptPath;
	_env["PATH_INFO"] = _scriptPath;
	size_t semicolon = _request.headers["Host"].find_first_of(':');
	_env["SERVER_NAME"] = _request.headers["Host"].substr(0, semicolon);
	_env["SERVER_PORT"] = _request.headers["Host"].substr(semicolon + 1);
	_env["PATH_TRANSLATED"] = "/translated/path" + _request.path;
	_env["SERVER_PROTOCOL"] = _request.version;
	/* std::string remoteAddr = "127.0.0.1"; // Replace with actual method to get remote address
    std::string remoteHost = "localhost"; // Replace with actual method to get remote host
    _env["REMOTE_ADDR"] = remoteAddr;
    _env["REMOTE_HOST"] = remoteHost; */

    _env["CONTENT_TYPE"] = _request.headers["Content-Type"];

    // Determine CONTENT_LENGTH based on request body size
    if (_request.method == "POST" && !_request.body.empty())
    {
        _env["CONTENT_LENGTH"] = intToString(_request.body.size());
    }
    else
    {
        _env["CONTENT_LENGTH"] = "0";
    }

	/* std::cout << GREEN_COLOR << "POST CONTENT TYPE: " << _env["CONTENT_TYPE"] << std::endl;
	std::cout << "POST CONTENT LENGTH: " << _env["CONTENT_LENGTH"] << std::endl;
	std::cout << "POST BODY: " << _request.body << RESET <<std::endl; */

	bool first = true;

	for (std::map<std::string, std::string>::iterator it = _request.headers.begin(); it != _request.headers.end(); ++it)
	{
		if (first)
		{
			it++;
			first = false;
		}
		if (!it->first.empty() || !it->second.empty() || it->first[0] != ':')
		{
			std::string envName = "HTTP_" + it->first;
			std::replace (envName.begin(), envName.end(), '-', '_');
			std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
			_env[envName] = it->second;
		}
	}
}

bool CGI::setInterpreters(LocationBlock location)
{
	std::vector<std::string> interpreters = location.getCgiPath();
	for (std::vector<std::string>::iterator it = interpreters.begin(); it != interpreters.end(); ++it)
	{
		//_interpreters[*it] = *it;
		std::istringstream ss(*it);
		std::string path;
		while (ss >> path)
		{
			if (path.find("python3") != std::string::npos)
				_interpreters[".py"] = path;
			else if (path.find("php") != std::string::npos)
				_interpreters[".php"] = path;
			else if (path.find("perl") != std::string::npos)
				_interpreters[".pl"] = path;
		}
	}
	std::string fileExt = '.' + get_file_extension(_scriptPath);
	_interpreter = _interpreters[fileExt];
	if (_interpreter.empty())
	{
		return false;
	}
	return true;
}


void freeEnvp(std::vector<char*> &envp)
{
	for (size_t i = 0; i < envp.size(); ++i)
	{
		delete[] envp[i];
	}
}

void CGI::printEnvp(std::vector<char*>envp)
{
	for(std::vector<char*>::iterator it = envp.begin(); it < envp.end(); ++it)
	{
		std::cout << *it << std::endl;
	}	
}

void CGI::printEnv()
{
	for(std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); ++it)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
}

int CGI::executeScript()
{
	int inPipe[2];
	int outPipe[2];

	if (pipe(inPipe) == -1 || pipe(outPipe) == -1)
		throw std::runtime_error("Failed to create pipes in CGI");

	pid_t pid = fork();
	if (pid == -1)
		throw std::runtime_error("Failed to fork in CGI");
	if (pid == 0) //CHILD
	{
		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);

		close(inPipe[1]);
        close(outPipe[0]);

		std::vector<char*> envp = setUpEnvp();
		
		
		char *arg[] = {const_cast<char*>(_interpreter.c_str()), const_cast<char*>(_scriptPath.c_str()), NULL};

        if (execve(_interpreter.c_str(), arg, &envp[0]) == -1)
        {
            freeEnvp(envp);
			std::cout << ERROR_MARKER << ": " << strerror(errno) <<std::endl;
			return 1;
        }
	}
	else //PARENT
	{
		close(inPipe[0]);
		close(outPipe[1]);
		
		if (_request.method == "POST" && !_request.body.empty()){
			write (inPipe[1], _request.body.c_str(), _request.body.size());
		}
		close(inPipe[1]);

		fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = 5; // Set timeout to 5 seconds
        timeout.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(outPipe[0], &readfds);

        int selectResult = select(outPipe[0] + 1, &readfds, NULL, NULL, &timeout);
        if (selectResult == -1)
        {
            close(outPipe[0]);
            throw std::runtime_error("select() failed");
        }
        else if (selectResult == 0) // timeout occured
        {

            kill(pid, SIGTERM); // Send SIGTERM to the child process
            sleep(1); // Give the child process some time to terminate
            kill(pid, SIGKILL); // Send SIGKILL if the child process is still running
            close(outPipe[0]);
            return 508;
        }
		

		char buffer[1024] = {0};
		std::string bufferStr;
		std::string errStr;
		bool eof = false;
		std::ostringstream output;
		ssize_t bytesRead;
		while ((bytesRead = read(outPipe[0], buffer,sizeof(buffer))) > 0)
		{
			buffer[bytesRead] = '\0';
			bufferStr = buffer;
			if (bufferStr.find(ERROR_MARKER) != std::string::npos)
			{
				errStr = bufferStr.substr(bufferStr.find(ERROR_MARKER) + strlen(ERROR_MARKER) + 2);
				//std::cout << RED << "ERROR_MARKER found: " << errStr << RESET << std::endl;
				/* kill(pid, SIGTERM);
				sleep(1); */
				kill(pid, SIGKILL);
				eof = true;
				break;
			}
			output.write(buffer, bytesRead);
		}

		close(outPipe[0]);

		if (eof)
		{
			_responseBody.clear();
			std::cout << errStr;
			if (errStr == "Permission denied")
				return 403;
			else if (errStr.find("No such file or directory") != std::string::npos)
				return 404;
			else
				return 500;
		}
		
		int status;
		waitpid(pid, &status, 0);

		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) != 0 || WIFSIGNALED(status))
			{
				std::cerr << "Child process exited with status " << WEXITSTATUS(status) << std::endl;
				return 500;
			}
		}
		else
		{
			std::cerr << "Child process exited abnormally" << std::endl;
			return 500;
		}

		_responseBody = output.str();
	}
	return 0;
}

/* std::string trimNewline(const std::string &str)
{
	size_t end = str.find_first_of("\r\n");
	if (end == std::string::npos)
		return str;
	return str.substr(0, end);
} */

void CGI::generateResponse()
{
	std::istringstream ss(_responseBody);
	_responseBody.clear();
	std::string file_extension = get_file_extension(_request.path);
	std::string line;

	std::ostringstream body;
	while (std::getline(ss, line))
	{
		if (line.find("Content-Type:") != std::string::npos){
			_typeSet = true;
			_contentType = line.substr(line.find("Content-Type:"));
			continue;
		}
		/* if (line.find("Content-Length:") !=  std::string::npos){
			_lengthSet = true;
			_contentLength = line.substr(line.find("Content-Length:"));
			continue;
		} */
		if (line.find("Status:") !=  std::string::npos){
			_statusSet = true;
			_responseStatus = "HTTP/1.1" + line.substr(line.find("Status:") + 7);
			continue;
		}
		if (ss.eof())
			break;
		body << line << "\n";
	}
	_responseBody = body.str();
}

/* void CGI::createhtml()
{
	std::ofstream html("cgi_output.html");
	if (!html.is_open())
	{
		std::cerr << "Failed to open html file" << std::endl;
		exit(1);
	}
	html << _responseBody;
	html.close();
} */
