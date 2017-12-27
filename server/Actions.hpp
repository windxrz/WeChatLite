//
// Created by 徐韧喆 on 27/12/2017.
//

#ifndef SERVER_ACTIONS_HPP
#define SERVER_ACTIONS_HPP

#include <string>

class Actions
{
public:
    static void *handleClient(void *);

    void sendMessageByName(const std::string &, const std::string &);
    void sendMessageAll(const std::string &);
    void sendMessageByConnfd(const std::string &, int);

    void handleQuit(int);
    void handleLogin(int, const std::string &, const std::string &);
    void handleSearch(int);
};

#endif //SERVER_ACTIONS_HPP
