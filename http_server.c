#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 256

void handleClient(int client) {
    char buffer[BUFFER_SIZE];

    // Nhận dữ liệu từ client
    int ret = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (ret <= 0) {
        perror("recv failed");
        close(client);
        return;
    }
    buffer[ret] = '\0';
    printf("Received from %d: %s\n", client, buffer);

    // Gửi phản hồi cho client
    char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
    send(client, response, strlen(response), 0);

    close(client);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) {
        perror("socket failed");
        return 1;
    }

    // Khai báo địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gán socket với địa chỉ
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind failed");
        close(listener);
        return 1;
    }

    // Chuyển sang chế độ lắng nghe kết nối
    if (listen(listener, MAX_CLIENTS) == -1) {
        perror("listen failed");
        close(listener);
        return 1;
    }

    // Tạo các tiến trình con để xử lý kết nối từ client
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            break;
        } else if (pid == 0) {
            // Tiến trình con chạy như một server
            while (1) {
                int client = accept(listener, NULL, NULL);
                printf("New client connected: %d\n", client);

                // Xử lý kết nối từ client
                handleClient(client);
            }

            exit(0);
        }
    }

    getchar();
    killpg(0, SIGKILL);

    return 0;
}