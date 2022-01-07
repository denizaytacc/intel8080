objs = main.cpp cpu.cpp
all: out

run:
	./a.out

out: $(objs)
	g++ $(objs)

clean:
	rm ./a.out