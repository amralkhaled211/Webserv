#pragma once

#include <string>
#include <map>
#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "Block.hpp"
#include "ServerBlock.hpp"

#define INVALID						0
#define	VALID						1
#define POSTFIX						".conf"
#define WHITESPACE					" \t\n\v\f\r" /// might only need " \t"
#define	DELIMETERS					"{}; "

#define DEBUG						0

// colors for debugging
#define RED							"\033[1;31m"
#define GREEN						"\033[1;32m"
#define YELLOW						"\033[1;33m"
#define BOLD_RED					"\033[1;91m"
#define BOLD_GREEN					"\033[1;92m"
#define BOLD_YELLOW					"\033[1;93m"
#define RESET						"\033[0m"

// not in use
#define IS_BRACE_O					ch == '{'
#define IS_BRACE_C					ch == '}'
#define IS_SEMICOLON				ch == ';'
#define IS_SPACE					ch == ' '

class Parser // http Block basically
{
	private:
		std::string						_configFile;
		std::vector<ServerBlock>		_serverVec; // these will always be ServerBlocks as elements, and those will have a vector with LocationBlocks as elements
		std::string						_content;

		Parser();

	public:
		Parser(std::string& fileName);
		Parser(const Parser& other);
		Parser&	operator=(const Parser& other);
		~Parser();

		static int	_generalErrors(std::string fileName);

		std::vector<ServerBlock>& getServerVec();

		void		_parser(); // other name: _callParserFunctions()

		/*			PREP FOR PARSING		*/

		void		_configToContent(); // Content is basically everything compressed in one string // other name: _fileContentToOneStr(), _fileToStr()
										// !!! in case of comments (#), skip rest of line (don't fill it inside _content) and coninue with the next line !!!
		void		_removeExcessSpace(); // excess spaces: before and after 1) block names (http, server, location), 2) semicolon (;), 3) ’{’ & '}'

		/*			ERROR			*/

		void		_syntaxError(); // check for syntax errors --> missing semicolon (directive specific mostly), missing brace (using booleans like in minishell with quotes),
									// missing block name (before a opening brace there should be a block name), missing directive (see notes), ...

		/*			ACTUAL PARSING			*/

		void		_fillBlocks(); // could also name httpBlock();
		void		_serverBlock(std::stringstream& ss);
		void		_locationBlock(std::stringstream& ss);

		void		_handleServerDirective(std::stringstream& ss, const std::string& directiveKey);
		void		_handleLocationDirective(std::stringstream& ss, const std::string& directiveKey);


		/*			SETUP DEFAULTS			*/
		void		_setupDefaults(); // other name: _setDefaults()


		/*				DEBUG				*/

		void	_printContent();
		void	_printServerVec();
};
