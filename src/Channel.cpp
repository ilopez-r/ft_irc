#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include <iostream>

Channel::Channel() : name("") {}

Channel::Channel(const std::string &name) : name(name) {}

Channel::~Channel() {}

const std::string &Channel::getName() const { return name; }

void Channel::addClient(Client *client) {
    clients.insert(client);
}

void Channel::removeClient(Client *client) {
    clients.erase(client);
}

void Channel::broadcastMessage(const std::string &message, Client *sender) {
    for (std::set<Client *>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (*it != sender) {
            (*it)->sendMessage(message);
        }
    }
}

