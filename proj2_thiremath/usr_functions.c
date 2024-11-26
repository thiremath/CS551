#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "common.h"
#include "usr_functions.h"

int writeToFile(int len, int fd_out, char line[], unsigned long size, int letter_counter_arr[],char *data, int flag){

    int letter_counter=0;

    while (letter_counter < 26) {
        
        len = snprintf(line, size, "%c %d\n", 'A' + letter_counter, letter_counter_arr[letter_counter]);

        if (write(fd_out, line, len) == len) {
            letter_counter++;
            continue;
        }

        printf("Unable to Write");
        return -1;
        // free(data);
    }

    if(flag == 1){
        free(data);
    }

    return 0;
}

/* User-defined map function for the "Letter counter" task.  
   This map function is called in a map worker process.
   @param split: The data split that the map function is going to work on.
                 Note that the file offset of the file descripter split->fd should be set to the properly
                 position when this map function is called.
   @param fd_out: The file descriptor of the itermediate data file output by the map function.
   @ret: 0 on success, -1 on error.
 */
int letter_counter_map(DATA_SPLIT * split, int fd_out)
{
    // add your implementation here ...
    int letter_counter_arr[26] = {0};
    char line[20];
    char c;
    int letter_counter = 0;

    if (split!=NULL && fd_out >= 0) {

        char *data = malloc(split->size);

        if (data!=NULL) {

            ssize_t bytesRead = read(split->fd, data, split->size);

            if (bytesRead == split->size) {

                while (letter_counter < split->size) {
                    c = data[letter_counter];
                    if (c >= 'A' && c <= 'Z') {
                        letter_counter_arr[c - 'A']++;
                    } else if (c >= 'a' && c <= 'z') {
                        letter_counter_arr[c - 'a']++;
                    }
                    letter_counter++;
                }
                return writeToFile(0, fd_out, line,sizeof(line), letter_counter_arr, data, 1);

            }

            free(data);

        }

    }

    return -1;
}

/* User-defined reduce function for the "Letter counter" task.  
   This reduce function is called in a reduce worker process.
   @param p_fd_in: The address of the buffer holding the intermediate data files' file descriptors.
                   The imtermeidate data files are output by the map worker processes, and they
                   are the input for the reduce worker process.
   @param fd_in_num: The number of the intermediate files.
   @param fd_out: The file descriptor of the final result file.
   @ret: 0 on success, -1 on error.
   @example: if fd_in_num == 3, then there are 3 intermediate files, whose file descriptor is 
             identified by p_fd_in[0], p_fd_in[1], and p_fd_in[2] respectively.

*/
int letter_counter_reduce(int * p_fd_in, int fd_in_num, int fd_out)
{
    // add your implementation here ...
    int letter_counter_arr[26] = {0};
    char data[256];
    char *ptr;
    char line[20];
    unsigned long data_size = sizeof(data);
    int fd_in;
    ssize_t byte_read;

    if ( p_fd_in!=NULL && fd_in_num >= 0){
        for (int i = 0; i < fd_in_num; i++)
        {   
            fd_in = p_fd_in[i];

            while ((byte_read = read(fd_in, data, data_size)) > 0)
            {
                ptr = data;
                while (ptr < data + byte_read)
                {   
                    int count;
                    char letter;
                    int n;

                    if ((n = sscanf(ptr, "%c %d\n", &letter, &count)) == 2)
                    {
                        letter_counter_arr[letter - 'A'] += count;

                        ptr = strchr(ptr, '\n');
                        if (ptr == NULL)
                        {
                            break;
                        }
                        ptr++;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if (byte_read < 0)
            {
                for (int j = 0; j <= i; j++)
                {
                    close(p_fd_in[j]);
                }
                return -1;
            }

            close(fd_in);
        }

        return writeToFile(0, fd_out, line, sizeof(line), letter_counter_arr, NULL, 0);

    }

    return -1;

}

/* User-defined map function for the "Word finder" task.  
   This map function is called in a map worker process.
   @param split: The data split that the map function is going to work on.
                 Note that the file offset of the file descripter split->fd should be set to the properly
                 position when this map function is called.
   @param fd_out: The file descriptor of the itermediate data file output by the map function.
   @ret: 0 on success, -1 on error.
 */
int word_finder_map(DATA_SPLIT * split, int fd_out)
{
    // add your implementation here ...
    if (split != NULL && split->usr_data != NULL) {

        // Allocate buffer to hold the data from the split
        char *data = malloc(split->size + 1);
        if (data != NULL) {

            // Read data into the buffer
            ssize_t bytesRead = read(split->fd, data, split->size);
            if (bytesRead >= 0) {
                // Null-terminate the buffer
                data[bytesRead] = '\0';

                // Extract the target word from user data
                const char *targetWord = (const char *)split->usr_data;
        
                // Tokenize the buffer line by line
                char *line = strtok(data, "\n");
                while (line) {
                    // Check if the current line contains the target word
                    if (strstr(line, targetWord)) {
                        // Write the matching line to the output file descriptor
                        if (dprintf(fd_out, "%s\n", line) < 0) {
                            free(data);
                            return -1;
                        }
                    }
                    line = strtok(NULL, "\n");
                }

                // Clean up and return success
                free(data);
                return 0;
            }
            free(data);
        }
        
    }
    return -1;
}

/* User-defined reduce function for the "Word finder" task.  
   This reduce function is called in a reduce worker process.
   @param p_fd_in: The address of the buffer holding the intermediate data files' file descriptors.
                   The imtermeidate data files are output by the map worker processes, and they
                   are the input for the reduce worker process.
   @param fd_in_num: The number of the intermediate files.
   @param fd_out: The file descriptor of the final result file.
   @ret: 0 on success, -1 on error.
   @example: if fd_in_num == 3, then there are 3 intermediate files, whose file descriptor is 
             identified by p_fd_in[0], p_fd_in[1], and p_fd_in[2] respectively.

*/
int word_finder_reduce(int * p_fd_in, int fd_in_num, int fd_out)
{
    // add your implementation here ...
    // Buffer to read intermediate data
    char buffer[256];
    ssize_t bytesRead;
    int fd_in;
    
    if (p_fd_in!= NULL && fd_in_num > 0) {
    
        for (int i = 0; i < fd_in_num; i++) {
            fd_in = p_fd_in[i];

            // Read from the current intermediate file
            while ((bytesRead = read(fd_in, buffer, sizeof(buffer))) > 0) {
                // Write the read data to the final result file descriptor
                if (write(fd_out, buffer, bytesRead) != bytesRead) {
                    return -1;
                }
            }

            // Check for reading error
            if (bytesRead < 0) {
                return -1;
            }

            // Close the current intermediate file
            close(fd_in);
        }
    return 0;

    }

    return -1;
}


