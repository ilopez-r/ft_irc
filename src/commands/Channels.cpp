/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channels.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:26:46 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:26:47 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

std::string to_string (int number)
{
	std::ostringstream oss;
	oss << number;
	return (oss.str());
}

void commandCHANNELS(Client &client, Server &server, const std::string &param, const std::string &other)
{
	if ((param != "all" && param != "") || !other.empty())//Verificar ningun otra palabara detras de CHANNELS
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'CHANNELS' does not accept any more parameters than ALL. Use: CHANNELS [all]\r\n"));
	if (server.getChannels().empty())//Si no hay canales creados
		return(client.messageToMyself("~ No channels are currently available\n"));
	if (param == "all")//Si se quieren ver todos los canales que existen
	{
		client.messageToMyself("~ List of all channels:\n");
		std::map<std::string, Channel>& channels = server.getChannels();
		for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); it++)//Recorrer todos los canales que hay ya creados
		{
			const std::string &channelName = it->first;
			const Channel &channel = it->second;
			client.messageToMyself("	* " + channelName);
			if (!channel.getModes().empty())// Mostrar los modos activos solo si hay alguno
				client.messageToMyself("[" + channel.getModes() + "]");
			if (!channel.isTopicEmpty()) //Mostrar el topic si no está vacío
				client.messageToMyself("[Topic: " + channel.getTopic() + "]");
			client.messageToMyself("(" + to_string(channel.getClientsNumber()) + " member(s)): ");
			for (std::set<Client *>::iterator clientIt = channel.getClients().begin(); clientIt != channel.getClients().end(); ++clientIt)//Recorrer todos los clientes que hay en ese canal
			{
				std::set<Client *>::iterator nextIt = clientIt; 
				++nextIt;//Nos guardamos el siguiente del cliente al que estamos comprobando, para poder ver si es el ultimo que queda del canal por mostrar
				if (nextIt == channel.getClients().end())//Si es el ultimo, salto de linea
					client.messageToMyself((*clientIt)->getNickname() + "\n");
				else//Si no es el ultimo, ponemos una coma
					client.messageToMyself((*clientIt)->getNickname() + ", ");
			}
		}
	}
	else //Si se quiere ver solo los canales en los que yo estoy
	{
		int flag = 0;
		std::map<std::string, Channel>& channels = server.getChannels();
		for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
		{
			const std::string &channelName = it->first;
			const Channel &channel = it->second;
			if (channel.hasClient(&client))
			{
				if (flag == 0)
				{
					flag = 1;
					client.messageToMyself("~ You are currently active in channel(s):\n");
				}
				client.messageToMyself("	* " + channelName);
				if (!channel.getModes().empty())// Mostrar los modos activos solo si hay alguno
					client.messageToMyself("[" + channel.getModes() + "]");
				if (!channel.isTopicEmpty()) //Mostrar el topic si no está vacío
					client.messageToMyself("[Topic: " + channel.getTopic() + "]");
				client.messageToMyself("(" + to_string(channel.getClientsNumber()) + " member(s)): ");
				if (channel.getClientsNumber() == 1)
					client.messageToMyself(client.getNickname() + "\n");
				else
				{
					for (std::set<Client *>::iterator clientIt = channel.getClients().begin(); clientIt != channel.getClients().end(); ++clientIt)
					{
						if ((*clientIt)->getNickname() != client.getNickname())
						{
							std::set<Client *>::iterator nextIt = clientIt;
							++nextIt;
							std::set<Client *>::iterator nextnextIt = nextIt;
							++nextnextIt;
							if (nextIt == channel.getClients().end() || ((*nextIt)->getNickname() == client.getNickname() && nextnextIt == channel.getClients().end()))
								client.messageToMyself((*clientIt)->getNickname() + " and " + client.getNickname() + "\n");
							else if (nextIt != channel.getClients().end())
								client.messageToMyself((*clientIt)->getNickname() + ", ");
						}
					}
				}
			}
		}
		if (flag == 0)
			client.messageToMyself("~ You are not active in any channels\n");
	}
}
