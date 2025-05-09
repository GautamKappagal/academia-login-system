#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to add student
void add_student(int sock) {
    char username[BUFFER_SIZE] = {0};
    char password[BUFFER_SIZE] = {0};

    // Get student username
    write(STDOUT_FILENO, "Enter student username: ", 25);
    int ulen = read(STDIN_FILENO, username, BUFFER_SIZE);
    if (ulen > 0 && username[ulen - 1] == '\n') {
        username[ulen - 1] = '\0';
    }

    // Get student password
    write(STDOUT_FILENO, "Enter student password: ", 25);
    int plen = read(STDIN_FILENO, password, BUFFER_SIZE);
    if (plen > 0 && password[plen - 1] == '\n') {
        password[plen - 1] = '\0';
    }

    // Send username and password separately
    write(sock, username, BUFFER_SIZE);
    write(sock, password, BUFFER_SIZE);

    // Read whether the student was added successfully
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);

    if (strcmp(response, "Username already exists.\n") == 0) {
        write(STDERR_FILENO, response, strlen(response));
    } else {
        write(STDOUT_FILENO, "Student added successfully.\n", 28);
    }
}

void view_student_details(int sock) {
    char username[BUFFER_SIZE] = {0};

    // Get student username
    write(STDOUT_FILENO, "Enter student username: ", 25);
    int ulen = read(STDIN_FILENO, username, BUFFER_SIZE);
    if (ulen > 0 && username[ulen - 1] == '\n') {
        username[ulen - 1] = '\0';
    }

    // Send student ID to server
    write(sock, username, BUFFER_SIZE);

    // Here you would typically wait for a response from the server
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);

    if (strcmp(response, "Student not found.\n") == 0) {
        write(STDERR_FILENO, response, strlen(response));
    } else {
        write(STDOUT_FILENO, response, strlen(response));
    }
}

// Function to handle user input and display menu
void handle_admin_input(int sock, char *buffer) {
    // Display menu options
    write(STDOUT_FILENO, "1. Add Student\n", 15);
    write(STDOUT_FILENO, "2. View Student Details\n", 24);
    write(STDOUT_FILENO, "3. Add Faculty\n", 15);
    write(STDOUT_FILENO, "4. View Faculty Details\n", 25);
    write(STDOUT_FILENO, "5. Activate Student\n", 20);
    write(STDOUT_FILENO, "6. Block Student\n", 18);
    write(STDOUT_FILENO, "7. Modify Student Details\n", 26);
    write(STDOUT_FILENO, "8. Modify Faculty Details\n", 26);
    write(STDOUT_FILENO, "9. Logout and Exit\n", 20);

    // Take user choice
    write(STDOUT_FILENO, "Enter your choice: ", 19);
    scanf("%s", buffer);
    // Clear the input buffer
    while (getchar() != '\n');  // This consumes the newline character

    // Handle user choice
    if (strcmp(buffer, "1") == 0) {
        write(sock, buffer, strlen(buffer));
        add_student(sock);
    } else if (strcmp(buffer, "2") == 0) {
        write(sock, buffer, strlen(buffer));
        view_student_details(sock);
    } else if (strcmp(buffer, "3") == 0) {
        write(STDOUT_FILENO, "Adding faculty...\n", 18);
    } else if (strcmp(buffer, "4") == 0) {
        write(STDOUT_FILENO, "Viewing faculty details...\n", 28);
    } else if (strcmp(buffer, "5") == 0) {
        write(STDOUT_FILENO, "Activating student...\n", 22);
    } else if (strcmp(buffer, "6") == 0) {
        write(STDOUT_FILENO, "Blocking student...\n", 21);
    } else if (strcmp(buffer, "7") == 0) {
        write(STDOUT_FILENO, "Modifying student details...\n", 30);
    } else if (strcmp(buffer, "8") == 0) {
        write(STDOUT_FILENO, "Modifying faculty details...\n", 30);
    } else if (strcmp(buffer, "9") == 0) {
        write(STDOUT_FILENO, "Logging out and exiting...\n", 28);
        close(sock);
        exit(0);
    } else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char role_input;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Zero out the structure
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or Address not supported");
        return -1;
    }

    // Try connecting
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    // Write a message to terminal using system call
    write(STDOUT_FILENO, "Connection established.\n", 24);

    // Take input for user role
    write(STDOUT_FILENO, "Enter 1 for Admin, 2 for Professor, 3 for Student: ", 51);
    read(STDIN_FILENO, &role_input, 1);

    // Clear the newline character left in the input buffer
    while (getchar() != '\n');  // This consumes the newline character

    // Handle role input (just printing it for now)
    write(STDOUT_FILENO, "You selected role: ", 19);
    write(STDOUT_FILENO, &role_input, 1);
    write(STDOUT_FILENO, "\n", 1);

    // Send selected role to server
    write(sock, &role_input, sizeof(role_input));

    if (role_input == '1') {
        write(STDOUT_FILENO, "Username: ", 10);
        scanf("%s", buffer);
        write(STDOUT_FILENO, "Password: ", 10);
        scanf("%s", buffer + 50);  // Store password in buffer after username

        // Clear the input buffer
        while (getchar() != '\n');  // This consumes the newline character
        // Simulate login process
        char *username = buffer;
        char *password = buffer + 50;  // Password is stored after username in buffer

        if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
            write(STDOUT_FILENO, "Login successful.\n", 18);
        } else {
            write(STDOUT_FILENO, "Invalid credentials.\n", 21);
            close(sock);
            return 0;
        }
        write(STDOUT_FILENO, "Admin menu selected.\n", 21);

        // Call admin menu function here
        while (1) {
            handle_admin_input(sock, buffer);
        }
    } else if (role_input == '2') {
        write(STDOUT_FILENO, "Professor menu selected.\n", 25);
    } else if (role_input == '3') {
        write(STDOUT_FILENO, "Student menu selected.\n", 23);
    } else {
        write(STDOUT_FILENO, "Invalid role selected.\n", 23);
    }
    return 0;
}