#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iostream>

#define OFF					0
#define ON					1
#define NOT_SET				2

#define	RETURN				1
#define	ERROR_PAGE			2

class Block // parent // the purpose of this is just to not have class with the same attibutes and methods, we won't do any Polymorphism, just Inheritence
{
	protected:
		std::string								_blockType; // Server or Location

		// all the directives, that are in both Server and Location
		std::string								_root;
		std::vector<std::vector<std::string> >	_error_page; // a block can have more then one error_page
		std::vector<std::string>				_return;
		std::vector<std::string>				_index;
		char									_autoindex;
		std::string								_client_max_body_size;

	public:
		Block();
		Block(std::string type);
		Block( const Block& other );
		Block& operator=( const Block& other );
		virtual ~Block();

		void		setRoot(std::string& root);
		void		setErrorPage(std::vector<std::vector<std::string> >& error_page);
		void		setReturn(std::vector<std::string>& ret);
		void		setIndex(std::vector<std::string>& index);
		void		setAutoindex(bool autoindex);

		const std::string&								getBlockType() const;
		const std::string&								getRoot() const;
		const std::vector<std::vector<std::string> >&	getErrorPage() const;
		const std::vector<std::string>&					getReturn() const;
		const std::vector<std::string>&					getTryFiles() const;
		const std::vector<std::string>&					getIndex() const;
		const char&										getAutoindex() const;

		bool				_addCommonDirective(const std::string& directiveKey, std::string& directiveValue);

		virtual void		setDirective(const std::string& directiveKey, std::string& directiveValue) = 0;

};

// will put these in a diff file
std::vector<std::string>	splitString(const std::string& input);
void						removeExcessSlashes(std::string& path);
