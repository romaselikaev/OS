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
int file_created = 0;
char FILENAME[256];

void create_projection() {
    if (fd != -1) {
        printf("Проецирование уже создано.\n");
        return;
    }
    
    printf("Введите имя файла для проецирования: ");
    fgets(FILENAME, sizeof(FILENAME), stdin);
    FILENAME[strcspn(FILENAME, "\n")] = '\0';
    if (strlen(FILENAME) == 0) {
        printf("Имя файла не может быть пустым.\n");
        return;
    }
    
    fd = open(FILENAME, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Ошибка open");
        return;
    }
    
    if (ftruncate(fd, FILESIZE) == -1) {
        perror("Ошибка ftruncate");
        close(fd);
        fd = -1;
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
    
    ptr->version = 0;
    ptr->data[0] = '\0';
    file_created = 1;
    printf("\nФайл спроецирован по адресу %p\n", ptr);
}

void write_data_and_wait() {
    if (!ptr) {
        printf("Нет файла проецирования. Сначала создайте его в пункте меню 1.\n");
        return;
    }
    
    printf("Введите строку для записи: ");
    char buffer[256];
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    strcpy(ptr->data, buffer);
    ptr->version++;
    printf("\nДанные записаны: %s\n", ptr->data);
    
    if (mkfifo(FIFO_NOTIFY, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo notify");
        return;
    }
    
    fd_fifo = open(FIFO_NOTIFY, O_WRONLY);
    if (fd_fifo == -1) {
        perror("Ошибка открытия FIFO notify");
        return;
    }
    
    char c = 1;
    write(fd_fifo, &c, 1);
    close(fd_fifo);
    fd_fifo = -1;
    printf("Ожидание подтверждения чтения от клиента...\n");
    
    if (mkfifo(FIFO_CONFIRM, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo confirm");
        return;
    }
    
    fd_confirm = open(FIFO_CONFIRM, O_RDONLY);
    if (fd_confirm == -1) {
        perror("Ошибка открытия FIFO confirm");
        return;
    }
    
    char confirm;
    read(fd_confirm, &confirm, 1);
    close(fd_confirm);
    fd_confirm = -1;
    printf("\nКлиент подтвердил чтение данных\n");
    printf("Чтение успешно завершено\n");
    printf("Теперь могут быть произведены отмена проецирования и удаление проецируемого файла\n");
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
        printf("Проецирование не было создано\n");
    }
    
    if (fd != -1) {
        close(fd);
        fd = -1;
        printf("Файловый дескриптор закрыт\n");
    }
    
    if (file_created) {
        if (unlink(FILENAME) == 0) {
            printf("Файл %s успешно удалён\n", FILENAME);
        } else {
            perror("Ошибка удаления файла");
        }
        file_created = 0;
    }
    unlink(FIFO_NOTIFY);
    unlink(FIFO_CONFIRM);
    printf("Все ресурсы освобождены\n");
    printf("\nНажмите Enter для завершения работы программы...");
    getchar();
}

int main() {
    int choice;
    char input[10];
    setbuf(stdout, NULL);
    
    do {
        printf("\nМеню сервера: \n");
        printf("1. Выполнить проецирование\n");
        printf("2. Записать данные\n");
        printf("3. Завершить работу\n");
        printf("Ваш выбор: ");
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);
        switch (choice) {
            case 1: create_projection(); break;
            case 2: write_data_and_wait(); break;
            case 3:
                finish();
                return 0;
            default:
                printf("Неверный выбор. Введите 1, 2 или 3.\n");
        }
    } while (1);
    return 0;
}