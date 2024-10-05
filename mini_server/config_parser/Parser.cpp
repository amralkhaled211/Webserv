#include "Parser.hpp"

Parser::Parser() {};

Parser::Parser(std::string& fileName) { this->_configFile = fileName; }

Parser::Parser(const Parser& other) { *this = other; }

Parser&	Parser::operator=(const Parser& other) {
	if (this == &other)
		return *this;
	this->_configFile = other._configFile;
	// this->_serverVec = other._serverVec;
	return *this;
}

Parser::~Parser() { /* delete stuff, if needed, hopefully not needed */ }

static bool invalidPostfix(std::string& fileName) {
	size_t	pfLen = std::string(POSTFIX).size();
	size_t	fnLen = fileName.size();

	if (fnLen <= pfLen)
		return true;

	if (fileName.substr(fnLen - pfLen, pfLen) == POSTFIX)
		return false;

	return true;
}

/* these functions are not used at the moment */

static int	checkSemicolon(std::string& line) {
	if (line.find(';') == std::string::npos)
		return INVALID;
	if (line.find_last_of(';') == line.size() - 1)
		return VALID;
	return INVALID;
	// MISSING CASES --> Whitespace or comment after semicolon
}

static bool	isKeyword(std::string& line) {
	std::stringstream	ss(line);

	std::vector<std::string>	array;
	std::string					word;

	while (ss >> word) {
		array.push_back(word);
	}

	if (array.size() > 2)
		return false;

	if (array[0] == "server" || array[0] == "location" || array[0] == "{" || array[0] == "}")
		return true;

	// possible check for array[1] to be anything other then "{" "}"
	return false;
}

/* the functions from here onwards are used again */

static bool	httpCheck(std::string& line) {
	std::stringstream	ss(line);
	std::string			token;

	std::getline(ss, token, '{'); // '{' is set as delimeter
	ss.clear();
	ss.str("");
	ss << token;
	ss >> token;
	if (!token.compare("http") && !(ss >> token))
		return true;

	return false;
}

int		Parser::_generalErrors(std::string fileName) {

	if (invalidPostfix(fileName))
		throw std::runtime_error("Invalid Postfix for Config File");

	std::ifstream	file(fileName.c_str());
	if (file.fail())
		throw std::runtime_error("Failed to Open File");

	try {
		std::string	line;
		if (!std::getline(file, line))
			throw std::runtime_error("Config File is Empty");

		do {
			if (line.empty() || line.find_first_not_of(WHITESPACE) == std::string::npos)
				continue;

			if (!httpCheck(line))
				throw std::runtime_error("Not Embeded in http Block");
			else
				break;
		} while (std::getline(file, line));

		if (line.empty())
			throw std::runtime_error("Config File is Empty (only empty lines)");

		file.close();
	}
	catch (const std::exception& e) {
		file.close();
		throw;
	}
	return VALID;
}



void		Parser::_parser() {
	// prep
	this->_configToContent();
	this->_printContent();
	this->_removeExcessSpace();
	this->_printContent();
	this->_syntaxError();

	// parsing
	this->_fillBlocks();
	// this->_printServerVec();
}

/*			PREP FOR PARSING			*/

size_t	amountBegSpaces(const std::string& line) { // utils --> returns amount of spaces at the beginning of a line
	size_t	amount = 0;

	while (line[amount] == ' ')
		amount++;
	return amount;
}

void		Parser::_configToContent() {
	std::ifstream	infileConfig(_configFile.c_str());
	std::string		line;
	size_t			beginOfComment;

	try
	{
		if (!std::getline(infileConfig, line))
			throw std::runtime_error("Empty File");

		do {
			if (line.empty() || line.find_first_not_of(WHITESPACE) == std::string::npos)
				continue;

			while (line.find('\t') != std::string::npos)
				line.replace(line.find('\t'), 1, " ");

			beginOfComment = line.find_first_of('#'); // need to also check whether inside of quote

			if (beginOfComment != std::string::npos)
				_content.append(line.substr(amountBegSpaces(line), beginOfComment - amountBegSpaces(line)));
			else
				_content.append(line.substr(amountBegSpaces(line)));

			_content.append(" "); // separate lines with Spaces, excess space will be removed later
		} while (std::getline(infileConfig, line));

		if (_content.size() < 6) // define this minimum later more accurate
			throw std::runtime_error("Not Enough Content");
	}
	catch(const std::exception& e)
	{
		throw;
	}
}

