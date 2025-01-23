#include "server.h"

void handleClient(int clientSocket){
	//send/receive
		char buffer[4096] = {0};
		int bytes_recv = recv(clientSocket,buffer,sizeof(buffer)-1,0);
		if(bytes_recv <= 0){
			std::cerr<<"Failed to receive data or client disconnected\n";
			return;
		}
		buffer[bytes_recv] = '\0';
		std::cout<<"Request Received:\n";
		std::cout<<buffer<<"\n";

		//send request to backend server
		int sockbe = socket(AF_INET,SOCK_STREAM,0);

		sockaddr_in servaddr;
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(servlist[servit]);
		inet_pton(AF_INET,BEIP,&servaddr.sin_addr);

		//switch to another server number
		servit++;
		servit = servit%servnum;

		if(connect(sockbe,(sockaddr*)&servaddr,sizeof(servaddr)) == -1){
			std::cout<<"Error in connecting to the backend server";
			return;
		}

		write(sockbe,buffer,bytes_recv);
		memset(buffer,0,sizeof(buffer));
		int bebytes = read(sockbe,buffer,sizeof(buffer));
		buffer[bebytes] = '\0';

		std::cout<<"from Server:\n"<<buffer<<"\n";

		//forward response from backend server to the client;
		send(clientSocket,buffer,bebytes,0);

		close(sockbe);

		close(clientSocket);
}

int main(int argc, char* argv[]){
	servnum = servlist.size();

	std::cout<<"Load Balancer started!\n";

	//create
	int sock = socket(AF_INET,SOCK_STREAM,0);

	if(sock == -1){
		std::cerr<<"Error in creating socket\n";
		return 1;
	}

	//bind
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(LBPORT);
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
	char host[NI_MAXHOST];
	char svc[NI_MAXSERV];
	int clientSocket;

	std::vector<std::thread> threads;

	while(true){
		clientSocket = accept(sock, (sockaddr*)&client, &clientsize);

		if(clientSocket == -1){
			std::cerr<<"Problem with client connection";
			return 4;
		}

		memset(host, 0, NI_MAXHOST);
		memset(svc, 0, NI_MAXSERV);

		int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);

		if(result){
			std::cout<<host<<" Connected on "<<svc<<std::endl;
		}else{
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			std::cout<<host<<" Connected on "<<ntohs(client.sin_port)<<std::endl;
		}

		
		threads.emplace_back(std::thread(handleClient,clientSocket));
		threads.back().detach();

	}

	//close
	close(sock);
	
	return 0;
}