#include "CGI.hpp"
#include <signal.h>

CGI::CGI() {}

CGI::CGI(const std::string &scriptPath, const parser &request) : _scriptPath(scriptPath) , _request(request), _typeSet(false)/* , _statusSet(false), _lengthSet(false) */
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

bool CGI::getTypeSet() const
{
	return _typeSet;
}

/* bool CGI::getLengthSet() const
{
	return _lengthSet;
}

bool CGI::getStatusSet() const
{
	return _statusSet;
} */

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
	_env["PATH_INFO"] = _request.path;
	_env["PATH_TRANSLATED"] = "/translated/path" + _request.path;
	_env["SERVER_NAME"] = join(server.getServerName(), ", ");//replace with actual host name etc THIS  IS HARDCODED RN
	_env["SERVER_PORT"] = "8080";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	std::string remoteAddr = "127.0.0.1"; // Replace with actual method to get remote address
    std::string remoteHost = "localhost"; // Replace with actual method to get remote host
    _env["REMOTE_ADDR"] = remoteAddr;
    _env["REMOTE_HOST"] = remoteHost;
	std::string file_extension = get_file_extension(_request.path);

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

void CGI::executeScript()
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

		/* std::ofstream child_out("cgi_child_debug.txt");
        if (!child_out.is_open())
        {
            std::cerr << "Failed to open debug log file" << std::endl;
            exit(1);
        }

		child_out << "Child process started" << std::endl; */

		std::vector<char*> envp = setUpEnvp();

		/* child_out << "Env is set up going into execve" << std::endl; */
		
		char *arg[] = {const_cast<char*>(_scriptPath.c_str()), NULL};
		/* child_out << "Executing script: " << _scriptPath << std::endl;
		child_out.flush(); */
        if (execve(_scriptPath.c_str(), arg, &envp[0]) == -1)
        {
            std::cerr << "Failed to execute CGI script: " << strerror(errno) << std::endl;
			throw std::runtime_error("Failed to execute CGI script");
            freeEnvp(envp);
            exit(1);
        }
	}
	else //PARENT
	{
		close(inPipe[0]);
		close(outPipe[1]);
		
		if (_request.method == "POST" && !_request.body.empty())
		{
			/* std::cout << CYAN_COLOR << "Writing POST body to pipe: " <<std::endl;
			std::cout << _request.body << RESET << std::endl; */	
			write (inPipe[1], _request.body.c_str(), _request.body.size());
		}
		close(inPipe[1]);

		fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = 15; // Set timeout to 5 seconds
        timeout.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(outPipe[0], &readfds);

        int selectResult = select(outPipe[0] + 1, &readfds, NULL, NULL, &timeout);
        if (selectResult == -1)
        {
            close(outPipe[0]);
            throw std::runtime_error("select() failed");
        }
        else if (selectResult == 0)
        {
            // Timeout occurred
            kill(pid, SIGTERM); // Send SIGTERM to the child process
            sleep(1); // Give the child process some time to terminate
            kill(pid, SIGKILL); // Send SIGKILL if the child process is still running
            close(outPipe[0]);
            throw std::runtime_error("CGI script execution timed out");
        }

		char buffer[1024];
		std::ostringstream output;
		ssize_t bytesRead;
		while ((bytesRead = read(outPipe[0], buffer,sizeof(buffer))) > 0)
			output.write(buffer, bytesRead);
		close(outPipe[0]);

		int status;
		waitpid(pid, &status, 0);

		_responseBody = output.str();
	}
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
			_contentType = line;
			continue;
		}
		/* if (line.find("Content-Length:") !=  std::string::npos){
			_lengthSet = true;
			_contentLength = line;
			continue;
		}
		if (line.find("Status:") !=  std::string::npos){
			_statusSet = true;
			_responseStatus = line;
			continue;
		} */
		if (ss.eof())
			break;
		body << line << "\n";
	}
	_responseBody = body.str();
}

void CGI::createhtml()
{
	std::ofstream html("cgi_output.html");
	if (!html.is_open())
	{
		std::cerr << "Failed to open html file" << std::endl;
		exit(1);
	}
	html << _responseBody;
	html.close();
}
