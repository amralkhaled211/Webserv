#pragma once

#include "Block.hpp"
#include "LocationBlock.hpp"
#include <map>

class ServerBlock : public Block
{
	private:
		std::vector<LocationBlock>		_locationVec;
		std::vector<int>				_listen;
		std::vector<std::string>		_server_name;
		// create a vector of string elements with all possible host:port combinations for easy comparison
		std::vector<std::string>		_hostPort; // could also be useful for matching
		std::map<int, std::string>		_socketFD_host; // save socketFDs with their _hostPort

	public:
		ServerBlock();
		ServerBlock(const ServerBlock& other);
		ServerBlock& operator=(const ServerBlock& other);
		~ServerBlock();

		std::vector<LocationBlock>&		getLocationVec();
		LocationBlock&					getLocationVecBack();
		std::vector<int>&				getListen();
		std::vector<std::string>&		getServerName();
		std::vector<std::string>&		getHostPort();
		std::string						getHostFromMap(int socketFD);

		void		setHostInMap(int socketFD, std::string hostPort);
		void		setDirective(const std::string& directiveKey, std::string& directiveValue);
		void		addLocationBlock();

		void		setupDefaults();
		void		contextError();

		void		createNamePortComb();

		/*		DEBUG		*/
		void	printServerBlock();
};
