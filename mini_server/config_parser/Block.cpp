#include "Block.hpp"

Block::Block() { }

Block::Block(std::string type) : _blockType(type) {
	this->_root = "";
	this->_error_page = std::vector<std::vector<std::string> >();
	this->_return = std::vector<std::string>();
	this->_index = std::vector<std::string>();
	this->_autoindex = NOT_SET;
	// this->_client_max_body_size = "";
}

Block::Block( const Block& other ) { *this = other; }

Block&	Block::operator=( const Block& other ) {
	if (this == &other)
		return *this;

	this->_blockType = other._blockType;
	this->_root = other._root;
	this->_error_page = other._error_page;
	this->_return = other._return;
	this->_index = other._index;
	this->_autoindex = other._autoindex;
	this->_client_max_body_size = other._client_max_body_size;

	return *this;
}

Block::~Block() { }


/*			SETTERS			*/

void	Block::setRoot(std::string& root) { this->_root = root; }

void	Block::setErrorPage(std::vector<std::vector<std::string> >& error_page) { this->_error_page = error_page; }

void	Block::setReturn(std::vector<std::string>& ret) { this->_return = ret; }

void	Block::setIndex(std::vector<std::string>& index) { this->_index = index; }

void	Block::setAutoindex(bool autoindex) { this->_autoindex = autoindex; }

void	Block::setClientMaxBodySize(std::string& client_max_body_size) { this->_client_max_body_size = client_max_body_size; }


/*			GETTERS			*/

const std::string&	Block::getBlockType() const { return this->_blockType; }

const std::string&	Block::getRoot() const { return this->_root; }

const std::vector<std::vector<std::string> >&	Block::getErrorPage() const { return this->_error_page; }

const std::vector<std::string>&	Block::getReturn() const { return this->_return; }

const std::vector<std::string>&	Block::getIndex() const { return this->_index; }

const char&	Block::getAutoindex() const { return this->_autoindex; }

std::string	Block::getClientMaxBodySize() { return this->_client_max_body_size; }

bool	isInvalidPath(std::string root) {
	size_t	pos1 = root.find("/../") != std::string::npos;
	size_t	pos2 = root.find("../") != std::string::npos && root.find("../") == 0;
	size_t	pos3 = root.find("/..") != std::string::npos && root.find("/..") == root.size() - (1 + 2);
	size_t	pos4 = root.find("~/") != std::string::npos;
	
	if (pos1 || pos2 || pos3 || pos4) {
		return true;
	}
	return false;
}

