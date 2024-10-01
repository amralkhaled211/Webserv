#pragma once

#include "Parser.hpp" // carefull with circular independency
#include "Block.hpp"
#include "LocationBlock.hpp"

class ServerBlock : public Block // gotta see what we put in Blocks (parent) and what in ServerBlock / LocationBlock (child)
{
	private:
		std::vector<LocationBlock>	_locationVec;
		// directives, that are only in server
		unsigned int					_listen;
		std::vector<std::string>		_serverName;
};