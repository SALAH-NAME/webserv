/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: karim <karim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 10:24:40 by karim             #+#    #+#             */
/*   Updated: 2025/07/06 10:44:12 by karim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"

void    printRequet(std::string requet) {
	for (size_t i = 0; i < requet.size(); i++) {
		if (requet[i] == '\r')
			std::cout << "\\r";
		else if (requet[i] == '\n')
			std::cout << "\\n\n";
		else
			std::cout << requet[i];
	}
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
