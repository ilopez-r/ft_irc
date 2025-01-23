/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:26:50 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:26:51 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandCOMMANDS(Client &client, const std::string &cmd, const std::string &other)
{
	std::string cmdUpper = cmd;
	for (std::size_t i = 0; i < cmd.size(); i++)
		cmdUpper[i] = toupper(cmd[i]);
	if (!other.empty())
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'COMMANDS' does not accept any more parameters than an existing command. Use: COMMANDS [cmd]\r\n"));
		return(client.messageToMyself(":ircserver 461 ERROR: Command 'COMMANDS' does not accept any more parameters than an existing command. Use: COMMANDS [cmd]\r\n"));
	}
	std::string QUIT = "~ QUIT: Disconnect from the server\n";
	if (cmdUpper == "QUIT")
		return(client.messageToMyself(QUIT));
	std::string PASS = "~ PASS <password>: Necessary to start using the chat\n";
	if (cmdUpper == "PASS")
		return(client.messageToMyself(PASS));
	std::string USER = "~ USER <username>: Register your username\n";
	if (cmdUpper == "USER")
		return(client.messageToMyself(USER));
	std::string NICK = "~ NICK <nickname>: Set your nickname\n";
	if (cmdUpper == "NICK")
		return(client.messageToMyself(NICK));
	std::string PROFILE = "~ PROFILE: Show your username and nickname\n";
	if (cmdUpper == "PROFILE")
		return(client.messageToMyself(PROFILE));
	std::string CHANNELS = "~ CHANNELS [all]: Show in which channels you are active. Type <all> to see all channels in server\n";
	if (cmdUpper == "CHANNELS")
		return(client.messageToMyself(CHANNELS));
	std::string MSG = "~ MSG <user/#channel> <message>: Send a message\n";
	if (cmdUpper == "MSG")
		return(client.messageToMyself(MSG));
	std::string JOIN = "~ JOIN <#channel> [key]: Join a channel\n";
	if (cmdUpper == "JOIN")
		return(client.messageToMyself(JOIN));
	std::string PART = "~ PART <#channel>: Disconnect from a channel\n";
	if (cmdUpper == "PART")
		return(client.messageToMyself(PART));
	std::string KICK = "~ KICK <#channel> <user> <reason>: For operators. Kick a user from a channel\n";
	if (cmdUpper == "KICK")
		return(client.messageToMyself(KICK));
	std::string INVITE = "~ INVITE <#channel> <user>: For operators. Invite a user to a channel\n";
	if (cmdUpper == "INVITE")
		return(client.messageToMyself(INVITE));
	std::string UNINVITE = "~ UNINVITE <#channel> <user>: For operators. Uninvite a user to a channel\n";
	if (cmdUpper == "UNINVITE")
		return(client.messageToMyself(UNINVITE));
	std::string TOPIC = "~ TOPIC <#channel> [new topic]: Show or set (if you add <new topic> and you are operator) a topic for a channel\n";
	if (cmdUpper == "TOPIC")
		return(client.messageToMyself(TOPIC));
	std::string KEY = "~ KEY <#channel>: For operators. Show the key from a channel\n";
	if (cmdUpper == "KEY")
		return(client.messageToMyself(KEY));
	std::string MODE = "~ MODE <#channel> [+|-mode] [param]: For operators. Change modes in a channel\n"
						"   * +|- i: Set/remove invite-only channel\n"
						"   * +|- t: Set/remove the restrictions of the topic command to channel operators\n"
						"   * +|- k: Set/remove the channel key (password)\n"
						"   * +|- o: Give/remove channel operator privileges\n"
						"   * +|- l: Set/remove the user limit to a channel\n"
						"   * +|- b: Ban/unban a user from a channel\n";
	if (cmdUpper == "MODE")
		return(client.messageToMyself(MODE));
	std::string REMOVE = "~ REMOVE <#channel> <topic/modes/invited/banned>: For operators. Remove topic, modes or the clients invited/banned list from a channel\n";
	if (cmdUpper == "REMOVE")
		return(client.messageToMyself(REMOVE));
	std::string COMMANDS = "~ COMMANDS [cmd]: Show instructions. Type a command <cmd> to see only its instructions\n";
	if (cmdUpper == "")
		return(client.messageToMyself(QUIT + PASS + USER + NICK + PROFILE + CHANNELS + MSG + JOIN + PART + KICK + INVITE + UNINVITE + TOPIC + KEY + MODE + REMOVE + COMMANDS));
	if (!client.getNickname().empty())
		return(client.messageToMyself(":ircserver 421 " + client.getNickname() + " ERROR: Command '" + cmdUpper + "' is not an existing command. Use: COMMANDS [cmd]\n"));
	return(client.messageToMyself(":ircserver 421 ERROR: Command '" + cmdUpper + "' is not an existing command. Use: COMMANDS [cmd]\n"));
}
