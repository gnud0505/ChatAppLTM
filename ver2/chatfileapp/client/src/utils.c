#include "../include/common.h"

void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int send_all(int sock, const void *buffer, size_t length) {
    size_t total_sent = 0;
    const char *ptr = buffer;
    while (total_sent < length) {
        ssize_t sent = send(sock, ptr + total_sent, length - total_sent, 0);
        if (sent <= 0) {
            return -1;
        }
        total_sent += sent;
    }
    return 0;
}

int recv_all(int sock, void *buffer, size_t length) {
    size_t total_received = 0;
    char *ptr = buffer;
    while (total_received < length) {
        ssize_t received = recv(sock, ptr + total_received, length - total_received, 0);
        if (received <= 0) {
            return -1;
        }
        total_received += received;
    }
    return total_received;
}

uint64_t htonll(uint64_t value) {
    int num = 1;
    if (*(char *)&num == 1) {
        // Little endian
        uint32_t high_part = htonl((uint32_t)(value >> 32));
        uint32_t low_part = htonl((uint32_t)(value & 0xFFFFFFFFLL));
        return (((uint64_t)high_part) << 32) | low_part;
    } else {
        // Big endian
        return value;
    }
}

uint64_t ntohll(uint64_t value) {
    int num = 1;
    if (*(char *)&num == 1) {
        // Little endian
        uint32_t high_part = ntohl((uint32_t)(value >> 32));
        uint32_t low_part = ntohl((uint32_t)(value & 0xFFFFFFFFLL));
        return (((uint64_t)high_part) << 32) | low_part;
    } else {
        // Big endian
        return value;
    }
}
