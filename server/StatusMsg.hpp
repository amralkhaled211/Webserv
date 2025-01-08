#pragma once

#include <vector>
#include <string>
#include <map>

class StatusMsg
{
	private:

		// better names later
		void	_oneHunderds();
		void	_twoHunderds();
		void	_threeHunderds();
		void	_fourHunderds();
		void	_fiveHunderds();

	public:
		std::map<int, std::vector<std::string> >	_statusMsg;
		StatusMsg();
		~StatusMsg();
};
