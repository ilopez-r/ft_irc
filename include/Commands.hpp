/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 00:44:28 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/13 00:46:21 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <sstream> //Para funcion to_string
#include <cstdlib> //Para atoi
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Server;
class Client;

class Commands
{
	public:
			//*------------------Constructors and destructors------------------*//
			Commands();
    		~Commands();

			//*------------------Handle Commands Functions------------------*//
			void handleCommand(Client &client, Server &server, const std::string &cmd, const std::string &param, const std::string &paramraw2, const std::string &param2, const std::string &param3);
	private:
			//*------------------Commands Utils------------------*//
			std::string to_string (int number) const;

			//*------------------Commands Functions------------------*//
			void commandQUIT(Client &client, Server &server, const std::string &param);
			void commandHELP(Client &client, const std::string &cmd, const std::string &other);
			void commandPASS(Client &client, Server &server, const std::string &pass,  const std::string &other);
			void commandUSER(Client &client, const std::string &username, const std::string &other);
			void commandNICK(Client &client, Server &server, const std::string &nickname, const std::string &other);
			void commandPROFILE(Client &client, const std::string &param);
			void commandCHANNELS(Client &client, Server &server, const std::string &param, const std::string &other);
			void commandMSG(Client &sender, Server &server, const std::string &receiver, const std::string &message);
			void commandJOIN(Client &client, Server &server, const std::string &channelName, const std::string &key, const std::string &other);
			void commandLEAVE(Client &client, Server &server, const std::string &channelName, const std::string &other);
			void commandKICK(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &reason);
			void commandBAN(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &reason);
			void commandUNBAN(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other);
			void commandINVITE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other);
			void commandUNINVITE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other);
			void commandTOPIC(Client &sender, Server &server, const std::string &channelName, const std::string &topic);
			void commandKEY(Client &sender, Server &server, const std::string &channelName, const std::string &other);
			void commandMODE(Client &sender, Server &server, const std::string &channelName, const std::string &mode, const std::string &param);
			void commandREMOVE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other);
};

#endif
