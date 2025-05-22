/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 11:44:17 by karim             #+#    #+#             */
/*   Updated: 2025/05/13 09:30:15 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"


Response::Response(void) {}

Response::Response(int fd) : socket_fd(fd) {}

void    Response::setResponse(std::string responseData) {
    responseHolder += responseData;
}

std::string Response::getResponse(void) {
    return responseHolder;
}

int			Response::getFD() {
    return socket_fd;
}