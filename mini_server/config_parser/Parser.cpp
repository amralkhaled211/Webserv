#include "Parser.hpp"

Parser::Parser() {};

Parser::Parser(std::string& fileName) {
	// fill serverVec
	this->_configFile = fileName;
}

Parser::Parser(const Parser& other) {
	*this = other;
}

Parser&	Parser::operator=(const Parser& other) {
	if (this == &other)
		return *this;
	this->_configFile = other._configFile;
	this->_serverVec = other._serverVec;
	return *this;
}

Parser::~Parser() {
	// delete stuff, if needed, hopefully not needed
}

static bool invalidPostfix(std::string& fileName) {
	size_t	pfLen = std::string(POSTFIX).size();
	size_t	fnLen = fileName.size();

	if (fnLen <= pfLen)
		return false;
	if (fileName.substr(fnLen - pfLen, pfLen) == POSTFIX)
		return true;
	return false;
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
	ss << token;
	ss >> token;
	if (!token.compare("http") && ss.str().empty())
		return true;
	return false;
}

int		Parser::_generalErrors(std::string& fileName) {

	if (invalidPostfix(fileName))
		throw std::runtime_error("Invalid Postfix for Config File");

	std::ifstream	file(fileName.c_str());
	if (!file.fail())
		throw std::runtime_error("Failed to Open File");

	try {
		std::string	line;
		if (!std::getline(file, line))
			throw std::runtime_error("Config File is Empty");

		do {
			if (line.empty()) {
				if (!std::getline(file, line))
					throw std::runtime_error("Config File is Empty (only empty lines)");
				else
					continue;
			}
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
	this->_removeExcessSpace();

	// parsing
}

/*			PREP FOR PARSING			*/

void		Parser::_configToContent() {
	std::ifstream	infileConfig(_configFile.c_str());
	std::string		line;
	size_t			beginOfComment;

	try
	{
		if (!std::getline(infileConfig, line))
			throw std::runtime_error("Empty File");

		do {
			if (line.empty() || line.find_first_not_of(WHITESPACE) == std::string::npos) { // sec condition is for lines with only spaces
				if (std::getline(infileConfig, line))
					continue;
				else
					break;
			}

			while (line.find('\t') != std::string::npos) {
				line.replace(line.find('\t'), 1, " ");
			}				

			beginOfComment = line.find_first_of('#'); // need to also check whether inside of quote

			if (beginOfComment != std::string::npos) 
				_content.append(line.substr(0, beginOfComment));
			else
				_content.append(line);

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
	std::string			newContent;
	std::stringstream	ss2; // will change name later
	std::string			tokenA; // will change name later
	std::string			token;
	size_t				httpCounter = 0, serverCounter = 0, locationCounter = 0;

	while (std::getline(ss, tokenA, ' ')) {
		// check does it start with '{', '}' or ';'
			// --> remove excess space before
		if (std::string("{};").find(tokenA[0])) {
			newContent.append(tokenA);
		}



		// check does it end with '{', '}' or ';'
			// --> remove excess space after
			// else --> 

		if (std::string("{};").find(tokenA[tokenA.size() - 1])) { // size() should not output 0 here --> still will test
			newContent.append(tokenA);
		}

		/* ss2.clear();
		ss2 << tokenA;
		ss2 >> token;
		if (IS_BLOCKNAME) {
			// find start of block name and end of last word --> remove from end of last word + 1 to start of block name - 1
			// find end of block name and start of next word --> remove from end of block name + 1 to start of next word - 1
			// issue with block names that are not separated by spaces  --> could solve with added Condition
			// issue with repeated block names -> server and location --> could solve with counter
			_content.erase();
		}
		else if (IS_SEMICOLON) {
			// remove excess spaces
		}
		else if (IS_BRACE) {
			// remove excess spaces
		}
		else if (!(IS_DIRECTIVE)) {
			throw std::runtime_error("Invalid Token");
		} */
	}
	_content = newContent;
}

