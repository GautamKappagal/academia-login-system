#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// File paths
#define STUDENT_DB "students.db"
#define FACULTY_DB "faculty.db"
#define COURSES_DB "courses.db"
#define ADMIN_DB "admin.db"

// Structure for user accounts
typedef struct {
    char username[50];
    char password[50];
    int is_active;
    int id;
} User;

typedef struct {
    int course_id;
    char name[100];
    char faculty[50];
    int seats;
    int enrolled;
} Course;

typedef struct {
    int student_id;
    int course_ids[10];
    int num_courses;
} Enrollment;

// Function prototypes
void *handle_client(void *arg);
void admin_menu(int client_sock);
void faculty_menu(int client_sock, char *username);
void student_menu(int client_sock, char *username);
int authenticate_user(int type, char *username, char *password);
int add_user(int type, User user);
int update_user(int type, User user);
int add_course(Course course);
int remove_course(int course_id);
int enroll_course(int student_id, int course_id);
int drop_course(int student_id, int course_id);
void view_courses(int client_sock);
void view_enrollments(int client_sock, char *faculty_name);
void view_student_enrollments(int client_sock, int student_id);
void send_message(int sock, char *message);
void receive_message(int sock, char *buffer);

// Semaphore for thread safety
sem_t db_semaphore;

int main() {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t threads[MAX_CLIENTS];
    int thread_count = 0;

    // Initialize semaphore
    sem_init(&db_semaphore, 0, 1);

    // Create initial admin account if not exists
    int fd = open(ADMIN_DB, O_RDONLY);
    if (fd < 0) {
        fd = open(ADMIN_DB, O_WRONLY | O_CREAT, 0644);
        User admin = {"admin", "admin123", 1, 1};
        write(fd, &admin, sizeof(User));
        close(fd);
    } else {
        close(fd);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 8080;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    printf("Server started and listening on port %d...\n", portno);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            continue;
        }

        if (thread_count >= MAX_CLIENTS) {
            printf("Max clients reached. Rejecting new connection.\n");
            close(newsockfd);
            continue;
        }

        if (pthread_create(&threads[thread_count], NULL, handle_client, (void *) &newsockfd) != 0) {
            perror("ERROR creating thread");
            close(newsockfd);
            continue;
        }

        thread_count++;
    }

    sem_destroy(&db_semaphore);
    close(sockfd);
    return 0;
}

void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int choice;
    char username[50], password[50];
    int authenticated = 0;
    int user_type = 0;

    send_message(client_sock, "\n......Welcome Back to Academia :: Course Registration......\n");

    while (!authenticated) {
        send_message(client_sock, "\nLogin Type\nEnter Your Choice { 1.Admin , 2.Professor, 3.Student } : ");
        receive_message(client_sock, buffer);
        user_type = atoi(buffer);

        if (user_type < 1 || user_type > 3) {
            send_message(client_sock, "Invalid choice. Please try again.\n");
            continue;
        }

        send_message(client_sock, "Enter username: ");
        receive_message(client_sock, username);
        printf("Received username: %s\n", username);

        send_message(client_sock, "Enter password: ");
        receive_message(client_sock, password);

        authenticated = authenticate_user(user_type, username, password);

        if (!authenticated) {
            send_message(client_sock, "Authentication failed. Please try again.\n");
        }
    }

    switch (user_type) {
        case 1: // Admin
            admin_menu(client_sock);
            break;
        case 2: // Faculty
            faculty_menu(client_sock, username);
            break;
        case 3: // Student
            student_menu(client_sock, username);
            break;
    }

    close(client_sock);
    pthread_exit(NULL);
}

void admin_menu(int client_sock) {
    char buffer[BUFFER_SIZE];
    int choice;
    
    while (1) {
        send_message(client_sock, "\n......Welcome to Admin Menu......\n");
        send_message(client_sock, "1. Add Student\n");
        send_message(client_sock, "2. View Student Details\n");
        send_message(client_sock, "3. Add Faculty\n");
        send_message(client_sock, "4. View Faculty Details\n");
        send_message(client_sock, "5. Activate Student\n");
        send_message(client_sock, "6. Block Student\n");
        send_message(client_sock, "7. Modify Student Details\n");
        send_message(client_sock, "8. Modify Faculty Details\n");
        send_message(client_sock, "9. Logout and Exit\n");
        send_message(client_sock, "Enter Your Choice: ");
        
        receive_message(client_sock, buffer);
        choice = atoi(buffer);

        switch (choice) {
            case 1: {
                User new_student;
                send_message(client_sock, "Enter student username: ");
                receive_message(client_sock, new_student.username);
                send_message(client_sock, "Enter student password: ");
                receive_message(client_sock, new_student.password);
                new_student.is_active = 1;
                new_student.id = rand() % 9000 + 1000; // Generate 4-digit ID
                
                sem_wait(&db_semaphore);
                int result = add_user(3, new_student);
                sem_post(&db_semaphore);
                
                if (result) {
                    send_message(client_sock, "Student added successfully.\n");
                } else {
                    send_message(client_sock, "Failed to add student.\n");
                }
                break;
            }
            case 9:
                send_message(client_sock, "Logging out...\n");
                return;
            default:
                send_message(client_sock, "Feature not implemented yet.\n");
                break;
        }
    }
}

