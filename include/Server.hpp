/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 15:00:10 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/16 19:21:31 by ilopez-r         ###   ########.fr       */
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
#include <sstream> //Para funcion to_string
#include <cstdlib> //Para atoi
#include <ctime>    // Para time()
#include <csignal> //Para las señales
#include "Client.hpp"
#include "Channel.hpp"

extern int g_signal;

class Client;

class Channel;

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
			void getHandleBotCommand(Client &sender, const std::string &command);
	private:
			//*------------------Server Utils------------------*//
			int _port;
			int _serverSocket;
			std::vector<pollfd> pollFds;
			std::string _design;
			std::string _password;
			std::map<int, Client*> clients;
			std::map<std::string, Channel> channels;

			//*------------------Server Functions------------------*//
			void initializeServer();
			void run();
			void acceptNewClient(); //Aceptar a un cliente en el servidor
			void handleClientActions(int clientFd); //Procesar acciones del cliente
			void processClientLine(Client *client, const std::string &rawInput); // Procesar input del cliente
			std::string trim(const std::string &str); //Funcion para quitar espacios
			void handleCommand(Client &client, Server &server, const std::string &cmd, const std::string &param, const std::string &paramraw2, const std::string &param2, const std::string &param3);
			void handleBotCommand(Client &sender, const std::string &command);
};

#endif

