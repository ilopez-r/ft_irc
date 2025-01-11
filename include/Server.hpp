/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 15:00:10 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/11 21:46:05 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h> //Para close
#include <arpa/inet.h> //Para los sockets
#include <cstdlib> //Para atoi
#include <map>
#include <vector>
#include <sstream>
#include <poll.h>
#include <fcntl.h>
#include "Client.hpp"
#include "Channel.hpp"

class Client;

class Channel;

class Server
{
	public:
			Server(int port, const std::string &password);
			~Server();

			void run();
			void commandQUIT(Client *client, const std::string &param);
			void commandHELP(Client *client, const std::string &cmd, const std::string &other);
			void commandPASS(Client *client, const std::string &pass,  const std::string &other);
			void commandUSER(Client *client, const std::string &username, const std::string &other);
			void commandNICK(Client *client, const std::string &nickname, const std::string &other);
			void commandPROFILE(Client *client, const std::string &param);
			void commandCHANNELS(Client *client, const std::string &param, const std::string &other);
			void commandMSG(Client *sender, const std::string &receiver, const std::string &message);
			void commandJOIN(Client *client, const std::string &channelName, const std::string &key, const std::string &other);
			void commandLEAVE(Client *client, const std::string &channelName, const std::string &other);
			void commandKICK(Client *sender, const std::string &channelName, const std::string &user, const std::string &reason);
			void commandBAN(Client *sender, const std::string &channelName, const std::string &user, const std::string &reason);
			void commandUNBAN(Client *sender, const std::string &channelName, const std::string &user, const std::string &other);
			void commandINVITE(Client *sender, const std::string &channelName, const std::string &user, const std::string &other);
			void commandUNINVITE(Client *sender, const std::string &channelName, const std::string &user, const std::string &other);
			void commandTOPIC(Client *sender, const std::string &channelName, const std::string &topic);
			void commandKEY(Client *sender, const std::string &channelName, const std::string &other);
			void commandMODE(Client *sender, const std::string &channelName, const std::string &mode, const std::string &param);
			void commandREMOVE(Client *sender, const std::string &channelName, const std::string &user, const std::string &other);
	private:
			int port;
			int serverSocket;
			std::vector<pollfd> pollFds;
			std::string _design;
			std::string _password;
			std::map<int, Client*> clients;
			std::map<std::string, Channel> channels;

			void initializeServer();
			void acceptNewClient();
			void handleClientMessage(int clientFd);
			std::string to_string (int number) const;
};

#endif

