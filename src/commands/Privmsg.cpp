/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:26 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/24 11:51:48 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandPRIVMSG(Client &sender, Server &server, const std::string &receiver, const std::string &message, const std::string &other)
{
	if (receiver.empty() || message.empty())//Verificar que el destinatario y el mensaje no esten vacios
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid MSG format. Use MSG <receiver> <message>\n"));
	if (receiver == sender.getNickname())//Verificar que no me este mandando un mensaje a mi mismo
		return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + "  ERROR: You cannot send a message to yourself ('" + sender.getNickname() + "')\n"));
	if (receiver == "bot" || receiver == "BOT")
	{
		if (!other.empty())
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'MSG BOT' does not accept any more parameters than help/joke/play. Use: MSG bot help/joke/play\n"));
		std::string newMessage = message;
		if (message[0] == ':')
			newMessage = message.substr(1);
		sender.messageToMyself(":bot PRIVMSG ");
		return(server.getHandleBotCommand(sender, newMessage));
	}
	if (receiver[0] == '#')// Si el receptor es un canal
	{
		std::map<std::string, Channel>& channels = server.getChannels();
		std::map<std::string, Channel>::iterator it = channels.find(receiver);
		Channel &channel = it->second;
		if (it == channels.end())//Comprobar si el canal existe
			return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel " + receiver + " does not exist\n"));
		if (!channel.hasClient(&sender))// Comprobar si el que envia no está en el canal
			return(sender.messageToMyself(":ircserver 404 " + sender.getNickname() + " ERROR: To send a message in channel: " + channel.getName() + " you have to JOIN it first\n"));
		if (message[0] == ':')
		{
			std::string newMessage = message.substr(1);
			return(channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + receiver + " " + newMessage + "\r\n", &sender));
		}
		return(channel.messageToGroup(":" + sender.getNickname() + " PRIVMSG " + receiver + " " + message + "\r\n"));
	}
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)//Recorrer todos los clientes que hay en el servidor
	{
		Client *destinatary = it->second;
		if (destinatary && destinatary->getNickname() == receiver)//Si encuentra al destinatario en el servidor, le manda el mensaje
		{
			if (message[0] == ':')
			{
				std::string newMessage = message.substr(1);
				return(destinatary->messageToMyself(":" + sender.getNickname() + " PRIVMSG " + receiver + " " + newMessage + "\r\n"));
			}
			else
				return(destinatary->messageToMyself(":" + sender.getNickname() + " PRIVMSG " + receiver + " " + message + "\r\n"));
		}
	}
	sender.messageToMyself(":ircserver 401 " + sender.getNickname() + " " + receiver + " ERROR: User not in server\r\n");// Si no se encuentra el receptor
}
