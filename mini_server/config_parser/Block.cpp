#include "Block.hpp"

Block::Block() { }

Block::Block(std::string type) : _blockType(type) {
	this->_root = "";
	this->_error_page = std::vector<std::string>();
	this->_return = std::vector<std::string>();
	this->_try_files = std::vector<std::string>();
	this->_index = std::vector<std::string>();
	this->_autoindex = false;
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

const bool	Block::getAutoindex() const { return this->_autoindex; }


/*			PURE VIRTUAL METHODS			*/

void	Block::addLocationBlock() { /* this must be implemented by Server and Location Block */ }

void	Block::setDirective(const std::string& directiveKey, std::string& directiveValue) { /* this must be implemented by Server and Location Block */ }

