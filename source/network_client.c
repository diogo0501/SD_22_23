/**
Grupo 47
Diogo Fernandes, fc54458
Gonçalo Lopes, fc56334
Miguel Santos, fc54461
*/

#include "client_stub-private.h"
#include "network_client.h"
#include <stdlib.h>
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
struct message_t *network_send_receive(struct rtree_t * rtree,
                                       struct message_t *msg){


}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t * rtree){
    return close(rtree->sockfd);
}
