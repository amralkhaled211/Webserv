#pragma once

#include <string>
#include <map>
#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ServerBlock.hpp"

#define INVALID		0
#define	VALID		1
#define POSTFIX		".conf"


class Parser // http Block basically
{
	private:
		std::string						_configFile;
		std::vector<ServerBlock>		_serverVec; // these will always be ServerBlocks as elements, and those will have a vector with LocationBlocks as elements

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
