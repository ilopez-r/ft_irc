/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 14:58:52 by ilopez-r          #+#    #+#             */
/*   Updated: 2024/12/22 14:58:56 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"

Channel::Channel() : _topic(""), inviteOnly(false), topicRestricted(false), name(""), userLimit(0) {}

Channel::Channel(const std::string &name) : _topic(""), inviteOnly(false), topicRestricted(false), name(name), userLimit(0) {}

Channel::~Channel() {}

const std::string &Channel::getName() const { return name; }

void Channel::addClient(Client *client) {
	clients.insert(client);
}

void Channel::removeClientChannnel(Client *client) {
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

bool Channel::isOperator(Client *client) const {
	return operators.find(client) != operators.end();
}

void Channel::addOperator(Client *client) {
	operators.insert(client);
}

void Channel::removeOperator(Client *client) {
	operators.erase(client);
}

// Configuraciones del canal
void Channel::setInviteOnly(bool status) {
	inviteOnly = status;
}

bool Channel::isInviteOnly() const {
	return inviteOnly;
}

void Channel::setTopicRestricted(bool status) {
	topicRestricted = status;
}

bool Channel::isTopicRestricted() const {
	return topicRestricted;
}

void Channel::setKey(const std::string &key) {
	this->key = key;
}

const std::string &Channel::getKey() const {
	return key;
}

void Channel::setUserLimit(size_t limit) {
	userLimit = limit;
}

size_t Channel::getUserLimit() const {
	return userLimit;
}

std::string Channel::getChannelSize (int number) const{
	std::ostringstream oss;
	oss << number;
	return oss.str();
}

void Channel::clearUserLimit() {
	userLimit = 0;
}

bool Channel::hasClient(Client *client) const {
	return clients.find(client) != clients.end();
}

bool Channel::isInvited(Client *client) const {
	return invitedClients.find(client) != invitedClients.end();
}

void Channel::inviteClient(Client *client) {
	invitedClients.insert(client);
}

