/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:26:55 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:26:56 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandINVITE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid INVITE command syntax. Use: INVITE <user> <#channel>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <user>
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'INVITE' does not accept any more parameters than CHANNEL and USER. Use: INVITE <#channel> <user>\r\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if (user == sender.getNickname())//Verificar que no me este invitando a mi mismo
		return(sender.messageToMyself(":ircserver 400 " + sender.getNickname() + " ERROR: You cannot invite yourself ('" + sender.getNickname() + "') to a channel\n"));
	if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.getClientsNumber())//Verificar si el canal esta lleno
		return(sender.messageToMyself(":ircserver 471 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is full\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a invitar
		{
			if (channel.isInvited(client))//Verificar si ya está en la lista de invitados
				return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: User '" + user + "' is already in the clients invited list in channel: " + channelName + "\n"));
			if (!channel.hasClient(client))//Si el usuario al que se invita no esta ya dentro del canal
			{
				channel.inviteClient(client);//Invitar al usuario al canal
				std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' invited Client (" << client->getFd() << ") '" << client->getNickname() << "' to channel: " << channelName << "\n";
				sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You invited '" + user + "' ~\n");
				return(client->messageToMyself("~ '" + sender.getNickname() + "' invited you to channel: " + channelName + ". Use: JOIN <#channel> [key]\n"));
			}
			else//Si no se cumple, ese usuario ya estaba dentro del canal
				return(sender.messageToMyself(":ircserver 443 " + sender.getNickname() + " ERROR: User '" + user + "' is already in channel: " + channelName + "\n"));
		}
	}
	sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + user + "' does not exist\n");//El usuario no esta en el servidor
}
