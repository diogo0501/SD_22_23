# SD_22_23

Observações:
- É utilizado o ficheiro objecto "tree.o" fornecido pelos professores, logo esse ficheiro já se encontra na pasta "object" e, consequentemente, não é eliminado quando se executa o target "clean" do makefile.
- Ao compilar o ficheiro sdmessage.proto o ficheiro .h e .c correspondentes ficam armazenados na mesma diretoria (include) pois não existe um comando para os separar em pastas diferentes.
- Sendo que não podemos alterar os ficheiros .h fornecidos e precisamos de usar o tipo definido 'message_t' nas funcões recorremos a usar um Wrapper para tal (como nos foi sugerido), wrapper que é definido no ficheiro message-private.h, isto faz com que, ao compilar o programa, ele mande o warning 'note: expected ‘struct message_t *’ but argument is of type ‘struct message_t *’' o que não faz muito sentido. No entanto, a solução para este problema é bem simples mas requeria
alterar o ficheiro tree_skel.h , adicionado aos includes: #include "message-private.h" e removendo assim qualquer warning, de salientar que, sendo um warning, não tem influencia no correr do programa.

Para correr:
- Verificar que existem pastas com os nomes "binary", "lib" e "object" dentro da pasta grupo47.
- Dentro da pasta grupo47, correr o comando make no terminal (que vai compilar todos os ficheiros, inclusive criar a library do cliente).
- Abrir dois *(ou mais) terminais na pasta binary, um para o cliente com o comando './tree-client hostname:port' e outro para o servidor com o comando './tree-server port nthreads'.

*este numero depende da macro definida em network_server "MAX_SOCKET" que por default é 4 (3 clientes), mas pode ser alterada.



