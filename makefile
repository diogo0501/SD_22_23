CC = gcc
OBJECTDIR = object
SOURCEDIR = source
HEADERDIR = include
BINARYDIR = binary
SOURCEFILES = $(wildcard $(SOURCEDIR)/*.c)										#lista de todos os .c na pasta source
OBJECTFILES = $(patsubst $(SOURCEDIR)/%.c, $(OBJECTDIR)/%.o, $(SOURCEFILES))	#substituir os nomes dos .c para .o
OBJECTS = data.o entry.o node-private.o tree.o serialization.o

data.o = $(HEADERDIR)/data.h
entry.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h
tree.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/node-private.h $(HEADERDIR)/tree-private.h $(HEADERDIR)/tree.h
serialization.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/serialization.h

sdmessage.pb-c.o = sdmessage.pb-c.h
message-private.o = message-private.h

table_skel.o = tree_skel.h
network_server.o = network_server.h message-private.h 
tree_server.o = network_server.h

client_stub.o = client_stub.h client_stub-private.h network_client.h
network_client.o = network_client.h client_stub-private.h message-private.h
tree_client.o = client_stub.h

all_objects: tree_client tree_server

%.o: $(SOURCEDIR)/%.c $($@)
	$(CC) -c $< -I $(HEADERDIR) -o $(OBJECTDIR)/$@

client-lib.o: sdmessage.pb-c.o network_client.o client_stub.o
	ld -r ./$(OBJECTDIR)/sdmessage.pb-c.o ./$(OBJECTDIR)/network_client.o ./$(OBJECTDIR)/client_stub.o -o ./lib/$@

tree_server: $(OBJECTS) client-lib.o tree_client.o
	$(CC) ./$(OBJECTDIR)/sdmessage.pb-c.o /usr/local/lib/libprotobuf-c.a ./$(OBJECTDIR)/tree_server.o ./$(OBJECTDIR)/network_server.o ./$(OBJECTDIR)/network_client.o ./$(OBJECTDIR)/tree_skel.o -o $(BINARYDIR)/tree_server

tree_client: $(OBJECTS) network_server.o tree_skel.o tree_client.o
	$(CC) /usr/local/lib/libprotobuf-c.a ./$(OBJECTDIR)/tree_client.o ./lib/client-lib.o -o $(BINARYDIR)/tree_client

sdmessage.pb-c.c: sdmessage.proto
	/usr/local/bin/protoc-c ./sdmessage.proto --c_out=./$(HEADERDIR)

sdmessage.pb-c.o: sdmessage.pb-c.c $($@)
	$(CC) -I include -o object/$@ -c ./include/$<

clean:
	rm -f $(addprefix $(OBJECTDIR)/,$(OBJECTFILES)) lib/client_lib.o $(BINARYDIR)/tree_server $(BINARYDIR)/tree_client

#FALTA ADICIONAR DEPENDENCIAS DOS .o DOS NOVOS FICHEIROS DESTA FASE