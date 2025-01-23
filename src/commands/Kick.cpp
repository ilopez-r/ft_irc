/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:06 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:27:07 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandKICK(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &reason)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid KICK command syntax. Use: KICK <#channel> <user> <reason>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if (user == sender.getNickname())//Verificar que no me este kickeando a mi mismo
		return(sender.messageToMyself(":ircserver 400 " + sender.getNickname() + " ERROR: You cannot kick yourself ('" + sender.getNickname() + "') from a channel\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a kickear
		{
			if (channel.hasClient(client))//Si el usuario al que se kickea está dentro del canal
			{
				if (channel.isInvited(client))//Eliminar al usuario de la lista de invitados del canal si lo estaba
					channel.removeInvitedClient(client);
				if (channel.isOperator(client))//Elimiar al usuario de la lista de operadores del canal si lo estaba
					channel.removeOperator(client);
				std::string reasonParsed = reason;
				if (!reason.empty() && reason[0] == ':')
					reasonParsed = reason.substr(1);
				if (!reason.empty() && reasonParsed != user)
					client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ You have been kicked from channel by '" + sender.getNickname() + "'. Reason: " + reasonParsed + " ~\r\n");
				else
					client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ You have been kicked from channel by '" + sender.getNickname() + "' ~\r\n");
				if (reasonParsed != user)
					client->messageToMyself(":" + sender.getNickname() + " KICK " + channelName + " " + user + " :" + reasonParsed + "\r\n");
				else
					client->messageToMyself(":" + sender.getNickname() + " KICK " + channelName + " " + user + "\r\n");
				channel.removeClientChannnel(client);//Eliminar al usuario del canal
				std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' has kicked Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
				channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has kicked '" + client->getNickname() + "' from channel ~\r\n", &sender);//Mensaje al grupo
				sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You have kicked '" + user + "' from channel ~\n");
				std::string userList = channelName + " :";
				for (std::set<Client*>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)//Recorrer lista de clientes del canal
				{
					if (channel.isOperator(*it))//Si el cliente es operador, ponerle un @ delante
						userList += "@";
					userList += (*it)->getNickname() + " ";//Añadir cliente a la lista
				}
				channel.messageToGroup(":ircserver 353 " + sender.getNickname() + " " + channelName + " " + userList + "\r\n");// Enviar la lista de usuarios (RPL_NAMREPLY 353)
				return(channel.messageToGroup(":ircserver 366 " + channelName + " " + channelName + "\r\n"));// Fin de la lista de usuarios (RPL_ENDOFNAMES 366)
				
			}
			else//Si no se cumple, ese usuario ya estaba fuera del canal
				return(sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + user + "' is not in channel: " + channelName + "\n"));
		}
	}
	sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + user + "' does not exist\n");//El usuario no esta en el servidor
}
