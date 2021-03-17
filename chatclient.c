#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#define BUFFER_SZ 1029

static int killflag = 0;

char *_strdup(const char *s) {
    size_t size = strlen(s) + 1;
    char *p = malloc(size);
    if (p) {
        memcpy(p, s, size);
    }
    return p;
}

void * handleRecieving(void * sockID){

	int clientSocket = (int) sockID;

	char *errorMsg = "Incorrect Password";

	int val = 1;
	while(val){
		char data[BUFFER_SZ];
		int rlen = read(clientSocket, data, sizeof(data));
		data[rlen] = '\0';
		printf("%s\n",data);
		if (strcmp(data,errorMsg) == 0) {
			val = 0;
			killflag = 1;
			pthread_exit(NULL);
		}
	}
}

int main(int argc, char *argv[]) 
{
	char *username;
	char *password;

	int opt;
    int PORT;

	while(1) {
        int long_index = 0;
        static struct option long_options[] = {
            {"port", required_argument, NULL,  'p' },
			{"username", required_argument, NULL,  'u' },
			{"passcode", required_argument, NULL,  'a' },
			{"host", required_argument, NULL,  'h' },
			{"join", no_argument, NULL,  'j' },
            {0,0,0,0}
        };
        opt = getopt_long_only(argc, argv, "", long_options, &long_index);

        if (opt == -1) break;

		switch(opt)  
		{    
			case 'p':
				PORT = atoi(optarg);
				break;
			case 'u':
				username = optarg;
				break;
			case 'a':
				password = optarg;
				break;
			case 'h':
				break;
			case 'j':
				break;
		}  
	}

	char user_pass[BUFFER_SZ];
	sprintf(user_pass, "%s %s", username, password);

	int sock;
	int valread;
	struct sockaddr_in serv_addr; 
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	if (PORT) {
        serv_addr.sin_port = htons(PORT);
    } else {
        serv_addr.sin_port = htons(5001);
    }
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
		printf("\nConnection Failed \n"); 
		return -1; 
	}

	if (write(sock , user_pass , strlen(user_pass)) < 0) {
		puts("Connection Error (username & password)");
		return -1;
	}

	pthread_t receiveThread;
	pthread_create(&receiveThread, NULL, handleRecieving, (void *) sock );
	printf("pepe\n");

	int connected = 1;

	while (connected) {
		if (killflag == 1) {
			connected = 0;
			continue;
		}
		
		
		char input[BUFFER_SZ-5];
		fgets(input,BUFFER_SZ-5,stdin);

		char input_cpy[BUFFER_SZ-5];
        strcpy(input_cpy,input);

		if (input[0] == ':') {
            char *command;
            command = strtok(input_cpy,"\n");
			if (strcmp(command,":Exit")==0) {
				char buff[BUFFER_SZ];
				sprintf(buff, "[%s] %s",username, input);
				write(sock,buff,sizeof(buff));
				printf("You have left the chatroom.\n");
				connected = 0;
				continue;
			} else {
				char buff[BUFFER_SZ];
				sprintf(buff, "[%s] %s",username, input);
				write(sock,buff,sizeof(buff));
			}
		} else {
			char buff[BUFFER_SZ];
			sprintf(buff, "[%s] %s",username, input);
			write(sock,buff,sizeof(buff));
		}
		*input = NULL;
	}

	// Close socket and kill thread
	pthread_cancel(receiveThread);
    close(sock);

	return 0;
} 
