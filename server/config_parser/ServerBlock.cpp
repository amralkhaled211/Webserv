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


static bool	isInvalidServerName(std::string server_name) {
	// Domain
	(void)server_name;
	if (server_name.find_first_of("$@!*") != std::string::npos) // invalid characters
		return true;
	if (server_name.find("..") != std::string::npos) // invalid format
		return true;
	// IP - no checks by nginx
	return false;
}

void			ServerBlock::setDirective(const std::string& directiveKey, std::string& directiveValue) {

	if (_addCommonDirective(directiveKey, directiveValue))
		return;

	std::vector<std::string>	valueArgs(splitString(directiveValue));
	size_t						amountArgs(valueArgs.size());

	if (directiveKey == "listen") {
		if (this->_listen.size() > 0)
			throw std::runtime_error("Duplicate listen Directive");

		std::istringstream	ss(directiveValue);
		while (!ss.eof()) {
			int	port;
			ss >> port;
			if (ss.fail() || (port < 1024 || port > 65535)) // below 1024 requires privilages
				throw std::runtime_error("Invalid listen Directive");
			this->_listen.push_back(port);
		}
		if (ss.fail())
			throw std::runtime_error("Invalid listen Directive");
	}
	else if (directiveKey == "server_name") {
		if (!_server_name.empty())
			throw std::runtime_error("Duplicate server_name Directive");
		for (size_t i = 0; i < amountArgs; i++) {
			if (isInvalidServerName(valueArgs[i]))
				throw std::runtime_error("Invalid server_name Directive");
			this->_server_name.push_back(valueArgs[i]);
		}
	}
	else
		throw std::runtime_error("Invalid Directive");
}


std::string						ServerBlock::getHostFromMap(int socketFD) { return _socketFD_host[socketFD]; }

void							ServerBlock::setHostInMap(int socketFD, std::string hostPort) { _socketFD_host[socketFD] = hostPort; }

std::vector<LocationBlock>&		ServerBlock::getLocationVec() { return this->_locationVec; }

LocationBlock&					ServerBlock::getLocationVecBack() {
	if (this->_locationVec.empty())
		throw std::runtime_error("No Location Blocks");
	return this->_locationVec.back();
}

std::vector<int>&				ServerBlock::getListen() { return this->_listen; }

std::vector<std::string>&		ServerBlock::getServerName() { return this->_server_name; }

std::vector<std::string>&		ServerBlock::getHostPort() { return this->_hostPort; }

void				ServerBlock::addLocationBlock() { this->_locationVec.push_back(LocationBlock()); }


void				ServerBlock::createNamePortComb() {

	std::vector<std::string>		hosts = this->getServerName();
	std::vector<std::string>		ports;
	std::stringstream				ss;
	std::string						port;

	for (size_t i = 0; i < this->getListen().size(); ++i) {
		ss << this->getListen()[i];
		ss >> port;
		if (ss.fail())
			throw std::runtime_error("sstream fail"); // investigate this further
		ports.push_back(port);
		ss.clear();
		ss.str("");
	}

	for (size_t i = 0; i < hosts.size(); ++i) {
		for (size_t j = i; j < ports.size(); ++j)
			this->_hostPort.push_back(hosts[i] + ":" + ports[j]);
	}
}

void				ServerBlock::setupDefaults() {
	if (this->_listen.empty())
		this->_listen.push_back(8000);

	if (this->_server_name.empty())
		this->_server_name.push_back("localhost");

	this->createNamePortComb();

	if (this->_root.empty())
		this->_root = "../www/";

	if (this->_index.empty())
		this->_index.push_back("index.html"); // note: only matters if a directory was requested

	if (this->_autoindex == NOT_SET)
		this->_autoindex = OFF;
	
	if (this->_return.size() == 1)
		this->_return.insert(_return.begin(), "302");

	if (this->_client_max_body_size.empty())
		this->_client_max_body_size = "1m";

	for (size_t i = 0; i < this->_locationVec.size(); ++i)
		this->_locationVec[i].setupDefaults(static_cast<Block&>(*this));
}

std::string			getExtentions(std::string fileName) {
	size_t point = fileName.find_last_of('.');
	if (point == std::string::npos)
		return (".");
	return fileName.substr(point);
}

void				ServerBlock::contextError() { // checkDupLocation()

	std::string		tmpPrefix;

	if (_locationVec.size() < 1)
		return ;

	for (size_t i = 0; i < _locationVec.size(); ++i) {
		tmpPrefix = _locationVec[i].getPrefix();
		for (size_t j = i + 1; j < _locationVec.size(); ++j) {
			if (tmpPrefix == _locationVec[j].getPrefix())
				throw std::runtime_error("Duplicate location Defintion");
		}
		if (!_locationVec[i].getCgiExt().empty()) { // is cgi
			
			std::vector<std::string> cgiExt = _locationVec[i].getCgiExt();
			std::vector<std::string> cgiIndex = _locationVec[i].getIndex();

			for (size_t k = 0; k < cgiIndex.size(); ++k) {
				std::string	currCgiIndexExt = getExtentions(cgiIndex[k]);
				bool		isCorrectExt = false;

				for (size_t l = 0; l < cgiExt.size(); ++l) {
					if (currCgiIndexExt == cgiExt[l])
						isCorrectExt = true;
				}
				
				if (!isCorrectExt)
				{
					throw std::runtime_error("Index File Extension doesn't match CGI Extension");
				}
			}
		}
	}

	// more checks to follow
}


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
	std::cout << std::endl;

	std::cout << "Location Vector Size: " << this->_locationVec.size() << std::endl;
	for (size_t i = 0; i < this->_locationVec.size(); i++) {
		std::cout << "Location " << i << std::endl;
		this->_locationVec[i].printLocationBlock();
	}
}
