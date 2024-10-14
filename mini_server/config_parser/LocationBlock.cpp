#include "LocationBlock.hpp"

LocationBlock::LocationBlock() : Block("locationBlock") {
	this->_allowed_methods = std::vector<std::string>();
	this->_cgi_path = std::vector<std::string>();
	this->_cgi_ext = std::vector<std::string>();
	this->_nestedLocationVec = std::vector<LocationBlock>();
}


LocationBlock::LocationBlock(const LocationBlock& other) : Block(other) { *this = other; }

LocationBlock&	LocationBlock::operator=(const LocationBlock& other) {
	if (this == &other)
		return *this;

	Block::operator=(other);

	this->_nestedLocationVec = other._nestedLocationVec;
	this->_allowed_methods = other._allowed_methods;
	this->_cgi_path = other._cgi_path;
	this->_cgi_ext = other._cgi_ext;
	this->_prefix = other._prefix;

	return *this;
}

LocationBlock::~LocationBlock() { }


std::vector<LocationBlock>&		LocationBlock::getNestedLocationVec() { return this->_nestedLocationVec; }

std::string&					LocationBlock::getPrefix() { return this->_prefix; }

std::vector<std::string>&		LocationBlock::getAllowedMethods() { return this->_allowed_methods; }

std::vector<std::string>&		LocationBlock::getCgiPath() { return this->_cgi_path; }

std::vector<std::string>&		LocationBlock::getCgiExt() { return this->_cgi_ext; }


void							LocationBlock::addLocationBlock() { this->_nestedLocationVec.push_back(LocationBlock()); }


void	LocationBlock::setDirective(const std::string& directiveKey, std::string& directiveValue) {

	if (_addCommonDirective(directiveKey, directiveValue))
		return;

	std::vector<std::string>	valueArgs(splitString(directiveValue));
	size_t						amountArgs(valueArgs.size());

	if (directiveKey == "allowed_methods") {

		if (amountArgs == 0 || amountArgs > 3) // 0 check not needed maybe
			throw std::runtime_error("Invalid allowed_methods Directive");

		for (size_t i = 0; i < amountArgs; i++) {
			if (valueArgs[i] != "GET" && valueArgs[i] != "POST" && valueArgs[i] != "DELETE")
				throw std::runtime_error("Invalid allowed_methods Directive");
		}
		this->_allowed_methods = valueArgs;
	}
	else if (directiveKey == "cgi_path") {
		for (size_t i = 0; i < amountArgs; i++)
			this->_cgi_path.push_back(valueArgs[i]);
	}
	else if (directiveKey == "cgi_ext") {
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

/*		DEBUG		*/

void		LocationBlock::printLocationBlock() {
	std::cout << "Block Type: " << this->_blockType << std::endl;
	std::cout << "Root: " << this->_root << std::endl;
	std::cout << "Error Page: ";
	for (size_t i = 0; i < this->_error_page.size(); i++)
		std::cout << this->_error_page[i] << " ";
	std::cout << std::endl;
	std::cout << "Return: ";
	for (size_t i = 0; i < this->_return.size(); i++)
		std::cout << this->_return[i] << " ";
	std::cout << std::endl;
	std::cout << "Try Files: ";
	for (size_t i = 0; i < this->_try_files.size(); i++)
		std::cout << this->_try_files[i] << " ";
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

	for (size_t i = 0; i < this->_nestedLocationVec.size(); i++) {
		std::cout << "Nested Location Block: " << i << std::endl;
		this->_nestedLocationVec[i].printLocationBlock();
	}

}