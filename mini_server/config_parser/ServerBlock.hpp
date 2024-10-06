#pragma once

#include "Block.hpp"
#include "LocationBlock.hpp"

class ServerBlock : public Block
{
	private:
		std::vector<LocationBlock>		_locationVec;
		unsigned int					_listen;
		std::vector<std::string>		_server_name;

	public:
		ServerBlock();
		ServerBlock(const ServerBlock& other);
		ServerBlock& operator=(const ServerBlock& other);
		~ServerBlock();

		std::vector<LocationBlock>&		getLocationVec();
		LocationBlock&					getLocationVecBack();

		void		setDirective(const std::string& directiveKey, std::string& directiveValue);
		void		addLocationBlock();

		/*		DEBUG		*/
		void	printServerBlock();
};
