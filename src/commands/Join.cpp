/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:26:59 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:27:00 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandJOIN(Client &client, Server &server, const std::string &channelName, const std::string &key, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: No channel name provided. Use: JOIN <#channel> [key]\n"));
	if (channelName[0] != '#')//Verificar que el canal empieze por #
		return(client.messageToMyself(":ircserver 421 " + client.getNickname() + " ERROR: Channel name must start with '#'\n"));
	if (channelName.size() < 2)
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Channel name cannot be empty\n"));
	if (!other.empty())
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'JOIN' does not accept any more parameters than #CHANNEL and KEY. Use: JOIN <#channel> [key]\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	if (channels.find(channelName) == channels.end())// Comprueba si el canal no existe en el contenedor channels
	{
		channels.insert(std::make_pair(channelName, Channel(channelName)));// Crea el canal con su nombre y un objeto de la clase channel (utilizando el constructor), y lo inserta en el contendor channels
        Channel &channel = channels.find(channelName)->second;// Accede al canal (->second es el objeto de la clase channel)
		channel.addClient(&client);//Añadir al cliente al canal
		channel.addOperator(&client); // Cliente se convierte en operador inicial.
		std::cout << "Client (" << client.getFd() << ") '" << client.getNickname() << "' created and joined channel: " << channelName << "\n";//Mensaje en servidor
		client.messageToMyself(":" + client.getNickname() + " JOIN " + channelName + " ~ You have created and joined channel ~\r\n");//Mensaje al cliente
		if (!key.empty())//Si se puso algo detrás de #channel
			client.messageToMyself(":ircserver 0 " + client.getNickname() + " " + channelName + " ~ Does not need a key to enter ~\r\n");//Mensaje al cliente
	}
	else if(channels.find(channelName) != channels.end())// Si el canal ya existe
	{
		Channel &channel = channels.find(channelName)->second;// Accede al canal (->second es el objeto de la clase channel)
		if (channel.hasClient(&client))// Comprobar si el cliente ya está en el canal
			return(client.messageToMyself(":ircserver 0 " + client.getNickname() + " " + channelName + " ~ You are already in channel~ \n"));
		if (channel.isBanned(&client))// Verificar si el cliente está baneado
			return(client.messageToMyself(":ircserver 465 " + client.getNickname() + " ERROR: You are banned in channel: " + channelName + "\n"));
		if (channel.isInviteOnly() && !channel.isInvited(&client))// Si el canal está en modo invite-only, verificar si el cliente ha sido invitado
			return(client.messageToMyself(":ircserver 473 " + client.getNickname() + " ERROR: Channel: " + channelName + " is on INVITE-ONLY mode\n"));
		if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.getClientsNumber())// Comprobar si hay límite de usuarios y si ya se ha alcanzado el máximo
			return(client.messageToMyself(":ircserver 471 " + client.getNickname() + " ERROR: Channel: " + channelName + " is full\n"));
		if (!channel.getKey().empty() && !channel.isInvited(&client))// Comprobar si el canal está en modo key y no estoy invitado
		{
			if (key.empty())//Si no he puesto ninguna key
				return(client.messageToMyself(":ircserver 475 " + client.getNickname() + " ERROR: Channel: " + channelName + " is in KEY mode. Use: JOIN <#channel> <key>\n"));
			if (key != channel.getKey())//Si la key es incorrecta
				return(client.messageToMyself(":ircserver 475 " + client.getNickname() + " ERROR: Incorrect key\n"));
		}
		channel.addClient(&client);//Añade al cliente.
		std::cout << "Client (" << client.getFd() << ") '" << client.getNickname() << "' joined channel: " << channelName << "\n";//Mensaje en servidor
		client.messageToMyself(":" + client.getNickname() + " JOIN " + channelName + " ~ You joined channel ~\r\n");//Mensaje al cliente
		channel.messageToGroupNoSender(":" + client.getNickname() + " PRIVMSG " + channelName + " ~ Joined channel ~\r\n", &client);//Mensaje al grupo
		if (channel.getKey().empty() && !key.empty())//Si no esta en modo key y yo he puesto una contraseña
			client.messageToMyself(":ircserver 0 " + client.getNickname() + " " + channelName + " ~ Does not need a key to enter ~\r\n");
		if (!channel.getKey().empty() && !key.empty() && channel.isInvited(&client))//Si esta en modo key y yo he puesto una key, pero estoy invitado
			client.messageToMyself(":ircserver 0 " + client.getNickname() + " " + channelName + " ~ You do not need a key to enter in channel beacause you are invited ~\r\n");
	}
	Channel &channel = channels.find(channelName)->second;// Accede al canal (->second es el objeto de la clase channel)
	if (!channel.isTopicEmpty())// Si el canal tiene topic
		client.messageToMyself(":ircserver 332 " + client.getNickname() + " " + channelName + " :" + channel.getTopic() + "\r\n");// Enviar el TEMA actual del canal (RPL_TOPIC 332)
	std::string userList = channelName + " :";
	for (std::set<Client*>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)//Recorrer lista de clientes del canal
	{
		if (channel.isOperator(*it))//Si el cliente es operador, ponerle un @ delante
			userList += "@";
		userList += (*it)->getNickname() + " ";//Añadir cliente a la lista
	}
	channel.messageToGroup(":ircserver 353 " + client.getNickname() + " " + channelName + " " + userList + "\r\n");// Enviar la lista de usuarios (RPL_NAMREPLY 353)
	channel.messageToGroup(":ircserver 366 " + channelName + " " + channelName + "\r\n");// Fin de la lista de usuarios (RPL_ENDOFNAMES 366)
}
