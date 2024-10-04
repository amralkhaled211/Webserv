#pragma once

// #include "Parser.hpp"
#include "Block.hpp"

class LocationBlock : public Block
{
	private:
		std::vector<LocationBlock>		_nestedLocationVec;
		std::vector<std::string>		_allowed_methods;
		// need to research about the cgi stuff, these are just from the example.conf
		std::vector<std::string>		_cgi_path;
		std::vector<std::string>		_cgi_ext;


	public:
		LocationBlock();
		LocationBlock(const LocationBlock& other);
		LocationBlock&	operator=(const LocationBlock& other);
		~LocationBlock();

		std::vector<LocationBlock>&		getNestedLocationVec();

		void		addLocationBlock();
		void		setDirective(const std::string& directiveKey, std::string& directiveValue);

		/*		DEBUG		*/
		void	printLocationBlock();
};
