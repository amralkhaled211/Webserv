#pragma once

#include "Parser.hpp"

class LocationBlock : public Block
{
	private:
		std::vector<LocationBlock>		_locationVec;
		// directives, that are inly in location
		// std::string						;

	public:
		LocationBlock();
		LocationBlock(const LocationBlock& other);
		LocationBlock&	operator=(const LocationBlock& other);
		~LocationBlock();
};
