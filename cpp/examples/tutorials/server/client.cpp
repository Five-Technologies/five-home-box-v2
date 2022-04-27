// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

#define SOCKET_HEADER_LENGTH 24

using namespace std;

int client();

int main(int argc, char const* argv[])
{
	thread t(client);
	t.detach();

	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(5100);

	bind(server_fd, (struct sockaddr*)&address, sizeof(address));
	listen(server_fd, 3);
	cout << "Listening on port " << 5100 << endl;
    
    while (true) {
		char headBuffer[SOCKET_HEADER_LENGTH] = {0};
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        valread = read(new_socket, headBuffer, SOCKET_HEADER_LENGTH);        
		string output = "";

		for (int i = 0; i < SOCKET_HEADER_LENGTH; i++) {
			if (isdigit(headBuffer[i])) {
				output += headBuffer[i];
			}
		}

		int msgLength = stoi(output);
		output = headBuffer;
		char msgBuffer[msgLength] = {0};
		valread = read(new_socket, msgBuffer, msgLength);

		for (int i = 0; i < msgLength; i++) {
			output += msgBuffer[i];
		}

		cout << "[MSG]" << output << endl;
    }

	return 0;
}

int client() {
	while(true) {
		int sock = 0, valread;
		struct sockaddr_in serv_addr;

		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("\n Socket creation error \n");
		}

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(5101);

		// Convert IPv4 and IPv6 addresses from text to binary
		// form
		if (inet_pton(AF_INET, "localhost", &serv_addr.sin_addr) <= 0) {
			printf("\nInvalid address/ Address not supported \n");
		}

		if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			printf("\nConnection Failed \n");
		}

		string response = "";
		cout << ">> Send message: ";

		getline(cin >> ws, response);		
		char fResponse[response.length() + 1];
		strcpy(fResponse, response.c_str());

		for (int i = 0; i < response.length(); i++) {
			fResponse[i] = response[i];
		}

		send(sock, fResponse, strlen(fResponse), 0);
	}

	return 0;
}
