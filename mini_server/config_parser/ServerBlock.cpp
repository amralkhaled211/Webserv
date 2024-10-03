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


void		ServerBlock::setDirective(const std::string& directiveKey, std::string& directiveValue) {

	if (directiveKey == "listen") {
		if (this->_listen != 0)
			throw std::runtime_error("Duplicate listen Directive");

		if (directiveValue.find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("Invalid listen Directive");

		std::stringstream	ss(directiveValue);
		ss >> this->_listen;
		if (ss.fail())
			throw std::runtime_error("Invalid listen Directive");
	}
	else if (directiveKey == "server_name")
		this->_server_name.push_back(directiveValue);
	else if (directiveKey == "root")
		;// will come to it
	else if (directiveKey == "error_page")
		;// will come to it
	else if (directiveKey == "return")
		;// will come to it
	else if (directiveKey == "try_files")
		;// will come to it
	else if (directiveKey == "index")
		;// will come to it
	else if (directiveKey == "autoindex") {
		if (directiveValue == "on")
			this->setAutoindex(true);
		else if (directiveValue == "off")
			this->setAutoindex(false);
		else
			throw std::runtime_error("Invalid autoindex Directive");
	}
	else
		throw std::runtime_error("Invalid Directive");
}
