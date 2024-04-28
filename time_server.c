#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

void signalHandler(int signo) {
    pid_t pid;
    int status;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Child process terminated, pid = %d\n", pid);
    }
}

int processRequest(int client) {
    char buf[256];

    while (1) {
        int ret = recv(client, buf, sizeof(buf) - 1, 0);
        if (ret <= 0) {
            // Đóng kết nối
            break;
        }

        buf[ret] = '\0';
        printf("Received from %d: %s\n", client, buf);

        // Xử lý lệnh gửi từ client
        char comm[10], format[12], tmp[32];
        int a = sscanf(buf, "%s %s %s", comm, format, tmp);
        char reply_msg[32]; // Kết quả trả về cho client

        if (a == 2) {
            // Đúng số tham số cần gửi
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime); 

            // Kiểm tra lệnh
            if (strcmp(format, "dd/mm/yyyy") == 0) {
                strftime(reply_msg, sizeof(reply_msg), "%d/%m/%Y\n", timeinfo);
            } else if (strcmp(format, "dd/mm/yy") == 0) {
                strftime(reply_msg, sizeof(reply_msg), "%d/%m/%y\n", timeinfo);
            } else if (strcmp(format, "mm/dd/yyyy") == 0) {
                strftime(reply_msg, sizeof(reply_msg), "%m/%d/%Y\n", timeinfo);
            } else if (strcmp(format, "mm/dd/yy") == 0) {
                strftime(reply_msg, sizeof(reply_msg), "%m/%d/%y\n", timeinfo);
            } else {
                // Sai lệnh
                char* msg = "Sai lệnh, hãy nhập lại!\n";
                send(client, msg, strlen(msg), 0);
                continue;
            }
        } else {
            char* msg = "Sai cú pháp, hãy nhập lại!\n";
            send(client, msg, strlen(msg), 0);
            continue;
        }

        // Trả lại kết quả cho client
        send(client, reply_msg, strlen(reply_msg), 0);
    }

    return 0;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket failed: ");
        return 1;
    }
    
    // Khai báo địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gán socket với địa chỉ
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind failed: ");
        return 1;
    }

    // Chuyển sang chế độ chờ kết nối
    if (listen(listener, 5) == -1) {
        perror("listen failed: ");
        return 1;
    }

    // Tạo server để lắng nghe
    char buf[256];
    // Khai báo xử lí sự kiện, signalHandler được gọi mỗi khi có một client đóng kết nối
    signal(SIGCHLD, signalHandler);

    while (1) {
        printf("Chờ client mới kết nối...\n");
        int client = accept(listener, NULL, NULL);
        printf("Client mới kết nối: %d\n", client);

        if (fork() == 0) {
            // Tiến trình con xử lý yêu cầu từ client

            // Đóng socket listener, bản sao này được tạo ra nhưng chương trình con không sử dụng đến
            close(listener);

            // Xử lý yêu cầu từ client
            processRequest(client);

            close(client);
            exit(0);
        }
        
        close(client);
    }

    return 0;
}