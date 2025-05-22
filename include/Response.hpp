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

#include <string>
#include <iostream>

class Response {
	private:
		int			socket_fd;
		std::string responseHolder;
	public:
		Response(void);
		Response(int fd);
		int			getFD();
		void		setResponse(std::string responseData);
		std::string	getResponse(void);
};

#endif