#pragma once

// #include "Parser.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iostream>

#define OFF						0
#define ON						1
#define NOT_SET					2

class Block // parent // the purpose of this is just to not have class with the same attibutes and methods, we won't do any Polymorphism, just Inheritence
{
	protected:
		std::string							_blockType; // Server or Location

		// all the directives, that are in both Server and Location
		std::string					_root;
		std::vector<std::string>	_error_page; // a block can have more then one error_page
		std::vector<std::string>	_return;
		std::vector<std::string>	_try_files; // won't handle as of now
		std::vector<std::string>	_index;
		char						_autoindex;
		std::string					_client_max_body_size;

		// std::map<std::string, std::string>	directives; // I think we need the directives individually


	public:
		Block();
		Block(std::string type);
		Block( const Block& other );
		Block& operator=( const Block& other );
		virtual ~Block();

		void		setRoot(std::string& root);
		void		setErrorPage(std::vector<std::string>& error_page);
		void		setReturn(std::vector<std::string>& ret);
		void		setTryFiles(std::vector<std::string>& try_files);
		void		setIndex(std::vector<std::string>& index);
		void		setAutoindex(bool autoindex);

		const std::string&				getBlockType() const;
		const std::string&				getRoot() const;
		const std::vector<std::string>&	getErrorPage() const;
		const std::vector<std::string>&	getReturn() const;
		const std::vector<std::string>&	getTryFiles() const;
		const std::vector<std::string>&	getIndex() const;
		const char&						getAutoindex() const;

		bool				_addCommonDirective(const std::string& directiveKey, std::string& directiveValue);

		virtual void		addLocationBlock() = 0;
		virtual void		setDirective(const std::string& directiveKey, std::string& directiveValue) = 0;

};

std::vector<std::string>	splitString(const std::string& input); // will put this in a diff file
