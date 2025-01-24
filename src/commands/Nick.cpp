/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:14 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/24 15:59:00 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandNICK(Client &client, Server &server, const std::string &nickname, const std::string &other)
{
	if (nickname.empty())// Validar nickname no esta vacio
		return(client.messageToMyself(":ircserver 431 ERROR: No nickname provided. Use: NICK <nickname>\r\n"));
	if (!other.empty()) // Verificar ningun otra palabara detras del nickname
		return(client.messageToMyself(":ircserver 461 ERROR: Command 'NICK' does not accept any more parameters than the NICKNAME. Use: NICK <nickname>\n"));
	for (int i = 0; nickname[i]; i++)
	{
		if (nickname[i] == '#' || nickname[i] == '@' || nickname[i] == '!' || nickname[i] == '?')
		{
			if (!client.getNickname().empty())
				return(client.messageToMyself(":ircserver 999 " + client.getNickname() + " ERROR: Nickname cannot include especial characters like: #, @, !, ?\r\n"));
			return(client.messageToMyself(":ircserver 999 ERROR: Nickname cannot include especial characters like: #, @, !, ?\r\n"));
		}
	}
	if (nickname.length() > 9)// Verificar que no sea mas largo de 9 caracteres
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 436 " + client.getNickname() + " ERROR: Nickname cannot be longer than 9 characters\r\n"));
		return(client.messageToMyself(":ircserver 436 ERROR: Nickname cannot be longer than 9 characters\r\n"));
	}
	if (nickname == "bot" || nickname == "BOT")
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 436 " + client.getNickname() + " ERROR: Nickname cannot be '" + nickname + "'\r\n"));
		return(client.messageToMyself(":ircserver 436 ERROR: Nickname cannot be '" + nickname + "'r\n"));
	}
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		Client *clients = it->second;
		if (clients && clients->getNickname() == nickname)// Verificar si el nickname ya está en uso
		{
			client.messageToMyself(":ircserver 433 " + nickname + " ERROR: Nickname '" + nickname + "' is already in use\r\n");
			return(client.messageToMyself(":ircserver 001\r\n"));
		}
	}
	std::string oldNickname = client.getNickname(); //Guardar el nickname anterior
	client.setNickname(nickname);// Asignar el nickname al cliente.
	if (oldNickname.empty()) //Si es el primer nickname que se pone
	{
		client.messageToMyself(":" + oldNickname + " NICK :" + nickname + "\r\n");
		std::cout << "Client (" << client.getFd() << ") setted his nickname to '" << client.getNickname() << "'\n";
		return(client.messageToMyself(":ircserver 999 " + client.getNickname() + " ~ You setted your nickname to '" + nickname + "' ~\r\n"));
	}
	client.messageToMyself(":" + oldNickname + " NICK :" + nickname + "\r\n");
	client.messageToMyself(":ircserver 999 " + client.getNickname() + " ~ You changed your nickname to '" + nickname + "' ~\r\n");
	std::cout << "Client (" << client.getFd() << ") '" << oldNickname << "' changed his nickname to '" << client.getNickname() << "'\n";
	std::map<std::string, Channel>& channels = server.getChannels();
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end();it++)
	{
		Channel &channel = it->second;
		std::string channelName = it->first;
		if (channel.hasClient(&client))
		{
			channel.messageToGroupNoSender(":" + oldNickname + " NICK :" + nickname + "\r\n", &client);
			channel.messageToGroupNoSender("'" + oldNickname + "' changed his nickname to '" + nickname + "'\r\n", &client);
		}
	}
}
