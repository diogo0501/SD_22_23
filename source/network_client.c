/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

#include "client_stub-private.h"
#include "network_client.h"
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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
	if(sockfd = socket(AF_INET, SOCK_STREAM, 0) < 0) {
		perror("Couldnt create socket");
		return -1;
	}

	server.sin_port = htons(rtree->port);
	server.sin_family = AF_INET;

	inet_pton(AF_INET, rtree->ip, &server.sin_addr);

	if(connect(sockfd, (struct sockaddr *) &server, sizeof(server) < 0)) {
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
   	int sockfd = rtree->sockfd;
    unsigned len;
    len = message_t__get_packed_size(msg);
    uint8_t *buf = malloc(len);
    if (buf == NULL)
    {
        close(sockfd);
        return NULL;
    }

    message_t__pack(msg, buf);
    int nbytes;

    uint8_t *buf1 = malloc(len);
    unsigned lenNet = htonl(len);
    memcpy(buf1, &lenNet, sizeof(unsigned));
    if ((nbytes = send_all(rtree->sockfd, buf1, sizeof(unsigned))) == -1)
    {
        perror("Couldnt send data to the server");
        close(rtree->sockfd);
        free(buf);
        free(buf1);
        return 0;
    }

    if ((nbytes = send_all(sockfd, buf, len)) == -1)
    {
        perror("Couldnt send data to the server");
        close(sockfd);
        free(buf);
        free(buf1);
        return NULL;
    }
    free(buf);
    free(buf1);


    uint8_t *resp = malloc(2048);		//valor arbitrario
    unsigned *msgLen = malloc(sizeof(unsigned));
    if ((nbytes = receive_all(sockfd, (uint8_t *)msgLen, sizeof(unsigned))) == -1)
    {
        perror("Couldnt receive data from the server");
        close(sockfd);
        free(resp);
        free(msgLen);
        return NULL;
    };
    *msgLen = ntohl(*msgLen);
    if (*msgLen > 0 && (nbytes = receive_all(sockfd, resp, *msgLen)) == -1)
    {
        perror("Couldnt receive data from the server");
        close(sockfd);
        free(resp);
        free(msgLen);
        return NULL;
    };
    MessageT *recv_msg = message_t__unpack(NULL, nbytes, resp);
    free(resp);
    if (recv_msg == NULL)
    {
        message_t__free_unpacked(recv_msg, NULL);
        fprintf(stdout, "error unpacking message\n");
        close(sockfd);
        free(msgLen);
        return NULL;
    }
    free(msgLen);
    return recv_msg;

}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t * rtree){
    return close(rtree->sockfd);
}
