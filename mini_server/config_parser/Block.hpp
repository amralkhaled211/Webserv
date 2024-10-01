#pragma once

#include "Parser.hpp"

class Block // parent // the purpose of this is just to not have class with the same attibutes and methods, we won't do any Polymorphism, just Inheritence
{
	protected:
		std::string							_blockType; // Server or Location

		// all the directives, that are in both Server and Location
		// root, error_page, index, autoindex, return, error_page
		// std::map<std::string, std::string>	directives; // I think we need the directives individually

	public:
	Block();
	Block( const Block& other );
	Block& operator=( const Block& other );
	virtual ~Block();


};