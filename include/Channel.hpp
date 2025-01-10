/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 14:58:23 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/10 17:15:14 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include "Client.hpp"
#include "Server.hpp"

class Client;

class Channel
{
	public:
			Channel();
			Channel(const std::string &name);
			~Channel();

			const std::string &getName() const;
			void addClient(Client *client);
			void removeClientChannnel(Client *client);
			void messageToGroup(const std::string &message);
			void messageToGroupNoSender(const std::string &message, Client *sender);
			void messageToGroupNoSenderNoReceiver(const std::string &message, Client *sender, Client *receiver);
			bool isOperator(Client *client) const;
			void addOperator(Client *client);
			void removeOperator(Client *client);
			bool hasClient(Client *client) const;
			bool isInvited(Client *client) const; // Verificar si el cliente ha sido invitado
			void inviteClient(Client *client); // Añadir cliente a la lista de invitados


			// Configuraciones del canal
			void setInviteOnly(bool status);
			bool isInviteOnly() const;

			void setTopicRestricted(bool status);
			bool isTopicRestricted() const;

			void setKey(const std::string &key);
			const std::string &getKey() const;

			void setUserLimit(size_t limit);
			size_t getUserLimit() const;
			std::string getChannelSize (int number) const;
			std::string getModes() const;

			void banClient(Client *client);
			bool isBanned(Client *client) const;
			void unbanClient(Client *client);
			void clearUserLimit();
			std::set<Client *> clients;
			std::set<Client *> operators;
			std::set<Client *> invitedClients;
			std::string _topic;
			bool inviteOnly;
			bool topicRestricted;

	private:
			std::string name;
			std::string key;
			size_t userLimit;
			std::set<Client*> bannedClients;
};

#endif
