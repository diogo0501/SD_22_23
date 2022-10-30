/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

#include "network_server.h"
#include "message-private.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int server_sock;

int network_server_init(short port){

	struct sockaddr_in server;
	int sockfd;

	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0 ) {
		perror("Error on creating socket\n");
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
		perror("Error on setsockopt() function\n");
		close(sockfd);
		return -1;
	}
    		
	if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0) {
		perror("Error on binding\n");
		close(sockfd);
		return -1;
	};

	if (listen(sockfd, 0) < 0) {
		perror("Error on listening\n");
		close(sockfd);
		return -1;
	};

	printf("Socket listening...\n");

	server_sock = sockfd;
	
	tree_skel_init();
	
	return sockfd;
}

int network_main_loop(int listening_socket){

	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, (sig_t)network_server_close);
	
	int connsockfd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof((struct addrsock *)&client_addr);
	struct message_t *recv_msg_str;

	while ((connsockfd = accept(listening_socket,(struct sockaddr *) &client_addr, &client_len)) != -1) {
		
		recv_msg_str = network_receive(connsockfd);

		if(recv_msg_str == NULL) {
			free(recv_msg_str);
			close(connsockfd);	
			continue;
		}

		int op = invoke(recv_msg_str);

		int response;

		if(op == -1) {

			recv_msg_str->recv_msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
			recv_msg_str->recv_msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			recv_msg_str->recv_msg->datalength = 0;

			response = network_send(connsockfd,recv_msg_str);

		} else {

			response = network_send(connsockfd,recv_msg_str);
		}
		
		free(recv_msg_str);

		close(connsockfd);
	}

	return 0;
}

struct message_t *network_receive(int client_socket) {
	
	struct message_t *msg_wrapper = malloc(sizeof(struct message_t));
	int recv_bytes;
    unsigned len = 0;

	recv_bytes = recv_all(client_socket, (uint8_t *)&len, sizeof(unsigned));

    if (recv_bytes == -1) {
        perror("Error on receiving data from the client\n");
        close(client_socket);
        return NULL;
    }

    len = ntohl(len);
    uint8_t *res = malloc(len);

	recv_bytes = recv_all(client_socket, res, len);

    if (recv_bytes == -1 || len <= 0) {

        perror("Error on receiving data from the client\n");
        free(res);
        return NULL;
    }

    MessageT *response = message_t__unpack(NULL, recv_bytes, res);

	msg_wrapper->recv_msg = response;

    if (response == NULL) {
        message_t__free_unpacked(response, NULL);
        perror("Error unpacking message\n");
        free(res);
		return NULL;
    }

    free(res);

    return msg_wrapper;
}

int network_send(int client_socket, struct message_t *msg) {

	unsigned data_len,len;
	uint8_t *data_buf,*len_buf;
	int send_bytes;

	MessageT *message = msg->recv_msg;

    data_len = message_t__get_packed_size(message);
    data_buf = malloc(data_len);

    if (data_buf == NULL) {
        perror("Error when mallocing buffer\n");
        free(data_buf);
        close(client_socket);
        return -1;
    }

    message_t__pack(message, data_buf);

    len_buf = malloc(data_len);
    len = htonl(data_len);
    memcpy(len_buf, &len, sizeof(unsigned));

	send_bytes = send_all(client_socket, len_buf, sizeof(unsigned));
    if (send_bytes == -1) {
        perror("Error on sending data to client\n");
        message_t__free_unpacked(message, NULL);
        free(data_buf);
        free(len_buf);
        return -1;
    }

	send_bytes = send_all(client_socket, data_buf, data_len);
	if (send_bytes == -1) {
		perror("Error on sending data to client\n");
		message_t__free_unpacked(message, NULL);
		free(data_buf);
		free(len_buf);
		return -1;
	}

    message_t__free_unpacked(message, NULL);

    free(data_buf);
    free(len_buf);

    return 0;
}

int network_server_close() {
	
	tree_skel_destroy();
	
	close(server_sock);
	
	printf("Abnormal termination of the process! The socket is now closed\n");

	exit(-1);

}
