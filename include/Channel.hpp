/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 14:58:23 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/11 21:45:16 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include "Client.hpp"
#include "Server.hpp"

class Client;

class Channel
{
	public:
			Channel(const std::string &name);
			~Channel();

			//*------------------Name------------------*//
			const std::string &getName() const;

			//*------------------Topic------------------*//
			void setTopic(const std::string &topic);
			const std::string &getTopic() const;
			bool isTopicEmpty() const;
			void clearTopic();

			//*------------------Clients Container------------------*//
			void addClient(Client *client);
			void removeClientChannnel(Client *client);
			bool hasClient(Client *client) const;
			size_t getClientsNumber() const;
			const std::set<Client *> &getClients() const;

			//*------------------Operators Container------------------*//
			bool isOperator(Client *client) const;
			void addOperator(Client *client);
			void removeOperator(Client *client);
			size_t getOperatorsNumber() const;

			//*------------------Banned Clients Container------------------*//
			bool isBanned(Client *client) const;
			void banClient(Client *client);
			void unbanClient(Client *client);
			void clearBannedClients();
			bool isBannedListEmpty() const;

			//*------------------Invited Clients Container------------------*//
			bool isInvited(Client *client) const; // Verificar si el cliente ha sido invitado
			void inviteClient(Client *client); // Añadir cliente a la lista de invitados
			void removeInvitedClient(Client *client);
			void clearInvitedClients();
			bool isInvitedListEmpty() const;

			//*------------------Channel Modes------------------*//
			std::string getModes() const;
			//*--------------Mode +i--------------*//
			bool inviteOnly;
			void setInviteOnly(bool status);
			bool isInviteOnly() const;
			//*--------------Mode +t--------------*//
			bool topicRestricted;
			void setTopicRestricted(bool status);
			bool isTopicRestricted() const;
			//*--------------Mode +l--------------*//
			void setUserLimit(size_t limit);
			size_t getUserLimit() const;
			void clearUserLimit();
			//*--------------Mode +k--------------*//
			void setKey(const std::string &key);
			const std::string &getKey() const;
			bool isKeyEmpty() const;
			void clearKey();

			//*------------------Channel Messages------------------*//
			void messageToGroup(const std::string &message);
			void messageToGroupNoSender(const std::string &message, Client *sender);
			void messageToGroupNoSenderNoReceiver(const std::string &message, Client *sender, Client *receiver);
	private:
			std::string _name; //Nombre del canal
			std::string _topic; //Topic del canal
			std::string _key; //Contraseña del canal
			size_t _userLimit; //Limite de usuarios del canal
			std::set<Client*> clients; //Contenedor de clientes del canal
			std::set<Client*> operators; //Contenedor de operadores del canal
			std::set<Client*> bannedClients; //Contenedor de baneados del canal
			std::set<Client*> invitedClients; //Contenedor de invitados al canal
};

#endif
