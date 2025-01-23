/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Remove.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:39 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:27:40 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandREMOVE(Client &sender, Server &server, const std::string &channelName, const std::string &param, const std::string &other)
{
	if (channelName.empty() || param.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid REMOVE command syntax. Use: REMOVE <#channel> <topic/modes/invited/banned>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <param>
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'REMOVE' does not accept any more parameters than CHANNEL and a parameter. Use: REMOVE <#channel> <topic/modes/invited/banned>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if (param == "topic")// Si el param es topic. Eliminar el topic
	{
		if (channel.isTopicEmpty())// Verificar si el topic ya está vacío
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel " + channelName + " has no topic to remove\n"));
		channel.clearTopic();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You removed topic from channel ~\r\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has removed the topic from channel ~\r\n", &sender);//Mensaje al grupo
	}
	else if (param == "modes")// Si el param es modes. Desactivar todos los modos activos
	{
		if (!channel.inviteOnly && !channel.topicRestricted && channel.getKey().empty() && channel.getUserLimit() == 0)// Verificar si todos los modos están ya desactivados
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel " + channelName + " has no modes to remove\n"));
		channel.setInviteOnly(false);
		channel.setTopicRestricted(false);
		channel.setKey("");
		channel.clearUserLimit();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You removed all modes from channel ~\r\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has removed all modes from channel ~\r\n", &sender);//Mensaje al grupo
	}
	else if (param == "invited")// Si el param es invited. Eliminar lista de inivitados
	{
		if (channel.isInvitedListEmpty())// Verificar si la lista de invitados ya está vacía
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel " + channelName + " has no invited users list to remove\n"));
		channel.clearInvitedClients();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You removed the invited users list from channel ~\r\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has removed the invited users list from channel ~\r\n", &sender);//Mensaje al grupo
	}
	else if (param == "banned")// Si el param es banned. Eliminar lista de baneados
	{
		if (channel.isBannedListEmpty())// Verificar si la lista de baneados ya está vacía
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel " + channelName + " has no banned users list to remove\n"));
		channel.clearBannedClients();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You removed the banned users list from channel ~\r\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has removed the banned users list from channel ~\r\n", &sender);//Mensaje al grupo
	}
	else
		sender.messageToMyself(":ircserver 421 " + sender.getNickname() + " ERROR: Invalid parameter for REMOVE. Use: topic, modes, invited or banned\r\n");
}