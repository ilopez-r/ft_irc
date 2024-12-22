/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 14:59:19 by ilopez-r          #+#    #+#             */
/*   Updated: 2024/12/22 22:11:16 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <queue>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include "Server.hpp"
#include "Channel.hpp"

class Server;

class Client
{
	public:
			Client(int fd, const std::string &ip);
			~Client();

			int getFd() const;
			std::string &getBuffer();
			const std::string &getNickname() const;
			void setNickname(const std::string &nickname);
			const std::string &getUsername() const;
			void setUsername(const std::string &username);
			void setPasswordSent(bool status);
			bool getPasswordSent();
			void messageToMyself(const std::string &message);
			void messageToSomeone(const std::string &message, Client *sender);
			void processLine(const std::string &message, Server &server);

	private:
			int fd;
			std::string ip;
			std::string buffer;
			std::string _nickname;
			std::string _username;
			bool _passwordSent;
			std::queue<std::string> messageQueue;
			void handleCommand(const std::string &cmd, const std::string &param,  const std::string &paramraw2, const std::string &param2, const std::string &param3, Server &server);
};

#endif
