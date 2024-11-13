#pragma once

#include "Server.hpp"
#include "RequestHandler.hpp"
#include "config_parser/LocationBlock.hpp"
#include <sys/wait.h>

class CGI
{
	private:
		
		std::string _scriptPath;
		std::string _responseBody;
		std::string _contentType;
		bool 		_typeSet;
		std::map<std::string, std::string> _env;
		parser _request;
		//Response		_response;

	public:

		CGI();
		CGI(const std::string &scriptPath, const parser &request);
		~CGI();

		std::string getResponse() const;
		std::string getContentType() const;
		bool getTypeSet() const;

		void setEnv(ServerBlock server);
		void executeScript();
		void generateResponse();
		void createhtml();

		std::vector<char*> setUpEnvp();

		void printEnv();
		void printEnvp(std::vector<char*> envp);
};