void faculty_menu(int client_sock, char *username) {
    char buffer[BUFFER_SIZE];
    int choice;
    
    while (1) {
        send_message(client_sock, "\n......Welcome to Faculty Menu......\n");
        send_message(client_sock, "1. View Offering Courses\n");
        send_message(client_sock, "2. Add New Course\n");
        send_message(client_sock, "3. Remove Course from Catalog\n");
        send_message(client_sock, "4. Update Course Details\n");
        send_message(client_sock, "5. Change Password\n");
        send_message(client_sock, "6. Logout and Exit\n");
        send_message(client_sock, "Enter Your Choice: ");
        
        receive_message(client_sock, buffer);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                view_courses(client_sock);
                break;
            case 2: {
                Course new_course;
                send_message(client_sock, "Enter course name: ");
                receive_message(client_sock, new_course.name);
                send_message(client_sock, "Enter available seats: ");
                receive_message(client_sock, buffer);
                new_course.seats = atoi(buffer);
                new_course.enrolled = 0;
                new_course.course_id = rand() % 9000 + 1000;
                strcpy(new_course.faculty, username);
                
                sem_wait(&db_semaphore);
                int result = add_course(new_course);
                sem_post(&db_semaphore);
                
                if (result) {
                    send_message(client_sock, "Course added successfully.\n");
                } else {
                    send_message(client_sock, "Failed to add course.\n");
                }
                break;
            }
            case 6:
                send_message(client_sock, "Logging out...\n");
                return;
            default:
                send_message(client_sock, "Feature not implemented yet.\n");
                break;
        }
    }
}

void student_menu(int client_sock, char *username) {
    char buffer[BUFFER_SIZE];
    int choice;
    int student_id;
    
    // Get student ID from username
    sem_wait(&db_semaphore);
    int fd = open(STUDENT_DB, O_RDONLY);
    User student;
    while (read(fd, &student, sizeof(User))){
        if (strcmp(student.username, username) == 0) {
            student_id = student.id;
            break;
        }
    }
    close(fd);
    sem_post(&db_semaphore);
    
    while (1) {
        send_message(client_sock, "\n......Welcome to Student Menu......\n");
        send_message(client_sock, "1. View All Courses\n");
        send_message(client_sock, "2. Enroll (pick) New Course\n");
        send_message(client_sock, "3. Drop Course\n");
        send_message(client_sock, "4. View Enrolled Course Details\n");
        send_message(client_sock, "5. Change Password\n");
        send_message(client_sock, "6. Logout and Exit\n");
        send_message(client_sock, "Enter Your Choice: ");
        
        receive_message(client_sock, buffer);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                view_courses(client_sock);
                break;
            case 2: {
                send_message(client_sock, "Enter course ID to enroll: ");
                receive_message(client_sock, buffer);
                int course_id = atoi(buffer);
                
                sem_wait(&db_semaphore);
                int result = enroll_course(student_id, course_id);
                sem_post(&db_semaphore);
                
                if (result == 1) {
                    send_message(client_sock, "Enrolled successfully.\n");
                } else if (result == 0) {
                    send_message(client_sock, "Course is full.\n");
                } else {
                    send_message(client_sock, "Failed to enroll.\n");
                }
                break;
            }
            case 6:
                send_message(client_sock, "Logging out...\n");
                return;
            default:
                send_message(client_sock, "Feature not implemented yet.\n");
                break;
        }
    }
}

int authenticate_user(int type, char *username, char *password) {
    printf("Authenticating user: %s\n", username);
    char *db_file;
    switch (type) {
        case 1: db_file = ADMIN_DB; break;
        case 2: db_file = FACULTY_DB; break;
        case 3: db_file = STUDENT_DB; break;
        default: return 0;
    }

    printf("Authenticating against %s\n", db_file);
    
    int fd = open(db_file, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open user database");
        return 0;
    }

    User user;
    while (read(fd, &user, sizeof(User))) {
        printf("Read user: %s, pass: %s, active: %d\n", 
               user.username, user.password, user.is_active);
        printf("Comparing with: %s, %s\n", username, password);
        if (strcmp(user.username, username) == 0 && 
            strcmp(user.password, password) == 0 && 
            user.is_active) {
                printf("User authenticated successfully.\n");
            close(fd);
            return 1;
        }
    }

    close(fd);
    return 0;
}

int add_user(int type, User new_user) {
    char *db_file;
    switch (type) {
        case 2: db_file = FACULTY_DB; break;
        case 3: db_file = STUDENT_DB; break;
        default: return 0;
    }

    int fd = open(db_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) return 0;

    int result = write(fd, &new_user, sizeof(User));
    close(fd);
    return result == sizeof(User);
}

int add_course(Course course) {
    int fd = open(COURSES_DB, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) return 0;

    int result = write(fd, &course, sizeof(Course));
    close(fd);
    return result == sizeof(Course);
}

int enroll_course(int student_id, int course_id) {
    // Implementation would check seat availability and update enrollments
    // This is a simplified version
    return 1;
}

void view_courses(int client_sock) {
    sem_wait(&db_semaphore);
    int fd = open(COURSES_DB, O_RDONLY);
    if (fd < 0) {
        send_message(client_sock, "No courses available.\n");
        sem_post(&db_semaphore);
        return;
    }

    Course course;
    send_message(client_sock, "\nAvailable Courses:\n");
    send_message(client_sock, "ID\tName\tFaculty\tSeats\tEnrolled\n");
    while (read(fd, &course, sizeof(Course))) {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%d\t%s\t%s\t%d\t%d\n", 
                course.course_id, course.name, course.faculty, 
                course.seats, course.enrolled);
        send_message(client_sock, buffer);
    }
    close(fd);
    sem_post(&db_semaphore);
}

void send_message(int sock, char *message) {
    write(sock, message, strlen(message));
}

void receive_message(int sock, char *buffer) {
    bzero(buffer, BUFFER_SIZE);
    read(sock, buffer, BUFFER_SIZE - 1);
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline
}