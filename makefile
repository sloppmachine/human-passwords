COMPILER = gcc
COMPILATIONFLAGS = -Isrc/headers
LINKINGFLAGS = -lm
OFILES = bin/objectfiles
EFILES = bin/executable

all: $(EFILES)/human-passwords

$(EFILES)/human-passwords: $(OFILES)/ $(EFILES)/ $(OFILES)/main.o $(OFILES)/bitbuffer.o $(OFILES)/commons.o $(OFILES)/huffmantree.o $(OFILES)/preprocessing.o $(OFILES)/seeds.o $(OFILES)/stringbuilder.o $(OFILES)/storage.o
		$(COMPILER) -o $(EFILES)/human-passwords $(OFILES)/main.o $(OFILES)/bitbuffer.o $(OFILES)/commons.o $(OFILES)/huffmantree.o $(OFILES)/preprocessing.o $(OFILES)/seeds.o $(OFILES)/stringbuilder.o $(OFILES)/storage.o $(LINKINGFLAGS)


$(OFILES)/:
		mkdir -p $(OFILES)

$(EFILES)/:
		mkdir -p $(EFILES)

$(OFILES)/%.o: src/implementations/%.c $(OFILES)/
		$(COMPILER) -c $< -o $(OFILES)/$*.o $(COMPILATIONFLAGS)
