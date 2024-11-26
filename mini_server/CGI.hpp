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
		std::string _contentLength;
		std::string _responseStatus;
		parser _request;
		bool 		_typeSet;
		bool		_lengthSet;
		bool		_statusSet;
		std::map<std::string, std::string> _env;
		//Response		_response;

	public:

		CGI();
		CGI(const std::string &scriptPath, const parser &request);
		~CGI();

		std::string getResponse() const;
		std::string getContentType() const;
		std::string getContentLength() const;
		std::string getResponseStatus() const;
		bool getTypeSet() const;
		bool getLengthSet() const;
		bool getStatusSet() const;

		void setEnv(ServerBlock server);
		void executeScript();
		void generateResponse();
		void createhtml();

		std::vector<char*> setUpEnvp();

		void printEnv();
		void printEnvp(std::vector<char*> envp);
};