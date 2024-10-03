#pragma once

#include "Parser.hpp"

class Block // parent // the purpose of this is just to not have class with the same attibutes and methods, we won't do any Polymorphism, just Inheritence
{
	protected:
		std::string							_blockType; // Server or Location

		// all the directives, that are in both Server and Location
		// root, error_page, index, autoindex, return, error_page
		std::string					_root;
		std::vector<std::string>	_error_page;
		std::vector<std::string>	_return;
		std::vector<std::string>	_try_files;
		std::vector<std::string>	_index;
		bool						_autoindex;

		// std::map<std::string, std::string>	directives; // I think we need the directives individually

		Block();

	public:
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
		std::string&				getRoot() const;
		std::vector<std::string>&	getErrorPage() const;
		std::vector<std::string>&	getReturn() const;
		std::vector<std::string>&	getTryFiles() const;
		std::vector<std::string>&	getIndex() const;
		bool						getAutoindex() const;

};