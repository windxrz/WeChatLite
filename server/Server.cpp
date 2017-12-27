//
// Created by 徐韧喆 on 27/12/2017.
//

#include "Server.hpp"

User::User()
{
    this->unsentMsg = "";
    this->connfd = -1;
}

User::User(const std::string &name, const std::string &password) : User()
{
    this->name = name;
    this->password = password;
}

std::vector<User *> Server::userList;

void Server::addUser(User *user)
{
    Server::userList.push_back(user);
}

void Server::addUnsentMsg(std::string name, std::string msg)
{
    for (User *i : Server::userList)
    {
        if (i->name == name)
        {
            i->unsentMsg += msg + "\r\n";
            break;
        }
    }
}

std::string Server::getUnsentMsg(std::string name)
{
    for (auto &i : userList)
    {
        if (i->name == name)
        {
            std::string tmp = i->unsentMsg;
            i->unsentMsg = "";
            return tmp;
        }
    }
    return "";
}
