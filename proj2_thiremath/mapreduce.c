#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mapreduce.h"
#include "common.h"

// add your code here ...
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

void mapreduce(MAPREDUCE_SPEC * spec, MAPREDUCE_RESULT * result)
{
    // add you code here ...
    pid_t pid;
    int i, fd_in, fd_out;
    
    struct timeval start, end;

    if (NULL == spec || NULL == result)
    {
        EXIT_ERROR(ERROR, "NULL pointer!\n");
    }
    
    gettimeofday(&start, NULL);

    // add your code here ...
    // Open input file
    fd_in = open(spec->input_data_filepath, O_RDONLY);
    if (fd_in < 0) {
        EXIT_ERROR(ERROR, "Failed to open input file!\n");
    }

    // Determine file size and split size
    int file_size = lseek(fd_in, 0, SEEK_END);
    if (file_size < 0) {
        close(fd_in);
        EXIT_ERROR(ERROR, "Failed to determine file size!\n");
    }
    // int split_size = file_size / spec->split_num;
    int split_size = (file_size + spec->split_num - 1) / spec->split_num; // Ensure all data is processed
    lseek(fd_in, 0, SEEK_SET);

    // Create map worker processes
    for (i = 0; i < spec->split_num; ++i) {

        // Calculate the maximum possible length needed for the filename
        int filename_length = strlen("mr-") + snprintf(NULL, 0, "%d", spec->split_num - 1) + strlen(".itm") + 1;  // Extra +1 for the null terminator

        // Allocate memory for the intermediate filename
        char *intermediate_file = (char *)malloc(filename_length * sizeof(char));
        if (intermediate_file == NULL) {
            EXIT_ERROR(ERROR, "Memory allocation failed for intermediate file name!\n");
        }

        // Generate the intermediate file name based on the split index
        snprintf(intermediate_file, filename_length, "mr-%d.itm", i);

        // Now you can safely use intermediate_file for creating the file
        remove(intermediate_file);
        fd_out = open(intermediate_file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (fd_out < 0) {
            free(intermediate_file);  // Free memory before returning
            close(fd_in);
            EXIT_ERROR(ERROR, "Failed to create intermediate file!\n");
        }


        // Fork a map worker
        pid = fork();
        if (pid < 0) {
            close(fd_in);
            close(fd_out);
            EXIT_ERROR(ERROR, "Fork failed!\n");
        } else if (pid == 0) {
            // Child process: execute map function
            int dup_fd_in = dup(fd_in); // Duplicate file descriptor for the worker
            if (dup_fd_in < 0) {
                close(fd_in);
                close(fd_out);
                _EXIT_ERROR(ERROR, "Failed to duplicate file descriptor!\n");
            }

            // lseek(dup_fd_in, i * split_size, SEEK_SET);  // Seek to split start
            DATA_SPLIT split = {
                .fd = dup_fd_in,
                .size = (i == spec->split_num - 1) ? file_size - i * split_size : split_size,
                .usr_data = spec->usr_data
            };

            if (spec->map_func(&split, fd_out) < 0) {
                close(dup_fd_in);
                close(fd_out);
                _EXIT_ERROR(ERROR, "Map function failed!\n");
            }

            close(dup_fd_in);
            close(fd_out);
            _exit(SUCCESS);
        }

        // Parent process: record worker PID and close file
        result->map_worker_pid[i] = pid;
        close(fd_out);
    }

    // Wait for all map workers to complete
    for (i = 0; i < spec->split_num; ++i) {
        int status;
        if (waitpid(result->map_worker_pid[i], &status, 0) < 0) {
            EXIT_ERROR(ERROR, "Error waiting for map worker!\n");
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            EXIT_ERROR(ERROR, "Map worker failed!\n");
        }
    }

    // Open intermediate files for reduce
    int intermediate_fds[spec->split_num];
    for (i = 0; i < spec->split_num; ++i) {
        char intermediate_file[32];
        snprintf(intermediate_file, sizeof(intermediate_file), "mr-%d.itm", i);
        intermediate_fds[i] = open(intermediate_file, O_RDONLY);
        if (intermediate_fds[i] < 0) {
            close(fd_in);
            for (int j = 0; j < i; ++j) {
                close(intermediate_fds[j]);
            }
            EXIT_ERROR(ERROR, "Failed to open intermediate file!\n");
        }
    }

    // Create result file
    fd_out = open(result->filepath, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd_out < 0) {
        close(fd_in);
        for (i = 0; i < spec->split_num; ++i) {
            close(intermediate_fds[i]);
        }
        EXIT_ERROR(ERROR, "Failed to create result file!\n");
    }

    // Execute reduce function
    if (spec->reduce_func(intermediate_fds, spec->split_num, fd_out) < 0) {
        close(fd_in);
        close(fd_out);
        for (i = 0; i < spec->split_num; ++i) {
            close(intermediate_fds[i]);
        }
        EXIT_ERROR(ERROR, "Reduce function failed!\n");
    }

    // Close all files
    close(fd_out);
    for (i = 0; i < spec->split_num; ++i) {
        close(intermediate_fds[i]);
    }

    gettimeofday(&end, NULL);   

    result->processing_time = (end.tv_sec - start.tv_sec) * US_PER_SEC + (end.tv_usec - start.tv_usec);
}
