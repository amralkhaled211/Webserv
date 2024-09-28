#pragma once

#include <string>
#include <map>
#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

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
		// next we go into the Constructor and start _indepthCheck, as we do that we fill the _serverVec
		// _indepthCheck();
};

class Blocks // parent
{
	private:
		std::string							block_name; // block_type: Server & Location
		std::map<std::string, std::string>	directives; // 
	

	public:
	

};


// only first idea, we have to see if it really makes sense ...
class ServerBlock : public Blocks // gotta see what we put in Blocks (parent) and what in ServerBlock / LocationBlock (child)
{
};

class LocationBlock : public Blocks
{
};
