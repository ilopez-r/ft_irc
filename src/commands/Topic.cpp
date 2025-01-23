/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:43 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:27:44 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandTOPIC(Client &sender, Server &server, const std::string &channelName, const std::string &topic)
{
	if (channelName.empty())//Comprobar si solo se ha escrito topic
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid TOPIC command syntax. Use: TOPIC <#channel> [new topic]\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	Channel &channel = it->second;
	if (topic.empty())//Comprobar si solamente quiero ver el topic
	{
		if (channel.isTopicEmpty())//Comprobar si el canal no tiene topic
			return(sender.messageToMyself(":ircserver 331 " + sender.getNickname() + " " + channelName + " ~ Has no TOPIC yet ~\r\n"));
		return(sender.messageToMyself(":ircserver 332 " + sender.getNickname() + " " + channelName + " '" + channel.getTopic() + "'\r\n"));
	}
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if ((channel.isTopicRestricted() && !channel.isOperator(&sender)))//Comprobar si el canal esta +t y yo no soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not allowed to change the topic in channel: " + channelName + ". Channel is in MODE +t and you are not an operator\n"));
	std::string topicParsed = topic;
	if (topic[0] == ':')
		topicParsed = topic.substr(1);
	if (topicParsed == "REMOVE" || topicParsed == "remove")// Eliminar el tema del canal
	{
		if (channel.getTopic() == "")//Comprobar si estoy intentando eliminar el topic en un canal que no tiene topic
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " has no TOPIC\n"));
		channel.setTopic("");
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' removed TOPIC in channel: " << channelName << "\n";
		sender.messageToMyself(":ircserver 332 " + sender.getNickname() + " " + channelName + " " + channel.getTopic() + "\r\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Removed TOPIC in channel ~\r\n", &sender);
		return(channel.messageToGroupNoSender(":ircserver 332 " + sender.getNickname() + " " + channelName + " " + channel.getTopic() + "\r\n", &sender));
	}
	if (channel.getTopic() == topicParsed)//Comprobar si estoy intentando poner un topic en un canal que ya tiene ese mismo topic
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: TOPIC in channel: " + channelName + " is already '" + topicParsed + "'\n"));
	channel.setTopic(topicParsed);
	std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' changed TOPIC to '" << topicParsed << "' in channel: " << channelName << "\n";
	sender.messageToMyself(":ircserver 332 " + sender.getNickname() + " " + channelName + " '" + channel.getTopic() + "'\r\n");
	channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Changed TOPIC to '" + topicParsed + "' in channel ~\r\n", &sender);
	channel.messageToGroupNoSender(":ircserver 332 " + sender.getNickname() + " " + channelName + " '" + channel.getTopic() + "'\r\n", &sender);
}
