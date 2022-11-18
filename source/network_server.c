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
#include <poll.h>
#include <pthread.h>

#define MAX_SOCKETS 4 //Mock max client. Im not sure if there is a limit or what that limit is
#define TIME_OUT 10

int server_sock;

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
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
		
	return sockfd;
}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */
int network_main_loop(int listening_socket){

	int connsockfd, nfds, kfds;
	struct pollfd desc_set[MAX_SOCKETS];

	for (int i = 0; i < MAX_SOCKETS; i++) {
		desc_set[i].fd = -1;
	}

	desc_set[0].fd = listening_socket;
    desc_set[0].events = POLLIN;

	nfds = 1;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, (sig_t)network_server_close);
	
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof((struct addrsock *)&client_addr);
	struct message_t *recv_msg_str;

	while ((kfds = poll(desc_set, nfds, TIME_OUT)) >= 0) {
		
		if(kfds > 0) {
			if((desc_set[0].revents & POLLIN) && (nfds < MAX_SOCKETS)) {
				for(int i = 1; i < MAX_SOCKETS; i++) {
					if(desc_set[i].fd == -1) {
						//desc_set[0].fd == serversocket
						if((desc_set[i].fd = accept(desc_set[0].fd,(struct sockaddr *) &client_addr, &client_len)) > 0) {
							desc_set[i].events = POLLIN;
							desc_set[i].revents = 0;
							nfds++;
							break;
						}
					}
				}
				
			}

			//corre so para o nr de fds dos clientes, nao do servidor
			for(int i = 1; i < nfds; i++) {
				
				if(desc_set[i].fd == -1) {
					continue;
				}

				if(desc_set[i].revents & POLLIN) {
					recv_msg_str = network_receive(desc_set[i].fd);

					if(recv_msg_str == NULL) {
						free(recv_msg_str);
						close(desc_set[i].fd);
						desc_set[i].fd = -1;
						continue;	
					}

					int op = invoke(recv_msg_str);

					int response;

					if(op == -1) {

						recv_msg_str->recv_msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
						recv_msg_str->recv_msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
						recv_msg_str->recv_msg->datalength = 0;

						response = network_send(desc_set[i].fd,recv_msg_str);

					} else {

						response = network_send(desc_set[i].fd,recv_msg_str);
					}
					
					free(recv_msg_str);
					continue;
				}
				if(desc_set[i].events == POLL_HUP || 
					desc_set[i].events == POLL_ERR ) {
						close(desc_set[i].fd);
						desc_set[i].fd = -1;
						continue;

				}
			}
		}
			
	}

	return 0;
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct message_t *network_receive(int client_socket) {
	
	struct message_t *msg_wrapper = malloc(sizeof(struct message_t));
	int recv_bytes;
    unsigned len = 0;

	recv_bytes = recv_all(client_socket, (uint8_t *)&len, sizeof(unsigned));

    if (recv_bytes == -1) {struct message_t *recv_msg_str;
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

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
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
    }struct message_t *recv_msg_str;

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
	if (send_bytes == -1) {struct message_t *recv_msg_str;
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

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close() {
		
	close(server_sock);
	
	printf("Abnormal termination of the process! The socket is now closed\n");

	exit(-1);

}
