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
            if (i->connfd != -1)
            {
                write(i->connfd, msg.c_str(), msg.length());
            }
            else
            {
                Server::addUnsentMsg(i->name, name, msg);
            }
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
    if (connfd != -1)
    {
        write(connfd, msg.c_str(), msg.length());
    }
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

        std::cout << "$$" << buff_in << "$$\n";
        auto msg = json::parse(buff_in);
        auto cmd = msg["cmd"];
        std::cout << cmd << " " << connfd << std::endl;
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
            actions->handleAdd(connfd, msg["friend"]);
        } else if (cmd == "ls")
        {
            actions->handleLS(connfd);
        } else if (cmd == "chat")
        {
            actions->handleChat(connfd, msg["friend"]);
        } else if (cmd == "sendmsg")
        {
            actions->handleSendMsg(connfd, msg["msg"], msg["friend"]);
        } else if (cmd == "sendfile")
        {
            actions->sendMessageByConnfd("RECEIVE SENDFILE\r\n", connfd);
        } else if (cmd == "exit")
        {
            actions->handleExit(connfd);
        } else if (cmd == "recvmsg")
        {
            actions->handleRecvMsg(connfd);
        } else if (cmd == "recvfile")
        {
            actions->sendMessageByConnfd("RECEIVE RECVFILE\r\n", connfd);
        } else
        {
            actions->sendMessageByConnfd("FUCK YOU!", connfd);
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
    bool exist = false;
    for (auto &i : Server::userList)
    {
        if (i->name == name)
        {
            exist = true;
            if (i->password == password)
            {
                result["status"] = "OK";
                i->connfd = connfd;
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

void Actions::handleAdd(int connfd, const std::string &name)
{
    json result = json::object();
    bool exist = false;
    for (auto &i : Server::userList)
    {
        if (i->name == name)
        {
            exist = true;
            break;
        }
    }
    if (!exist)
    {
        result["status"] = "FUCK";
    }
    else
    {
        for (auto &i : Server::userList)
        {
            if (i->connfd == connfd)
            {
                i->addFriend(name);
            }
        }
        result["status"] = "OK";
    }
    sendMessageByConnfd(result.dump(), connfd);
}

void Actions::handleLS(int connfd)
{
    json result = json::object();
    User *current = nullptr;
    for (auto &i : Server::userList)
    {
        if (i->connfd == connfd)
        {
            current = i;
        }
    }
    for (auto &i : current->friends)
    {
        result[i] = false;
        for (auto &j : Server::userList)
        {
            if (j->name == i && j->connfd != -1)
            {
                result[i] = true;
            }
        }
    }
    sendMessageByConnfd(result.dump(), connfd);
}

void Actions::handleChat(int connfd, const std::string &name)
{
    json result = json::object();
    User *current = nullptr;
    for (auto &i : Server::userList)
    {
        if (i->connfd == connfd)
        {
            current = i;
        }
    }
    result["status"] = "FUCK";
    if (current != nullptr)
    {
        bool exist = false;
        for (auto &i : current->friends)
        {
            if (i == name)
            {
                exist = true;
                break;
            }
        }
        if (exist)
        {
            current->current = name;
            result["status"] = "OK";
        }
    }
    sendMessageByConnfd(result.dump(), connfd);
}

void Actions::handleSendMsg(int connfd, const std::string &msg, const std::string &name)
{
    json result = json::object();
    std::string src = "-----";
    for (auto &i : Server::userList)
    {
        if (i->connfd == connfd)
        {
            result["friend"] = i->name;
            src = i->name;
        }
    }
    bool chatting = false;
    for (auto &i : Server::userList)
    {
        if (i->name == name && i->current == src)
        {
            chatting = true;
            break;
        }
    }
    if (chatting)
    {
        result["msg"] = msg;
        sendMessageByName(result.dump(), name);
    } else
    {
        Server::addUnsentMsg(name, src, msg);
    }
}

void Actions::handleRecvMsg(int connfd)
{
    json result = json::object();
    User* current = nullptr;
    for (auto &i : Server::userList)
    {
        if (i->connfd == connfd)
        {
            current = i;
            break;
        }
    }
    if (current != nullptr)
    {
        result["msg"] = current->unsentMsg;
        std::cout << result.dump() << std::endl;
        sendMessageByConnfd(result.dump(), connfd);
        Server::resetUnsentMsg(current->name);
    }
}

void Actions::handleExit(int connfd)
{
    json result = json::object();
    for (auto &i : Server::userList)
    {
        if (i->connfd == connfd)
        {
            i->current = "";
        }
    }
    result["status"] = "FUCK";
    sendMessageByConnfd(result.dump(), connfd);
}
