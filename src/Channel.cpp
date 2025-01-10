/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 14:37:04 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/10 18:17:12 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"

Channel::Channel() : _topic(""), inviteOnly(false), topicRestricted(false), name(""), userLimit(0) {}

Channel::Channel(const std::string &name) : _topic(""), inviteOnly(false), topicRestricted(false), name(name), userLimit(0) {}

Channel::~Channel() {}

const std::string &Channel::getName() const
{
	return name;
}

void Channel::addClient(Client *client)
{
	clients.insert(client);
}

void Channel::removeClientChannnel(Client *client)
{
	clients.erase(client);
	operators.erase(client); // Elimina al cliente si era operador.
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

// Configuraciones del canal
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
	return topicRestricted;
}

void Channel::setKey(const std::string &key)
{
	this->key = key;
}

const std::string &Channel::getKey() const
{
	return key;
}

void Channel::setUserLimit(size_t limit)
{
	userLimit = limit;
}

size_t Channel::getUserLimit() const
{
	return userLimit;
}

std::string Channel::getChannelSize (int number) const
{
	std::ostringstream oss;
	oss << number;
	return oss.str();
}

void Channel::clearUserLimit()
{
	userLimit = 0;
}

bool Channel::hasClient(Client *client) const
{
	return clients.find(client) != clients.end();
}

bool Channel::isInvited(Client *client) const
{
	return invitedClients.find(client) != invitedClients.end();
}

void Channel::inviteClient(Client *client)
{
	invitedClients.insert(client);
}

std::string Channel::getModes() const
{
	std::string modes = "+";
	bool firstMode = true;  // Bandera para manejar la primera coma

	if (inviteOnly)
	{
		if (!firstMode) modes += ", ";
		modes += "i";
		firstMode = false;
	}
	if (topicRestricted)
	{
		if (!firstMode) modes += ", ";
		modes += "t";
		firstMode = false;
	}
	if (!key.empty())
	{
		if (!firstMode) modes += ", ";
		modes += "k";
		firstMode = false;
	}
	if (userLimit > 0)
	{
		if (!firstMode) modes += ", ";
		modes += "l";
		firstMode = false;
	}
	if (modes == "+")  // No hay modos activos
		return ("");
	return (modes);
}

void Channel::banClient(Client *client)// Agregar un cliente a la lista de baneados
{
    bannedClients.insert(client);
}

bool Channel::isBanned(Client *client) const// Verificar si un cliente está baneado
{
    return (bannedClients.find(client) != bannedClients.end());
}

void Channel::unbanClient(Client *client)// Eliminar a un cliente de la lista de baneados
{
    bannedClients.erase(client);
}
