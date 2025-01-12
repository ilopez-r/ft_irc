/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 15:00:10 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/13 00:48:02 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream> //Para std::cout
#include <cstring> //Para memset
#include <unistd.h> //Para close
#include <arpa/inet.h> //Para los sockets
#include <map> //Para el contenedor map
#include <vector> //Para el contenedor vector
#include <poll.h> //Para pollfd
#include <fcntl.h> //Para fcntl
#include "Client.hpp"
#include "Channel.hpp"
#include "Commands.hpp"

class Client;

class Channel;

class Commands;

class Server
{
	public:
			//*------------------Constructors and destructors------------------*//
			Server(int port, const std::string &password);
			~Server();

			//*------------------Getters------------------*//
			const std::string getPassword() const;
			std::map<int, Client*>& getClients();
			std::map<std::string, Channel>& getChannels();
			std::vector<struct pollfd>& getPollFds();
	private:
			//*------------------Server Utils------------------*//
			int _port;
			int _serverSocket;
			std::vector<pollfd> pollFds;
			std::string _design;
			std::string _password;
			std::map<int, Client*> clients;
			std::map<std::string, Channel> channels;
			Commands* _commands;

			//*------------------Server Functions------------------*//
			void initializeServer();
			void run();
			void acceptNewClient(); //Aceptar a un cliente en el servidor
			void handleClientActions(int clientFd); //Procesar acciones del cliente
			void processClientLine(Client *client, const std::string &rawInput); // Procesar input del cliente
			std::string trim(const std::string &str); //Funcion para quitar espacios
};

#endif

