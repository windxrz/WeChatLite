//
// Created by 徐韧喆 on 27/12/2017.
//

#include <cstdio>
#include "Client.hpp"


void Client::printAddress()
{
    printf("%d.%d.%d.%d",
           addr.sin_addr.s_addr & 0xFF,
           (addr.sin_addr.s_addr & 0xFF00) >> 8,
           (addr.sin_addr.s_addr & 0xFF0000) >> 16,
           (addr.sin_addr.s_addr & 0xFF000000) >> 24);
}

Client *ClientList::clients[MAX_CLIENTS];

void ClientList::add(Client *cl)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == nullptr)
        {
            clients[i] = cl;
            return;
        }
    }
}

void ClientList::remove(int uid)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != nullptr)
        {
            if (clients[i]->uid == uid)
            {
                clients[i] = nullptr;
                return;
            }
        }
    }
}