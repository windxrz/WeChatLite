//
// Created by 徐韧喆 on 27/12/2017.
//

#include "Server.hpp"

User::User()
{
    friends.clear();
    files.clear();
    this->unsentMsg = json::object();
    this->connfd = -1;
    this->current = "";
}

User::User(const std::string &name, const std::string &password) : User()
{
    this->name = name;
    this->password = password;
}

void User::addFriend(const std::string &name)
{
    friends.push_back(name);
}

std::vector<User *> Server::userList;

void Server::addUser(User *user)
{
    Server::userList.push_back(user);
}

void Server::addUnsentMsg(const std::string &name, const std::string &src, const std::string &msg)
{
    for (User *i : Server::userList)
    {
        if (i->name == name)
        {
            i->unsentMsg[src] += msg;
            break;
        }
    }
}

void Server::resetUnsentMsg(const std::string &name)
{
    for (auto &i : userList)
    {
        if (i->name == name)
        {
            i->unsentMsg = json::object();
        }
    }
}

void Server::addFile(const std::string &name, File *file)
{
    for (auto &i : userList)
    {
        if (i->name == name)
        {
            i->files.push_back(file);
        }
    }
}
