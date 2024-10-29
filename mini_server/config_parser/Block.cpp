#include "Block.hpp"

Block::Block() { }

Block::Block(std::string type) : _blockType(type) {
	this->_root = "";
	this->_error_page = std::vector<std::string>();
	this->_return = std::vector<std::string>();
	this->_try_files = std::vector<std::string>();
	this->_index = std::vector<std::string>();
	this->_autoindex = NOT_SET;
	this->_client_max_body_size = "";
}

Block::Block( const Block& other ) { *this = other; }

Block&	Block::operator=( const Block& other ) {
	if (this == &other)
		return *this;

	this->_blockType = other._blockType;
	this->_root = other._root;
	this->_error_page = other._error_page;
	this->_return = other._return;
	this->_try_files = other._try_files;
	this->_index = other._index;
	this->_autoindex = other._autoindex;

	return *this;
}

Block::~Block() { }


/*			SETTERS			*/

void	Block::setRoot(std::string& root) { this->_root = root; }

void	Block::setErrorPage(std::vector<std::string>& error_page) { this->_error_page = error_page; }

void	Block::setReturn(std::vector<std::string>& ret) { this->_return = ret; }

void	Block::setTryFiles(std::vector<std::string>& try_files) { this->_try_files = try_files; }

void	Block::setIndex(std::vector<std::string>& index) { this->_index = index; }

void	Block::setAutoindex(bool autoindex) { this->_autoindex = autoindex; }


/*			GETTERS			*/

const std::string&	Block::getBlockType() const { return this->_blockType; }

const std::string&	Block::getRoot() const { return this->_root; }

const std::vector<std::string>&	Block::getErrorPage() const { return this->_error_page; }

const std::vector<std::string>&	Block::getReturn() const { return this->_return; }

const std::vector<std::string>&	Block::getTryFiles() const { return this->_try_files; }

const std::vector<std::string>&	Block::getIndex() const { return this->_index; }

const char&	Block::getAutoindex() const { return this->_autoindex; }


static bool	isValidRedirCode(const std::string& code) {
	std::stringstream	ss(code.c_str());
	int					statusCode;

	if (ss.str().find_first_not_of("0123456789") == std::string::npos)
			throw std::runtime_error("Invalid Return Directive Code");
	if (!(ss >> statusCode) || !(statusCode >= 100 && statusCode <= 599))
		return false;
	return true;
}

static bool isValidRedirURLI(std::string& redir) { // URL URI check, (if comments remove)
	// will see what to check for, maybe something regarding quotes
}

void		invalidReturnSyntax(std::vector<std::string>& valueArgs) {
	if (valueArgs.empty() || valueArgs.size() > 2)
		throw std::runtime_error("Invalid Return Directive");

	if (valueArgs.size() == 1) { // accept either only a status code OR a link (not a location)
		if (valueArgs[0][0] >= '0' && valueArgs[0][0] <= '9') // isdigit() is from c library
			isValidRedirCode(valueArgs[0]);
		else
			isValidRedirURLI(valueArgs[0]);
	}
	else if (valueArgs.size() == 2) {
		isValidRedirCode(valueArgs[0]);
		isValidRedirURLI(valueArgs[1]);
	}
	else
		throw std::runtime_error("Invalid Return Directive");
}


bool		Block::_addCommonDirective(const std::string& directiveKey, std::string& directiveValue) { // to add: invalid values
	std::vector<std::string>	valueArgs(splitString(directiveValue));
	size_t						amountArgs(valueArgs.size());

	if (directiveKey == "root") {
		if (!this->_root.empty())
			throw std::runtime_error("Duplicate root Directive");
		if (amountArgs != 1)
			throw std::runtime_error("Invalid root Directive");
		this->_root = directiveValue;
		return true;
	}
	else if (directiveKey == "error_page") {
		if (!this->_error_page.empty())
			throw std::runtime_error("Duplicate error_page Directive");
		this->_error_page = valueArgs;
		return true;
	}
	else if (directiveKey == "return") {
		invalidReturnSyntax(valueArgs);
		if (!this->_return.empty()) {
			return true;
		}
		this->_return = valueArgs;
		return true;
	}
	else if (directiveKey == "try_files") {
		if (!this->_try_files.empty())
			throw std::runtime_error("Duplicate try_files Directive");
		this->_try_files = valueArgs;
		return true;
	}
	else if (directiveKey == "index") { // requires new implementation, not handling some cases
		if (!this->_index.empty())
			throw std::runtime_error("Duplicate index Directive");
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
			throw std::runtime_error("Invalid client_max_body_size Directive");
		else
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