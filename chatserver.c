#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <getopt.h>

#define BUFFER_SZ 2048
#define MAX_CLIENTS 100

static _Atomic unsigned int cli_count = 0;
static int UID = 10;

/* Client structure */
typedef struct {
    struct sockaddr_in addr; /* Client remote address */
    int connfd;              /* Connection file descriptor */
    int client_uid;
    char *username;                 /* Client unique identifier */
} client_t;

client_t *clients[100];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

char *_strdup(const char *s) {
    size_t size = strlen(s) + 1;
    char *p = malloc(size);
    if (p) {
        memcpy(p, s, size);
    }
    return p;
}

/* Add client to queue */
void  queue_add(client_t *cl){
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!clients[i]) {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Delete client from queue */
void queue_delete(int uid){
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->client_uid == uid) {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Send message to all clients but the sender */
void send_message(char *s, int uid){
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->client_uid != uid) {
                if (write(clients[i]->connfd, s, strlen(s)) < 0) {
                    perror("Write to descriptor failed");
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Send message to all clients */
void send_message_all(char *s){
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i <MAX_CLIENTS; ++i){
        if (clients[i]) {
            if (write(clients[i]->connfd, s, strlen(s)) < 0) {
                perror("Write to descriptor failed");
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Send message to sender */
void send_message_self(const char *s, int connfd){
    if (write(connfd, s, strlen(s)) < 0) {
        perror("Write to descriptor failed");
        exit(-1);
    }
}

/* Send message to client */
void send_message_client(char *s, int uid){
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i){
        if (clients[i]) {
            if (clients[i]->client_uid == uid) {
                if (write(clients[i]->connfd, s, strlen(s))<0) {
                    perror("Write to descriptor failed");
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Strip CRLF */
void strip_newline(char *s){
    while (*s != '\0') {
        if (*s == '\r' || *s == '\n') {
            *s = '\0';
        }
        s++;
    }
}

/* Print ip address */
void print_client_addr(struct sockaddr_in addr){
    printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

/* Handle all communication with the client */
void *handle_client(void *arg){
    char buff_in[BUFFER_SZ / 2];
    int rlen;

    cli_count++;
    client_t *cli = (client_t *)arg;

    print_client_addr(cli->addr);
    printf(" << accepted");
    printf(" and referenced by %s\n", cli->username);

    char userBuff[BUFFER_SZ];
    userBuff[0] = '\0';
    sprintf(userBuff, "<< %s has joined\r\n", cli->username);
    send_message_all(userBuff);

    char *welcome = "Welcome to the chatroom. Type a message below:\n";
    send_message_client(welcome, cli->client_uid);

    /* Receive input from client */
    while ((rlen = read(cli->connfd, buff_in, sizeof(buff_in) - 1)) > 0) {
        buff_in[rlen] = '\0';
        char buff_out[BUFFER_SZ];
        buff_out[0] = '\0';
        strip_newline(buff_in);
        

        /* Ignore empty buffer */
        if (!strlen(buff_in)) {
            continue;
        }

        char buff_in_cpy[BUFFER_SZ];
        strcpy(buff_in_cpy,buff_in);

        char *tok_user = strtok(buff_in_cpy, " ");
        char *tok = strtok(NULL,"");

        /* Special options */
        if (tok[0] == ':') {
            char *command;
            command = strtok(tok," ");
            if (!strcmp(command, ":Exit")) {
                sprintf(buff_out, "%s has left the chatroom.", tok_user);
                send_message(buff_out, cli->client_uid);
                cli_count--;
                break;
            } else if (!strcmp(command, ":)")) {
                sprintf(buff_out, "%s [feeling happy]\r\n", tok_user);
                send_message_all(buff_out);
            } else if (!strcmp(command, ":(")) {
                sprintf(buff_out, "%s [feeling sad]\r\n", tok_user);
                send_message_all(buff_out);
            } else if (!strcmp(command, ":mytime")) {
                char timeStr[30];
                time_t call_time = (unsigned int)time(NULL);
                struct tm *loc_time;
                loc_time = localtime(&call_time);
                strftime(timeStr, 30, "My current time: %I:%M %p", loc_time);
                sprintf(buff_out, "%s %s",tok_user,timeStr);
                send_message_all(buff_out);
            } else if (!strcmp(command, ":+1hr")) {
                char timeStr[30];
                time_t call_time = (unsigned int)time(NULL);
                struct tm *loc_time;
                loc_time = localtime(&call_time);
                loc_time->tm_hour++;
                strftime(timeStr, 30, "My current time + 1hr: %I:%M %p", loc_time);
                sprintf(buff_out, "%s %s",tok_user,timeStr);
                send_message_all(buff_out);
            } else {
                send_message_self("<< unknown command\r\n", cli->connfd);
            }
        } else {
            /* Send message */
            sprintf(buff_out, "%s\r\n", buff_in);
            send_message(buff_out, cli->client_uid);
        }
    }

    /* Close connection */
    char buff_out[BUFFER_SZ];
    close(cli->connfd);

    /* Delete client from queue and yield thread */
    queue_delete(cli->client_uid);
    print_client_addr(cli->addr);
    printf(" << quit");
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char *argv[]){
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    int opt;
    int PORT;

	while(1) {
        int long_index = 0;
        static struct option long_options[] = {
            {"port", required_argument, NULL,  'p' },
            {"start", no_argument, NULL,  'a' },
            {0,0,0,0}
        };
        opt = getopt_long_only(argc, argv, "", long_options, &long_index);

        if (opt == -1) break;

		switch(opt)  
		{    
			case 'p':
				PORT = atoi(optarg);
				break;
            case 'a':
				break;
		}  
	}

    /* Socket settings */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (PORT) {
        serv_addr.sin_port = htons(PORT);
    } else {
        serv_addr.sin_port = htons(5001);
    }
    /* Ignore pipe signals */
    signal(SIGPIPE, SIG_IGN);

    /* Bind */
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Socket binding failed");
        return EXIT_FAILURE;
    }

    /* Listen */
    if (listen(listenfd, 10) < 0) {
        perror("Socket listening failed");
        return EXIT_FAILURE;
    }

    printf("<[ SERVER STARTED ON PORT %d ]>\n", PORT);

    /* Accept clients */
    while (1) {
        char buff_in[BUFFER_SZ / 2];
        int rlen = 0;
        char *username;
        char *password;
        char *PASS = "cs3251secret";

        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        rlen = read(connfd, buff_in, sizeof(buff_in) - 1);
        buff_in[rlen] = '\0';
        strip_newline(buff_in);

        username = strtok(buff_in, " ");
        password = strtok(NULL, " ");

        if (strcmp(password,PASS) != 0) {
            char *msg = "Incorrect Password";
            write(connfd, msg, strlen(msg));
            continue;
        }

        /* Client settings */
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->addr = cli_addr;
        cli->connfd = connfd;
        cli->client_uid = UID++;
        cli->username = username;

        /* Add client to the queue and fork thread */
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);
    }

    return EXIT_SUCCESS;
}