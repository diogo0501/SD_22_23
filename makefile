CC = gcc
OBJECTDIR = object
SOURCEDIR = source
HEADERDIR = include
BINARYDIR = binary
SOURCEFILES = $(wildcard $(SOURCEDIR)/*.c)										#lista de todos os .c na pasta source
OBJECTFILES = $(patsubst $(SOURCEDIR)/%.c, $(OBJECTDIR)/%.o, $(SOURCEFILES))	#substituir os nomes dos .c para .o
OBJECTS = data.o entry.o node-private.o tree.o message-private.o serialization.o

data.o = $(HEADERDIR)/data.h
entry.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h
tree.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/node-private.h $(HEADERDIR)/tree-private.h $(HEADERDIR)/tree.h
serialization.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/serialization.h

sdmessage.pb-c.o = $(HEADERDIR)/sdmessage.pb-c.h
message-private.o = $(HEADERDIR)/message-private.h

tree_skel.o = $(HEADERDIR)/tree_skel.h
network_server.o = $(HEADERDIR)/network_server.h $(HEADERDIR)/message-private.h 
tree_server.o = $(HEADERDIR)/network_server.h

client_stub.o = $(HEADERDIR)/client_stub.h $(HEADERDIR)/client_stub-private.h $(HEADERDIR)/network_client.h
network_client.o = $(HEADERDIR)/network_client.h $(HEADERDIR)/client_stub-private.h $(HEADERDIR)/message-private.h
tree_client.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/client_stub.h

all_objects: tree_client tree_server

%.o: $(SOURCEDIR)/%.c $($@)
	$(CC) -c $< -I $(HEADERDIR) -o $(OBJECTDIR)/$@

client-lib.o: sdmessage.pb-c.o network_client.o client_stub.o
	ld -r ./object/sdmessage.pb-c.o ./object/network_client.o ./object/client_stub.o -o ./lib/$@

tree_server: $(OBJECTS) tree_server.o tree_skel.o network_server.o
	$(CC) $(addprefix $(OBJECTDIR)/,$(OBJECTS)) ./object/sdmessage.pb-c.o /usr/local/lib/libprotobuf-c.a ./object/tree_server.o ./object/network_server.o ./object/network_client.o ./object/tree_skel.o -o $(BINARYDIR)/tree_server

tree_client: $(OBJECTS) tree_client.o client-lib.o
	$(CC) $(addprefix $(OBJECTDIR)/,$(OBJECTS)) ./lib/client-lib.o object/tree_client.o /usr/local/lib/libprotobuf-c.a -o binary/tree_client

sdmessage.pb-c.c: sdmessage.proto
	/usr/local/bin/protoc-c ./sdmessage.proto --c_out=./$(HEADERDIR)

sdmessage.pb-c.o: sdmessage.pb-c.c $($@)
	$(CC) -I include -o object/$@ -c ./include/$<

clean:
	rm -f $(addprefix $(OBJECTDIR)/,$(OBJECTFILES)) lib/client_lib.o $(BINARYDIR)/tree_server $(BINARYDIR)/tree_client

#FALTA ADICIONAR DEPENDENCIAS DOS .o DOS NOVOS FICHEIROS DESTA FASE