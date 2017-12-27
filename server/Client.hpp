//
// Created by 徐韧喆 on 27/12/2017.
//
#ifndef SERVER_CLIENT_HPP
#define SERVER_CLIENT_HPP

#include <netinet/in.h>

class Client
{
public:
    struct sockaddr_in addr;
    int connfd;
    int uid;
    char name[32];

    void printAddress();
};

static unsigned int cli_count = 0;
#define MAX_CLIENTS 100

extern Client *clients[];

void queueAdd(Client *cl);
void queueDelete(int uid);

#endif