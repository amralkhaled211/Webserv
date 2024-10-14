#include "ServerBlock.hpp"


ServerBlock::ServerBlock() : Block("serverBlock") {
	this->_listen = std::vector<int>();
	this->_server_name = std::vector<std::string>();
	this->_locationVec = std::vector<LocationBlock>();
}

ServerBlock::ServerBlock(const ServerBlock& other) : Block(other) { *this = other; }

ServerBlock&	ServerBlock::operator=(const ServerBlock& other) {
	if (this == &other)
		return *this;

	this->_listen = other._listen;
	this->_server_name = other._server_name;
	this->_locationVec = other._locationVec;

	return *this;
}

ServerBlock::~ServerBlock() { }


// will rewrite this one later to be more compact
void		ServerBlock::setDirective(const std::string& directiveKey, std::string& directiveValue) {

	if (_addCommonDirective(directiveKey, directiveValue))
		return;

	std::vector<std::string>	valueArgs(splitString(directiveValue));
	size_t						amountArgs(valueArgs.size());

	if (directiveKey == "listen") {
		if (this->_listen.size() > 0)
			throw std::runtime_error("Duplicate listen Directive");

		if (amountArgs != 1 || directiveValue.find_first_not_of("0123456789") != std::string::npos) // to add: check for port range
			throw std::runtime_error("Invalid listen Directive");

		std::istringstream	ss(directiveValue);
		while (!ss.eof()) {
			int		port;
			ss >> port;
			if (ss.fail())
				throw std::runtime_error("Invalid listen Directive");
			this->_listen.push_back(port);
		}
		if (ss.fail())
			throw std::runtime_error("Invalid listen Directive");
	}
	else if (directiveKey == "server_name") {
		if (!_server_name.empty())
			throw std::runtime_error("Duplicate server_name Directive");
		if (amountArgs != 1)
			throw std::runtime_error("Invalid root Directive");

		this->_server_name.push_back(directiveValue);
	}
	else
		throw std::runtime_error("Invalid Directive");
}


std::vector<LocationBlock>&		ServerBlock::getLocationVec() { return this->_locationVec; }

LocationBlock&					ServerBlock::getLocationVecBack() {
	if (this->_locationVec.empty())
		throw std::runtime_error("No Location Blocks");
	return this->_locationVec.back();
}

std::vector<int>&				ServerBlock::getListen() { return this->_listen; }

std::vector<std::string>&		ServerBlock::getServerName() { return this->_server_name; }



void							ServerBlock::addLocationBlock() { this->_locationVec.push_back(LocationBlock()); }


/*			DEBUG			*/

void	ServerBlock::printServerBlock() {
	for (size_t i = 0; i < this->_listen.size(); i++)
		std::cout << "Listen[" << i << "]: " << this->_listen[i] << std::endl;
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