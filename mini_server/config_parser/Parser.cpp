#include "Parser.hpp"

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

/* these functions are not used at the moment */

static bool invalidPostfix(std::string& fileName) {
	size_t	pfLen = std::string(POSTFIX).size();
	size_t	fnLen = fileName.size();

	if (fnLen <= pfLen)
		return false;
	if (fileName.substr(fnLen - pfLen, pfLen) == POSTFIX)
		return true;
	return false;
}

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

	ss >> token;
	if (!token.compare("http") || !token.compare("http{"))
		return true;
	return false;
}

int		Parser::_generalErrors(std::string& fileName) {

	if (invalidPostfix(fileName))
		throw std::runtime_error("Wrong Postfix for Config File");

	std::ifstream	file(fileName.c_str());
	if (!file.is_open())
		throw std::runtime_error("Failed to Open File");

	try {
		std::string	line;
		if (std::getline(file, line))
			throw std::runtime_error("Config File is Empty");

		do {
			if (line.empty())
				continue;
			if (!httpCheck(line))
				throw std::runtime_error("Not Embeded in http Block");
			else
				break;
		} while (std::getline(file, line));

		file.close();
	}
	catch (const std::exception& e) {
		file.close();
		throw;
	}
	return VALID;
}
