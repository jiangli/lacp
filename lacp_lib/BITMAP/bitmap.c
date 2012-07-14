#include <string.h>
#include "bitmap.h"

void BitmapClear(BITMAP_T *BitmapPtr) 
{
	memset(BitmapPtr->part, 0, LAC_MAX_BITMAP_BYTE); 
	return;
}

void BitmapSetAllBits(BITMAP_T * BitmapPtr)
{
	memset(BitmapPtr->part, 0xFF, LAC_MAX_BITMAP_BYTE);
	return;
}

void BitmapClearBits(BITMAP_T * BitmapPtr, BITMAP_T * BitmapBitsPtr)
{
	int i; 
	for (i = 0; i < LAC_MAX_BITMAP_BYTE; i++)
	{
		BitmapPtr->part[i] &= ~(BitmapBitsPtr->part[i]); 
	}

	return;
}
         
void BitmapSetBits(BITMAP_T * BitmapPtr, BITMAP_T * BitmapBitsPtr) 
{
	int i; 
	for (i = 0; i < LAC_MAX_BITMAP_BYTE; i++)
	{
		BitmapPtr->part[i] |= BitmapBitsPtr->part[i]; 
	}
}

void BitmapOr(BITMAP_T * ResultPtr, BITMAP_T * BitmapPtr1,BITMAP_T * BitmapPtr2)
{
	int i; 
	for (i = 0; i < LAC_MAX_BITMAP_BYTE; i++) 
	{
		ResultPtr->part[i] = (BitmapPtr1)->part[i] | (BitmapPtr2)->part[i]; 
	 
	}
}

void BitmapAnd(BITMAP_T * ResultPtr,BITMAP_T * BitmapPtr1,BITMAP_T * BitmapPtr2) 
{ 
	int i; 
	for (i = 0; i < LAC_MAX_BITMAP_BYTE; i++) 
		(ResultPtr)->part[i] = (BitmapPtr1)->part[i] & (BitmapPtr2)->part[i];  
}

void BitmapNot(BITMAP_T * ResultPtr, BITMAP_T * BitmapPtr) 
{ 
	int i; 
	for (i = 0; i < LAC_MAX_BITMAP_BYTE; i++) 
		(ResultPtr)->part[i]= ~((BitmapPtr)->part[i]);   
}

/* Return zero if identical */
int BitmapCmp(BITMAP_T * BitmapPtr1, BITMAP_T * BitmapPtr2) 
{
	return memcmp(BitmapPtr1->part, BitmapPtr2->part, LAC_MAX_BITMAP_BYTE);
}

int BitmapIsZero(BITMAP_T *BitmapPtr) 
{
	BITMAP_T  tempBitmap;	
	memset(tempBitmap.part, 0, LAC_MAX_BITMAP_BYTE);	
	return !memcmp((BitmapPtr)->part, &tempBitmap, LAC_MAX_BITMAP_BYTE);	
}
        

int BitmapIsAllOnes(BITMAP_T * BitmapPtr) 
{
	BITMAP_T tempBitmap;	
	memset(tempBitmap.part, 0xFF, LAC_MAX_BITMAP_BYTE);	
	return memcmp((BitmapPtr)->part, &tempBitmap, LAC_MAX_BITMAP_BYTE);	
}

int BitmapGetBit(BITMAP_T * BitmapPtr, int Bit) 
{ 
	return   (BitmapPtr)->part[Bit/8] & (1 << (Bit % 8));
}

void BitmapSetBit(BITMAP_T * BitmapPtr, int Bit) 
{
	(BitmapPtr)->part[Bit/8] |= (1 << (Bit % 8)); 
	return;
}

void BitmapClearBit(BITMAP_T * BitmapPtr, int Bit) 
{
	(BitmapPtr)->part[Bit/8]&= ~(1 << (Bit % 8));
	return;
}

void BitmapCopy(BITMAP_T * BitmapDstPtr, BITMAP_T * BitmapSrcPtr) 
{ 
	memcpy((BitmapDstPtr)->part, (BitmapSrcPtr)->part, LAC_MAX_BITMAP_BYTE); 
    return;
}

void BitmapXor(BITMAP_T * ResultPtr, BITMAP_T * BitmapPtr1,BITMAP_T * BitmapPtr2) 
{ 
	int i; 
	for (i = 0; i < LAC_MAX_BITMAP_BYTE; i++) 
		(ResultPtr)->part[i]= (BitmapPtr1)->part[i]^ (BitmapPtr2)->part[i];   
		
	return;	
}