// excess spaces: before and after 1) block names (http, server, location), 2) semicolon ';', 3) ’{’ & '}'
// http{server{listen 8090   ;server_name 127.0.0.1;location /hello{alias /home/aismaili/webSite;index index.html;}} }
void		Parser::_removeExcessSpace() {

	std::stringstream	ss(_content);
	std::string			newContent, snippet;

	bool				addSpaceBefore = false;
	bool				addSpaceAfter = false;
	// bool				addedSpaceAfterLast = false;

	while (ss >> snippet) {
		addSpaceBefore = ((std::string("{};").find(snippet[0]) == std::string::npos)
								&& addSpaceAfter);

		if (addSpaceBefore) {
			newContent.append(" ");
			newContent.append(snippet);
		} else
			newContent.append(snippet);

		addSpaceAfter = (std::string("{};").find(snippet[snippet.size() - 1]) == std::string::npos);
		// addedSpaceAfterLast = addSpaceAfter;
	}
	_content = newContent;
}

/*			SYNTAX ERRORS		*/

void		Parser::_syntaxError() {

	if (std::count(_content.begin(), _content.end(), '{')
		!= std::count(_content.begin(), _content.end(), '}'))
		throw std::runtime_error("Invalid amount of Braces");

	if (_content.find('{') == std::string::npos)
		throw std::runtime_error("Missing Opening Brace");

	if (_content.find('}') == std::string::npos)
		throw std::runtime_error("Missing Closing Brace");

	if (_content.find(';') == std::string::npos)
		throw std::runtime_error("Missing Semicolon");

	if (_content[_content.find(";") + 1] == ';')
		throw std::runtime_error("Semicolons in Series");

	if (_content[_content.find("{") + 1] == '{')
		throw std::runtime_error("Opening Braces in Series");
}

/*			PARSING			*/

void		Parser::_fillBlocks() {

	std::stringstream		ss(_content);
	std::string				token;

	char					ch;

	std::getline(ss, token, '{');
	if (token != "http")
		throw std::runtime_error("Missing Valid http Block");


	while (std::getline(ss, token, '{')) { // till the next condition check, I  should handle the server Block, then the next will start
		std::cerr << "H Token: >" << token << "<" << std::endl;
		if (token == "server")
			std::cerr << "Server Block" << std::endl;
		if (token == "server")
			_serverBlock(ss); // this should return/finish only when ss is right before the next server, that means after the last Server closes '}'
		// else
		// 	throw std::runtime_error("H Unexpected Block");
		// ss.get(ch); // skip the ’}’
		if (ss.fail())
			ss.clear();
		std::cerr << "H Token: >" << token << "<" << std::endl;
		std::cerr << "Done with One/Another Server Block\n" << std::endl;
		this->_printServerVec();

	}
}

void		Parser::_serverBlock(std::stringstream& ss) {
	std::string				token;
	std::string				deliSet(DELIMETERS);
	char					ch; // trying to go through ss char by char, because we have multiple delimeters to deal with
	// size_t					index;
	std::cerr << "new Server Block" << std::endl;
	_serverVec.push_back(ServerBlock());
	std::cerr << "Addess of the new Server: " << &_serverVec.back() << std::endl;
	std::cerr << "Addess of the first Server: " << &_serverVec.front() << std::endl;

	while (ss.get(ch)) {

		if (token == "location") {
			_locationBlock(ss); // this should return/finish only when ss is finished with this specific location '}'
			token.clear();
			// continue;
		}

		// gotta see if it is a delimeter
		else if (deliSet.find(ch) != std::string::npos) {
			// we must have a token
				// case where we won't have a token, despite a delimeter:
					// ;}
					// }}
			if (ch == ' ')
				_handleServerDirective(ss, token); // must go through the whole directive here
			else if (ch == ';') // wrong syntax, or issue in handleServerDirective() implementation
				throw std::runtime_error("Unexpected Semicolon");
			else if (ch == '{') {
				std::cerr << "Token: " << token << std::endl;
				throw std::runtime_error("S Unexpected Opening Brace");
			}
			else if (ch == '}') { // I need to handle here only the server block closing brace, which I think I do (maybe)
				std::cerr << "Exiting _serverBlock: Stream position: " << ss.tellg() << " out of " << _content.size() << std::endl;
				break;
			}
				// throw std::runtime_error("Unexpected Closing Brace"); // this one needs more review
			token.clear();
		} else
			token += ch;
	}
}


