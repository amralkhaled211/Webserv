#pragma once

#include "Block.hpp"

class LocationBlock : public Block
{
	private:
		std::vector<LocationBlock>		_nestedLocationVec;
		std::string						_prefix; // definition of location
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
		std::string&					getPrefix();

		void		addLocationBlock();
		void		setDirective(const std::string& directiveKey, std::string& directiveValue);
		void		setPrefix(const std::string& prefix);

		/*		DEBUG		*/
		void		printLocationBlock();
};
