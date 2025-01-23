/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:28:13 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:28:14 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Command.hpp"

void Server::handleCommand(Client &client, Server &server, const std::string &cmd, const std::string &param, const std::string &paramraw2, const std::string &param2, const std::string &param3)
{
	if (cmd == "CAP")
		return(client.messageToMyself("CAP * LS\r\n"));
	if (cmd == "QUIT")// Sintaxis: QUIT
		commandQUIT(client, server);
	else if (cmd == "VIVA")
		client.messageToMyself("~ ESPAÑA\n");
	else if (cmd == "COMMANDS")// Sintaxis: COMMANDS [cmd]
		commandCOMMANDS(client, param, param2);
	else if (cmd == "PASS")// Sintaxis: PASS <password>
		commandPASS(client, server, param, param2);
	else if (!client.getPasswordSent()) //Obligar a que el primer comando que se debe introducir es pass
		client.messageToMyself(":ircserver 999 " + client.getNickname() + " ERROR: You must authenticate with 'PASS' before any other command. Use: PASS <password>\r\n");
	else if (cmd == "USER")// Sintaxis: USER <username> 
		commandUSER(client, param, param2, paramraw2);
	else if (cmd == "NICK")// Sintaxis: NICK <nickname>
		commandNICK(client, server, param, param2);
	else if (client.getNickname().empty() || client.getUsername().empty())// Obligar a que haya un nickname para poder hacer el resto de comandos
		client.messageToMyself(":ircserver 444 " + client.getNickname() + " ERROR: First you have to use commands 'NICK' and 'USER' and specify your nickname and username. Use: NICK <nickname> and USER <username>\r\n");
	else if (cmd == "PROFILE")
		commandPROFILE(client, param);
	else if (cmd == "CHANNELS")// Sintaxis: CHANNELS [all]
		commandCHANNELS(client, server, param, param2);
	else if (cmd == "MSG" ||  cmd == "PRIVMSG")// Sintaxis: MSG <user/#channel> <message>
		commandPRIVMSG(client, server, param, paramraw2, param3);
	else if (cmd == "JOIN")// Sintaxis: JOIN <#channel> [key]. 
		commandJOIN(client, server, param, param2, param3);
	else if (cmd == "WHO")
		return;
	else if (cmd == "PART")// Sintaxis: PART <#channel>
		commandPART(client, server, param, param2);
	else if (cmd == "KICK")// Sintaxis: KICK <#channel> <user> <reason>
		commandKICK(client, server, param, param2, param3);
	else if (cmd == "INVITE")// Sintaxis: INVITE <#channel> <user> 
		commandINVITE(client, server, param, param2, param3);
	else if (cmd == "UNINVITE")// Sintaxis: UNINVITE <#channel> <user> 
		commandUNINVITE(client, server, param, param2, param3);
	else if (cmd == "TOPIC")// Sintaxis: TOPIC <#channel> [new topic]
		commandTOPIC(client, server, param, paramraw2);
	else if (cmd == "KEY")// Sintaxis: KEY <#channel>
		commandKEY(client, server, param, param2);
	else if (cmd == "MODE")// Sintaxis: MODE <#channel> [+|-mode] [arg]
		commandMODE(client, server, param, param2, param3);
	else if (cmd == "REMOVE")// Sintaxis: REMOVE <#channel> <topic/modes/invited/banned> 
		commandREMOVE(client, server, param, param2, param3);
	else
		client.messageToMyself(":ircserver 421 " + client.getNickname() + " ERROR: Unknown command: " + cmd + "\r\n");
}