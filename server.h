#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

#define SERVERIP 	"0.0.0.0"
#define BEIP		"127.0.0.1"
#define LBPORT		80
#define BEPORT		8080
#define INTERVAL	10000
#define MXSIZE		8192

std::vector<int> servlist{8080,8081,8082};
std::vector<bool> health;
int servnum;
int servit=0;
std::mutex mtx;
std::mutex healthmtx;

bool getSetHealth(int,bool,bool);

int sendReqServer(int,int&,char*,int);

void healthCheck();

void handleClient(int);

int main(int, char**);