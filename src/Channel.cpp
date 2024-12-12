#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include <iostream>

Channel::Channel() : name(""), inviteOnly(false), topicRestricted(false), userLimit(0) {}

Channel::Channel(const std::string &name) : name(name), inviteOnly(false), topicRestricted(false), userLimit(0) {}

Channel::~Channel() {}

const std::string &Channel::getName() const { return name; }

void Channel::addClient(Client *client) {
    clients.insert(client);
    operators.erase(client); // Elimina al cliente si era operador.
}

void Channel::removeClient(Client *client) {
    clients.erase(client);
}

void Channel::broadcastMessage(const std::string &message, Client *sender) {
    for (std::set<Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
        if (*it != sender)
            (*it)->sendMessage(message);
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

void Channel::notifyNicknameChange(const std::string &oldNickname, const std::string &newNickname, Client *sender) {
    std::string message = "[" + getName() + "]: '" + oldNickname + "' changed his nickname to '" + newNickname + "'\r\n";
    for (std::set<Client *>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (*it != sender)
            (*it)->sendMessage(message);
    }
}
