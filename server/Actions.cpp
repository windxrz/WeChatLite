//
// Created by 徐韧喆 on 27/12/2017.
//

#include "Actions.hpp"
#include "Client.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <zconf.h>
#include <pthread.h>

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

void Actions::sendMessage(char *s, int uid)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != nullptr)
        {
            if (clients[i]->uid != uid)
            {
                write(clients[i]->connfd, s, strlen(s));
            }
        }
    }
}

void Actions::sendMessageAll(char *s)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != nullptr)
        {
            write(clients[i]->connfd, s, strlen(s));
        }
    }
}

void Actions::sendMessageSelf(const char *s, int connfd)
{
    write(connfd, s, strlen(s));
}

void Actions::sendMessageClient(char *s, int uid)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != nullptr)
        {
            if (clients[i]->uid == uid)
            {
                write(clients[i]->connfd, s, strlen(s));
            }
        }
    }
}

void Actions::sendActiveClients(int connfd)
{
    int i;
    char s[64];
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != nullptr)
        {
            sprintf(s, "<<CLIENT %d | %s\r\n", clients[i]->uid, clients[i]->name);
            sendMessageSelf(s, connfd);
        }
    }
}

void *Actions::handleClient(void *arg)
{
    auto *actions = new Actions();
    char buff_out[1024];
    char buff_in[1024];
    int rlen;

    cli_count++;
    auto *cli = (Client *) arg;

    printf("<<ACCEPT ");
    cli->printAddress();
    printf(" REFERENCED BY %d\n", cli->uid);

    sprintf(buff_out, "HELLO %s\r\n", cli->name);
    actions->sendMessageAll(buff_out);

    /* Receive input from client */
    while ((rlen = read(cli->connfd, buff_in, sizeof(buff_in) - 1)) > 0)
    {
        buff_in[rlen] = '\0';
        buff_out[0] = '\0';
        stripNewline(buff_in);

        /* Ignore empty buffer */
        if (!strlen(buff_in))
        {
            continue;
        }

        /* Special options */
        if (buff_in[0] == '\\')
        {
            char *command, *param;
            command = strtok(buff_in, " ");
            if (!strcmp(command, "\\QUIT"))
            {
                break;
            } else if (!strcmp(command, "\\PING"))
            {
                actions->sendMessageSelf("<<PONG\r\n", cli->connfd);
            } else if (!strcmp(command, "\\NAME"))
            {
                param = strtok(nullptr, " ");
                if (param)
                {
                    char *old_name = strdup(cli->name);
                    strcpy(cli->name, param);
                    sprintf(buff_out, "<<RENAME, %s TO %s\r\n", old_name, cli->name);
                    free(old_name);
                    actions->sendMessageAll(buff_out);
                } else
                {
                    actions->sendMessageSelf("<<NAME CANNOT BE nullptr\r\n", cli->connfd);
                }
            } else if (!strcmp(command, "\\PRIVATE"))
            {
                param = strtok(nullptr, " ");
                if (param)
                {
                    int uid = atoi(param);
                    param = strtok(nullptr, " ");
                    if (param)
                    {
                        sprintf(buff_out, "[PM][%s]", cli->name);
                        while (param != nullptr)
                        {
                            strcat(buff_out, " ");
                            strcat(buff_out, param);
                            param = strtok(nullptr, " ");
                        }
                        strcat(buff_out, "\r\n");
                        actions->sendMessageClient(buff_out, uid);
                    } else
                    {
                        actions->sendMessageSelf("<<MESSAGE CANNOT BE nullptr\r\n", cli->connfd);
                    }
                } else
                {
                    actions->sendMessageSelf("<<REFERENCE CANNOT BE nullptr\r\n", cli->connfd);
                }
            } else if (!strcmp(command, "\\ACTIVE"))
            {
                sprintf(buff_out, "<<CLIENTS %d\r\n", cli_count);
                actions->sendMessageSelf(buff_out, cli->connfd);
                actions->sendActiveClients(cli->connfd);
            } else if (!strcmp(command, "\\HELP"))
            {
                strcat(buff_out, "\\QUIT     Quit chatroom\r\n");
                strcat(buff_out, "\\PING     Server test\r\n");
                strcat(buff_out, "\\NAME     <name> Change nickname\r\n");
                strcat(buff_out, "\\PRIVATE  <reference> <message> Send private message\r\n");
                strcat(buff_out, "\\ACTIVE   Show active clients\r\n");
                strcat(buff_out, "\\HELP     Show help\r\n");
                actions->sendMessageSelf(buff_out, cli->connfd);
            } else
            {
                actions->sendMessageSelf("<<UNKOWN COMMAND\r\n", cli->connfd);
            }
        } else
        {
            /* Send message */
            sprintf(buff_out, "[%s] %s\r\n", cli->name, buff_in);
            actions->sendMessage(buff_out, cli->uid);
        }
    }

    /* Close connection */
    close(cli->connfd);
    sprintf(buff_out, "<<LEAVE, BYE %s\r\n", cli->name);
    actions->sendMessageAll(buff_out);

    /* Delete client from queue and yeild thread */
    queueDelete(cli->uid);
    printf("<<LEAVE ");
    cli->printAddress();
    printf(" REFERENCED BY %d\n", cli->uid);
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

    return nullptr;
}

