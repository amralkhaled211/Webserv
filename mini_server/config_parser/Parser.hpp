#pragma once

#include <string>
#include <map>
#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

// #include "ServerBlock.hpp"

#define INVALID						0
#define	VALID						1
#define POSTFIX						".conf"
#define WHITESPACE					" \t\n\v\f\r" /// might only need " \t"
#define IS_BLOCKNAME				token == "http" || token == "server" || token == "location"
#define IS_BLOCKNAME_WITH_BRACE		token == "http{" || token == "server{" || token == "location{" // this is obviously overkill, because it needs two more similar variations
#define IS_BRACE		token == "{" || token == "}"
#define IS_SEMICOLON	token == ";"
#define IS_DIRECTIVE	token == "listen" || token == "server_name" || token == "root" || token == "index" || token == "autoindex" || token == "error_page" || token == "client_max_body_size" ||  token == "return" // more to follow // might meed to separate into more MACROS, because to long

class Parser // http Block basically
{
	private:
		std::string						_configFile;
		// std::vector<ServerBlock>		_serverVec; // these will always be ServerBlocks as elements, and those will have a vector with LocationBlocks as elements
		std::string						_content;

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

		void		_parser(); //

		/*			PREP FOR PARSING			*/
		void		_configToContent(); // Content is basically everything compressed in one string // other name: _fileContentToOneStr(), _fileToStr()
										// !!! in case of comments (#), skip rest of line (don't fill it inside _content) and coninue with the next line !!!
		void		_removeExcessSpace(); // excess spaces: before and after 1) block names (http, server, location), 2) semicolon (;), 3) ’{’ & '}'


		/*			ACTUAL PARSING				*/
		

		void	_printContent();


};
