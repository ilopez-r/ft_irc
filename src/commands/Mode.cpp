/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:10 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/24 11:55:44 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandMODE(Client &sender, Server &server, const std::string &channelName, const std::string &mode, const std::string &param)
{
	if (channelName.empty())//Verificar que canal no esté vacío
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid MODE command syntax. Use: MODE <channel> [+|-mode] [param]\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (mode.empty())// Si no se especifica modo, mostrar los modos activos del canal
	{
		if (!channel.getModes().empty())
			return(sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ Active mode(s) for channel: " + channel.getModes() + " ~\r\n"));
		return(sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ No active modes for channel ~\r\n"));
	}
	if (!param.empty() && (mode == "+i" || mode == "-i" || mode == "+t" || mode == "-t" || mode  == "-l" || mode  == "-k"))
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'MODE " + mode +"' does not accept any more parameters than CHANNEL and <+|-mode>. Use: MODE <channel> <+|-mode>\n"));
	size_t spacePos = param.find(' ');
	if (spacePos != std::string::npos)
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'MODE " + mode +"' does not accept any more parameters than CHANNEL, <+|-mode> and [param]. Use: MODE <channel> <+|-mode> [param]\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if (mode == "+o")// Asignar privilegio de operador
	{
		if (param == "")
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: You need to specify a user for MODE +o\n"));
		std::map<int, Client*>& clients = server.getClients();
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.hasClient(client))
				{
					if (channel.isOperator(client))
						return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: '" + param + "' is already an operator in channel: " + channelName + "\n"));
					channel.addOperator(client);
					std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' gave operator privileges to '" << param << "' in channel: " << channelName << "\n";
					sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You gave OPERATOR PRIVILEGES to '" + param + "' in channel ~\n");
					channel.messageToGroupNoSenderNoReceiver(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Gave OPERATOR PRIVILEGES to '" + param + "' in channel ~\r\n", &sender, client);//Mensaje al grupo
					client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ '" + sender.getNickname() + "' gave you OPERATOR PRIVILEGES in channel ~\n");
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
				else
					return(sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' is not in channel: " + channelName + "\n"));
			}
		}
		sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' does not exist\n");//El usuario no esta en el servidor

	}
	else if (mode == "-o")// Eliminar privilegio de operador
	{
		if (param == "")
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: You need to specify a user for MODE -o\n"));
		std::map<int, Client*>& clients = server.getClients();
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.hasClient(client))
				{
					if (!channel.isOperator(client))
						return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: '" + param + "' is not an operator in channel: " + channelName + "\n"));
					if (channel.getOperatorsNumber() == 1)// Si soy el unico operador, que no me deje quitarmelo
						return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: You are the only operator in channel: " + channelName + "\n"));
					channel.removeOperator(client);
					std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' removed operator privileges to '" << param << "' in channel: " << channelName << "\n";
					sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You remove OPERATOR PRIVILEGES to '" + param + "' in channel ~\n");
					channel.messageToGroupNoSenderNoReceiver(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Removed OPERATOR PRIVILEGES to '" + param + "' in channel ~\r\n", &sender, client);//Mensaje al grupo
					client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ '" + sender.getNickname() + "' removed your OPERATOR PRIVILEGES in channel ~\n");
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
				else
					return(sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' is not in channel: " + channelName + "\n"));
			}
		}
		sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' does not exist\n");//El usuario no esta en el servidor

	}
	else if (mode == "+i")//Establecer solo invitados pueden unirse al canal
	{
		if (channel.inviteOnly == true)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is already on INVITE-ONLY mode\n"));
		channel.setInviteOnly(true);
		for (std::set<Client *>::iterator itClient = channel.getClients().begin(); itClient != channel.getClients().end(); ++itClient)// Agregar a todos los clientes actuales a la lista de invitados
			if (!channel.isInvited(*itClient))// Verifica si ya está invitado
				channel.inviteClient(*itClient);// Agrega a la lista de invitados
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You enabled INVITE-ONLY mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Enabled INVITE-ONLY mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled invite-only mode in channel: " << channelName << "\n";
	}
	else if (mode == "-i")//Eliminar solo invitados pueden unirse al canal
	{
		if (channel.inviteOnly == false)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is not on INVITE-ONLY mode\n"));
		channel.setInviteOnly(false);
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You disabled INVITE-ONLY mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Disabled INVITE-ONLY mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled invite-only mode in channel: " << channelName << "\n";
	}
	else if (mode == "+t")//Establecer topic solo lo pueden cambiar operadores
	{
		if (channel.topicRestricted == true)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is already on TOPIC-RESTRICTED mode\n"));
		channel.setTopicRestricted(true);
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You enabled TOPIC-RESTRICTED mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Enabled TOPIC-RESTRICTED mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled topic-restricted mode in channel: " << channelName << "\n";
	}
	else if (mode == "-t")//Eliminar topic solo lo pueden cambiar operadores
	{
		if (channel.topicRestricted == false)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is not on TOPIC-RESTRICTED mode\n"));
		channel.setTopicRestricted(false);
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You disabled TOPIC-RESTRICTED mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Disabled TOPIC-RESTRICTED mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled topic-restricted mode in channel: " << channelName << "\n";
	}
	else if (mode == "+k")//Establecer contraseña al canal
	{
		if (param == "")
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: You need to specify a key for MODE +k\n"));
		if (param.size() > 9)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Key cannot be longer than 9 characters\n"));
		if(channel.getKey() == param)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: KEY is already '" + param + "' in channel\n"));
		channel.setKey(param);
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You enabled KEY (" + param + ") mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Enabled KEY (" + param + ") mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled key (" << param << ") mode in channel: " << channelName << "\n";
	}
	else if (mode == "-k")//Eliminar contraseña del canal
	{
		if(channel.isKeyEmpty())
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is not on KEY mode\n"));
		channel.clearKey();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You disabled KEY mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Disabled KEY mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled key mode in channel: " << channelName << "\n";
	}
	else if (mode == "+l")//Establecer numero maximo de usuarios en el canal
	{
		if (param == "")
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: You need to specify a number for MODE +l\n"));
		for (std::size_t i = 0; i < param.size(); i++)
			if (!std::isdigit(param[i]))
				return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Limit cannot be something different from a positive number\n"));
		size_t limit = std::atoi(param.c_str());
		if (limit == 0 || limit == 1)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Limit cannot be less than 2\n"));
		if (limit < channel.getClientsNumber())
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Limit cannot be less than the channel's actual number of members (" + to_string(channel.getClientsNumber()) + ")\n"));
		if (limit == channel.getUserLimit())
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is already limited to " + param + "\n"));
		channel.setUserLimit(limit);
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You enabled USER-LIMIT (" + param + ") mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Enabled USER-LIMIT (" + param + ") mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled user-limit (" << param << ") mode in channel: " << channelName << "\n";
	}
	else if (mode == "-l")//Eliminar numero maximo de usuarios en el canal
	{
		if (channel.getUserLimit() == 0)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " already has no users limit\n"));
		channel.clearUserLimit();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You disabled USER-LIMIT mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Disabled USER-LIMIT mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled user-limit mode in channel: " << channelName << "\n";
	}
	else if (mode == "+b")
	{
		if (param == "")
		{
			if (channel.getBannedClients().empty()) // Verificar si no hay baneados
				return;
			for (std::set<Client*>::const_iterator it = channel.getBannedClients().begin(); it != channel.getBannedClients().end(); ++it)// Recorrer la lista de usuarios baneados y enviar un mensaje por cada uno
				sender.messageToMyself(":ircserver 367 " + sender.getNickname() + " " + channelName + " " + (*it)->getNickname() + " " + sender.getNickname() + "\r\n");
			return;
		}
		if (param == sender.getNickname())//Verificar que no me este baneando a mi mismo
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: You cannot ban yourself ('" + sender.getNickname() + "') from a channel\n"));
		std::map<int, Client*>& clients = server.getClients();
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.isBanned(client))// Verificar si ya está baneado
					return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: '" + param + "' is already banned in channel: " + channelName + "\n"));
				if (channel.isInvited(client))//Eliminar al usuario de la lista de invitados del canal si lo estaba
					channel.removeInvitedClient(client);
				channel.banClient(client);// Banear al usuario
				std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' has banned Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
				sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You have BANNED '" + param + "' from channel ~\n");
				channel.messageToGroupNoSenderNoReceiver(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has BANNED '" + param + "' from channel ~\r\n", &sender, client);//Mensaje al grupo
				if (!channel.hasClient(client))
					client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ '" + sender.getNickname() + "' has BANNED you from channel ~\n");
				if (channel.hasClient(client))//Eliminar al usuario del canal si está dentro
				{
					channel.removeClientChannnel(client);
					client->messageToMyself(":" + client->getNickname() + " PART " + channelName + " ~ '" + sender.getNickname() + "' has BANNED you from channel ~\r\n");
					if (channel.isOperator(client))//Elimiar al usuario de la lista de operadores del canal si lo estaba
						channel.removeOperator(client);
					std::string userList = channelName + " :";
					for (std::set<Client*>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)//Recorrer lista de clientes del canal
					{
						if (channel.isOperator(*it))//Si el cliente es operador, ponerle un @ delante
							userList += "@";
						userList += (*it)->getNickname() + " ";//Añadir cliente a la lista
					}
					channel.messageToGroup(":ircserver 353 " + sender.getNickname() + " " + channelName + " " + userList + "\r\n");// Enviar la lista de usuarios (RPL_NAMREPLY 353)
					channel.messageToGroup(":ircserver 366 " + channelName + " " + channelName + "\r\n");// Fin de la lista de usuarios (RPL_ENDOFNAMES 366)
				}
				return;
			}
		}
		sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' does not exist\n");//El usuario no esta en el servidor
	}
	else if (mode == "-b")
	{
		if (param == "")
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: You need to specify a user for MODE -b\n"));
		if (param == sender.getNickname())//Verificar que no me este desbaneando a mi mismo
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: You cannot unban yourself ('" + sender.getNickname() + "') from a channel\n"));
		std::map<int, Client*>& clients = server.getClients();
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (!channel.isBanned(client))// Verificar que no está baneado ya
					return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: User '" + param + "' is not banned in " + channelName + ".\n"));
				channel.unbanClient(client);// Desbanear al usuario
				std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' has unbanned Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
				sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You have UNBANNED '" + param + "' from channel ~\n");
				channel.messageToGroupNoSenderNoReceiver(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has UNBANNED '" + param + "' from channel ~\r\n", &sender, client);//Mensaje al grupo
				return(client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ '" + sender.getNickname() + "' has UNBANNED you from channel ~\n"));
			}
		}
		sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' does not exist\n");//El usuario no esta en el servidor
	}
	else
		sender.messageToMyself(":ircserver 472 " + sender.getNickname() + " ERROR: Invalid mode command. Modes are: +|- i, t, k, o, l, b\r\n");
}
