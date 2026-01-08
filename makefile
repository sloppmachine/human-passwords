COMPILER = gcc
CFLAGS = -Iheaders
OFILES = bin/objectfiles
EFILES = bin/executable

$(OFILES)/%.o: src/%.c
		$(COMPILER) -c $< -o $@ $(CFLAGS)

$(EFILES)/main: $(OFILES)/main.o $(OFILES)/randomtest.o
		$(COMPILER) -o $(EFILES)/main $(OFILES)/main.o $(OFILES)/randomtest.o