static void	isValidCode(const std::string& code, int directive) {
	std::stringstream	ss(code.c_str());
	int					statusCode;

	if (ss.str().find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("Invalid return Directive Code");
	if (directive == IS_RETURN && (!(ss >> statusCode) || !(statusCode >= 100 && statusCode <= 599)))
		throw std::runtime_error("Invalid return Directive Code");
	if (directive == IS_ERROR_PAGE && (!(ss >> statusCode) || !(statusCode >= 300 && statusCode <= 599)))
		throw std::runtime_error("Invalid error_page Directive Code");
}


static void		invalidReturnSyntax(std::vector<std::string>& valueArgs) {
	if (valueArgs.empty() || valueArgs.size() > 2)
		throw std::runtime_error("Invalid return Directive");

	if (valueArgs.size() == 1) { // accept only external redir here
		// if (std::isdigit(valueArgs[0][0]))
		// 	isValidCode(valueArgs[0], IS_RETURN);
		// else 
		if (valueArgs[0].substr(0, 5) != "https")
			throw std::runtime_error("Invalid return Directive");
	}
	else if (valueArgs.size() == 2) {
		isValidCode(valueArgs[0], IS_RETURN);
		if (isInvalidPath(valueArgs[1]))
			throw std::runtime_error("Invalid Path for return");
	}
	else
		throw std::runtime_error("Invalid return Directive");
}


void	removeExcessSlashes(std::string& path) // this should be used when working with paths, except for location prefix
{
	std::string		result;
	bool			lastWasSlash = false;

	for (size_t i = 0; i < path.size(); ++i)
	{
		if (path[i] == '/')
		{
			if (!lastWasSlash)
			{
				result += path[i];
				lastWasSlash = true;
			}
		}
		else
		{
			result += path[i];
			lastWasSlash = false;
		}
	}

	path = result;
}

static bool		isInvalidSize(std::string sizeStr, int maxSizeByte) {
	if (sizeStr.find_first_not_of("1234567890") != std::string::npos)
		return true;
	std::stringstream	ss(sizeStr.c_str());
	int value;
	ss >> value;
	if (ss.fail() || value > maxSizeByte)
		return true;
	return false;
}

static bool		invalidMeasurementPostfix(std::string& directiveValue) {

	size_t		len = directiveValue.size();
	std::string	measurementUnitSet("kmKM"); // kilo and mega byte

	size_t		postfixNotSet = measurementUnitSet.find(directiveValue[len - 1]) == std::string::npos;

	int maxSizeByte = 512 * 1048576;
	if (postfixNotSet && !std::isdigit(directiveValue[len - 1]))
		return true;
	else if (postfixNotSet)
		return (isInvalidSize(directiveValue, maxSizeByte));

	std::stringstream	ss(directiveValue.c_str());

	int			size;
	std::string	word;

	ss >> size;
	if (ss.fail()) {
		return true;
	}

	ss >> word;
	if (!ss.eof() || word.size() != 1)
		return true;

	if ((word == "m" && size * 1048576 > maxSizeByte) ||
		(word == "k" && size * 1024 > maxSizeByte)) {
			return true;
		}

	return false;
}

static void		checkValidErrorPage(std::vector<std::string> valueArgs) {
	size_t amountArgs = valueArgs.size();
	
	if (amountArgs < 2)
		throw std::runtime_error("Invalid Number of Args for error_page");

	for (size_t i = 0; i < amountArgs; ++i) {
		if (i != (amountArgs - 1)) {
			isValidCode(valueArgs[i], IS_ERROR_PAGE);
		}
		if (i == (amountArgs - 1) && isInvalidPath(valueArgs[i]))
			throw std::runtime_error("Invalid Path used for error_page");
	}
}

static void		checkValidIndex(std::vector<std::string> valueArgs) {
	// nginx gives warning for absolute paths, if they are not the last one
	for (size_t i = 0; i < valueArgs.size(); ++i) {
		if (i != (valueArgs.size() - 1)) {
			if (valueArgs[i][0] == '/')
				throw std::runtime_error("only last index can be absolute path");
		}
		if (i == (valueArgs.size() - 1) && isInvalidPath(valueArgs[i]))
			throw std::runtime_error("Invalid Path used for index");
	}
}

bool		Block::_addCommonDirective(const std::string& directiveKey, std::string& directiveValue) { // to add: invalid values
	std::vector<std::string>	valueArgs(splitString(directiveValue));
	size_t						amountArgs(valueArgs.size());

	if (directiveKey == "root") {
		if (!this->_root.empty())
			throw std::runtime_error("Duplicate root Directive");
		if (amountArgs != 1)
			throw std::runtime_error("Invalid root Directive");
		if (isInvalidPath(directiveValue))
			throw std::runtime_error("Invalid root Directive Path");
		removeExcessSlashes(directiveValue);
		this->_root = directiveValue;
		return true;
	}
	else if (directiveKey == "error_page") {
		checkValidErrorPage(valueArgs);
		this->_error_page.push_back(valueArgs);
		return true;
	}
	else if (directiveKey == "return") {
		invalidReturnSyntax(valueArgs);
		if (!this->_return.empty())// no duplicate check, nginx multiple but will only use the first, maybe we should do it different???
			return true;
		this->_return = valueArgs;
		return true;
	}
	else if (directiveKey == "index") {
		if (!this->_index.empty())
			throw std::runtime_error("Duplicate index Directive");
		if (valueArgs.size() < 1)
			throw std::runtime_error("Invalid Number of Args for index");
		checkValidIndex(valueArgs);
		this->_index = valueArgs;
		return true;
	}
	else if (directiveKey == "autoindex") {
		if (this->_autoindex != NOT_SET)
			throw std::runtime_error("Duplicate autoindex Directive");
		if (directiveValue == "on")
			this->_autoindex = ON;
		else if (directiveValue == "off")
			this->_autoindex = OFF;
		else
			throw std::runtime_error("Invalid autoindex Directive");
		return true;
	}
	else if (directiveKey == "client_max_body_size") {
		if (!this->_client_max_body_size.empty())
			throw std::runtime_error("Duplicate client_max_body_size Directive");
		else if (amountArgs != 1)
			throw std::runtime_error("Invalid client_max_body_size Directive, too many args");
		// client_max_body_size error handling
		else if (invalidMeasurementPostfix(directiveValue)) // TODO: 0 means unlimited and limit max size
			throw std::runtime_error("Invalid client_max_body_size Directive");
		this->_client_max_body_size = directiveValue;
		return true;
	}
	return false;
}


/*			UTILS			*/

std::vector<std::string>	splitString(const std::string& input) { // utils  --> c++ version of ft_split
	std::istringstream			iss(input);
	std::vector<std::string>	splitedInput;
	std::string					word;

	while (iss >> word) {
		splitedInput.push_back(word);
	}
	return splitedInput;
}
