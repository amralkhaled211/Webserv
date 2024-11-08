#pragma once

#include "Server.hpp"
#include "RequestHandler.hpp"
#include <sys/wait.h>

class CGI
{
	private:
		
		std::string _scriptPath;
		std::map<std::string, std::string> _env;
		parser _request;
		Response		_response;

	public:

		CGI();
		CGI(const std::string &scriptPath, const parser &request);
		~CGI();

		Response getResponse() const;

		void setEnv();
		void executeScript();
		void generateResponse();
		void createhtml();

		std::vector<char*> setUpEnvp();

		void printEnv();
		void printEnvp(std::vector<char*> envp);
};