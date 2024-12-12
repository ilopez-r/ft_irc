#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include <vector>

class Client;

class Channel {
public:
    Channel();
    Channel(const std::string &name);
    ~Channel();

    const std::string &getName() const;
    void addClient(Client *client);
    void removeClient(Client *client);
    void broadcastMessage(const std::string &message, Client *sender);
    bool isOperator(Client *client) const;
    void addOperator(Client *client);
    void removeOperator(Client *client);
    bool hasClient(Client *client) const;
    bool isInvited(Client *client) const; // Verificar si el cliente ha sido invitado
    void inviteClient(Client *client); // Añadir cliente a la lista de invitados
    void notifyNicknameChange(const std::string &oldNickname, const std::string &newNickname, Client *sender);


    // Configuraciones del canal
    void setInviteOnly(bool status);
    bool isInviteOnly() const;

    void setTopicRestricted(bool status);
    bool isTopicRestricted() const;

    void setKey(const std::string &key);
    const std::string &getKey() const;

    void setUserLimit(size_t limit);
    size_t getUserLimit() const;

    void clearUserLimit();
    std::set<Client *> clients;
private:
    std::string name;
    std::set<Client *> operators;
    std::set<Client *> invitedClients;

    // Configuración del canal
    bool inviteOnly;
    bool topicRestricted;
    std::string key;
    size_t userLimit;
};

#endif
