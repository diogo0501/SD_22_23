/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

#include "message-private.h"
#include "client_stub-private.h"
#include "network_client.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t *rtree){

	if(rtree == NULL) {
		return -1;
	}

	struct sockaddr_in server;
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Couldnt create socket");
		return -1;
	}

	server.sin_port = htons(rtree->port);
	server.sin_family = AF_INET;

	inet_pton(AF_INET, rtree->ip, &server.sin_addr);

	if(connect(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0) {
		perror("Couldnt connect to server");
		close(sockfd);
		return -1;
	}

	rtree->sockfd = sockfd;
	return 0;

}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtree_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct message_t *network_send_receive(struct rtree_t * rtree, struct message_t *msg){

   //Obter o descritor da ligação (socket) da estrutura rtree_t;
   	int socket = rtree->sockfd;
    
    unsigned int packed_size;
    int nbytes;
    packed_size = message_t__get_packed_size(msg->recv_msg);
    uint8_t *msg_content_buf = malloc(packed_size);

    if (msg_content_buf == NULL) {
        close(socket);
        return NULL;
    }

    //Serializar a mensagem contida em msg;
    message_t__pack(msg->recv_msg, msg_content_buf);
    uint8_t *msg_size_buf = malloc(packed_size);
    unsigned net_byte_order_message_size = htonl(packed_size);
    memcpy(msg_size_buf, &net_byte_order_message_size, sizeof(unsigned));

    //Enviar a mensagem serializada para o servidor;
    if((nbytes = send_all(rtree->sockfd, msg_size_buf, sizeof(unsigned))) == -1) {
        perror("Couldnt send data to the server");
        close(rtree->sockfd);
        free(msg_content_buf);
        free(msg_size_buf);
        return 0;
    }

    if((nbytes = send_all(socket, msg_content_buf, packed_size)) == -1) {
        perror("Couldnt send data to the server");
        close(socket);
        free(msg_content_buf);
        free(msg_size_buf);
        return NULL;
    }

    free(msg_content_buf);
    free(msg_size_buf);

    uint8_t *resp = malloc(2048);		//valor arbitrario
    unsigned *received_message_size = malloc(sizeof(unsigned));

    if((nbytes = recv_all(socket, (uint8_t *)received_message_size, sizeof(unsigned))) == -1) {
        perror("Couldnt receive data from the server");
        close(socket);
        free(resp);
        free(received_message_size);
        return NULL;
    }

    *received_message_size = ntohl(*received_message_size);

    if((nbytes = recv_all(socket, resp, *received_message_size)) == -1 && *received_message_size > 0) {
        perror("Couldnt receive data from the server");
        close(socket);
        free(resp);
        free(received_message_size);
        return NULL;
    }

    //De-serializar a mensagem de resposta;
    MessageT *recv_msg = message_t__unpack(NULL, nbytes, resp);
    free(resp);

    if (recv_msg == NULL) {
        message_t__free_unpacked(recv_msg, NULL);
        close(socket);
        free(received_message_size);
        return NULL;
    }

    struct message_t *msg_wrapper = malloc(sizeof(struct message_t));
	msg_wrapper->recv_msg = recv_msg;
    free(received_message_size);

    return msg_wrapper;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t * rtree){
    return close(rtree->sockfd);
}
