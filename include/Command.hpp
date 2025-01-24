/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:26:17 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/24 11:55:33 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_H
# define COMMAND_H
# include "Server.hpp"

void commandQUIT(Client &client, Server &server);
void commandCOMMANDS(Client &client, const std::string &cmd, const std::string &other);
void commandPASS(Client &client, Server &server, const std::string &pass, const std::string &other);
void commandUSER(Client &client, const std::string &username, const std::string &other, const std::string &other2);
void commandNICK(Client &client, Server &server, const std::string &nickname, const std::string &other);
void commandPROFILE(Client &client, const std::string &param);
void commandCHANNELS(Client &client, Server &server, const std::string &param, const std::string &other);
void commandPRIVMSG(Client &sender, Server &server, const std::string &receiver, const std::string &message, const std::string &other);
void commandJOIN(Client &client, Server &server, const std::string &channelName, const std::string &key, const std::string &other);
void commandPART(Client &client, Server &server, const std::string &channelName, const std::string &other);
void commandKICK(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &reason);
void commandINVITE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other);
void commandUNINVITE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other);
void commandTOPIC(Client &sender, Server &server, const std::string &channelName, const std::string &topic);
void commandKEY(Client &sender, Server &server, const std::string &channelName, const std::string &other);
void commandMODE(Client &sender, Server &server, const std::string &channelName, const std::string &mode, const std::string &param);
void commandREMOVE(Client &sender, Server &server, const std::string &channelName, const std::string &param, const std::string &other);
std::string to_string (int number);

#endif