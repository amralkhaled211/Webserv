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
			if (line.empty()) {
				if (std::getline(infileConfig, line))
					continue;
				else
					break;
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

// excess spaces: before and after 1) block names (http, server, location), 2) semicolon (;), 3) ’{’ & '}'
void		Parser::_removeExcessSpace() {



}

