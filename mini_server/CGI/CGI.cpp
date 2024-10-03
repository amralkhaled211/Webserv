/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aszabo <aszabo@student.42vienna.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 12:34:27 by aszabo            #+#    #+#             */
/*   Updated: 2024/10/03 15:47:50 by aszabo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI() {}

CGI::CGI(const std::string &scriptPath, const parser &request) : _scriptPath(scriptPath) , _request(request)
{}

CGI::~CGI() {}

void CGI::setEnv()
{
	_env["REQUEST_METHOD"] = _request.method;
	_env["QUERY_STRING"] = _request.queryString;
	_env["CONTENT_TYPE"] = _request.headers["Content-Type"];
	_env["CONTENT_LENGTH"] = _request.headers["Content-Length"];
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
}