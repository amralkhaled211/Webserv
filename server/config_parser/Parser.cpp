#include "Parser.hpp"

Parser::Parser() {};

Parser::Parser(std::string& fileName) { this->_configFile = fileName; }

Parser::Parser(const Parser& other) { *this = other; }

Parser&	Parser::operator=(const Parser& other) {
	if (this == &other)
		return *this;

	this->_configFile = other._configFile;
	this->_serverVec = other._serverVec;
	this->_content = other._content;

	return *this;
}

Parser::~Parser() { /* delete stuff, if needed, hopefully not needed */ }

std::vector<ServerBlock>&	Parser::getServerVec() { return this->_serverVec; }

static bool invalidPostfix(std::string& fileName) {
	size_t	postFixLen = std::string(POSTFIX).size();
	size_t	fileNameLen = fileName.size();

	if (fileNameLen <= postFixLen)
		return true;

	if (fileName.substr(fileNameLen - postFixLen, postFixLen) == POSTFIX)
		return false;

	return true;
}

static bool	httpCheck(std::string& line) {
	std::stringstream	ss(line);
	std::string			token;

	std::getline(ss, token, '#');
	ss.clear();
	ss.str("");
	ss << token;
	std::getline(ss, token, '{');
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
		bool		commentsAtTheBeginning;
		if (!std::getline(file, line))
			throw std::runtime_error("Config File is Empty");

		do {
			commentsAtTheBeginning = (line.find_first_of('#') < line.find_first_not_of(std::string(WHITESPACE) + "#"));
			if (line.empty() || line.find_first_not_of(WHITESPACE) == std::string::npos || commentsAtTheBeginning)
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
	this->_removeExcessSpace();

	this->_fillBlocks();

	this->_setupDefaults();

	this->_errorsAfterParsing();
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

			beginOfComment = line.find_first_of('#');

			if (beginOfComment != std::string::npos)
				_content.append(line.substr(amountBegSpaces(line), beginOfComment - amountBegSpaces(line)));
			else
				_content.append(line.substr(amountBegSpaces(line)));

			_content.append(" "); // separate lines with Spaces, excess space will be removed later
			
			if (_content.find_first_of("\"") != std::string::npos)
				throw std::runtime_error("Quotes Not Allowed");

		} while (std::getline(infileConfig, line));

		if (_content.size() < 6) // define this minimum later more accurate
			throw std::runtime_error("Not Enough Content");
	}
	catch(const std::exception& e)
	{
		throw;
	}
}

void		Parser::_removeExcessSpace() {

	std::stringstream	ss(_content);
	std::string			newContent, snippet;

	bool				addSpaceBefore = false;
	bool				addSpaceAfter = false;

	while (ss >> snippet) {
		addSpaceBefore = ((std::string("{};").find(snippet[0]) == std::string::npos)
								&& addSpaceAfter);

		if (addSpaceBefore) {
			newContent.append(" ");
			newContent.append(snippet);
		} else
			newContent.append(snippet);

		addSpaceAfter = (std::string("{};").find(snippet[snippet.size() - 1]) == std::string::npos);
	}
	_content = newContent;
}

/*			PARSING			*/

void		Parser::_fillBlocks() {

	std::stringstream		ss(_content);
	std::string				token;

	std::getline(ss, token, '{');
	if (token != "http")
		throw std::runtime_error("Missing Valid http Block");

	while (std::getline(ss, token, '{')) { // till the next condition check, I  should handle the server Block, then the next will start

		if (DEBUG)
			std::cerr << "H Token before: >" << token << "<" << std::endl;

		if (token == "server")
			_serverBlock(ss); // this should return/finish only when ss is right before the next server, that means after the last Server closes '}'
		else
			throw std::runtime_error("Expected Server Block");

		if (ss.fail())
			ss.clear();
		int	pos = ss.tellg();
		if (DEBUG) {
			std::cerr << "H Token after: >" << token << "<" << std::endl;
			std::cerr << BOLD_RED << "- Loop last line in _fillBlocks: Stream position: " << pos - 1 << " out of " << _content.size() << RESET << std::endl;
			std::cerr << BOLD_RED << "= Loop last line in _fillBlocks: Stream position: " << pos 	<< " out of " << _content.size() << RESET << std::endl;
			std::cerr << BOLD_RED << "+ Loop last line in _fillBlocks: Stream position: " << pos + 1 << " out of " << _content.size() << RESET << std::endl;
			std::cerr << "_content at pos: '" << _content[pos - 1] << "' and '" << _content[pos] << "' and '" << _content[pos + 1] << "'" << std::endl;
			std::cerr << BOLD_GREEN << "Done with One/Another Server Block\n" << RESET << std::endl;
		}

		if (pos != -1 && pos == static_cast<int>(_content.size()) -1 && _content[pos] == '}') // this is the end of the http block
			return;

		// this->_printServerVec();
	}
	throw std::runtime_error("Missing Closing Brace");
}

/*			SERVER BLOCK HANDLING			*/

