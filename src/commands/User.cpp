/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:51 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/24 13:56:30 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandUSER(Client &client, const std::string &username, const std::string &other, const std::string &other2)
{
	if (username.empty())// Validar username no esta vacio
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: No username provided. Use: USER <username>\r\n"));
		return(client.messageToMyself(":ircserver 461 ERROR: No username provided. Use: USER <username>\r\n"));
	}
	if (other2 == "0 * :realname")
	{
		client.setUsername(username);
		return(client.messageToMyself(":ircserver 999 " + client.getNickname() + " ~ You setted your username to '" + username + "' ~\r\n"));
	}
	if (!other.empty()) // Verificar ningun otra palabara detras del username
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'USER' does not accept any more parameters than the USERNAME. Use: USER <username>\r\n"));
		return(client.messageToMyself(":ircserver 461 ERROR: Command 'USER' does not accept any more parameters than the USERNAME. Use: USER <username>\r\n"));
	}
	if (username.length() > 9)// Verificar que no sea mas largo de 9 caracteres
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 436 " + client.getNickname() + " ERROR: Username cannot be longer than 9 characters\r\n"));
		return(client.messageToMyself(":ircserver 436 ERROR: Username cannot be longer than 9 characters\r\n"));
	}
	client.setUsername(username);
	client.messageToMyself(":ircserver 999 " + client.getNickname() + " ~ You setted your username to '" + username + "' ~\r\n");
}
