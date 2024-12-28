FLAGS = g++ -Wall -Ofast -std=c++23

GIT_SHA = $(shell git rev-parse --short HEAD 2>/dev/null)
TIMESTAMP = $(shell date +%Y%m%d_%H%M)

.PHONY: all alll main runtest gui clean cleano archive test

all: main runtest gui archive

test: all
	./runtest perft 400000

archive:
	@cp main ../archive/main_$(TIMESTAMP)_$(GIT_SHA)
	@cp runtest ../archive/runtest_$(TIMESTAMP)_$(GIT_SHA)
	@cp gui ../archive/gui_$(TIMESTAMP)_$(GIT_SHA)

alll:
	$(FLAGS) -o main main.cpp game.cpp board.cpp movegen.cpp search.cpp
	$(FLAGS) -o runtest runtest.cpp game.cpp board.cpp movegen.cpp search.cpp
	$(FLAGS) -o gui gui.cpp game.cpp board.cpp movegen.cpp search.cpp `pkg-config --cflags --libs gtk4`

main: main.o board.o movegen.o search.o
	$(FLAGS) -o main main.o board.o movegen.o search.o
runtest: runtest.o board.o movegen.o search.o
	$(FLAGS) -o runtest runtest.o board.o movegen.o search.o
gui: gui.o board.o movegen.o game.o search.o
	$(FLAGS) -o gui gui.o board.o movegen.o game.o search.o `pkg-config --cflags --libs gtk4`
clean: cleano
	rm main runtest gui
cleano:
	rm *.o

main.o: main.cpp
	$(FLAGS) -c main.cpp
game.o: game.cpp
	$(FLAGS) -c game.cpp
board.o: board.cpp
	$(FLAGS) -c board.cpp
movegen.o: movegen.cpp
	$(FLAGS) -c movegen.cpp
runtest.o: runtest.cpp
	$(FLAGS) -c runtest.cpp -o runtest.o 
search.o: search.cpp
	$(FLAGS) -c search.cpp
gui.o: gui.cpp
	$(FLAGS) -c gui.cpp `pkg-config --cflags --libs gtk4`