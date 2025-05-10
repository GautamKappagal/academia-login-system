#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int student_exists(const char *received_username) {
    int fd = open("students.txt", O_RDONLY);
    if (fd < 0) {
        char *err = "Error opening students.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return 0;  // Assume doesn't exist if file can't be opened
    }

    char buffer[1];
    char line[BUFFER_SIZE];
    int idx = 0;
    int bytes_read;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string

            // Extract username (before first colon)
            char *colon_pos = strchr(line, ':');
            if (colon_pos != NULL) {
                *colon_pos = '\0'; // temporarily terminate at colon
                if (strcmp(line, received_username) == 0) {
                    close(fd);
                    return 1; // found match
                }
            }

            idx = 0; // reset for next line
        } else {
            line[idx++] = buffer[0];
        }
    }

    close(fd);
    return 0; // not found
}

int faculty_exists(const char *received_username) {
    int fd = open("faculties.txt", O_RDONLY);
    if (fd < 0) {
        char *err = "Error opening faculties.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return 0;  // Assume doesn't exist if file can't be opened
    }

    char buffer[1];
    char line[BUFFER_SIZE];
    int idx = 0;
    int bytes_read;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string

            // Extract username (before first colon)
            char *colon_pos = strchr(line, ':');
            if (colon_pos != NULL) {
                *colon_pos = '\0'; // temporarily terminate at colon
                if (strcmp(line, received_username) == 0) {
                    close(fd);
                    return 1; // found match
                }
            }

            idx = 0; // reset for next line
        } else {
            line[idx++] = buffer[0];
        }
    }

    close(fd);
    return 0; // not found
}

void add_student(int sock) {// Receive username and password
    char received_username[BUFFER_SIZE] = {0};
    char received_password[BUFFER_SIZE] = {0};

    read(sock, received_username, BUFFER_SIZE);
    read(sock, received_password, BUFFER_SIZE);

    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, received_username, strlen(received_username));
    write(STDOUT_FILENO, "\n", 1);

    // Check if username already exists in database
    if (student_exists(received_username)) {
        char *err = "Username already exists.\n";
        write(sock, err, strlen(err));
        return;
    }

    write(STDOUT_FILENO, "Received password: ", 19);
    write(STDOUT_FILENO, received_password, strlen(received_password));
    write(STDOUT_FILENO, "\n", 1);


    // Construct student entry
    char entry[BUFFER_SIZE];
    int entry_len = snprintf(entry, BUFFER_SIZE, "%s:%s: :1\n", received_username, received_password);

    // Open students.txt in append mode, create if doesn't exist
    int fd = open("students.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        char *err = "Error opening students.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    // Lock file for writing
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    fcntl(fd, F_SETLKW, &lock);

    // Write the new entry
    write(fd, entry, entry_len);

    // Unlock file
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd);

    // Notify client
    char *msg = "Student added successfully.\n";
    write(sock, msg, strlen(msg));
}

void add_faculty(int sock) {
    char received_username[BUFFER_SIZE] = {0};
    char received_password[BUFFER_SIZE] = {0};

    read(sock, received_username, BUFFER_SIZE);
    read(sock, received_password, BUFFER_SIZE);

    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, received_username, strlen(received_username));
    write(STDOUT_FILENO, "\n", 1);

    // Check if username already exists in database
    if (faculty_exists(received_username)) {
        char *err = "Username already exists.\n";
        write(sock, err, strlen(err));
        return;
    }

    write(STDOUT_FILENO, "Received password: ", 19);
    write(STDOUT_FILENO, received_password, strlen(received_password));
    write(STDOUT_FILENO, "\n", 1);


    // Construct student entry
    char entry[BUFFER_SIZE];
    int entry_len = snprintf(entry, BUFFER_SIZE, "%s:%s: :1\n", received_username, received_password);

    // Open students.txt in append mode, create if doesn't exist
    int fd = open("faculties.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        char *err = "Error opening faculties.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    // Lock file for writing
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    fcntl(fd, F_SETLKW, &lock);

    // Write the new entry
    write(fd, entry, entry_len);

    // Unlock file
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd);

    // Notify client
    char *msg = "Faculty added successfully.\n";
    write(sock, msg, strlen(msg));
}

