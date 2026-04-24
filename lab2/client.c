#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

#define FILESIZE 4096
#define FIFO_NOTIFY "/tmp/fifo_notify"
#define FIFO_CONFIRM "/tmp/fifo_confirm"

typedef struct {
    int version;
    char data[FILESIZE - sizeof(int)];
} SharedData;

int fd = -1;
int fd_fifo = -1;
int fd_confirm = -1;
SharedData *ptr = NULL;
char FILENAME[256];

void open_projection() {
    if (fd != -1) {
        printf("Проецирование уже открыто.\n");
        return;
    }
    
    printf("Введите имя файла для открытия: ");
    fgets(FILENAME, sizeof(FILENAME), stdin);
    FILENAME[strcspn(FILENAME, "\n")] = '\0';
    
    if (strlen(FILENAME) == 0) {
        printf("Имя файла не может быть пустым.\n");
        return;
    }
    
    fd = open(FILENAME, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Ошибка open");
        printf("Возможно, сервер ещё не создал файл или имя введено неверно.\n");
        return;
    }
    
    ptr = (SharedData*)mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Ошибка mmap");
        close(fd);
        fd = -1;
        ptr = NULL;
        return;
    }
    
    printf("\nФайл спроецирован по адресу %p\n", ptr);
}

void read_data() {
    if (!ptr) {
        printf("Нет файла проецирования. Сначала откройте его в пункте меню 1.\n");
        return;
    }
    
    if (mkfifo(FIFO_NOTIFY, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo notify");
        return;
    }
    
    printf("\nОжидание уведомления и данных от сервера...\n");
    fd_fifo = open(FIFO_NOTIFY, O_RDONLY);
    if (fd_fifo == -1) {
        perror("Ошибка открытия FIFO notify");
        return;
    }
    
    char notify;
    read(fd_fifo, &notify, 1);
    close(fd_fifo);
    fd_fifo = -1;
    printf("\nПолучено уведомление от сервера.\n");
    printf("Чтение данных завершено.\n");
    printf("Полученные данные: %s\n", ptr->data);
    
    if (mkfifo(FIFO_CONFIRM, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo confirm");
        return;
    }
    
    fd_confirm = open(FIFO_CONFIRM, O_WRONLY);
    if (fd_confirm == -1) {
        perror("Ошибка открытия FIFO confirm");
        return;
    }
    
    char confirm = 1;
    write(fd_confirm, &confirm, 1);
    close(fd_confirm);
    fd_confirm = -1;
    printf("Сообщение об успешном чтении данных отправлено на сервер\n");
    printf("\nНажмите Enter для возврата в меню...");
    getchar();
}

void finish() {
    printf("\nОтменяем проецирование (munmap)\n");
    if (ptr) {
        if (munmap(ptr, FILESIZE) == 0) {
            printf("Проецирование успешно отменено\n");
        } else {
            perror("Ошибка munmap");
        }
        ptr = NULL;
    } else {
        printf("Проецирование не было открыто\n");
    }
    
    if (fd != -1) {
        close(fd);
        fd = -1;
        printf("Файловый дескриптор закрыт\n");
    }
    
    printf("Все ресурсы освобождены\n");
    printf("\nНажмите Enter для завершения работы программы...");
    getchar();
}

int main() {
    int choice;
    char input[10];
    setbuf(stdout, NULL);
    
    do {
        printf("\nМеню клиента: \n");
        printf("1. Выполнить проецирование\n");
        printf("2. Прочитать данные\n");
        printf("3. Завершить работу\n");
        printf("Ваш выбор: ");
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);
        switch (choice) {
            case 1: open_projection(); break;
            case 2: read_data(); break;
            case 3:
                finish();
                return 0;
            default:
                printf("Неверный выбор. Введите 1, 2 или 3.\n");
        }
    } while (1);
    
    return 0;
}