/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 11:31:09 by karim             #+#    #+#             */
/*   Updated: 2025/06/30 16:20:17 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <iostream>

class Response {
	private:
		int			socketFD;
		std::string responseHolder;
	public:
					Response(void);
					Response(int fd);

		int			getFD();
		std::string	getResponse(void);

		void		setResponse(std::string responseData);
};

#endif