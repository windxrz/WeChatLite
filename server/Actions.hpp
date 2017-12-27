//
// Created by 徐韧喆 on 27/12/2017.
//

#ifndef SERVER_ACTIONS_HPP
#define SERVER_ACTIONS_HPP

class Actions
{
public:
    void sendMessage(char *s, int uid);
    void sendMessageAll(char *s);
    void sendMessageSelf(const char *s, int connfd);
    void sendMessageClient(char *s, int uid);
    void sendActiveClients(int connfd);
    static void *handleClient(void *arg);
};

#endif //SERVER_ACTIONS_HPP
