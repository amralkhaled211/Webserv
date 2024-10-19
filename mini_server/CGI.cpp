/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aszabo <aszabo@student.42vienna.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 12:34:27 by aszabo            #+#    #+#             */
/*   Updated: 2024/10/07 18:19:27 by aszabo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI() {}

CGI::CGI(const std::string &scriptPath, const parser &request) : _scriptPath(scriptPath) , _request(request)
{}

CGI::~CGI() {}

Response CGI::getResponse() const
{
	return _response;
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

void CGI::setEnv()
{
	_env["REQUEST_METHOD"] = _request.method;
	_env["QUERY_STRING"] = _request.queryString;
	_env["SCRIPT_NAME"] = _scriptPath;
	_env["PATH_INFO"] = _request.path;
	_env["PATH_TRANSLATED"] = "/translated/path" + _request.path;
	_env["SERVER_NAME"] = "localhost";
	_env["SERVER_PORT"] = "8080";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	std::string remoteAddr = "127.0.0.1"; // Replace with actual method to get remote address
    std::string remoteHost = "localhost"; // Replace with actual method to get remote host
    _env["REMOTE_ADDR"] = remoteAddr;
    _env["REMOTE_HOST"] = remoteHost;
	std::string file_extension = get_file_extension(_request.path);
    if (mimeTypesMap_G.find(file_extension) != mimeTypesMap_G.end())
    {
        _env["CONTENT_TYPE"] = mimeTypesMap_G[file_extension];
    }
    else
    {
        _env["CONTENT_TYPE"] = "application/octet-stream"; // Default MIME type
    }

    // Determine CONTENT_LENGTH based on request body size
    if (_request.method == "POST" && !_request.body.empty())
    {
        _env["CONTENT_LENGTH"] = intToString(_request.body.size());
    }
    else
    {
        _env["CONTENT_LENGTH"] = "0";
    }
	//HERE WE WOULD NEED INFO FROM CONFIG FILE IF IT SPECIFIES SMTH FOR THE CGI

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
		/* dup2(outPipe[1], STDERR_FILENO); */

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
            freeEnvp(envp);
            exit(1);
        }
	}
	else //PARENT
	{
		close(inPipe[0]);
		close(outPipe[1]);

		/* make_socket_non_blocking(inPipe[1]);
		make_socket_non_blocking(outPipe[0]); */
		
		if (_request.method == "POST" && !_request.body.empty())
			write (inPipe[1], _request.body.c_str(), _request.body.size());
		close(inPipe[1]);
		
		char buffer[1024];
		std::ostringstream output;
		ssize_t bytesRead;
		while ((bytesRead = read(outPipe[0], buffer,sizeof(buffer))) > 0)
			output.write(buffer, bytesRead);
		close(outPipe[0]);

		int status;
		waitpid(pid, &status, 0);

		/* std::cout << "Exited child process, returning response.." << std::endl;
		std::cout << "Output from child: " << output.str() << std::endl; */
		_response.body = output.str();
	}
}

std::string trimNewline(const std::string &str)
{
	size_t end = str.find_first_of("\r\n");
	if (end == std::string::npos)
		return str;
	return str.substr(0, end);
}

std::string CGI::generateResponse()
{
	std::istringstream ss(_response.body);
	_response.body.clear();
	std::string file_extension = get_file_extension(_request.path);
	std::string line;

	/* _response.status = "HTPP/1.1 200 OK\r\n"; */ //Default status
	//_response.contentType = "Content-Type: " + mimeTypesMap_G[file_extension] + ";" + "\r\n";

	std::ostringstream body;
	/* bool lengthSet = false;
	bool typeSet = false; */
	while (std::getline(ss, line))
	{
		/* if (line.find("Content-Type:") != std::string::npos){
			typeSet = true;
			_response.contentType = trimNewline(line);
			continue;
		}
		if (line.find("Content-Length:") != std::string::npos){
			lengthSet = true;
			_response.contentLength = "Content-Length:" + line.substr(15);
			continue;
		} */
		if (ss.eof())
			break;
		body << line << "\n";
	}
	_response.body = body.str();
	/* if (typeSet == false){
		_response.contentType = "Content-Type: text/html;\r\n";
	}
	else{
		_response.contentType += ";";
		_response.contentType += "\r\n";
	}
	if (lengthSet == false){
	unsigned int content_len = _response.body.size();
		_response.contentLength = "Content-Length: " + intToString(content_len) + "\r\n";
	} */
	return _response.body;
}