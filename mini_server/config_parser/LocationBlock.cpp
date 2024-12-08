#include "LocationBlock.hpp"

LocationBlock::LocationBlock() : Block("locationBlock") {
	this->_allowed_methods = std::vector<std::string>();
	this->_cgi_path = std::vector<std::string>();
	this->_cgi_ext = std::vector<std::string>();
}


LocationBlock::LocationBlock(const LocationBlock& other) : Block(other) { *this = other; }

LocationBlock&	LocationBlock::operator=(const LocationBlock& other) {
	if (this == &other)
		return *this;

	Block::operator=(other);

	this->_allowed_methods = other._allowed_methods;
	this->_cgi_path = other._cgi_path;
	this->_cgi_ext = other._cgi_ext;
	this->_prefix = other._prefix;

	return *this;
}

LocationBlock::~LocationBlock() { }


std::string&					LocationBlock::getPrefix() { return this->_prefix; }

std::vector<std::string>&		LocationBlock::getAllowedMethods() { return this->_allowed_methods; }

std::vector<std::string>&		LocationBlock::getCgiPath() { return this->_cgi_path; }

std::vector<std::string>&		LocationBlock::getCgiExt() { return this->_cgi_ext; }


static bool	isCgiExtNotAllowed(std::vector<std::string> enteredCgiExt) {
	for (size_t i = 0; i < enteredCgiExt.size(); ++i) {
		if (enteredCgiExt[i] != ".py" && enteredCgiExt[i] != ".php")
			return true;
	}
	return false;
}

void	LocationBlock::setDirective(const std::string& directiveKey, std::string& directiveValue) {

	if (_addCommonDirective(directiveKey, directiveValue))
		return;

	std::vector<std::string>	valueArgs(splitString(directiveValue));
	size_t						amountArgs(valueArgs.size());

	if (directiveKey == "allowed_methods") {

		if (amountArgs > 3)
			throw std::runtime_error("Invalid allowed_methods Directive");

		for (size_t i = 0; i < amountArgs; i++) {
			if (valueArgs[i] != "GET" && valueArgs[i] != "POST" && valueArgs[i] != "DELETE")
				throw std::runtime_error("Invalid allowed_methods Directive");
		}
		this->_allowed_methods = valueArgs;
	}
	else if (directiveKey == "cgi_path") { // will see if we will use it, not using so far
		// check specifically from a range of allowed ones
		for (size_t i = 0; i < amountArgs; i++)
			this->_cgi_path.push_back(valueArgs[i]);
	}
	else if (directiveKey == "cgi_ext") {
		// check specifically from a range of allowed ones
		// if (isCgiExtNotAllowed(valueArgs))
		// 	throw std::runtime_error("CGI extension not Supported");
		for (size_t i = 0; i < amountArgs; i++)
			this->_cgi_ext.push_back(valueArgs[i]);
	}
	else {
		std::cerr << "Directive Value: " << directiveValue << "<" << std::endl;
		std::cerr << "Directive Key: " << directiveKey << "<" << std::endl;
		throw std::runtime_error("Invalid Location Directive");
	}
}

void		LocationBlock::setPrefix(const std::string& prefix) { this->_prefix = prefix; }

void		LocationBlock::setupDefaults(Block& parentServer) {

	if (this->_root.empty())
		this->_root = parentServer.getRoot();

	if (this->_index.empty())
		this->_index = parentServer.getIndex(); // note: only matters if a directory was requested

	if (this->_autoindex == NOT_SET)
		this->_autoindex = parentServer.getAutoindex();

	if (this->_error_page.empty())
		this->_error_page = parentServer.getErrorPage();

	if (this->_return.empty())
		this->_return = parentServer.getReturn();
	else if (this->_return.size() == 1)
		this->_return.insert(this->_return.begin(), "302");

	if (this->_autoindex == NOT_SET)
		this->_autoindex = parentServer.getAutoindex();

	if (this->_allowed_methods.empty()) {
		this->_allowed_methods.push_back("GET");
		this->_allowed_methods.push_back("POST");
		this->_allowed_methods.push_back("DELETE");
	}
}

/*		DEBUG		*/

void		LocationBlock::printLocationBlock() {
	std::cout << "Block Type: " << this->_blockType << std::endl;
	std::cout << "Root: " << this->_root << std::endl;
	std::cout << "Error Page: ";
	for (size_t i = 0; i < this->_error_page.size(); ++i) {
		for (size_t j = 0; j < this->_error_page[i].size(); ++j)
			std::cout << this->_error_page[i][j] << " ";
	}
	std::cout << std::endl;
	std::cout << "Return: ";
	for (size_t i = 0; i < this->_return.size(); i++)
		std::cout << this->_return[i] << " ";
	std::cout << std::endl;
	std::cout << "Index: ";
	for (size_t i = 0; i < this->_index.size(); i++)
		std::cout << this->_index[i] << " ";
	std::cout << std::endl;
	std::cout << "Autoindex: " << this->_autoindex << std::endl;
	std::cout << "Allowed Methods: ";
	for (size_t i = 0; i < this->_allowed_methods.size(); i++)
		std::cout << this->_allowed_methods[i] << " ";
	std::cout << std::endl;
	std::cout << "Cgi Path: ";
	for (size_t i = 0; i < this->_cgi_path.size(); i++)
		std::cout << this->_cgi_path[i] << " ";
	std::cout << std::endl;
	std::cout << "Cgi Ext: ";
	for (size_t i = 0; i < this->_cgi_ext.size(); i++)
		std::cout << this->_cgi_ext[i] << " ";
	std::cout << std::endl;
	std::cout << std::endl;
}