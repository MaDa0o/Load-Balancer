CC=g++
FLAGS=-o

All:
	$(CC) LoadBalancer.cpp $(FLAGS) lb
	$(CC) Backend.cpp $(FLAGS) be

clean:
	rm -rf be lb serv