void		Parser::_handleServerDirective(std::stringstream& ss, const std::string& directiveKey) {
	// access the server block and fill the directive
	std::string			directiveValue; // will be splited
	// std::streampos		lastPos(ss.tellg()); // save pos

	if (std::getline(ss, directiveValue, ';')) {
		// size_t	posLocation = directiveValue.find("location");
		// size_t	posOpenedB = directiveValue.find('{');
		// size_t	posClosedB = directiveValue.find('}');

		// if (posLocation != std::string::npos && posLocation < posOpenedB) { // gotta work on this check
		// 	// ss.seekg(lastPos); // retract to saved/old pos
		// 	return ;
		// }
		// fill the directive
		_serverVec.back().setDirective(directiveKey, directiveValue); // potential errors must be dedected there
	}
	else {
		std::cerr << "Directive Key  in S: " << directiveKey << std::endl;
		throw std::runtime_error("Missing Semicolon");
	}
}


void		Parser::_locationBlock(std::stringstream& ss) { // this is gonna be recurcive --> possibility of nested location
	std::string				token;
	std::string				deliSet(DELIMETERS);
	char					ch;

	_serverVec.back().getLocationVec().push_back(LocationBlock());

	while (ss.get(ch)) {
		// std::cerr << "Char: '" << ch << "'" << std::endl;
		if (token == "location") {
			_locationBlock(ss); // this should return/finish only when ss is finished with this specific location
			continue;
		}

		if (deliSet.find(ch) != std::string::npos) {
			if (ch == ' ')
				_handleLocationDirective(ss, token); // must go through the whole directive here
			else if (ch == ';')
				throw std::runtime_error("Unexpected Semicolon");
			else if (ch == '{' && _serverVec.back().getLocationVec().back().getPrefix().empty()) {
				if (token.empty())
					throw std::runtime_error("Empty Location Prefix");
				else
					_serverVec.back().getLocationVec().back().setPrefix(token);
			}
			else if (ch == '{'/*  && !_serverVec.back().getLocationVec().back().getPrefix().empty() */) { // redundant
				std::cerr << "Token: " << token << std::endl;
				throw std::runtime_error("L Unexpected Opening Brace");
			}
			// else if (ch == '}' && _serverVec.back().getLocationVec().back().getPrefix().empty()) // redundant
			// 	throw std::runtime_error("Unexpected Closing Brace");
			else if (ch == '}') {
				std::cerr << "L Token: " << token << std::endl;
				if (!_serverVec.back().getLocationVec().back().getPrefix().empty() && token.empty())
					break;
				else
					throw std::runtime_error("L Unexpected Closing Brace");
			}
			// else if (ch == '}' && !token.empty()) // redundant
			// 	throw std::runtime_error("Unexpected Closing Brace");
			token.clear();
		}
		else
			token += ch;
	}
}


void		Parser::_handleLocationDirective(std::stringstream& ss, const std::string& directiveKey) {
	// just copied the logic from _handleServerDirective(), maybe needs adjustment
	std::string			directiveValue; // will be splited
	// std::streampos		lastPos(ss.tellg()); // save pos

	if (std::getline(ss, directiveValue, ';')) {
		std::cerr << "L Directive Key: " << directiveKey << std::endl;
		std::cerr << "L Directive Value: " << directiveValue << std::endl;
		// size_t	posLocation = directiveValue.find("location");
		// size_t	posOpenedB = directiveValue.find('{');
		// size_t	posClosedB = directiveValue.find('}');

		// if (posLocation != std::string::npos && posLocation < posOpenedB) {
		// 	// ss.seekg(lastPos); // retract to saved/old pos
		// 	return ;
		// }
		// fill the directive
		_serverVec.back().getLocationVec().back().setDirective(directiveKey, directiveValue); // potential errors must be dedected there
	}
	else {
		std::cerr << "Directive Key in L: " << directiveKey << std::endl;
		throw std::runtime_error("Missing Semicolon");
	}
}

/*			DEBUG			*/

void	Parser::_printContent() { std::cout << _content << std::endl; }

void	Parser::_printServerVec() {
	std::cout << "Server Vector Size: " << _serverVec.size() << std::endl;
	for (size_t i = 0; i < _serverVec.size(); i++) {
		std::cout << "Server " << i << std::endl;
		std::cout << "Adrress: " << &_serverVec[i] << std::endl;
		_serverVec[i].printServerBlock();
		std::cout << std::endl;
	}
}
