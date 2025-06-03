/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 11:31:09 by karim             #+#    #+#             */
/*   Updated: 2025/05/12 15:27:14 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <sstream>
#include "Request.hpp"
#include "GlobalConfig.hpp"
#include "ServerConfig.hpp"

class ResponseHandler {
	private:
		int			socket_fd;
		std::string file_path;
		LocationConfig const *loc_config;

	public:
		ResponseHandler(int sockfd);
		void ProccessRequest(Request &req, ServerConfig &conf);
		void RouteResolver(const std::string &path, ServerConfig &conf);
		~ResponseHandler();

		class RequestError : std::exception
		{
			private:
				std::string error;
			public:
				RequestError(const std::string &Errmsg);
				const char *what() noexcept;
		};
};

#endif