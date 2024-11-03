/*
 * CS 551 Project "Memory manager".
 * This file needs to be turned in.	
 */

#include "common.h"

/*
 * Find the position (starting from 0) of the first bit that has value "val" in the "bitmap". 
 * @param bitmap: start address of the bitmap
 * @param size: size in bytes of the bitmap
 * @param val: 0 or non-0
 * @return: the position in the bitmap of the first 0 bit (if "val" is zero) or the first 1 bit (if "val" is non-zero)
 *          BITMAP_OP_NOT_FOUND (defined in "common.h") if the bit of interest does not exist in the bitmap
 *          BITMAP_OP_ERROR (defined in "common.h") on any other errors
 */
int bitmap_find_first_bit(unsigned char * bitmap, int size, int val)
{
    if (!bitmap || size <= 0) return BITMAP_OP_ERROR;

    for (int byte = 0; byte < size; ++byte) {
        for (int bit = 0; bit < BIT_PER_BYTE; ++bit) {
            int current_bit = (bitmap[byte] >> bit) & 1;
            if (current_bit == val) {
                return byte * BIT_PER_BYTE + bit;
            }
        }
    }
    return BITMAP_OP_NOT_FOUND;
}

/*
 * Change the value of the "target_pos"-th bit (starting from 0) in the "bitmap" to 1.
 * @param bitmap: start address of the bitmap
 * @param size: size in bytes of the bit map
 * @param target_pos: the position of the bit of interest
 * @return: BITMAP_OP_SUCCEED (defined in "common.h") on success
 *          BITMAP_OP_ERROR (defined in "common.h") on any errors
 */
int bitmap_set_bit(unsigned char * bitmap, int size, int target_pos)
{
    if (!bitmap || target_pos < 0 || target_pos >= size * BIT_PER_BYTE) return BITMAP_OP_ERROR;

    int byte_index = target_pos / BIT_PER_BYTE;
    int bit_index = target_pos % BIT_PER_BYTE;
    bitmap[byte_index] |= (1 << bit_index);
    return BITMAP_OP_SUCCEED;
}

/*
 * Change the value of the "target_pos"-th bit (starting from 0) in the "bitmap" to 0.
 * @param bitmap: start address of the bitmap
 * @param size: size in bytes of the bit map
 * @param target_pos: the position of the bit of interest
 * @return: BITMAP_OP_SUCCEED (defined in "common.h") on success
 *          BITMAP_OP_ERROR (defined in "common.h") on any errors
 */
int bitmap_clear_bit(unsigned char * bitmap, int size, int target_pos)
{
    if (!bitmap || target_pos < 0 || target_pos >= size * BIT_PER_BYTE) return BITMAP_OP_ERROR;

    int byte_index = target_pos / BIT_PER_BYTE;
    int bit_index = target_pos % BIT_PER_BYTE;
    bitmap[byte_index] &= ~(1 << bit_index);
    return BITMAP_OP_SUCCEED;
}


/*
 * Test if the value of the "pos"-th bit (starting from 0) in the "bitmap" is 1.
 * @param bitmap: start address of the bitmap
 * @param size: size in bytes of the bit map
 * @param pos: the position of the bit of interest
 * @return: the value of the bit of interest (i.e., 0 or 1) on success
 *          BITMAP_OP_ERROR (defined in "common.h") on any errors
 */
int bitmap_bit_is_set(unsigned char * bitmap, int size, int pos)
{
    if (!bitmap || pos < 0 || pos >= size * BIT_PER_BYTE) return BITMAP_OP_ERROR;

    int byte_index = pos / BIT_PER_BYTE;
    int bit_index = pos % BIT_PER_BYTE;
    return (bitmap[byte_index] >> bit_index) & 1;
}

/*
 * Print the content of a bitmap in starting from the first bit, and insert a space every 4 bits
 * @param bitmap: start address of the bitmap
 * @param size: size in bytes of the bit map
 * @return: BITMAP_OP_SUCCEED (defined in "common.h") on success
 *          BITMAP_OP_ERROR (defined in "common.h") on any errors
 * Do not change the implementation of this function.
 */
int bitmap_print_bitmap(unsigned char * bitmap, int size)
{
    int pos = 0;
    int total_bits = size * BIT_PER_BYTE;
    unsigned char current_byte = 0;

    if (NULL == bitmap)
    {
        printf("ERROR: NULL bit map!\n");
        return BITMAP_OP_ERROR;
    }

    printf("bitmap %p size %d bytes: ", bitmap, size);

    while (pos < total_bits)
    {
        int v = 0;
        
        current_byte = *(bitmap + pos/BIT_PER_BYTE);
        v = ((current_byte >> pos % BIT_PER_BYTE) & 0x01);
        printf("%d", v);
        if ((pos + 1) % 4 == 0)
        {
            printf("  ");
        }
        
        pos++;
    }

    printf("\n");

    return BITMAP_OP_SUCCEED;
}
