#include "ServerBlock.hpp"


ServerBlock::ServerBlock() : Block("serverBlock") {
	this->_listen = 0;
	this->_server_name = std::vector<std::string>();
}

ServerBlock::ServerBlock(const ServerBlock& other) : Block(other) { *this = other; }

ServerBlock&	ServerBlock::operator=(const ServerBlock& other) {
	if (this == &other)
		return *this;

	this->_listen = other._listen;
	this->_server_name = other._server_name;

	return *this;
}

ServerBlock::~ServerBlock() { }


std::vector<std::string>	splitString(const std::string& input) { // utils  --> c++ version of ft_split
	std::istringstream			iss(input);
	std::vector<std::string>	splitedInput;
	std::string					word;

	while (iss >> word) {
		splitedInput.push_back(word);
	}
	return splitedInput;
}

// will rewrite this one later to be more compact
void		ServerBlock::setDirective(const std::string& directiveKey, std::string& directiveValue) {

	std::vector<std::string>	valueArgs(splitString(directiveValue));
	size_t						amountArgs(valueArgs.size());

	if (directiveKey == "listen") {
		if (this->_listen != 0)
			throw std::runtime_error("Duplicate listen Directive");

		if (amountArgs != 1)
			throw std::runtime_error("Invalid listen Directive");

		if (directiveValue.find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("Invalid listen Directive");

		std::istringstream	ss(directiveValue);
		ss >> this->_listen;
		if (ss.fail())
			throw std::runtime_error("Invalid listen Directive");
	}
	else if (directiveKey == "server_name") {
		if (amountArgs != 1)
			throw std::runtime_error("Invalid root Directive");

		this->_server_name.push_back(directiveValue);
	}
	else if (directiveKey == "root") {
		if (amountArgs != 1)
			throw std::runtime_error("Invalid root Directive");

		this->_root = directiveValue;
	}
	else if (directiveKey == "error_page") {
		this->_error_page = valueArgs;
	}
	else if (directiveKey == "return") {
		this->_return = valueArgs;
	}
	else if (directiveKey == "try_files") {
		this->_try_files = valueArgs;
	}
	else if (directiveKey == "index") {
		this->_index = valueArgs;
	}
	else if (directiveKey == "autoindex") {
		if (directiveValue == "on")
			this->_autoindex = true;
		else if (directiveValue == "off")
			this->_autoindex = false;
		else
			throw std::runtime_error("Invalid autoindex Directive");
	}
	else
		throw std::runtime_error("Invalid Directive");
}


std::vector<LocationBlock>&		ServerBlock::getLocationVec() { return this->_locationVec; }

void							ServerBlock::addLocationBlock() { this->_locationVec.push_back(LocationBlock()); }

int								ServerBlock::getListen(){ return this->_listen;}
std::vector<std::string>		ServerBlock::getName(){ return this->_server_name;}


/*			DEBUG			*/

void	ServerBlock::printServerBlock() {
	std::cout << "Listen: " << this->_listen << std::endl;
	std::cout << "Server Name: ";
	for (size_t i = 0; i < this->_server_name.size(); i++)
		std::cout << this->_server_name[i] << " ";
	std::cout << std::endl;
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
	std::cout << std::endl;

	std::cout << "Location Vector Size: " << this->_locationVec.size() << std::endl;
	for (size_t i = 0; i < this->_locationVec.size(); i++) {
		std::cout << "Location " << i << std::endl;
		this->_locationVec[i].printLocationBlock();
	}
}