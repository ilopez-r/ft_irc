/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 14:59:47 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/12 23:16:47 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

Client::Client(int fd, const std::string &ip)
{
	_fd = fd;
	_ip = ip;
	_nickname = "";
	_username = "";
	_passwordSent = false;
}

Client::~Client()
{
	close(_fd);
}

int Client::getFd() const
{
	return (_fd);
}

std::string &Client::getBuffer()
{
	return (_buffer);
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

void Client::messageToMyself(const std::string &message)
{
	if (send(_fd, message.c_str(), message.size(), 0) < 0)
		std::cerr << "Error: Failed to send message to client: " << _fd << "\n";
}

void Client::messageToSomeone(const std::string &message, Client *receiver)
{
	if (send(_fd, message.c_str(), message.size(), 0) < 0)
		std::cerr << "Error: Failed to send message to sender: " << _fd << "\n";
	if (send(receiver->getFd(), message.c_str(), message.size(), 0) < 0)
		std::cerr << "Error: Failed to send message to receiver: " << _fd << "\n";
}
