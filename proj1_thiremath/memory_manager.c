/*
 * CS 551 Project "Memory manager".
 * This file needs to be turned in.	
 */


#include "memory_manager.h"

static STRU_MEM_LIST * mem_pool = NULL;

/*
 * Print out the current status of the memory manager.
 * Reading this function may help you understand how the memory manager organizes the memory.
 * Do not change the implementation of this function. It will be used to help the grading.
 */
void mem_mngr_print_snapshot(void)
{
    STRU_MEM_LIST * mem_list = NULL;

    printf("============== Memory snapshot ===============\n");

    mem_list = mem_pool; // Get the first memory list
    while(NULL != mem_list)
    {
        STRU_MEM_BATCH * mem_batch = mem_list->first_batch; // Get the first mem batch from the list 

        printf("mem_list %p slot_size %d batch_count %d free_slot_bitmap %p\n", 
                   mem_list, mem_list->slot_size, mem_list->batch_count, mem_list->free_slots_bitmap);
        bitmap_print_bitmap(mem_list->free_slots_bitmap, mem_list->bitmap_size);

        while (NULL != mem_batch)
        {
            printf("\t mem_batch %p batch_mem %p\n", mem_batch, mem_batch->batch_mem);
            mem_batch = mem_batch->next_batch; // get next mem batch
        }

        mem_list = mem_list->next_list;
    }

    printf("==============================================\n");
}

/*
 * Initialize the memory manager with 16 bytes(defined by the macro MEM_ALIGNMENT_BOUNDARY) slot size mem_list.
 * Initialize this list with 1 batch of slots.
 */
void mem_mngr_init(void)
{
    if (!mem_pool) {
        mem_pool = create_memory_list(MEM_ALIGNMENT_BOUNDARY);
    }
}

/*
 * Clean up the memory manager (e.g., release all the memory allocated)
 */
void mem_mngr_leave(void)
{
    STRU_MEM_LIST *current_list = mem_pool;
    while (current_list) {
        STRU_MEM_BATCH *current_batch = current_list->first_batch;
        while (current_batch) {
            STRU_MEM_BATCH *next_batch = current_batch->next_batch;
            free(current_batch->batch_mem);
            free(current_batch);
            current_batch = next_batch;
        }
        STRU_MEM_LIST *next_list = current_list->next_list;
        free(current_list->free_slots_bitmap);
        free(current_list);
        current_list = next_list;
    }
    mem_pool = NULL;
}

/*
 * Allocate a chunk of memory 	
 * @param size: size in bytes to be allocated
 * @return: the pointer to the allocated memory slot
 */
void * mem_mngr_alloc(size_t size)
{
    int aligned_size = SLOT_ALLINED_SIZE(size);
    if (aligned_size > 5 * MEM_ALIGNMENT_BOUNDARY) {
        return NULL;
    }

    // Find or create the list that matches the aligned slot size
    STRU_MEM_LIST *list = find_or_create_list(aligned_size);
    if (!list) return NULL;

    // Find the first available slot in the current bitmap
    int pos = bitmap_find_first_bit(list->free_slots_bitmap, list->bitmap_size, 1);
    
    if (pos == BITMAP_OP_NOT_FOUND) {
        // Create new batch
        STRU_MEM_BATCH *new_batch = (STRU_MEM_BATCH *)malloc(sizeof(STRU_MEM_BATCH));
        if (!new_batch) return NULL;

        new_batch->batch_mem = malloc(aligned_size * MEM_BATCH_SLOT_COUNT);
        if (!new_batch->batch_mem) {
            free(new_batch);
            return NULL;
        }

        // Add new batch to the list (append at end)
        STRU_MEM_BATCH *last_batch = list->first_batch;
        while (last_batch->next_batch != NULL) {
            last_batch = last_batch->next_batch;
        }
        last_batch->next_batch = new_batch;
        new_batch->next_batch = NULL;
        list->batch_count++;

        // Create new bitmap
        int new_bitmap_size = ((list->batch_count * MEM_BATCH_SLOT_COUNT + BIT_PER_BYTE - 1) / BIT_PER_BYTE);
        unsigned char *new_bitmap = (unsigned char *)malloc(new_bitmap_size);
        if (!new_bitmap) {
            free(new_batch->batch_mem);
            free(new_batch);
            return NULL;
        }

        // Copy old bitmap and set new slots as free
        if (list->free_slots_bitmap) {
            memcpy(new_bitmap, list->free_slots_bitmap, list->bitmap_size);
            memset(new_bitmap + list->bitmap_size, 0xFF, new_bitmap_size - list->bitmap_size);
            free(list->free_slots_bitmap);
        }
        
        list->free_slots_bitmap = new_bitmap;
        list->bitmap_size = new_bitmap_size;
        
        // Find first available slot in new bitmap
        pos = bitmap_find_first_bit(list->free_slots_bitmap, list->bitmap_size, 1);
    }

    // Mark slot as used
    bitmap_clear_bit(list->free_slots_bitmap, list->bitmap_size, pos);

    // Find the correct batch and slot
    int batch_index = pos / MEM_BATCH_SLOT_COUNT;
    int slot_index = pos % MEM_BATCH_SLOT_COUNT;

    STRU_MEM_BATCH *target_batch = list->first_batch;
    for (int i = 0; i < batch_index; i++) {
        target_batch = target_batch->next_batch;
    }

    return (void *)((char *)target_batch->batch_mem + slot_index * aligned_size);
}

