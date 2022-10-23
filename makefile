CC = gcc
OBJECTDIR = object
SOURCEDIR = source
HEADERDIR = include
BINARYDIR = binary
LIBDIR = lib
SOURCEFILES = $(wildcard $(SOURCEDIR)/*.c)										#lista de todos os .c na pasta source
OBJECTFILES = $(patsubst $(SOURCEDIR)/%.c, $(OBJECTDIR)/%.o, $(SOURCEFILES))	#substituir os nomes dos .c para .o
BASE_OBJECTS = data.o entry.o node-private.o tree.o message-private.o serialization.o

data.o = $(HEADERDIR)/data.h
entry.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h
tree.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/node-private.h $(HEADERDIR)/tree-private.h $(HEADERDIR)/tree.h
serialization.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/serialization.h
message-private.o = $(HEADERDIR)/message-private.h
sdmessage.pb-c.o = $(HEADERDIR)/sdmessage.pb-c.h

tree_skel.o = $(HEADERDIR)/tree_skel.h
network_server.o = $(HEADERDIR)/network_server.h $(HEADERDIR)/message-private.h 
tree_server.o = $(HEADERDIR)/network_server.h

client_stub.o = $(HEADERDIR)/client_stub.h $(HEADERDIR)/client_stub-private.h $(HEADERDIR)/network_client.h
network_client.o = $(HEADERDIR)/network_client.h $(HEADERDIR)/client_stub-private.h $(HEADERDIR)/message-private.h
tree_client.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/client_stub.h

all_objects: tree_client tree_server

%.o: $(SOURCEDIR)/%.c $($@)
	$(CC) -g -c $< -I $(HEADERDIR) -o $(OBJECTDIR)/$@

# target necessario pois o target sdmessage.pb-c.c coloca o .c e .h do sdmessage.pb-c na mesma folder
sdmessage.pb-c.o: sdmessage.pb-c.c $($@)
	$(CC) -g -c $(HEADERDIR)/$< -I $(HEADERDIR) -o $(OBJECTDIR)/$@

sdmessage.pb-c.c: sdmessage.proto
	/usr/local/bin/protoc-c ./sdmessage.proto --c_out=./$(HEADERDIR)

client-lib.o: client_stub.o network_client.o sdmessage.pb-c.o
	ld -r $(OBJECTDIR)/client_stub.o $(OBJECTDIR)/network_client.o $(OBJECTDIR)/sdmessage.pb-c.o -o $(LIBDIR)/$@

tree_server: $(BASE_OBJECTS) tree_skel.o network_server.o tree_server.o
	$(CC) -g $(addprefix $(OBJECTDIR)/,$(BASE_OBJECTS)) $(OBJECTDIR)/tree_server.o $(OBJECTDIR)/sdmessage.pb-c.o /usr/local/lib/libprotobuf-c.a $(OBJECTDIR)/network_server.o $(OBJECTDIR)/network_client.o $(OBJECTDIR)/tree_skel.o -o $(BINARYDIR)/tree_server

tree_client: $(BASE_OBJECTS) client-lib.o tree_client.o
	$(CC) -g $(addprefix $(OBJECTDIR)/,$(BASE_OBJECTS)) $(OBJECTDIR)/tree_client.o $(LIBDIR)/client-lib.o /usr/local/lib/libprotobuf-c.a -o $(BINARYDIR)/tree_client

clean:
	rm -f $(OBJECTFILES) $(OBJECTDIR)/sdmessage.pb-c.o $(LIBDIR)/client-lib.o $(BINARYDIR)/tree_server $(BINARYDIR)/tree_client
