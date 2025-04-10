
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#define TRUE 1

int main(int argc, char *argv[]) {
  int server_fd;
  struct sockaddr_in server_addr; // IPV4 address

  // create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd == -1) {
    perror("failed to create socket.\n");
    exit(EXIT_FAILURE);
  }

  // config socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(8080);

  // bind the socket to the port
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("failed to bind socket.\n");
    exit(EXIT_FAILURE);
  };

  // listen for the connection
  if (listen(server_fd, 10) == -1) {
    perror("fail to listen the socket.\n");
    exit(EXIT_FAILURE);
  }

  while (TRUE) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int *client_fd = malloc(sizeof(int));
  }

  return 0;
}