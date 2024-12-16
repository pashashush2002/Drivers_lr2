#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE_NAME "/dev/reaction_time"

void test_reaction_time_device() {
    int fd;
    ssize_t bytes_written;
    ssize_t bytes_read;
    struct timespec *response_times;
    size_t response_count = 300; // Количество ожидаемых времен реакции

    // Открываем устройство
    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return;
    }

    printf("Device opened successfully.\n");

    // Имитируем события реакции
    for (int i = 0; i < response_count; i++) {
        // Записываем событие (здесь мы просто пишем что-то в устройство)
        bytes_written = write(fd, "1", 1);
        if (bytes_written < 0) {
            perror("Failed to write to device");
            close(fd);
            return;
        }

        sleep(1); // Ждем 1 секунду перед следующим событием
    }

    // Читаем времена реакции
    response_times = malloc(sizeof(struct timespec) * response_count);
    if (!response_times) {
        perror("Failed to allocate memory for response times");
        close(fd);
        return;
    }

    bytes_read = read(fd, response_times, sizeof(struct timespec) * response_count);
    if (bytes_read < 0) {
        perror("Failed to read from device");
        free(response_times);
        close(fd);
        return;
    }

    printf("Read %zd bytes from device:\n", bytes_read);
    
    // Выводим времена реакции
    for (int i = 0; i < bytes_read / sizeof(struct timespec); i++) {
        printf("Reaction Time %d: %ld ns\n", i*2 +1, 
               response_times[i].tv_sec);
        printf("Reaction Time %d: %ld ns\n", i * 2 + 2, 
               response_times[i].tv_nsec);
    }

    // Освобождаем память и закрываем устройство
    free(response_times);
    close(fd);
}

int main() {
    test_reaction_time_device();
    return 0;
}

