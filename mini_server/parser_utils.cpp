#include "RequestHandler.hpp"

std::string get_file_extension(const std::string& file_path)
{
	std::string extension = file_path.substr(file_path.find_last_of(".") + 1);
	return extension;
}

std::string get_file_name(const std::string& file_path)
{
	std::string file_name = file_path.substr(file_path.find_last_of("/") + 1);
	return file_name;
}

std::string deleteSpaces(std::string const &str)
{
	size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first)
        return str;

    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

std::string intToString(int value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

int stringToInt(const std::string& str) // issue: value gets not initialized, if str is an empty string
{
	std::stringstream ss(str);
	int value;
	// std::cout << BOLD_RED << "str to int: " << str.size() << RESET << std::endl;
	ss >> value;
	return value;
}


/* size_t stringToSizeT(const std::string& str)
{
	std::stringstream ss(str);
	size_t value;
	ss >> value;
	return value;
} */


size_t stringToSizeT(const std::string& str)
{
	std::stringstream ss(str);
	size_t value;
	ss >> value;
	return value;
}


bool isDirectory(const std::string& path)
{
	struct stat path_stat;
	if (stat(path.c_str(), &path_stat) == -1)
	{
		// std::cerr << BOLD_RED << "stat: " << strerror(errno) << RESET << std::endl;
		return false;
	}
	return S_ISDIR(path_stat.st_mode);
}

std::vector<std::string> split(const std::string& str, char delimiter)
{
	// std::cout << BOLD_RED << "in split: " << str << RESET << std::endl;
	std::vector<std::string> tokens;
	std::string token;
	std::stringstream ss(str);
	while (std::getline(ss, token, delimiter))
	{
		if (!token.empty())
	    	tokens.push_back("/" + token);
	}
	if (tokens.empty())
	    	tokens.push_back("/");
	return tokens;
}


///debugging function
void print_map(const std::map<std::string, std::string>& m)
{
    for (std::map<std::string, std::string>::const_iterator it = m.begin(); it != m.end(); ++it)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}

void	printServerVec(std::vector<ServerBlock>& _serverVec) {
	std::cout << "Server Vector Size: " << _serverVec.size() << std::endl;
	for (size_t i = 0; i < _serverVec.size(); i++) {
		std::cout << "Server " << i << std::endl;
		std::cout << "Adrress: " << &_serverVec[i] << std::endl;
		_serverVec[i].printServerBlock();
		std::cout << std::endl;
	}
}