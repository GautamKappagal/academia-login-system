#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_LEN 1024

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[MAX_LEN] = {0};
    int len;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    // Get username
    write(1, "Enter username: ", 17);
    len = read(0, buffer, MAX_LEN);
    if (len <= 0) _exit(1);
    buffer[len - 1] = '\0'; // Remove newline
    write(sock, buffer, strlen(buffer));
    sleep(1); // Separate messages

    // Get password
    write(1, "Enter password: ", 17);
    len = read(0, buffer, MAX_LEN);
    if (len <= 0) _exit(1);
    buffer[len - 1] = '\0';
    write(sock, buffer, strlen(buffer));

    // Get result from server
    memset(buffer, 0, MAX_LEN);
    len = read(sock, buffer, MAX_LEN);
    if (len > 0) write(1, buffer, len);

    close(sock);
    return 0;
}