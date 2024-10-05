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