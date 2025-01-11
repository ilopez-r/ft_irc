/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 14:58:08 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/11 21:32:46 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>\n";
		return (1);
	}
	int port = std::atoi(argv[1]);
	std::string password = argv[2];
	try
	{
		Server server(port, password);
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return (1);
	}
	return (0);
}