void add_course(int sock) {
    char course_name[BUFFER_SIZE] = {0};

    read(sock, course_name, BUFFER_SIZE);
    write(STDOUT_FILENO, "Received course: ", 17);

    
}

void view_student_details(int sock) {
    char username[BUFFER_SIZE] = {0};

    // Get student username
    read(sock, username, BUFFER_SIZE);
    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, username, strlen(username));
    write(STDOUT_FILENO, "\n", 1);

    // Open students.txt in read mode
    int fd = open("students.txt", O_RDONLY);
    if (fd < 0) {
        char *err = "Error opening students.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    char buffer[1], line[BUFFER_SIZE];
    int idx = 0, bytes_read, found = 0;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string
            idx = 0;

            // Extract username (before first colon)
            char temp_line[BUFFER_SIZE];
            strcpy(temp_line, line); // to preserve original
            char *token = strtok(temp_line, ":"); // username
            if (token && strcmp(token, username) == 0) {
                found = 1;

                // Skip username and password
                token = strtok(NULL, ":");
                token = strtok(NULL, ":");
                // Extract courses
                if (strcmp(token, "x") != 0){
                    write(sock, token, strlen(token));
                } else {
                    write(sock, "No courses found.\n", 18);
                }

                break;
            }
        } else {
            line[idx++] = buffer[0];
        }
    }

    if (!found) {
        char *err = "Student not found.\n";
        write(STDERR_FILENO, err, strlen(err));
    }

    close(fd);
}

int validate_student(const char *username, const char *password) {
    int fd = open("students.txt", O_RDONLY);
    if (fd < 0) {
        write(STDERR_FILENO, "Error opening students.txt\n", 28);
        return 0; // Assume doesn't exist if file can't be opened
    }

    char buffer[1];
    char line[BUFFER_SIZE];
    int idx = 0;
    int bytes_read;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string

            // Extract username and password
            char *token = strtok(line, ":");
            if (token && strcmp(token, username) == 0) {
                token = strtok(NULL, ":");
                if (token && strcmp(token, password) == 0) {
                    close(fd);
                    return 1; // found match
                }
            }

            idx = 0; // reset for next line
        } else {
            line[idx++] = buffer[0];
        }
    }

    close(fd);
    return 0; // not found
}

int validate_faculty(const char *username, const char *password) {
    int fd = open("faculties.txt", O_RDONLY);
    if (fd < 0) {
        write(STDERR_FILENO, "Error opening faculties.txt\n", 28);
        return 0; // Assume doesn't exist if file can't be opened
    }

    char buffer[1];
    char line[BUFFER_SIZE];
    int idx = 0;
    int bytes_read;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string

            // Extract username and password
            char *token = strtok(line, ":");
            if (token && strcmp(token, username) == 0) {
                token = strtok(NULL, ":");
                if (token && strcmp(token, password) == 0) {
                    close(fd);
                    return 1; // found match
                }
            }

            idx = 0; // reset for next line
        } else {
            line[idx++] = buffer[0];
        }
    }

    close(fd);
    return 0; // not found
}

int run_admin_menu(int sock){
    // Receive task choice from client
    char task_choice;
    read(sock, &task_choice, sizeof(task_choice));
    printf("Received task choice: %c\n", task_choice);

    // Process task choice

    if (task_choice == '1') {
        add_student(sock);
    }
    else if (task_choice == '2') {
        view_student_details(sock);
    }
    else if (task_choice == '3') {
        add_faculty(sock);
    }
    else if (task_choice == '4') {
        write(STDOUT_FILENO, "Viewing faculty details...\n", 28);
    }
    else if (task_choice == '5') {
        write(STDOUT_FILENO, "Activating student...\n", 22);
    }
    else if (task_choice == '6') {
        write(STDOUT_FILENO, "Blocking student...\n", 21);
    }
    else if (task_choice == '7') {
        write(STDOUT_FILENO, "Modifying student details...\n", 30);
    }
    else if (task_choice == '8') {
        write(STDOUT_FILENO, "Modifying faculty details...\n", 30);
    }
    else if (task_choice == '9'){
        char *msg = "Logging out and exiting...\n";
        write(sock, msg, strlen(msg));
        close(sock);
        return 1;
    }
    else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }

    return 0;
}

