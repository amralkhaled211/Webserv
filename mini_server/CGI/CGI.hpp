/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aszabo <aszabo@student.42vienna.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 12:05:59 by aszabo            #+#    #+#             */
/*   Updated: 2024/10/03 15:33:36 by aszabo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Server.hpp"
#include "../RequestHandler.hpp"

class CGI
{
	private:
		
		std::string _scriptPath;
		std::map<std::string, std::string> _env;
		parser _request;
		Response		_response;

	public:

		CGI();
		CGI(const std::string &scriptPath, const parser &request);
		~CGI();

		Response getResponse() const;

		void setEnv();
		void executeScript();
		void generateResponse();
};