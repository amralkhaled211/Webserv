#pragma once

// #include "Parser.hpp" // carefull with circular independency
#include "Block.hpp"
#include "LocationBlock.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>

class ServerBlock : public Block // gotta see what we put in Blocks (parent) and what in ServerBlock / LocationBlock (child)
{
	private:
		std::vector<LocationBlock>	_locationVec;
		// directives, that are only in server
		unsigned int					_listen;
		std::vector<std::string>		_server_name;


	public:
		ServerBlock();
		ServerBlock(const ServerBlock& other);
		ServerBlock& operator=(const ServerBlock& other);
		~ServerBlock();

		std::vector<LocationBlock>&		getLocationVec();

		void		setDirective(const std::string& directiveKey, std::string& directiveValue); // might need to be virtual
		void		addLocationBlock();

		/*		DEBUG		*/
		void	printServerBlock();
};

std::vector<std::string>	splitString(const std::string& input); // will put this in a diff file
