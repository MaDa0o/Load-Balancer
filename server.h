#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <thread>
#include <vector>

#define SERVERIP 	"0.0.0.0"
#define BEIP		"127.0.0.1"
#define LBPORT		80
#define BEPORT		8080

void handleClient(int);

int main(int, char**);