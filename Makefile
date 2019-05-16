CPPFLAGS = -I.
CFLAGS   = -Wall -Wextra -pedantic -D_SVID_SOURCE -std=c99
LDLIBS   = -lpthread

BIN = controler

.PHONY: all
all: $(BIN)

controler: fish.o vue.o aquarium.o

Display/build:
	mkdir Display/build

display: Display/build
	javac -d Display/build Display/DisplayConsole.java Display/Display.java Display/Vue.java Display/Fish.java

run: display
	java -ea -cp Display/build DisplayConsole

test:
	javac -d client/ client/Client.java Display/Fish.java
	java -ea -cp client/ Client

clean:
	rm -f controler *.o Display/build/*
