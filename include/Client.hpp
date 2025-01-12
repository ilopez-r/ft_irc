/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 14:59:19 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/13 00:48:07 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream> //Para std::cout
#include <unistd.h> //Para close
#include <arpa/inet.h> //Para send
#include "Server.hpp"
#include "Channel.hpp"

class Server;

class Client
{
	public:
			//*------------------Constructors and destructors------------------*//
			Client(int fd, const std::string &ip);
			~Client();
			
			//*------------------Fd------------------*//
			int getFd() const;

			//*------------------Buffer------------------*//
			std::string &getBuffer();

			//*------------------Nickname------------------*//
			const std::string &getNickname() const;
			void setNickname(const std::string &nickname);
			
			//*------------------Username------------------*//
			const std::string &getUsername() const;
			void setUsername(const std::string &username);
			
			//*------------------Password------------------*//
			void setPasswordSent(bool status);
			bool getPasswordSent();
			
			//*------------------Client Messages------------------*//
			void messageToMyself(const std::string &message);
			void messageToSomeone(const std::string &message, Client *sender);
	private:
			//*------------------Client Utils------------------*//
			int _fd;
			std::string _ip;
			std::string _buffer;
			std::string _nickname;
			std::string _username;
			bool _passwordSent;
};

#endif
