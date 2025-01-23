/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Uninvite.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:47 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:27:48 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandUNINVITE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid UNINVITE command syntax. Use: UNINVITE <user> <#channel>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <user>
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'UNINVITE' does not accept any more parameters than CHANNEL and USER. Use: UNINVITE <#channel> <user>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if (user == sender.getNickname())//Verificar que no me este desinvitando a mi mismo
		return(sender.messageToMyself(":ircserver 400 " + sender.getNickname() + " ERROR: You cannot uninvite yourself ('" + sender.getNickname() + "') to a channel\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a invitar
		{
			if (!channel.isInvited(client))//Verificar si no está en la lista de invitados
				return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: User '" + user + "' is not in the clients invited list in channel: " + channelName + "\n"));
			channel.removeInvitedClient(client);//Eliminar al usuario de la lista de invitados del canal
			std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' uninvited Client (" << client->getFd() << ") '" << client->getNickname() << "' to channel: " << channelName << "\n";
			sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You uninvited '" + user + "' ~\n");
			return(client->messageToMyself("~ '" + sender.getNickname() + "' uninvited you to channel: " + channelName + ". You are not in the invited list anymore\n"));
		}
	}
	sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + user + "' does not exist\n");//El usuario no esta en el servidor
}
