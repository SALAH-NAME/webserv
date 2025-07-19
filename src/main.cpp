/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 10:24:40 by karim             #+#    #+#             */
/*   Updated: 2025/07/19 13:18:08 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"

void    printRequestAndResponse(std::string str, std::string request) {
	std::cout << "\n********************  " << str << "  *********************\n";
	for (size_t i = 0; i < request.size(); i++) {
		if (request[i] == '\r')
			std::cout << "\\r";
		else if (request[i] == '\n')
			std::cout << "\\n\n";
		else
			std::cout << request[i];
	}
	std::cout << "\n*****************************************\n\n";
}

int main(int argc, char** argv)
{
	std::string config_file = "conf/default.conf";
	if (argc > 1)
	{
		config_file = argv[1];
	}

	ConfigManager config_manager(config_file);
	if (!config_manager.load())
	{
		return 1;
	}

	ConfigPrinter printer(config_manager);
	printer.print();

    try {

		ServerManager	serverManager(config_manager.getServers());
		serverManager.waitingForEvents();

    }
    catch (const char* errorMssg) {
		perror(errorMssg);
		return 1;
	}
	return 0;
}
