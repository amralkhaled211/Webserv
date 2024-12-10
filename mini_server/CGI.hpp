#pragma once

#include "Server.hpp"
#include "RequestHandler.hpp"
#include "config_parser/LocationBlock.hpp"
#include <sys/wait.h>
#define ERROR_MARKER "ERROR_MARKER"

class CGI
{
	private:
		
		std::string _scriptPath;
		std::string _interpreter;
		std::string _responseBody;
		std::string _contentType;
		std::string _contentLength;
		std::string _responseStatus;
		parser _request;
		bool 		_typeSet;
		bool		_lengthSet;
		bool		_statusSet;
		std::map<std::string, std::string> _env;
		std::map<std::string, std::string> _interpreters;
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
		bool setInterpreters(LocationBlock location);
		int executeScript();
		void generateResponse();
		void createhtml();

		std::vector<char*> setUpEnvp();

		void printEnv();
		void printEnvp(std::vector<char*> envp);
};