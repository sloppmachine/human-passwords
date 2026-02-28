COMPILER = gcc
CFLAGS = -Iheaders
OFILES = bin/objectfiles
EFILES = bin/executable

all: $(EFILES)/main $(EFILES)/testing

$(EFILES)/main: $(OFILES)/ $(EFILES)/ $(OFILES)/main.o $(OFILES)/commons.o $(OFILES)/huffmantree.o $(OFILES)/preprocessing.o $(OFILES)/randomtest.o 
		$(COMPILER) -o $(EFILES)/main $(OFILES)/main.o $(OFILES)/commons.o $(OFILES)/huffmantree.o $(OFILES)/preprocessing.o $(OFILES)/randomtest.o

$(EFILES)/testing: $(OFILES)/ $(EFILES)/ $(OFILES)/testing.o $(OFILES)/commons.o $(OFILES)/huffmantree.o $(OFILES)/preprocessing.o $(OFILES)/randomtest.o 
		$(COMPILER) -o $(EFILES)/testing $(OFILES)/testing.o $(OFILES)/commons.o $(OFILES)/huffmantree.o $(OFILES)/preprocessing.o $(OFILES)/randomtest.o



$(OFILES)/:
		mkdir -p $(OFILES)

$(EFILES)/:
		mkdir -p $(EFILES)

$(OFILES)/%.o: src/%.c $(OFILES)/
		$(COMPILER) -c $< -o $(OFILES)/$*.o $(CFLAGS)
