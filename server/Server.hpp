//
// Created by 徐韧喆 on 27/12/2017.
//

#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <string>
#include <vector>
#include "json.hpp"
using json = nlohmann::json;

class User
{
public:
    std::string name, password;
    std::vector<std::string> friends;
    int connfd;
    std::string current;
    json unsentMsg;

    User();
    User(const std::string &, const std::string &);

    void addFriend(const std::string &);
};

class Server
{
public:
    static std::vector<User *> userList;
    static void addUser(User*);
    static void addUnsentMsg(const std::string &, const std::string &, const std::string &);
    static void resetUnsentMsg(const std::string &);
};


#endif //SERVER_SERVER_HPP
