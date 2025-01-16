/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 18:12:08 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/16 19:21:21 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../include/Server.hpp"

bool isValidPort(const char *str)
{
	for (int i = 0; str[i] != '\0'; i++) // Bucle para recorrer todos los caracteres de la cadena
		if (!std::isdigit(str[i]))  // Verificar si hay un caracter que no es dígito
			return (false);
	if (std::atoi(str) < 1024 || std::atoi(str) > 65535)// Verificar si el puerto está fuera del rango permitido
		return (false);
	return (true);
}

int g_signal = 0;

void signalHandler(int signum) { g_signal = signum; }

int main(int argc, char **argv)
{
	try
	{
		if (argc != 3)
			throw(std::runtime_error("You must use ./ircserv <port> <password>"));
		if (!isValidPort(argv[1]))
			throw(std::runtime_error("Port must be a number between 1024 and 65535"));
		if (std::strlen(argv[2]) > 10)
			throw(std::runtime_error("Password cannot be longer than 10 characters"));
		signal(SIGINT, signalHandler);
		signal(SIGQUIT, signalHandler);
		Server server(std::atoi(argv[1]), argv[2]);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return (1);
	}
	return (0);
}
