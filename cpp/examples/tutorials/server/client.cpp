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
	char buffer[1024] = { 0 };
	char* hello = "Hello from server";

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
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        valread = read(new_socket, buffer, 1024);
        printf("%s\n", buffer);
		for (int i = 0; i < 1024; i++) {
			buffer[i] = 0;
		}
    }

	return 0;
}

int client() {
	while(true) {
		int sock = 0, valread;
		struct sockaddr_in serv_addr;
		char buffer[1024] = { 0 };

		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("\n Socket creation error \n");
		}

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(5101);

		// Convert IPv4 and IPv6 addresses from text to binary
		// form
		if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
			printf("\nInvalid address/ Address not supported \n");
		}

		if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			printf("\nConnection Failed \n");
		}

		cout << "Socket connected with 127.0.0.1:5101\n";

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
