//
// Created by 徐韧喆 on 27/12/2017.
//

#include "Actions.hpp"
#include <zconf.h>
#include <pthread.h>
#include <iostream>
#include "json.hpp"
#include "Server.hpp"
using json = nlohmann::json;

void stripNewline(char *s)
{
    while (*s != '\0')
    {
        if (*s == '\r' || *s == '\n')
        {
            *s = '\0';
        }
        s++;
    }
}

void Actions::sendMessageByName(const std::string &msg, const std::string &name)
{
    for (auto *i : Server::userList)
    {
        if (i->name == name)
        {
            write(i->connfd, msg.c_str(), msg.length());
        }
    }
}

void Actions::sendMessageAll(const std::string &msg)
{
    for (auto *i : Server::userList)
    {
        write(i->connfd, msg.c_str(), msg.length());
    }
}


void Actions::sendMessageByConnfd(const std::string &msg, int connfd)
{
    write(connfd, msg.c_str(), msg.length());
}

void *Actions::handleClient(void *arg)
{
    auto *actions = new Actions();
    std::string buff_out;
    char buff_in[1024];
    ssize_t rlen;

    auto connfd = *((int *) arg);

    std::cout << "ACCEPT, connfd id = " << connfd << "\n";
    buff_out = "HELLO\r\n";
    actions->sendMessageByConnfd(buff_out, connfd);

    /* Receive input from client */
    while ((rlen = read(connfd, buff_in, sizeof(buff_in) - 1)) > 0)
    {
        buff_in[rlen] = '\0';
        buff_out[0] = '\0';
        stripNewline(buff_in);
        if (strlen(buff_in) == 0u)
        {
            continue;
        }

        auto msg = json::parse(buff_in);
        auto cmd = msg["cmd"];
        std::cout << cmd << std::endl;
        if (cmd == "quit")
        {
            actions->handleQuit(connfd);
            break;
        }
        if (cmd == "login")
        {
            actions->handleLogin(connfd, msg["username"], msg["password"]);
        } else if (cmd == "search")
        {
            actions->handleSearch(connfd);
        } else if (cmd == "add")
        {
            actions->sendMessageByConnfd("<< RECEIVE ADD\r\n", connfd);
        } else if (cmd == "ls")
        {
            actions->sendMessageByConnfd("<< RECEIVE LS\r\n", connfd);
        } else if (cmd == "chat")
        {
            actions->sendMessageByConnfd("<< RECEIVE CHAT\r\n", connfd);
        } else if (cmd == "sendmsg")
        {
            actions->sendMessageByConnfd("<< RECEIVE SENDMSG\r\n", connfd);
        } else if (cmd == "sendfile")
        {
            actions->sendMessageByConnfd("<< RECEIVE SENDFILE\r\n", connfd);
        } else if (cmd == "exit")
        {
            actions->sendMessageByConnfd("<< RECEIVE EXIT\r\n", connfd);
        } else if (cmd == "recvmsg")
        {
            actions->sendMessageByConnfd("<< RECEIVE RECVMSG\r\n", connfd);
        } else if (cmd == "recvfile")
        {
            actions->sendMessageByConnfd("<< RECEIVE RECVFILE\r\n", connfd);
        } else if (cmd == "profile")
        {
            actions->sendMessageByConnfd("<< RECEIVE PROFILE\r\n", connfd);
        } else if (cmd == "sync")
        {
            actions->sendMessageByConnfd("<< RECEIVE SYNC\r\n", connfd);
        }
        else
        {
            actions->sendMessageByConnfd("<< FUCK YOU!", connfd);
        }
    }

    close(connfd);
    buff_out += "LEAVE, BYE\r\n";
    actions->sendMessageByConnfd(buff_out, connfd);

    std::cout << "LEAVE, connfd id = " << connfd << "\n";
    pthread_detach(pthread_self());

    return nullptr;
}

void Actions::handleSearch(int connfd)
{
    json result = json::object();
    for (auto &i : Server::userList)
    {
        result[i->name] = (i->connfd != -1);
    }
    this->sendMessageByConnfd(result.dump(), connfd);
}

void Actions::handleLogin(int connfd, const std::string &name, const std::string &password)
{
    json result = json::object();
    std::cout << "name : " << name << "\npassword : " << password << "\n";
    bool exist = false;
    for (auto &i : Server::userList)
    {
        if (i->name == name)
        {
            exist = true;
            if (i->password == password)
            {
                result["status"] = "OK";
            }
            else
            {
                result["status"] = "FUCK";
            }
        }
    }
    if (!exist)
    {
        auto *user = new User(name, password);
        user->connfd = connfd;
        Server::addUser(user);
        result["status"] = "OK";
    }
    sendMessageByConnfd(result.dump(), connfd);
}

void Actions::handleQuit(int connfd)
{
    for (auto &i : Server::userList)
    {
        if (i->connfd == connfd)
        {
            i->connfd = -1;
        }
    }
}


