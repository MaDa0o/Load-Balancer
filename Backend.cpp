#include "server.h"

int main(int argc, char* argv[]){
	std::cout<<"Starting Backend server\n";
	char buffer[4096] = {0};

	//create
	int sock = socket(AF_INET,SOCK_STREAM,0);

	if(sock == -1){
		std::cerr<<"Error in creating socket\n";
		return 1;
	}

	//bind
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(BEPORT);
	inet_pton(AF_INET,SERVERIP, &hint.sin_addr);

	int bound = bind(sock,(sockaddr*)&hint,sizeof(hint));
	if(bound == -1){
		std::cerr<<"Error in binding socket\n";
		return 2;
	}

	//listen 
	if(listen(sock, SOMAXCONN) == -1){
		std::cerr<<"Can't listen to connections";
		return 3;
	}

	//accept
	sockaddr_in client;
	socklen_t clientsize = sizeof(client);

	int clientSocket;

	//server loop
	while(true){
		clientSocket = accept(sock, (sockaddr*)&client, &clientsize);

		if(clientSocket == -1){
			std::cerr<<"Problem with client connection";
			return 4;
		}

		std::cout<<"client connected\n";

		//send/receive
		int bytes_recv = recv(clientSocket,buffer,sizeof(buffer)-1,0);
		if(bytes_recv <= 0){
			std::cerr<<"Failed to receive data or client disconnected\n";
			return 5;
		}
		buffer[bytes_recv] = '\0';
		std::cout<<"Request Received:\n";
		std::cout<<buffer<<"\n";
		const char* response = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 12\r\n\r\n"
                "Hello World";
        send(clientSocket, response, strlen(response), 0);
        std::cout<<"Replied with a hello message\n";
        close(clientSocket);
	}
	//close
	close(sock);
	return 0;
}