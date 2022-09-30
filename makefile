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
test_data.o = $(HEADERDIR)/data.h
test_entry.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h
test_tree.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/tree.h $(HEADERDIR)/tree-private.h $(HEADERDIR)/node-private.h
#test_serialization.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/serialization.h

all_objects: $(OBJECTS) test_data test_entry test_tree

%.o: $(SOURCEDIR)/%.c $($@)
	$(CC) -c $< -I $(HEADERDIR) -o $(OBJECTDIR)/$@

#targets para gerar os executaveis para testagem
test_data: test_data.o data.o
	$(CC) $(OBJECTDIR)/test_data.o $(OBJECTDIR)/data.o -o $(BINARYDIR)/test_data
test_entry: test_entry.o entry.o
	$(CC) $(OBJECTDIR)/test_entry.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o -o $(BINARYDIR)/test_entry
test_tree: test_tree.o tree.o entry.o data.o node-private.o
	$(CC) $(OBJECTDIR)/test_tree.o $(OBJECTDIR)/tree.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o $(OBJECTDIR)/node-private.o -o $(BINARYDIR)/test_tree
#test_serialization: $(OBJECTDIR)/test_serialization.o $(OBJECTDIR)/serialization.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o
#	$(CC) $(OBJECTDIR)/test_serialization.o $(OBJECTDIR)/serialization.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o -o $(BINARYDIR)/test_serialization

clean:
	rm -f $(OBJECTFILES) $(BINARYDIR)/test_data $(BINARYDIR)/test_entry $(BINARYDIR)/test_tree
#INCOMPLETO , falta tirar os comments referentes ao serialization
