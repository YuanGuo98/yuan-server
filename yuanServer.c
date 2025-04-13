
#include <ctype.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define TRUE 1
#define BUFF_SIZE 104857600

char *url_decode(const char *src) {
  size_t src_len = strlen(src);
  char *decoded = malloc(src_len + 1);
  if (!decoded)
    return NULL;

  size_t decoded_len = 0;

  for (size_t i = 0; i < src_len; i++) {
    if (src[i] == '%' && i + 2 < src_len && isxdigit(src[i + 1]) &&
        isxdigit(src[i + 2])) {
      int hex_val;
      sscanf(src + i + 1, "%2x", &hex_val);
      decoded[decoded_len++] = hex_val;
      i += 2;
    } else if (src[i] == '+') {
      decoded[decoded_len++] = ' ';
    } else {
      decoded[decoded_len++] = src[i];
    }
  }

  decoded[decoded_len] = '\0';
  return decoded;
}

const char *get_file_extension(const char *file_name) {
  const char *dot = strrchr(file_name, '.');
  if (dot == NULL || dot == file_name) {
    return "";
  }
  return dot + 1;
}

void build_http_response(const char *file_name, const char *file_ext,
                         char *response, size_t *response_len) {}

void *handle_client(void *arg) {
  int client_fd = *(int *)arg;
  char *buffer = (char *)malloc(BUFF_SIZE * sizeof(char));

  // receive data from client and store it into the buffer
  ssize_t bytes_received = recv(client_fd, buffer, BUFF_SIZE, 0);
  if (bytes_received > 0) {
    // check if the request is GET
    regex_t regex;
    const char *pattern = "^GET /([^ ]*) HTTP/1";
    regmatch_t match[2];

    // compile the regx
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
      printf("Failed to compile regex.\n");
      exit(EXIT_FAILURE);
    };

    if (regexec(&regex, buffer, 2, match, 0) == 0) {
      buffer[match[1].rm_eo] = '\0';
      const char *url_encoded_file_name = buffer + match[1].rm_so;
      const char *file_name = url_decode(url_encoded_file_name);

      char file_ext[32];
      strcpy(file_ext, get_file_extension(file_name));

      char *response = (char *)malloc(BUFF_SIZE * 2 * sizeof(char));
      size_t response_len;
      build_http_response(file_name, file_ext, response, &response_len);
      send(client_fd, response, response_len, 0);
      free(response);
      free(file_name);
    }
    regfree(&regex);
  }
  close(client_fd);
  free(arg);
  free(buffer);
  return NULL;
}

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
    // client info
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int *client_fd = malloc(sizeof(int));

    // accept client connection
    if (connect(*client_fd, (struct sockaddr *)&client_addr,
                sizeof(client_addr)) == -1) {
      perror("fail to accept from client");
      continue;
    };

    // create a new thread to handle client request
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void *)client_fd);
    pthread_detach(thread_id);
  }

  return 0;
}