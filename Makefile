objs = main.cpp cpu.cpp display.cpp
all: out

run:
	./a.out

out: $(objs)
	g++ $(objs) -lSDL2

clean:
	rm ./a.out