#pragma once

#include "Parser.hpp"

class LocationBlock : public Block
{
	private:
		std::vector<LocationBlock>		_locationVec;
		std::vector<std::string>		_allowed_methods;
		// more to follow, regarding cgi

		LocationBlock();

	public:
		LocationBlock(std::string& type);
		LocationBlock(const LocationBlock& other);
		LocationBlock&	operator=(const LocationBlock& other);
		~LocationBlock();
};
