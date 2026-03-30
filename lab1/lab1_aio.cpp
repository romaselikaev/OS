#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <cstddef>
#endif
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <atomic>
#include<sys/stat.h>
#include<sys/types.h>
#include<inttypes.h>
#include<stdint.h>

struct cp_context {
    int fd_src;
    int fd_dest;
    std::atomic<int> active_ops{0};
    struct timespec start_time;
    
    cp_context(int src, int dest) : fd_src(src), fd_dest(dest) {
        clock_gettime(CLOCK_MONOTONIC, &start_time);
    }

    double get_elapsed_time() {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        return (now.tv_sec - start_time.tv_sec) + (now.tv_nsec - start_time.tv_nsec) * 1e-9;
    }
};

struct aio_operation {
    struct aiocb aio;
    char *buffer;
    int write_operation;
    void* next_operation;
    cp_context* ctx;

    aio_operation(size_t buffer_size, cp_context* context): write_operation(1), ctx(context){
        buffer = new char[buffer_size];
        memset(&aio, 0, sizeof(struct aiocb));
    }

    ~aio_operation() {
        delete[] buffer;
    }
};

void aio_completion_handler(sigval_t sigval) {
    aio_operation *aio_op = (struct aio_operation *)sigval.sival_ptr;
    cp_context *ctx = aio_op->ctx;
    
    ssize_t bytes = aio_return(&aio_op->aio);
    if (aio_op->write_operation) {
        aio_op -> write_operation = 0;
        aio_op -> aio.aio_fildes = ctx -> fd_dest;
        aio_op -> aio.aio_nbytes = bytes;
        aio_write(&aio_op->aio);
    }
    else {
        ctx->active_ops--;
        delete aio_op;
    }
}

double aio_copy(const char* src, const char* dst, size_t block_size, int max_overlapped){
    int read_file = open(src, O_RDONLY | O_NONBLOCK);
    int wright_file = open(dst, O_CREAT | O_WRONLY | O_TRUNC | O_NONBLOCK, 0666);

    struct stat read_file_data;
    fstat(read_file, &read_file_data);
    off_t read_file_size = read_file_data.st_size;
    off_t cur_read_position = 0;

    cp_context *ctx = new cp_context(read_file, wright_file);


    while(cur_read_position < read_file_size || ctx->active_ops >0){
        while(ctx->active_ops < max_overlapped && cur_read_position < read_file_size){
            size_t read_block;
            if(cur_read_position + block_size <= read_file_size){
                read_block = block_size;
            }
            else {
                read_block = read_file_size - cur_read_position;
            }

            aio_operation *cur_op = new aio_operation(read_block, ctx);
            cur_op -> aio.aio_fildes = read_file;
            cur_op -> aio.aio_buf = cur_op->buffer;
            cur_op -> aio.aio_nbytes = read_block;
            cur_op -> aio.aio_offset = cur_read_position;
            
            cur_op -> aio.aio_sigevent.sigev_notify = SIGEV_THREAD;
            cur_op -> aio.aio_sigevent.sigev_notify_function = aio_completion_handler;
            cur_op -> aio.aio_sigevent.sigev_value.sival_ptr = cur_op;

            ctx -> active_ops ++;
            aio_read(&cur_op->aio);
            cur_read_position += read_block;
        }
    }

    double time = ctx->get_elapsed_time();

    delete ctx;
    return time;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << ": ./название кода <файл исходный> <файл назначения> <размер блока> <количество операций" << std::endl;
        return 1;
    }

    double t = aio_copy(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
    printf("Выполнено. Время: %.4f сек\n", t);
    return 0;
}