/*
 * Free a chunk of memory pointed by ptr
 * Print out any error messages
 * @param: the pointer to the allocated memory slot
 */
void mem_mngr_free(void * ptr)
{
    if (!ptr) return;

    STRU_MEM_LIST *current_list = mem_pool;
    while (current_list) {
        STRU_MEM_BATCH *current_batch = current_list->first_batch;
        int batch_index = 0;

        while (current_batch) {
            char *batch_start = (char *)current_batch->batch_mem;
            char *batch_end = batch_start + (MEM_BATCH_SLOT_COUNT * current_list->slot_size);
            char *ptr_addr = (char *)ptr;

            // Verify if the pointer belongs to the batch
            if (ptr_addr >= batch_start && ptr_addr < batch_end) {
                ptrdiff_t offset = ptr_addr - batch_start;
                if (offset % current_list->slot_size != 0) {
                    printf("Error: ptr is not the starting address of any slot\n");
                    return;
                }

                // Calculate the slot index
                int slot_index = batch_index * MEM_BATCH_SLOT_COUNT + (offset / current_list->slot_size);

                // Check if already freed (double-free detection)
                if (bitmap_bit_is_set(current_list->free_slots_bitmap, current_list->bitmap_size, slot_index) == 1) {
                    printf("Error: ptr is the starting address of an unassigned slot - double freeing\n");
                    return;
                }

                // Free the slot by setting the bitmap bit
                bitmap_set_bit(current_list->free_slots_bitmap, current_list->bitmap_size, slot_index);
                return;
            }

            current_batch = current_batch->next_batch;
            batch_index++;
        }
        current_list = current_list->next_list;
    }

    printf("Error: ptr is outside of memory managed by the manager\n");
}

// Helper Functions

STRU_MEM_LIST* create_memory_list(int slot_size) {
    STRU_MEM_LIST *new_list = (STRU_MEM_LIST *)malloc(sizeof(STRU_MEM_LIST));
    if (!new_list) return NULL;

    new_list->slot_size = slot_size;
    new_list->batch_count = 1;
    new_list->bitmap_size = (MEM_BATCH_SLOT_COUNT + BIT_PER_BYTE - 1) / BIT_PER_BYTE;
    new_list->free_slots_bitmap = (unsigned char *)malloc(new_list->bitmap_size);
    if (!new_list->free_slots_bitmap) {
        free(new_list);
        return NULL;
    }

    memset(new_list->free_slots_bitmap, 0xFF, new_list->bitmap_size); // All slots initially free
    STRU_MEM_BATCH *first_batch = (STRU_MEM_BATCH *)malloc(sizeof(STRU_MEM_BATCH));
    if (!first_batch) {
        free(new_list->free_slots_bitmap);
        free(new_list);
        return NULL;
    }

    first_batch->batch_mem = malloc(slot_size * MEM_BATCH_SLOT_COUNT);
    if (!first_batch->batch_mem) {
        free(first_batch);
        free(new_list->free_slots_bitmap);
        free(new_list);
        return NULL;
    }
    first_batch->next_batch = NULL;
    new_list->first_batch = first_batch;
    new_list->next_list = NULL;

    return new_list;
}


STRU_MEM_LIST* find_or_create_list(int slot_size) {
    STRU_MEM_LIST *current_list = mem_pool;
    while (current_list) {
        if (current_list->slot_size == slot_size) {
            return current_list;
        }
        if (!current_list->next_list) break;
        current_list = current_list->next_list;
    }

    STRU_MEM_LIST *new_list = create_memory_list(slot_size);
    if (new_list) {
        if (current_list)
            current_list->next_list = new_list;
        else
            mem_pool = new_list;
    }
    return new_list;
}