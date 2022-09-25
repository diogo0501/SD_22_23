CC = gcc
OBJECTDIR = object
SOURCEDIR = source
HEADERDIR = include
BINARYDIR = binary
SOURCEFILES = $(wildcard $(SOURCEDIR)/*.c)										#lista de todos os .c presentes na pasta source
OBJECTFILES = $(patsubst $(SOURCEDIR)/%.c, $(OBJECTDIR)/%.o, $(SOURCEFILES))	#substituir os nomes dos .c para .o

data.o = $(HEADERDIR)/data.h
entry.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h
list.o = $(HEADERDIR)/list-private.h $(HEADERDIR)/list.h
table.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/list-private.h $(HEADERDIR)/list.h $(HEADERDIR)/table-private.h $(HEADERDIR)/table.h
serialization.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/serialization.h

test_data.o = $(HEADERDIR)/data.h
test_entry.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h
test_list.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/list.h
test_table.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/table.h
test_serialization.o = $(HEADERDIR)/data.h $(HEADERDIR)/entry.h $(HEADERDIR)/serialization.h

all_objects: $(OBJECTFILES)
	$(CC) $^ -o $@ 

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.c
	$(CC) -c $< -I $(HEADERDIR) -o $@

#targets para gerar os executaveis para testagem
test_data: $(OBJECTDIR)/test_data.o $(OBJECTDIR)/data.o
	$(CC) $(OBJECTDIR)/test_data.o $(OBJECTDIR)/data.o -o $(BINARYDIR)/test_data
test_entry: $(OBJECTDIR)/test_entry.o $(OBJECTDIR)/entry.o
	$(CC) $(OBJECTDIR)/test_entry.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o -o $(BINARYDIR)/test_entry
test_list: $(OBJECTDIR)/test_list.o $(OBJECTDIR)/list.o $(OBJECTDIR)/data.o $(OBJECTDIR)/entry.o
	$(CC) $(OBJECTDIR)/test_list.o $(OBJECTDIR)/list.o $(OBJECTDIR)/data.o $(OBJECTDIR)/entry.o -o $(BINARYDIR)/test_list
test_table: $(OBJECTDIR)/test_table.o $(OBJECTDIR)/table.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o
	$(CC) $(OBJECTDIR)/test_table.o $(OBJECTDIR)/table.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o -o $(BINARYDIR)/test_table
test_serialization: $(OBJECTDIR)/test_serialization.o $(OBJECTDIR)/serialization.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o
	$(CC) $(OBJECTDIR)/test_serialization.o $(OBJECTDIR)/serialization.o $(OBJECTDIR)/entry.o $(OBJECTDIR)/data.o -o $(BINARYDIR)/test_serialization

