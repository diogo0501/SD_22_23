CC = gcc
OBJECTDIR = object
SOURCEDIR = source
HEADERDIR = include
BINARYDIR = binary
SOURCEFILES = $(wildcard $(SOURCEDIR)/*.c)										#lista de todos os .c presentes na pasta source
OBJECTFILES = $(patsubst $(SOURCEDIR)/%.c, $(OBJECTDIR)/%.o, $(SOURCEFILES))	#substituir os nomes dos .c para .o

data.o = $(HEADERDIR)/data.h
entry.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h
tree.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/node-private.h $(HEADERDIR)/tree-private.h $(HEADERDIR)/tree.h
serialization.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/serialization.h
test_data.o = $(HEADERDIR)/data.h
test_entry.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h
test_tree.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/tree.h $(HEADERDIR)/tree-private.h $(HEADERDIR)/node-private.h
#test_serialization.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/serialization.h

all_objects: $(OBJECTFILES)
	$(CC) $^ -o $@ 

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.c
	$(CC) -c $< -I $(HEADERDIR) -o $@

#targets para gerar os executaveis para testagem
test_data: $(OBJECTDIR)/test_data.o $(OBJECTDIR)/data.o
	$(CC) $(OBJECTDIR)/test_data.o $(OBJECTDIR)/data.o -o $(BINARYDIR)/test_data
test_entry: $(OBJECTDIR)/test_entry.o $(OBJECTDIR)/entry.o
	$(CC) $(OBJECTDIR)/test_entry.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o -o $(BINARYDIR)/test_entry
#test_list: $(OBJECTDIR)/test_list.o $(OBJECTDIR)/list.o $(OBJECTDIR)/data.o $(OBJECTDIR)/entry.o
#	$(CC) $(OBJECTDIR)/test_list.o $(OBJECTDIR)/list.o $(OBJECTDIR)/data.o $(OBJECTDIR)/entry.o -o $(BINARYDIR)/test_list
test_tree: $(OBJECTDIR)/test_tree.o $(OBJECTDIR)/tree.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o $(OBJECTDIR)/node-private.o
	$(CC) $(OBJECTDIR)/test_tree.o $(OBJECTDIR)/tree.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o $(OBJECTDIR)/node-private.o -o $(BINARYDIR)/test_tree
#test_serialization: $(OBJECTDIR)/test_serialization.o $(OBJECTDIR)/serialization.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o
#	$(CC) $(OBJECTDIR)/test_serialization.o $(OBJECTDIR)/serialization.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o -o $(BINARYDIR)/test_serialization

#INCOMPLETO , so para ter um makefile
