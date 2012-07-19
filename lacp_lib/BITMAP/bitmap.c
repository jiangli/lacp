#include <string.h>
#include "bitmap.h"

void lacp_bitmap_clear(lacp_bitmap_t *BitmapPtr) 
{
	memset(BitmapPtr->bmp_array, 0, LACP_MAX_BITMAP_BYTE); 
	return;
}

void lacp_bitmap_set_all_bits(lacp_bitmap_t * BitmapPtr)
{
	memset(BitmapPtr->bmp_array, 0xFF, LACP_MAX_BITMAP_BYTE);
	return;
}

void lacp_bitmap_clear_bits(lacp_bitmap_t * BitmapPtr, lacp_bitmap_t * BitmapBitsPtr)
{
	int i; 
	for (i = 0; i < LACP_MAX_BITMAP_BYTE; i++)
	{
		BitmapPtr->bmp_array[i] &= ~(BitmapBitsPtr->bmp_array[i]); 
	}

	return;
}
         
void lacp_bitmap_set_bits(lacp_bitmap_t * BitmapPtr, lacp_bitmap_t * BitmapBitsPtr) 
{
	int i; 
	for (i = 0; i < LACP_MAX_BITMAP_BYTE; i++)
	{
		BitmapPtr->bmp_array[i] |= BitmapBitsPtr->bmp_array[i]; 
	}
}

void lacp_bitmap_or(lacp_bitmap_t * ResultPtr, lacp_bitmap_t * BitmapPtr1,lacp_bitmap_t * BitmapPtr2)
{
	int i; 
	for (i = 0; i < LACP_MAX_BITMAP_BYTE; i++) 
	{
		ResultPtr->bmp_array[i] = (BitmapPtr1)->bmp_array[i] | (BitmapPtr2)->bmp_array[i]; 
	 
	}
}

void lacp_bitmap_and(lacp_bitmap_t * ResultPtr,lacp_bitmap_t * BitmapPtr1,lacp_bitmap_t * BitmapPtr2) 
{ 
	int i; 
	for (i = 0; i < LACP_MAX_BITMAP_BYTE; i++) 
		(ResultPtr)->bmp_array[i] = (BitmapPtr1)->bmp_array[i] & (BitmapPtr2)->bmp_array[i];  
}

void lacp_bitmap_not(lacp_bitmap_t * ResultPtr, lacp_bitmap_t * BitmapPtr) 
{ 
	int i; 
	for (i = 0; i < LACP_MAX_BITMAP_BYTE; i++) 
		(ResultPtr)->bmp_array[i]= ~((BitmapPtr)->bmp_array[i]);   
}

/* Return zero if identical */
int lacp_bitmap_cmp(lacp_bitmap_t * BitmapPtr1, lacp_bitmap_t * BitmapPtr2) 
{
	return memcmp(BitmapPtr1->bmp_array, BitmapPtr2->bmp_array, LACP_MAX_BITMAP_BYTE);
}

int lacp_bitmap_is_zero(lacp_bitmap_t *BitmapPtr) 
{
	lacp_bitmap_t  tempBitmap;	
	memset(tempBitmap.bmp_array, 0, LACP_MAX_BITMAP_BYTE);	
	return !memcmp((BitmapPtr)->bmp_array, &tempBitmap, LACP_MAX_BITMAP_BYTE);	
}
        

int lacp_bitmap_is_all_ones(lacp_bitmap_t * BitmapPtr) 
{
	lacp_bitmap_t tempBitmap;	
	memset(tempBitmap.bmp_array, 0xFF, LACP_MAX_BITMAP_BYTE);	
	return memcmp((BitmapPtr)->bmp_array, &tempBitmap, LACP_MAX_BITMAP_BYTE);	
}

int lacp_bitmap_get_bit(lacp_bitmap_t * BitmapPtr, int Bit) 
{ 
	return   (BitmapPtr)->bmp_array[Bit/8] & (1 << (Bit % 8));
}

void lacp_bitmap_set_bit(lacp_bitmap_t * BitmapPtr, int Bit) 
{
	(BitmapPtr)->bmp_array[Bit/8] |= (1 << (Bit % 8)); 
	return;
}

void lacp_bitmap_clear_bit(lacp_bitmap_t * BitmapPtr, int Bit) 
{
	(BitmapPtr)->bmp_array[Bit/8]&= ~(1 << (Bit % 8));
	return;
}

void lacp_bitmap_copy(lacp_bitmap_t * BitmapDstPtr, lacp_bitmap_t * BitmapSrcPtr) 
{ 
	memcpy((BitmapDstPtr)->bmp_array, (BitmapSrcPtr)->bmp_array, LACP_MAX_BITMAP_BYTE); 
    return;
}

void lacp_bitmap_xor(lacp_bitmap_t * ResultPtr, lacp_bitmap_t * BitmapPtr1,lacp_bitmap_t * BitmapPtr2) 
{ 
	int i; 
	for (i = 0; i < LACP_MAX_BITMAP_BYTE; i++) 
		(ResultPtr)->bmp_array[i]= (BitmapPtr1)->bmp_array[i]^ (BitmapPtr2)->bmp_array[i];   
		
	return;	
}

