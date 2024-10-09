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
		std::string envVar = it->first + "=" + it->second;
		char* envCStr = new char[envVar.size() + 1];
		std::strcpy(envCStr, envVar.c_str());
		envp.push_back(envCStr);
	}
	envp.push_back(NULL);
	return (envp);
}

void CGI::setEnv()
{
	_env["REQUEST_METHOD"] = _request.method;
	_env["QUERY_STRING"] = _request.queryString;
	/* _env["CONTENT_TYPE"] = _request.headers["Content-Type"];
	_env["CONTENT_LENGTH"] = _request.headers["Content-Length"]; */
	_env["SCRIPT_NAME"] = _scriptPath;
	_env["PATH_INFO"] = _request.path;
	_env["PATH_TRANSLATED"] = "/translated/path" + _request.path;
	_env["SERVER_NAME"] = "localhost";
	_env["SERVER_PORT"] = "8080";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	/* _env["REMOTE_ADDR"] = _request.clientIP;
	_env["REMOTE_HOST"] = _request.clientHost; */

	//HERE WE WOULD NEED INFO FROM CONFIG FILE IF IT SPECIFIES SMTH FOR THE CGI

	for (std::map<std::string, std::string>::iterator it = _request.headers.begin(); it != _request.headers.end(); ++it)
	{
		std::string envName = "HTTP_" + it->first;
		std::replace (envName.begin(), envName.end(), '-', '_');
		std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
		_env[envName] = it->second;
	}
	if (!_env.empty())
	{
		_env.erase(_env.begin());
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

		std::vector<char*> envp = setUpEnvp();
		
		char *arg[] = {const_cast<char*>(_scriptPath.c_str()), NULL};
		execve(_scriptPath.c_str(), arg, &envp[0]);

		std::cerr << "Failed to execute CGI script" << std::endl;
		freeEnvp(envp);
		exit(1);
	}
	else //PARENT
	{
		close(inPipe[0]);
		close(outPipe[1]);
		
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

		_response.body = output.str();
	}
}

void CGI::generateResponse()
{
	std::istringstream ss(_response.body);
	std::string line;
	bool headersParsed = false;

	_response.status = "HTPP/1.1 200 OK\r\n"; //Default status

	while (std::getline(ss, line))
	{
		if (line == "\r" || line.empty())
		{
			headersParsed = true;
			break;
		}

		size_t delimPos = line.find(":");
		if (delimPos != std::string::npos)
		{
			std::string key = line.substr(0, delimPos);
			std::string value = line.substr(delimPos + 1);
			key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
			value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());

			if (key == "Status")
				_response.status = "HTTP/1.1" + value + "\r\n";
			else if (key == "Content-Type")
				_response.contentType = "Content-Type: " + value + "\r\n";
			else if (key == "Content-Length")
				_response.contentLength = "Content-Length: " + value + "\r\n";
		}
	}

	std::ostringstream body;
	while (std::getline(ss, line))
		body << line << "\n";
	_response.body = body.str();
}