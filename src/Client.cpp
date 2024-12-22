/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 14:59:47 by ilopez-r          #+#    #+#             */
/*   Updated: 2024/12/22 22:11:08 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

Client::Client(int fd, const std::string &ip): fd(fd), ip(ip), _nickname(""), _username (""), _passwordSent(false) {}

Client::~Client()
{
	close(fd);
}

int Client::getFd() const
{
	return (fd);
}

std::string &Client::getBuffer()
{
	return (buffer);
}

const std::string &Client::getNickname() const
{
	return (_nickname);
}

void Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

const std::string &Client::getUsername() const
{ 
	return (_username);
}

void Client::setUsername(const std::string &username)
{
	_username = username;
}

void Client::setPasswordSent(bool status)
{ 
	_passwordSent = status;
}
bool Client::getPasswordSent()
{
	return (_passwordSent);
}

void Client::messageToSomeone(const std::string &message, Client *receiver)
{
	if (send(fd, message.c_str(), message.size(), 0) < 0)
		std::cerr << "Failed to send message to sender: " << fd << "\n";
	if (send(receiver->getFd(), message.c_str(), message.size(), 0) < 0)
		std::cerr << "Failed to send message to receiver: " << fd << "\n";
}

void Client::messageToMyself(const std::string &message)
{
	if (send(fd, message.c_str(), message.size(), 0) < 0)
		std::cerr << "Failed to send message to client: " << fd << "\n";
}

void Client::processLine(const std::string &rawInput, Server &server)
{
	std::string line = server.trim(rawInput);
	if (line.empty())
		return; // No hacer nada si la línea está vacía.
	// Separar el mensaje en 3 partes.
	size_t spacePos = line.find(' ');
	std::string cmd = line.substr(0, spacePos);
	std::string paramraw = "";
	std::string param = "";
	std::string paramraw2 = "";
	std::string param2 = "";
	std::string param3 = "";
	if (spacePos != std::string::npos)//Si encuentra el espacio
	{
		paramraw =  server.trim(line.substr(spacePos + 1));
		spacePos = paramraw.find(' ');
		if (spacePos != std::string::npos)//Si encuentra el espacio
		{
			param = paramraw.substr(0, spacePos);
			if (spacePos != std::string::npos)
			{
				paramraw2 = server.trim(paramraw.substr(spacePos + 1));
				spacePos = paramraw2.find(' ');
				param2 = paramraw2.substr(0, spacePos);
				if (spacePos != std::string::npos)
					param3 = server.trim(paramraw2.substr(spacePos + 1));
			}
		}
		else
			param = paramraw;
	}
	for (std::size_t i = 0; i < cmd.size(); i++)
		cmd[i] = toupper(cmd[i]);
	std::cout << "paramraw:" << paramraw << ".\n";
	std::cout << "param:" << param << ".\n";
	std::cout << "paramraw2:" << paramraw2 << ".\n";
	std::cout << "param2:" << param2 << ".\n";
	std::cout << "param3:" << param3 << ".\n";
	handleCommand(cmd, param, paramraw2, param2, param3, server);
}

void Client::handleCommand(const std::string &cmd, const std::string &param, const std::string &paramraw2, const std::string &param2, const std::string &param3, Server &server)
{
	if (cmd == "QUIT")// Sintaxis: QUIT
		server.commandQUIT(this, param);
	else if (cmd == "VIVA")
		messageToMyself("~ ESPAÑA\n");
	else if (cmd == "HELP")// Sintaxis: HELP [cmd]
		server.commandHELP(this, param, param2);
	else if (cmd == "PASS")// Sintaxis: PASS <password>
		server.commandPASS(this, param, param2);
	else if (getPasswordSent() == false) //Obligar a que el primer comando que se debe introducir es pass
		messageToMyself("~ ERROR: You must authenticate with 'PASS' before any other command. Use: PASS <password>\n");
	else if (cmd == "USER")// Sintaxis: USER <username> 
		server.commandUSER(this, param, param2);
	else if (cmd == "NICK")// Sintaxis: NICK <nickname>
		server.commandNICK(this, param, param2);
	else if (_nickname.empty())// Obligar a que haya un nickname para poder hacer el resto de comandos
		messageToMyself("~ ERROR: First you have to use command 'NICK' and specify your nickname. Use: NICK <nickname>\n");
	else if (cmd == "CHANNELS")// Sintaxis: CHANNELS [all]
		server.commandCHANNELS(this, param, param2);
	else if (cmd == "MSG")// Sintaxis: MSG <user/#channel> <message>
		server.commandMSG(this, param, paramraw2);
	else if (cmd == "JOIN")// Sintaxis: JOIN <#channel> [key]. 
		server.commandJOIN(this, param, param2, param3);
	else if (cmd == "LEAVE")// Sintaxis: LEAVE <#channel>
		server.commandLEAVE(this, param, param2);
	else if (cmd == "KICK")// Sintaxis: KICK <#channel> <user> <reason>
		server.commandKICK(this, param, param2, param3);
	else if (cmd == "INVITE")// Sintaxis: INVITE <#channel> <user> 
		server.commandINVITE(this, param, param2, param3);
	else if (cmd == "TOPIC")// Sintaxis: TOPIC <#channel> [new topic]
		server.commandTOPIC(this, param, paramraw2);
	else if (cmd == "KEY")// Sintaxis: KEY <#channel>
		server.commandKEY(this, param, param2);
	else if (cmd == "MODE")// Sintaxis: MODE <#channel> <+|-mode> [arg]
		server.commandMODE(this, param, param2, param3);
	else
		messageToMyself("~ Unknown command: " + cmd + "\n");
}
