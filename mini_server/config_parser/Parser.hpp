#pragma once

#include <string>
#include <map>
#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

/*
	one config file;
	multiple server blocks;
	possible blocks:
		server
		location
		? only these two?
		CGIBlock?

	
	possible directives:
		listen
		server_name
		root
		index
		error_page
		allowed_methods

	possible location directives:
		alias ? unsure
		autoindex
		allowed_methods
		error_page
		index
		root

	possible server directives:
		listen
		server_name
		root
		index
		error_page
		allowed_methods
		location
	
*/

#define INVALID		0
#define	VALID		1
#define POSTFIX		".conf"


class Parser
{
	private:
		std::string				_configFile;
		std::vector<Blocks>		_serverVec;
	
		Parser();

	public:
		Parser(std::string& fileName);
		Parser(const Parser& other);
		Parser&	operator=(const Parser& other);
		~Parser();


		// static function to check for _generallErrors -> postfix, emtpy file, no server block, 
			// lines with no semicolon (is it possible to have one broken into two?), ...
		static int	_generalErrors(std::string fileName);
		// next we go into the Constructor and start _indepthCheck -> 
		// _indepthCheck();
};

class Blocks
{
	private:
		std::string block_name; // block_type
		std::map<std::string, std::string> directives;
	
	public:

};



class ServerBlock : public Blocks
{
};



class LocationBlock : public Blocks
{
};
