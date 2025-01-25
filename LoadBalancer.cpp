#include "server.h"

bool getSetHealth(int id,bool val = true,bool get = true){
	healthmtx.lock();
	if(get){
		healthmtx.unlock();
		return health[id];
	}
	health[id] = val;
	healthmtx.unlock();
	return val;
}

int sendReqServer(int beport, int& sockbe,char* buffer,int bytes_recv){
	mtx.lock();
		sockaddr_in servaddr;
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(beport);
		inet_pton(AF_INET,BEIP,&servaddr.sin_addr);

		if(connect(sockbe,(sockaddr*)&servaddr,sizeof(servaddr)) == -1){
			mtx.unlock();
			return 0;
		}

		std::cout<<"connected to server"<<BEIP<<":"<<beport<<"\n";
		std::cout<<"Sending message to server:\n"<<buffer<<"\n";

		write(sockbe,buffer,bytes_recv);
		memset(buffer,0,MXSIZE);
		int bebytes = 0, total_bytes = 0;
		while ((bebytes = read(sockbe, buffer + total_bytes, MXSIZE - total_bytes - 1)) > 0) {
    		total_bytes += bebytes;
    		if (total_bytes >= MXSIZE - 1) break; // Prevent buffer overflow
		}
		buffer[total_bytes] = '\0';
		std::cout<<"Response from server:\n"<<buffer<<"\n";

		mtx.unlock();
		return total_bytes;
}

void healthCheck(){
	//iterate through the serverlist 
	//check each server by sending an http get request
	//update health status in health list
	//wait the mentioned amount of time
	//repeat

	while(true){
		for(int i = 0;i<servnum;i++){
			char buffer[MXSIZE] = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";				//put get request message here
			int bytes_recv = 54;
			buffer[bytes_recv] = '\0';
			int beport = servlist[i];
			int sockbe = socket(AF_INET,SOCK_STREAM,0);
			int bebytes = sendReqServer(beport,sockbe,buffer,bytes_recv);
			if(!bebytes){
				//it means the server is not well
				getSetHealth(i,false,false);
			}else{
				getSetHealth(i,true,false);
				std::cout<<"Health report of server"<<BEIP<<":"<<beport<<":\n"<<buffer<<"\n";
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}
}

void handleClient(int clientSocket){
	//send/receive
		char buffer[MXSIZE] = {0};
		int bytes_recv = recv(clientSocket,buffer,sizeof(buffer)-1,0);
		if(bytes_recv <= 0){
			std::cerr<<"Failed to receive data or client disconnected\n";
			return;
		}
		buffer[bytes_recv] = '\0';
		std::cout<<"Request Received:\n";
		std::cout<<buffer<<"\n";

		int chk = servit;
		while(!getSetHealth(servit)){
			servit++;
			servit %=servnum;
			if(chk == servit){
				break;
			}
		}

		int beport = servlist[servit];
		int sockbe = socket(AF_INET,SOCK_STREAM,0);

		//send request to backend server
		int bebytes = sendReqServer(beport,sockbe,buffer,bytes_recv);
		if(bebytes == 0){
			std::cout<<"Error in connecting to the backend server";
			return;
		}

		//switch to another port number
		servit++;
		servit %=servnum;

		// std::cout<<"from Server:\n"<<buffer<<"\n";

		//forward response from backend server to the client;
		send(clientSocket,buffer,bebytes,0);

		close(sockbe);

		close(clientSocket);
}

int main(int argc, char* argv[]){
	servnum = servlist.size();
	health.resize(servnum);
	fill(health.begin(),health.end(),true);

	//start a background task of health check
	std::thread Health = std::thread(healthCheck);
	Health.detach();

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