void		Parser::_serverBlock(std::stringstream& ss) {
	std::string				token;
	std::string				deliSet(DELIMETERS);
	char					ch; // trying to go through ss char by char, because we have multiple delimeters to deal with

	if (DEBUG) std::cerr << BOLD_YELLOW << "new Server Block" << RESET << std::endl;
	_serverVec.push_back(ServerBlock());

	while (ss.get(ch)) {

		if (DEBUG) {
			size_t	pos = ss.tellg();
			std::cerr << "Char at " << pos - 1 << ": '" << ch << "'" << std::endl;
		}

		if (token == "location") {
			_locationBlock(ss); // must return right after '}' of the location block
			token.clear();
		}
		else if (deliSet.find(ch) != std::string::npos) {
			if (ch == ' ')
				_handleServerDirective(ss, token); // must go through the whole directive here
			else if (ch == ';') // wrong syntax, or issue in handleServerDirective() implementation
				throw std::runtime_error("Unexpected Semicolon");
			else if (ch == '{') {
				throw std::runtime_error("S Unexpected Opening Brace");
			}
			else if (ch == '}') { // I need to handle here only the server block closing brace
				if (!token.empty())
					throw std::runtime_error("S Unexpected Closing Brace");
				
				if (DEBUG)
					std::cerr << BOLD_RED << "Exiting _serverBlock: Stream position: " << ss.tellg() << " out of " << _content.size() << RESET << std::endl;
				break;
			}
			token.clear();
		} else
			token += ch;
	}
}


void		Parser::_handleServerDirective(std::stringstream& ss, const std::string& directiveKey) {

	std::string			directiveValue; // will be splited

	if (std::getline(ss, directiveValue, ';')) {
		if (DEBUG) {
			std::cerr << std::setw(20) << "S Dir Key: " << directiveKey << "		";
			std::cerr << std::setw(20) << "S Dir Value: " << directiveValue << std::endl;
		}

		_serverVec.back().setDirective(directiveKey, directiveValue); // errors must be dedected there
	}
	else {
		std::cerr << "error S Directive Key in S: " << directiveKey << std::endl;
		throw std::runtime_error("Missing Semicolon");
	}
}


/*			LOCATION BLOCK HANDLING			*/

void		Parser::_locationBlock(std::stringstream& ss) {
	std::string				token;
	std::string				deliSet(DELIMETERS);
	char					ch;

	if (DEBUG) std::cerr << BOLD_YELLOW <<  "new Location Block" <<  RESET << std::endl;

	_serverVec.back().addLocationBlock();

	while (ss.get(ch)) {

		if (deliSet.find(ch) != std::string::npos) {
			if (ch == ' ')
				_handleLocationDirective(ss, token); // must go through the whole directive here
			else if (ch == ';')
				throw std::runtime_error("Unexpected Semicolon");
			else if (ch == '{' && _serverVec.back().getLocationVec().back().getPrefix().empty()) {
				if (token.empty())
					throw std::runtime_error("Empty Location Prefix");
				else {
					if (isInvalidPath(token))
						throw std::runtime_error("Invalid Path for Location URI");
					_serverVec.back().getLocationVecBack().setPrefix(token);
				}
			}
			else if (ch == '{') {
				throw std::runtime_error("Unexpected Opening Brace");
			}
			else if (ch == '}') {
				// later: cases where token wouldn't be empty --> should be an error at this point
				if (!_serverVec.back().getLocationVec().back().getPrefix().empty() && token.empty()) {
					
					if (DEBUG) {
						std::cerr << BOLD_RED << "Exiting _locationBlock: Stream position: " << ss.tellg() << " out of " << _content.size() << RESET << std::endl;
						size_t	pos = ss.tellg();
						std::cerr << "- _content[" << pos - 1 << "]: " << _content[pos - 1] << std::endl;
						std::cerr << "=_content[" << pos << "]: " << _content[pos] << std::endl;
						std::cerr << "+ _content[" << pos + 1 << "]: " << _content[pos + 1] << std::endl;
						std::cerr << "Done with One/Another Location Block\n" << std::endl;
					}
					break;
				}
				else
					throw std::runtime_error("Unexpected Closing Brace");
			}
			token.clear();
		}
		else
			token += ch;
	}
}


void		Parser::_handleLocationDirective(std::stringstream& ss, const std::string& directiveKey) {

	std::string			directiveValue;

	if (std::getline(ss, directiveValue, ';')) {
		if (DEBUG) {
			std::cerr << std::setw(20) << "L Dir Key: " << directiveKey << "		";
			std::cerr << std::setw(20) << "L Dir Value: " << directiveValue << std::endl;
		}
		_serverVec.back().getLocationVec().back().setDirective(directiveKey, directiveValue); // potential errors must be detected there
	}
	else {
		throw std::runtime_error("Missing Semicolon");
	}
}

/*		SETUP DEFAULTS		*/

void		Parser::_setupDefaults() { // change name or create new function for errors
	for (size_t i = 0; i < _serverVec.size(); ++i) {
		_serverVec[i].setupDefaults();
	}
}

/*		ERRORS AFTER PARSING		*/

void			Parser::_errorsAfterParsing() {
	for (size_t i = 0; i < _serverVec.size(); ++i) {
		_serverVec[i].contextError(); // name should be more specific
	}
	this->_identServer();
}

static bool		foundIdentServers(ServerBlock& serverOne, ServerBlock& serverTwo) {

	std::vector<std::string>	hostPortOne = serverOne.getHostPort();
	std::vector<std::string>	hostPortTwo = serverTwo.getHostPort();

	for (size_t k = 0; k < hostPortOne.size(); ++k) {
		for (size_t l = k; l < hostPortTwo.size(); ++l) {
			if (hostPortOne[k] == hostPortTwo[l])
				return true;
		}
	}

	return false;
}

void		Parser::_identServer() {
	// error -> two servers with the same port and server_name
	for (size_t i = 0; i < _serverVec.size(); ++i) {
		for (size_t j = i + 1; j < _serverVec.size(); ++j) {
			if (foundIdentServers(_serverVec[i], _serverVec[j]))
				std::cout << YELLOW << "Warning: Found Server Blocks with Ident host:port!" << RESET << std::endl;
		}
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
