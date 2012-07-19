
#ifndef __LACP_BITMAP_H
#define __LACP_BITMAP_H

#define MAX_NUMBER_OF_PORTS 32

#define LACP_MAX_BITMAP_BYTE 18

typedef struct lacp_bitmap_s
{
	char bmp_array[LACP_MAX_BITMAP_BYTE];     
} lacp_bitmap_t;

int lacp_bitmap_get_bit(lacp_bitmap_t * BitmapPtr, int Bit) ;
void lacp_bitmap_set_bit(lacp_bitmap_t * BitmapPtr, int Bit) ;
void lacp_bitmap_clear(lacp_bitmap_t *BitmapPtr) ;
void lacp_bitmap_clear_bit(lacp_bitmap_t * BitmapPtr, int Bit) ;


#endif /* __BITMAP_H */