int run_professor_menu(int sock) {
    // Receive username and password for validation
    char received_username[BUFFER_SIZE] = {0};
    char received_password[BUFFER_SIZE] = {0};

    read(sock, received_username, BUFFER_SIZE);
    read(sock, received_password, BUFFER_SIZE);

    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, received_username, strlen(received_username));
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, "Received password: ", 19);
    write(STDOUT_FILENO, received_password, strlen(received_password));
    write(STDOUT_FILENO, "\n", 1);

    if (faculty_exists(received_username)){
        write(STDOUT_FILENO, "Faculty exists.\n", 16);
    }
    else {
        write(STDOUT_FILENO, "Faculty does not exist.\n", 24);
        return 1;
    }

    return 0;
}

int run_student_menu(int sock) {
    // Receive username and password for validation
    char received_username[BUFFER_SIZE] = {0};
    char received_password[BUFFER_SIZE] = {0};

    read(sock, received_username, BUFFER_SIZE);
    read(sock, received_password, BUFFER_SIZE);

    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, received_username, strlen(received_username));
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, "Received password: ", 19);
    write(STDOUT_FILENO, received_password, strlen(received_password));
    write(STDOUT_FILENO, "\n", 1);

    if (!student_exists(received_username)){
        write(STDOUT_FILENO, "Student does not exist.\n", 24);
        write(sock, "Student not found.\n", 20);
        return 1;
    }

    if (validate_student(received_username, received_password)) {
        write(STDOUT_FILENO, "Student login successful.\n", 26);
        write(sock, "Student login successful.\n", 26);
    } else {
        write(STDOUT_FILENO, "Invalid credentials.\n", 21);
        write(sock, "Invalid credentials.\n", 21);
        return 1;
    }

    char task_choice;
    read(sock, &task_choice, sizeof(task_choice));
    printf("Received task choice: %c\n", task_choice);

    // Process task choice
    if (task_choice == '1') {
        // View courses
        view_student_details(sock);
    } else if (task_choice == '2') {
        // Enroll in course
        write(STDOUT_FILENO, "Enrolling in course...\n", 23);
    } else if (task_choice == '3') {
        // Drop course
        write(STDOUT_FILENO, "Dropping course...\n", 20);
    } else if (task_choice == '4') {
        // View enrolled courses
        write(STDOUT_FILENO, "Viewing enrolled courses...\n", 28);
    } else if (task_choice == '9') {
        char *msg = "Logging out and exiting...\n";
        write(sock, msg, strlen(msg));
        close(sock);
        return 1;
    } else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }

    return 0;
}

int main() {
    int server_fd, sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Creating socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created successfully.\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    address.sin_port = htons(PORT);

    // Binding socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Server bound to port %d.\n", PORT);

    // Listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    // Accepting client connection
    sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (sock < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Server connected to client.\n");

    // Receive role input from client
    char role_input;
    read(sock, &role_input, sizeof(role_input));
    printf("Received role input: %c\n", role_input);

    if (role_input == '1') {
        // Admin menu
        int ret = run_admin_menu(sock);
        if (ret == 1) {
            close(sock);
            return 0; // Exit after admin menu
        }
    } else if (role_input == '2') {
        // Professor menu
        int ret = run_professor_menu(sock);
    } else if (role_input == '3') {
        // Student menu
        int ret = run_student_menu(sock);
    } else if (role_input == '9') {
        write(STDOUT_FILENO, "Client disconnected.\n", 23);
        close(sock);
        return 0; // Exit after client disconnect
    } else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }
}