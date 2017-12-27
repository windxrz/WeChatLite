#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include "actions.h"

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    /* Socket settings */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    /* Ignore pipe signals */
    signal(SIGPIPE, SIG_IGN);

    /* Bind */
    if (bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Socket binding failed");
        return 1;
    }

    /* Listen */
    if (listen(listenfd, 10) < 0)
    {
        perror("Socket listening failed");
        return 1;
    }

    printf("<[SERVER STARTED]>\n");

    /* Accept clients */
    while (1)
    {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &clilen);

        /* Check if max clients is reached */
        if ((cli_count + 1) == MAX_CLIENTS)
        {
            printf("<<MAX CLIENTS REACHED\n");
            printf("<<REJECT ");
            print_client_addr(cli_addr);
            printf("\n");
            close(connfd);
            continue;
        }

        /* Client settings */
        client_t *cli = (client_t *) malloc(sizeof(client_t));
        cli->addr = cli_addr;
        cli->connfd = connfd;
        cli->uid = uid++;
        sprintf(cli->name, "%d", cli->uid);

        /* Add client to the queue and fork thread */
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void *) cli);

        /* Reduce CPU usage */
        sleep(1);
    }
}