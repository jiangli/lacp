/************************************************************************ 
 * RSTP library - Rapid Spanning Tree (802.1t, 802.1w) 
 * Copyright (C) 2001-2003 Optical Access 
 * Author: Alex Rozin 
 * 
 * This file is part of RSTP library. 
 * 
 * RSTP library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation; version 2.1 
 * 
 * RSTP library is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser 
 * General Public License for more details. 
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with RSTP library; see the file COPYING.  If not, write to the Free 
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 * 02111-1307, USA. 
 **********************************************************************/

#ifndef __BITMAP_H
#define __BITMAP_H

#define MAX_NUMBER_OF_PORTS 32

#define LACP_MAX_BITMAP_BYTE 18

typedef struct lacp_bitmap_s
{
	char bit_array[LACP_MAX_BITMAP_BYTE];     
} lacp_bitmap_t;

int lacp_bitmap_get_bit(lacp_bitmap_t * BitmapPtr, int Bit) ;
void lacp_bitmap_set_bit(lacp_bitmap_t * BitmapPtr, int Bit) ;
void lacp_bitmap_clear(lacp_bitmap_t *BitmapPtr) ;
void lacp_bitmap_clear_bit(lacp_bitmap_t * BitmapPtr, int Bit) ;


#endif /* __BITMAP_H */


