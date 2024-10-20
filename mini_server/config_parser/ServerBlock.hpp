#pragma once

#include "Block.hpp"
#include "LocationBlock.hpp"

class ServerBlock : public Block
{
	private:
		std::vector<LocationBlock>		_locationVec;
		std::vector<int>				_listen;
		std::vector<std::string>		_server_name;

	public:
		ServerBlock();
		ServerBlock(const ServerBlock& other);
		ServerBlock& operator=(const ServerBlock& other);
		~ServerBlock();

		std::vector<LocationBlock>&		getLocationVec();
		LocationBlock&					getLocationVecBack();
		std::vector<int>&				getListen();
		std::vector<std::string>&		getServerName();

		void		setDirective(const std::string& directiveKey, std::string& directiveValue);
		void		addLocationBlock();

		void		setupDefaults();
		void		contextError();


		/*		DEBUG		*/
		void	printServerBlock();
};
