COMPILER = gcc
CFLAGS = -Isrc/headers
OFILES = bin/objectfiles
EFILES = bin/executable

all: $(EFILES)/main $(EFILES)/testing

$(EFILES)/main: $(OFILES)/ $(EFILES)/ $(OFILES)/main.o $(OFILES)/bitbuffer.o $(OFILES)/commons.o $(OFILES)/huffmantree.o $(OFILES)/preprocessing.o $(OFILES)/storage.o
		$(COMPILER) -o $(EFILES)/main $(OFILES)/main.o $(OFILES)/bitbuffer.o $(OFILES)/commons.o $(OFILES)/huffmantree.o $(OFILES)/preprocessing.o $(OFILES)/storage.o

$(EFILES)/testing: $(OFILES)/ $(EFILES)/ $(OFILES)/testing.o $(OFILES)/bitbuffer.o $(OFILES)/commons.o $(OFILES)/huffmantree.o $(OFILES)/preprocessing.o $(OFILES)/storage.o
		$(COMPILER) -o $(EFILES)/testing $(OFILES)/testing.o $(OFILES)/bitbuffer.o $(OFILES)/commons.o $(OFILES)/huffmantree.o $(OFILES)/preprocessing.o $(OFILES)/storage.o



$(OFILES)/:
		mkdir -p $(OFILES)

$(EFILES)/:
		mkdir -p $(EFILES)

$(OFILES)/%.o: src/implementations/%.c $(OFILES)/
		$(COMPILER) -c $< -o $(OFILES)/$*.o $(CFLAGS)
