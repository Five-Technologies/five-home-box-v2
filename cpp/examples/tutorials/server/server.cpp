#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

#define PORT 8080

using namespace std;

int main(int argc, char const* argv[])
{
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
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	bind(server_fd, (struct sockaddr*)&address, sizeof(address));
	listen(server_fd, 3);
    
    while (true) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        valread = read(new_socket, buffer, 1024);
        printf("%s\n", buffer);
		for (int i = 0; i < 1024; i++) {
			buffer[i] = 0;
		}
        send(new_socket, hello, strlen(hello), 0);
        printf("{ \"messageLength\": 500, Hello message sent\n");
    }

	return 0;
}
