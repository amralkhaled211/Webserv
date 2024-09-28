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


class Parser // http Block basically
{
	private:
		std::string				_configFile;
		std::vector<Blocks>		_serverVec; // these will always be ServerBlocks as elements, and those will have a vector with LocationBlocks as elements

		Parser();

	public:
		Parser(std::string& fileName);
		Parser(const Parser& other);
		Parser&	operator=(const Parser& other);
		~Parser();


		// static function to check for _generallErrors -> postfix, emtpy file, no server block, 
			// lines with no semicolon (is it possible to have one broken into two?), ...
		static int	_generalErrors(std::string& fileName);
		// next we go into the Constructor and start _indepthCheck, as we do that we fill the _serverVec
		// _indepthCheck();
};

class Blocks // parent // the purpose of this is just to not have class with the same attibutes and methods, we won't do any Polymorphism, just Inheritence
{
	protected:
		std::string							_block_name; // block_type: Server & Location
		// all the directives, that are in both Server and Location
		//	
		// std::map<std::string, std::string>	directives; // I think we need the directives individually

	public:


};


// only first idea, we have to see if it really makes sense ...
class ServerBlock : public Blocks // gotta see what we put in Blocks (parent) and what in ServerBlock / LocationBlock (child)
{
	private:
		std::vector<LocationBlock>	_locationVec;
		// directives, that are only in server
		unsigned int	_listen;
		std::string		_serverName;
};

class LocationBlock : public Blocks
{
	private:
		std::string		_path;
		// directives, that are inly in location
};
