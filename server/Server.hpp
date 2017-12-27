//
// Created by 徐韧喆 on 27/12/2017.
//

#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <string>
#include <vector>

class User
{
public:
    std::string name, password;
    std::string unsentMsg;
    int connfd;
    User();
    User(const std::string &, const std::string &);
};

class Server
{
public:
    static std::vector<User *> userList;
    static void addUser(User*);
    static void addUnsentMsg(std::string, std::string);
    static std::string getUnsentMsg(std::string);
};


#endif //SERVER_SERVER_HPP
