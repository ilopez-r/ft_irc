/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pass.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:22 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:27:23 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandPASS(Client &client, Server &server, const std::string &pass, const std::string &other)
{
	if (client.getPasswordSent() == true)//Verificar que no se mande 2 veces la misma pass
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 462 " + client.getNickname() + " ERROR: 'PASS' command already sent\r\n"));
		return(client.messageToMyself(":ircserver 462 ERROR: 'PASS' command already sent\r\n"));
	}
	if (other != "")// Verificar ninguna otra palabara detras de la password
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'PASS' does not accept any more parameters than the PASSWORD. Use: PASS <password>\r\n"));
		return(client.messageToMyself(":ircserver 461 ERROR: Command 'PASS' does not accept any more parameters than the PASSWORD. Use: PASS <password>\r\n"));
	}
	if (pass == server.getPassword())// Verificar si la pass es correcta
	{
		std::cout << "Client (" << client.getFd() << ") accepted in server\n";
		client.setPasswordSent(true);
		client.messageToMyself("~ Correct password! You must now use commands USER <username> and NICK <nickname> to start using the server\n");
	}
	else //password incorrecta
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 464 " + client.getNickname() + " ERROR: Incorrect password. Use: PASS <password>\r\n"));
		return(client.messageToMyself(":ircserver 464 ERROR: Incorrect password. Use: PASS <password>\r\n"));
	}
}
