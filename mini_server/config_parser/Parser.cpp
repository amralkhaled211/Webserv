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
	// this->_serverVec = other._serverVec;
	return *this;
}

Parser::~Parser() {
	// delete stuff, if needed, hopefully not needed
}

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
	// this->_printContent();
	this->_removeExcessSpace();
	// this->_printContent();
	this->_syntaxError();

	// parsing
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
	std::string			newContent;
	std::string			snippet;

	bool				addSpaceBefore = false;
	bool				addSpaceAfter = false;
	bool				addedSpaceAfterLast = false;

	while (ss >> snippet) {
		addSpaceBefore = ((std::string("{};").find(snippet[0]) == std::string::npos)
								&& addedSpaceAfterLast);
		addSpaceAfter = (std::string("{};").find(snippet[snippet.size() - 1]) == std::string::npos);

		if (addSpaceBefore) {
			newContent.append(" ");
			newContent.append(snippet);
		} else
			newContent.append(snippet);

		addedSpaceAfterLast = addSpaceAfter;
	}
	_content = newContent;
}

/*			SYNTAX ERRORS		*/

void		Parser::_syntaxError() {

	if (std::count(_content.begin(), _content.end(), '{') != std::count(_content.begin(), _content.end(), '}'))
		throw std::runtime_error("Invalid amount of Braces");

	if (_content.find('{') == std::string::npos)
		throw std::runtime_error("Missing Opening Brace");

	if (_content.find('}') == std::string::npos)
		throw std::runtime_error("Missing Closing Brace");

	if (_content.find(';') == std::string::npos)
		throw std::runtime_error("Missing Semicolon");

	if (_content[_content.find(";") + 1] == ';')
		throw std::runtime_error("Semicolon in Series");

	if (_content[_content.find("{") + 1] == '{')
		throw std::runtime_error("Opening Brace in Series");
}

/*			PARSING			*/

void		Parser::_fillBlocks() {
	

}


/*			DEBUG			*/

void	Parser::_printContent() { std::cout << _content << std::endl; }