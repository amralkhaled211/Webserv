#pragma once

#include "Block.hpp"

// colors for debugging
#define RED							"\033[1;31m"
#define GREEN						"\033[1;32m"
#define YELLOW						"\033[1;33m"
#define BOLD_RED					"\033[1;91m"
#define BOLD_GREEN					"\033[1;92m"
#define BOLD_YELLOW					"\033[1;93m"
#define RESET						"\033[0m"


class LocationBlock : public Block
{
	private:
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
		std::vector<std::string>&		getAllowedMethods();
		std::vector<std::string>&		getCgiPath();
		std::vector<std::string>&		getCgiExt();


		void		setDirective(const std::string& directiveKey, std::string& directiveValue);
		void		setPrefix(const std::string& prefix);

		void		setupDefaults(Block& parentServer);

		/*		DEBUG		*/
		void		printLocationBlock();
};
