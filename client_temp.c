#include <stdio.h>
#include <unistd.h>
#include <string.h>

void write_str(const char *str) {
    write(STDOUT_FILENO, str, strlen(str));
}

void read_input(char *buf, int size) {
    int n = read(STDIN_FILENO, buf, size);
    if (n > 0) buf[n - 1] = '\0'; // Remove newline
}

int main() {
    char role[20], username[20], password[20];

    write_str("....................Welcome Back to Academia :: Course Registration....................\n");
    write_str("Login Type\nEnter Your Choice { 1.Admin , 2.Professor, 3. Student }: ");
    read_input(role, sizeof(role));

    write_str("Username: ");
    read_input(username, sizeof(username));

    write_str("Password: ");
    read_input(password, sizeof(password));

    // Fake check (replace with actual server communication later)
    if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
        write_str("........ Welcome to Admin Menu .......\n");
        write_str("1. Add Student\n");
        write_str("2. View Student Details\n");
        write_str("3. Add Faculty\n");
        write_str("4. View Faculty Details\n");
        write_str("5. Activate Student\n");
        write_str("6. Block Student\n");
        write_str("7. Modify Student Details\n");
        write_str("8. Modify Faculty Details\n");
        write_str("9. Logout and Exit\n");
        write_str("Enter Your Choice: ");
    } else {
        write_str("Login failed.\n");
    }

    return 0;
}
