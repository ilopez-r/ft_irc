/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 14:37:04 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/11 20:56:56 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"

Channel::Channel(const std::string &name)
{
	inviteOnly = false;
	topicRestricted = false;
	_name = name;
	_topic = "";
	_userLimit = 0;
}

Channel::~Channel()
{
	
}

const std::string &Channel::getName() const
{
	return (_name);
}

void Channel::setTopic(const std::string &topic)// Establece un nuevo topic en el canal
{
	_topic = topic;
}

const std::string &Channel::getTopic() const// Devuelve el topic actual del canal
{
	return (_topic);
}

bool Channel::isTopicEmpty() const// Verifica si el topic está vacío
{
	return (_topic.empty());
}

void Channel::clearTopic()// Limpia el topic del canal
{
	_topic.clear();
}

void Channel::addClient(Client *client)
{
	clients.insert(client);
}

void Channel::removeClientChannnel(Client *client)
{
	clients.erase(client);
}

bool Channel::hasClient(Client *client) const
{
	return clients.find(client) != clients.end();
}

size_t Channel::getClientsNumber() const
{
	return (clients.size());
}

const std::set<Client *> &Channel::getClients() const
{
	return (clients);
}

bool Channel::isOperator(Client *client) const
{
	return operators.find(client) != operators.end();
}

void Channel::addOperator(Client *client)
{
	operators.insert(client);
}

void Channel::removeOperator(Client *client)
{
	operators.erase(client);
}

size_t Channel::getOperatorsNumber() const
{
	return (operators.size());
}

bool Channel::isBanned(Client *client) const// Verificar si un cliente está baneado
{
    return (bannedClients.find(client) != bannedClients.end());
}

void Channel::banClient(Client *client)// Agregar un cliente a la lista de baneados
{
    bannedClients.insert(client);
}

void Channel::unbanClient(Client *client)// Eliminar a un cliente de la lista de baneados
{
    bannedClients.erase(client);
}

void Channel::clearBannedClients()
{
	bannedClients.clear();
}

bool Channel::isBannedListEmpty() const
{
	return (bannedClients.empty());
}

bool Channel::isInvited(Client *client) const
{
	return invitedClients.find(client) != invitedClients.end();
}

void Channel::inviteClient(Client *client)
{
	invitedClients.insert(client);
}

void Channel::removeInvitedClient(Client *client)// Elimina un cliente específico de la lista de invitados
{
	invitedClients.erase(client);
}

void Channel::clearInvitedClients()// Limpia toda la lista de invitados
{
	invitedClients.clear();
}

bool Channel::isInvitedListEmpty() const// Verifica si la lista de invitados está vacía
{
	return (invitedClients.empty());
}

std::string Channel::getModes() const
{
	std::string modes = "+";
	bool firstMode = true;  // Flag para manejar la primera coma

	if (inviteOnly)
	{
		if (!firstMode)
			modes += ", ";
		modes += "i";
		firstMode = false;
	}
	if (topicRestricted)
	{
		if (!firstMode)
			modes += ", ";
		modes += "t";
		firstMode = false;
	}
	if (!_key.empty())
	{
		if (!firstMode)
			modes += ", ";
		modes += "k";
		firstMode = false;
	}
	if (_userLimit > 0)
	{
		if (!firstMode)
			modes += ", ";
		modes += "l";
		firstMode = false;
	}
	if (modes == "+")  // No hay modos activos
		return ("");
	return (modes);
}

void Channel::setInviteOnly(bool status)
{
	inviteOnly = status;
}

bool Channel::isInviteOnly() const
{
	return inviteOnly;
}

void Channel::setTopicRestricted(bool status)
{
	topicRestricted = status;
}

bool Channel::isTopicRestricted() const
{
	return (topicRestricted);
}

void Channel::setUserLimit(size_t limit)
{
	_userLimit = limit;
}

size_t Channel::getUserLimit() const
{
	return (_userLimit);
}

void Channel::clearUserLimit()
{
	_userLimit = 0;
}

void Channel::setKey(const std::string &key)
{
	_key = key;
}

const std::string &Channel::getKey() const
{
	return (_key);
}

bool Channel::isKeyEmpty() const// Verifica si la key está vacía
{
	return (_key.empty());
}

void Channel::clearKey()// Limpia la key del canal
{
	_key.clear();
}

void Channel::messageToGroup(const std::string &message)
{
	for (std::set<Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
		(*it)->messageToMyself(message);
}

void Channel::messageToGroupNoSender(const std::string &message, Client *sender)
{
	for (std::set<Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
		if (*it != sender)
			(*it)->messageToMyself(message);
}

void Channel::messageToGroupNoSenderNoReceiver(const std::string &message, Client *sender, Client *receiver)
{
	for (std::set<Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
		if (*it != sender && *it != receiver)
			(*it)->messageToMyself(message);
}
