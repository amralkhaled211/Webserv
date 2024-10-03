#include "LocationBlock.hpp"

LocationBlock::LocationBlock() : Block() { }

LocationBlock::LocationBlock(std::string& type) : Block(type) { }


LocationBlock::LocationBlock(const LocationBlock& other) : Block(other) { *this = other; }

LocationBlock&	LocationBlock::operator=(const LocationBlock& other) {
	if (this == &other)
		return *this;

	this->_locationVec = other._locationVec;
	this->_allowed_methods = other._allowed_methods;

	return *this;
}

LocationBlock::~LocationBlock() { }
