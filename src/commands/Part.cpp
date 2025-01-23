/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Part.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:18 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:27:19 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandPART(Client &client, Server &server, const std::string &channelName, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: No channel name provided. Use: PART <#channel>\r\n"));
	if (channelName[0] != '#')//Verificar que el canal empieze por #
		return(client.messageToMyself(":ircserver 403 " + client.getNickname() + " ERROR: Channel name must start with '#'\r\n"));
	std::string parsedOther = other;
	if (other[0] == ':')
		parsedOther = other.substr(1);
	if (!parsedOther.empty() && other != ":Leaving")// Verificar ningun otra palabara detras del canal
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'PART' does not accept any more parameters than #CHANNEL. Use: PART <#channel>\r\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(client.messageToMyself(":ircserver 403 " + client.getNickname() + " ERROR: Channel " + channelName + " does not exist\r\n"));
	if (!channel.hasClient(&client))//Comprobar si el cliente está dentro del canal
		return(client.messageToMyself(":ircserver 442 " + client.getNickname() + " ERROR: You are not in channel: " + channelName + "\r\n"));
	if (channel.isOperator(&client))//Elimiar al usuario de la lista de operadores del canal si lo estaba
		channel.removeOperator(&client);
	client.messageToMyself(":" + client.getNickname() + " PART " + channelName + " ~ You left channel ~\r\n");
	channel.removeClientChannnel(&client);
	std::cout << "Client (" << client.getFd() << ") '" << client.getNickname() << "' left channel: " << channelName << "\r\n";
	channel.messageToGroupNoSender(":" + client.getNickname() + " PRIVMSG " + channelName + " ~ Left channel ~\r\n", &client);
	if (channel.getOperatorsNumber() == 0 && channel.getClientsNumber() > 0)// Si no hay operadores pero quedan clientes dentro del canal, asignar el operador al cliente más antiguo
	{
		Client *firstClient = *channel.getClients().begin();//Guardamos el cliente que hay en el principio del contenedor del canal
		channel.addOperator(firstClient);//Lo insertamos en el contenedor de operadores
		std::cout << "Client (" << firstClient->getFd() << ") '" << firstClient->getNickname() << "' is now an operator in channel: " << channelName << "\r\n";
		firstClient->messageToMyself(":" + firstClient->getNickname() + " PRIVMSG " + channelName + " ~ You are now an operator in channel ~\r\n");
		channel.messageToGroupNoSender(":" + firstClient->getNickname() + " PRIVMSG " + channelName + " ~ Is now an operator in channel ~\r\n", firstClient);
	}
	std::string userList = channelName + " :";
	for (std::set<Client*>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)//Recorrer lista de clientes del canal
	{
		if (channel.isOperator(*it))//Si el cliente es operador, ponerle un @ delante
			userList += "@";
		userList += (*it)->getNickname() + " ";//Añadir cliente a la lista
	}
	channel.messageToGroup(":ircserver 353 " + client.getNickname() + " " + channelName + " " + userList + "\r\n");// Enviar la lista de usuarios (RPL_NAMREPLY 353)
	channel.messageToGroup(":ircserver 366 " + channelName + " " + channelName + "\r\n");// Fin de la lista de usuarios (RPL_ENDOFNAMES 366)
	if (channel.getClientsNumber() == 0)//Si no hay nadie más en el canal, eliminar canal
	{
		std::cout << "Channel: " << channelName << " was removed\n";
		channels.erase(it);
	}
}
