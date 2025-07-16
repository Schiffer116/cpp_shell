run: shell
	./shell

build: src/*.cpp
	g++ -std=c++17 -g src/*.cpp -o shell -lreadline

clean:
	rm -f shell

