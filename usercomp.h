/*
 *  usercomp.h - definitions for static huffman-table
 *  Adapted from LinKT
 *
 *  Copyright (C) 1999 Johann Hanne, DH3MB. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <string.h>


//#ifdef _USERCOMP


struct huff_comp_table_struct
{
  unsigned short int code;
  unsigned char len;
};

struct huff_decomp_tree_struct
{
  signed short int node1;
  signed short int node2;
};


/*
 *
 * Pascal2C-Translation of the #HUF#-Compression by DF... (Andy).
 *
 * Translated by Jochen Sarrazin, DG6VJ
 *
 * Original Pascal-Code is Copyright by Andy, DF...
 * This C-Translation is Copyright by Jochen Sarrazin, DG6VJ (1999)
 */

struct TableTyp_struct
{
   unsigned short code;
   unsigned char len;
};



//#endif

