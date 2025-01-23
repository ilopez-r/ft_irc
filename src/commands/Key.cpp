/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Key.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:03 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:27:04 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandKEY(Client &sender, Server &server, const std::string &channelName, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid KEY command syntax. Use: KEY <#channel>\n"));
	if (!other.empty())
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'KEY' does not accept any more parameters than CHANNEL. Use: KEY <#channel>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if(channel.getKey() == "")//Comprobar si el acanal tiene key
		return(sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ Channel has no key ~\n"));
	sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ Channel has '" + channel.getKey() + "' as a key ~\n");
}