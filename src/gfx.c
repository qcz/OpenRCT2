/*****************************************************************************
 * Copyright (c) 2014 Ted John, Peter Hill, Duncan Frost
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * This file is part of OpenRCT2.
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <limits.h>
#include "addresses.h"
#include "gfx.h"
#include "rct2.h"
#include "string_ids.h"
#include "sprites.h"
#include "window.h"
#include "osinterface.h"

typedef struct {
	uint32 num_entries;
	uint32 total_size;
} rct_g1_header;

void *_g1Buffer = NULL;

// HACK These were originally passed back through registers
int gLastDrawStringX;
int gLastDrawStringY;

uint8 _screenDirtyBlocks[5120];

//Originally 0x9ABE0C, 12 elements from 0xF3 are the peep top colour, 12 elements from 0xCA are peep trouser colour
uint8 peep_palette[0x100] = { 
	0x00, 0xF3, 0xF4, 0xF5, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

//Originally 0x9ABE04
uint8 text_palette[0x8] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Previously 0x97FCBC use it to get the correct palette from g1_elements
uint16 palette_to_g1_offset[] = {
	0x1333, 0x1334, 0x1335, 0x1336,
	0x1337, 0x1338, 0x1339, 0x133A,
	0x133B, 0x133C, 0x133D, 0x133E,
	0x133F, 0x1340, 0x1341, 0x1342,
	0x1343, 0x1344, 0x1345, 0x1346,
	0x1347, 0x1348, 0x1349, 0x134A,
	0x134B, 0x134C, 0x134D, 0x134E,
	0x134F, 0x1350, 0x1351, 0x1352,
	0x1353, 0x0C1C, 0x0C1D, 0x0C1E,
	0x0C1F, 0x0C20, 0x0C22, 0x0C23,
	0x0C24, 0x0C25, 0x0C26, 0x0C21,
	0x1354, 0x1355, 0x1356, 0x1357,
	0x1358, 0x1359, 0x135A, 0x135B,
	0x135C, 0x135D, 0x135E, 0x135F,
	0x1360, 0x1361, 0x1362, 0x1363,
	0x1364, 0x1365, 0x1366, 0x1367,
	0x1368, 0x1369, 0x136A, 0x136B,
	0x136C, 0x136D, 0x136E, 0x136F,
	0x1370, 0x1371, 0x1372, 0x1373,
	0x1374, 0x1375, 0x1376, 0x1377,
	0x1378, 0x1379, 0x137A, 0x137B,
	0x137C, 0x137D, 0x137E, 0x137F,
	0x1380, 0x1381, 0x1382, 0x1383,
	0x1384, 0x1385, 0x1386, 0x1387,
	0x1388, 0x1389, 0x138A, 0x138B,
	0x138C, 0x138D, 0x138E, 0x138F,
	0x1390, 0x1391, 0x1392, 0x1393,
	0x1394, 0x1395, 0x1396, 0x1397,
	0x1398, 0x1399, 0x139A, 0x139B,
	0x139C, 0x139D, 0x139E, 0x139F,
	0x13A0, 0x13A1, 0x13A2, 0x13A3,
	0x13A4, 0x13A5, 0x13A6, 0x13A7,
	0x13A8, 0x13A9, 0x13AA, 0x13AB,
	0x13AC, 0x13AD, 0x13AE, 0x13AF,
	0x13B0, 0x13B1, 0x13B2, 0x13B3,
	0x13B4, 0x13B5, 0x13B6, 0x13B7,
};

static void gfx_draw_dirty_blocks(int x, int y, int columns, int rows);

/**
 * 
 *  rct2: 0x00678998
 */
int gfx_load_g1()
{
	FILE *file;
	rct_g1_header header;
	unsigned int i;

	rct_g1_element *g1Elements = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element);

	file = fopen(get_file_path(PATH_ID_G1), "rb");
	if (file != NULL) {
		if (fread(&header, 8, 1, file) == 1) {
			// number of elements is stored in g1.dat, but because the entry headers are static, this can't be variable until
			// made into a dynamic array
			header.num_entries = 29294;

			// Read element headers
			fread(g1Elements, header.num_entries * sizeof(rct_g1_element), 1, file);

			// Read element data
			_g1Buffer = rct2_malloc(header.total_size);
			fread(_g1Buffer, header.total_size, 1, file);

			fclose(file);

			// Fix entry data offsets
			for (i = 0; i < header.num_entries; i++)
				g1Elements[i].offset += (int)_g1Buffer;

			// Successful
			return 1;
		}
		fclose(file);
	}

	// Unsuccessful
	RCT2_ERROR("Unable to load g1.dat");
	return 0;
}

/*
* 0x6C19AC
*/
void gfx_load_character_widths(){

	uint8* char_width_pointer = RCT2_ADDRESS(RCT2_ADDRESS_FONT_CHAR_WIDTH, uint8);
	for (int char_set_offset = 0; char_set_offset < 4*0xE0; char_set_offset+=0xE0){
		for (uint8 c = 0; c < 0xE0; c++, char_width_pointer++){
			rct_g1_element g1 = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[c + SPR_CHAR_START + char_set_offset];
			int width;

			if (char_set_offset == 0xE0*3) width = g1.width + 1;
			else width = g1.width - 1;

			if (c >= (FORMAT_ARGUMENT_CODE_START - 0x20) && c < (FORMAT_COLOUR_CODE_END - 0x20)){
				width = 0;
			}
			*char_width_pointer = (uint8)width;
		}
		
	}
	
	uint8 drawing_surface[0x40];
	rct_drawpixelinfo dpi = { 
		.bits = (char*)&drawing_surface, 
		.width = 8, 
		.height = 8, 
		.x = 0, 
		.y = 0, 
		.pitch = 0, 
		.zoom_level = 0};

	
	for (int i = 0; i < 0xE0; ++i){
		memset(drawing_surface, 0, sizeof(drawing_surface));
		gfx_draw_sprite(&dpi, i + 0x10D5, -1, 0, 0);

		for (int x = 0; x < 8; ++x){
			uint8 val = 0;
			for (int y = 0; y < 8; ++y){
				val >>= 1;
				if (dpi.bits[x + y * 8]==1){
					val |= 0x80;
				}
			}
			RCT2_ADDRESS(0xF4393C, uint8)[i * 8 + x] = val;
		}

	}

	for (int i = 0; i < 0x20; ++i){
		rct_g1_element* g1 = &(RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[0x606 + i]);
		uint8* unknown_pointer = RCT2_ADDRESS(0x9C3852, uint8) + 0xa12 * i;
		g1->offset = unknown_pointer;
		g1->width = 0x40;
		g1->height = 0x28;
		*((uint16*)unknown_pointer) = 0xFFFF;
		*((uint32*)(unknown_pointer + 0x0E)) = 0;
	}
}

/**
 * Clears the screen with the specified colour.
 *  rct2: 0x00678A9F
 */
void gfx_clear(rct_drawpixelinfo *dpi, int colour)
{
	int y, w, h;
	char* ptr;

	w = dpi->width >> dpi->zoom_level;
	h = dpi->height >> dpi->zoom_level;

	ptr = dpi->bits;
	for (y = 0; y < h; y++) {
		memset(ptr, colour, w);
		ptr += w + dpi->pitch;
	}
}

void gfx_draw_pixel(rct_drawpixelinfo *dpi, int x, int y, int colour)
{
	gfx_fill_rect(dpi, x, y, x, y, colour);
}

/*
* Draws a horizontal line of specified colour to a buffer.
* rct2: 0x68474C
*/
void gfx_draw_line_on_buffer(rct_drawpixelinfo *dpi, char colour, int y, int x, int no_pixels)
{
	y -= dpi->y;

	//Check to make sure point is in the y range
	if (y < 0)return;
	if (y >= dpi->height)return;
	//Check to make sure we are drawing at least a pixel
	if (!no_pixels) no_pixels++;

	x -= dpi->x;

	//If x coord outside range leave
	if (x < 0){
		//Unless the number of pixels is enough to be in range
		no_pixels += x;
		if (no_pixels <= 0)return;
		//Resets starting point to 0 as we don't draw outside the range
		x = 0;
	}

	//Ensure that the end point of the line is within range
	if (x + no_pixels - dpi->width > 0){
		//If the end point has any pixels outside range
		//cut them off. If there are now no pixels return.
		no_pixels -= x + no_pixels - dpi->width;
		if (no_pixels <= 0)return;
	}

	char* bits_pointer;
	//Get the buffer we are drawing to and move to the first coordinate.
	bits_pointer = dpi->bits + y*(dpi->pitch + dpi->width) + x;

	//Draw the line to the specified colour
	for (; no_pixels > 0; --no_pixels, ++bits_pointer){
		*((uint8*)bits_pointer) = colour;
	}
}


/**
 * Draws a line on dpi if within dpi boundaries
 *  rct2: 0x00684466
 * dpi (edi)
 * x1 (ax)
 * y1 (bx)
 * x2 (cx)
 * y2 (dx)
 * colour (ebp)
 */
void gfx_draw_line(rct_drawpixelinfo *dpi, int x1, int y1, int x2, int y2, int colour)
{
	// Check to make sure the line is within the drawing area
	if ((x1 < dpi->x) && (x2 < dpi->x)){
		return;
	}

	if ((y1 < dpi->y) && (y2 < dpi->y)){
		return;
	}

	if ((x1 >(dpi->x + dpi->width)) && (x2 >(dpi->x + dpi->width))){
		return;
	}

	if ((y1 > (dpi->y + dpi->height)) && (y2 > (dpi->y + dpi->height))){
		return;
	}

	//Bresenhams algorithm

	//If vertical plot points upwards
	int steep = abs(y2 - y1) > abs(x2 - x1);
	if (steep){
		int temp_y2 = y2;
		int temp_x2 = x2;
		y2 = x1;
		x2 = y1;
		y1 = temp_x2;
		x1 = temp_y2;
	}

	//If line is right to left swap direction
	if (x1 > x2){
		int temp_y2 = y2;
		int temp_x2 = x2;
		y2 = y1;
		x2 = x1;
		y1 = temp_y2;
		x1 = temp_x2;
	}

	int delta_x = x2 - x1;
	int delta_y = abs(y2 - y1);
	int error = delta_x / 2;
	int y_step;
	int y = y1;

	//Direction of step
	if (y1 < y2)y_step = 1;
	else y_step = -1;

	for (int x = x1, x_start = x1, no_pixels = 1; x < x2; ++x,++no_pixels){
		//Vertical lines are drawn 1 pixel at a time
		if (steep)gfx_draw_line_on_buffer(dpi, colour, x, y, 1);

		error -= delta_y;
		if (error < 0){
			//Non vertical lines are drawn with as many pixels in a horizontal line as possible
			if (!steep)gfx_draw_line_on_buffer(dpi, colour, y, x_start, no_pixels);

			//Reset non vertical line vars
			x_start = x + 1;
			no_pixels = 1;
			y += y_step;
			error += delta_x;
		}

		//Catch the case of the last line
		if (x + 1 == x2 && !steep){
			gfx_draw_line_on_buffer(dpi, colour, y, x_start, no_pixels);
		}
	}
	return;
}

/**
 *
 *  rct2: 0x00678AD4
 * dpi (edi)
 * left (ax)
 * top (cx)
 * right (bx)
 * bottom (dx)
 * colour (ebp)
 */
void gfx_fill_rect(rct_drawpixelinfo *dpi, int left, int top, int right, int bottom, int colour)
{
	int left_, right_, top_, bottom_;
	rct_drawpixelinfo* dpi_;
	left_ = left;
	right_ = right;
	top_ = top;
	bottom_ = bottom;
	dpi_ = dpi;
  
	if ((left > right) || (top > bottom) || (dpi->x > right) || (left >= (dpi->x + dpi->width)) ||
		(bottom < dpi->y) || (top >= (dpi->y + dpi->height)))
		return;

	colour |= RCT2_GLOBAL(0x009ABD9C, uint32);

	uint16 cross_pattern = 0;

	int start_x = left - dpi->x;
	if (start_x < 0){
		start_x = 0;
		cross_pattern ^= start_x;
	}

	int end_x = right - dpi->x;
	end_x++;
	if (end_x > dpi->width)
		end_x = dpi->width;

	int width = end_x - start_x;

	int start_y = top - dpi->y;
	if (start_y < 0){
		start_y = 0;
		cross_pattern ^= start_y;
	}
	int end_y = bottom - dpi->y;
	end_y++;

	if (end_y > dpi->height)
		end_y = dpi->height;

	int height = end_y - start_y;
	if (colour&0x1000000){
		// 00678B2E    00678BE5
		//Cross hatching
		uint8* dest_pointer = (start_y * (dpi->width + dpi->pitch)) + start_x + dpi->bits;
	
		uint32 ecx;
		for (int i = 0; i < height; ++i) {
			uint8* next_dest_pointer = dest_pointer + dpi->width + dpi->pitch;
			ecx = cross_pattern;
			// Rotate right
			ecx = (ecx >> 1) | (ecx << (sizeof(ecx) * CHAR_BIT - 1));
			ecx = (ecx & 0xFFFF0000) | width; 
			// Fill every other pixel with the colour
			for (; (ecx & 0xFFFF) > 0; ecx--) {
				ecx = ecx ^ 0x80000000;
				if ((int)ecx < 0) {
					*dest_pointer = colour & 0xFF;
				}
				dest_pointer++;
			}
			cross_pattern ^= 1;
			dest_pointer = next_dest_pointer;
			
		}
		return;
	}
	if (colour & 0x2000000){
		//0x2000000
		// 00678B7E   00678C83
		// Location in screen buffer?
		uint8* dest_pointer = dpi->bits + (uint32)((start_y >> (dpi->zoom_level)) * ((dpi->width >> dpi->zoom_level) + dpi->pitch) + (start_x >> dpi->zoom_level));

		// Find colour in colour table?
		uint16 eax = palette_to_g1_offset[(colour & 0xFF)];
		rct_g1_element g1_element = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[eax];

		// Fill the rectangle with the colours from the colour table
		for (int i = 0; i < height>>dpi->zoom_level; ++i) {
			uint8* next_dest_pointer = dest_pointer + (dpi->width >> dpi->zoom_level) + dpi->pitch;
			for (int j = 0; j < width; ++j) {
				*dest_pointer = g1_element.offset[*dest_pointer];
				dest_pointer++;
			}
			dest_pointer = next_dest_pointer;
		}
		return;
	}
	if (colour & 0x4000000){
		//0x4000000
		// 00678B8A   00678E38
		char* dest_pointer;
		dest_pointer = start_y * (dpi->width + dpi->pitch) + start_x + dpi->bits;

		//The pattern loops every 15 lines this is which
		//part the pattern is on.
		int pattern_y = (start_y + dpi->y) % 16;

		//The pattern loops every 15 pixels this is which
		//part the pattern is on.
		int start_pattern_x = (start_x + dpi_->x) % 16;
		int pattern_x = start_pattern_x;

		uint16* pattern_pointer;
		pattern_pointer = RCT2_ADDRESS(0x0097FEFC,uint16*)[colour >> 28]; // or possibly uint8)[esi*4] ?

		for (int no_lines = height; no_lines > 0; no_lines--) {
			char* next_dest_pointer = dest_pointer + dpi->width + dpi->pitch;
			uint16 pattern = pattern_pointer[pattern_y]; 

			for (int no_pixels = width; no_pixels > 0; --no_pixels) {
				if (pattern & (1 << pattern_x))
					*dest_pointer = colour & 0xFF;

				pattern_x = (pattern_x + 1) % 16;
				dest_pointer++;
			}
			pattern_x = start_pattern_x;
			pattern_y = (pattern_y + 1) % 16;
			dest_pointer = next_dest_pointer;
		}
		return;
	}
	if (colour & 0x8000000){
		//0x8000000
		// 00678B3A    00678EC9 still to be implemented
		//RCT2_CALLPROC_X(0x00678AD4, left, right, top, bottom, 0, dpi, colour);
		int esi = left - RCT2_GLOBAL(0x1420070,sint16);
		RCT2_GLOBAL(0xEDF824,uint32) = esi;
		esi = top - RCT2_GLOBAL(0x1420072,sint16);
		RCT2_GLOBAL(0xEDF828,uint32) = esi;
		left -= dpi->x;//0x4
		if ( left < 0 ){
			RCT2_GLOBAL(0xEDF824,sint32) -= left;
			left = 0; 
		}
		right -= dpi->x;
		right++;
		if ( right > dpi->width ){
			right = dpi->width;
		}
		right -= left;
		top -= dpi->y;
		if ( top < 0 ){
			RCT2_GLOBAL(0xEDF828,sint32) -= top;
			top = 0;
		}
		bottom -= dpi->y;
		bottom++;
		if (bottom > dpi->height){
			bottom = dpi->height;
		}
		bottom -= top;
		RCT2_GLOBAL(0xEDF824,sint32) &= 0x3F;
		RCT2_GLOBAL(0xEDF828,sint32) &= 0x3F;
		esi = dpi->width;
		esi += dpi->pitch;
		esi *= top;
		esi += left;
		esi += (uint32)dpi->bits;
		RCT2_GLOBAL(0xEDF82C,sint32) = right;
		RCT2_GLOBAL(0xEDF830,sint32) = bottom;
		left = dpi->width;
		left+= dpi->pitch;
		left-= right;
		RCT2_GLOBAL(0xEDF834,sint32) = left;
		colour &= 0xFF;
		colour--;
		right = colour;
		colour <<= 8;
		right |= colour;
		RCT2_GLOBAL(0xEDF838,sint32) = right;
		//right <<= 4;
		int edi = esi;
		esi = RCT2_GLOBAL(0xEDF828,sint32);
		esi *= 0x40;
		left = 0;
		esi += (uint32)(RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS,rct_g1_element)[right]).offset;//???
		//Not finished
		//Start of loop
		return;
	}
	//0x0000000
	uint8* dest_pointer = start_y * (dpi->width + dpi->pitch) + start_x + dpi->bits;

	for (int i = 0; i < height; ++i) {
		memset(dest_pointer, (colour & 0xFF), width);
		dest_pointer += dpi->width + dpi->pitch;
	}
	// RCT2_CALLPROC_X(0x00678AD4, left, right, top, bottom, 0, dpi, colour);
}

/**
 *  Draw a rectangle, with optional border or fill
 *
 *  rct2: 0x006E6F81
 * dpi (edi)
 * left (ax)
 * top (cx)
 * right (bx)
 * bottom (dx)
 * colour (ebp)
 * flags (si)
 */
void gfx_fill_rect_inset(rct_drawpixelinfo* dpi, short left, short top, short right, short bottom, int colour, short flags)
{
	uint8 shadow, fill, hilight;

	// Flags
	int no_border, no_fill, pressed;

	no_border = 8;
	no_fill = 0x10;
	pressed = 0x20;

	if (colour & 0x180) {
		if (colour & 0x100) {
			colour = colour & 0x7F;
		} else {
			colour = RCT2_ADDRESS(0x009DEDF4,uint8)[colour];
		}

		colour = colour | 0x2000000; //Transparent

		if (flags & no_border) {
			gfx_fill_rect(dpi, left, top, bottom, right, colour);
		} else if (flags & pressed) {
			// Draw outline of box
			gfx_fill_rect(dpi, left, top, left, bottom, colour + 1);
			gfx_fill_rect(dpi, left, top, right, top, colour + 1);
			gfx_fill_rect(dpi, right, top, right, bottom, colour + 2);
			gfx_fill_rect(dpi, left, bottom, right, bottom, colour + 2);

			if (!(flags & no_fill)) {
				gfx_fill_rect(dpi, left+1, top+1, right-1, bottom-1, colour);
			}
		} else {
			// Draw outline of box
			gfx_fill_rect(dpi, left, top, left, bottom, colour + 2);
			gfx_fill_rect(dpi, left, top, right, top, colour + 2);
			gfx_fill_rect(dpi, right, top, right, bottom, colour + 1);
			gfx_fill_rect(dpi, left, bottom, right, bottom, colour + 1);

			if (!(flags & no_fill)) {
				gfx_fill_rect(dpi, left+1, top+1, right-1, bottom-1, colour);
			}
		}
	} else {
		if (flags & 0x80) {
			shadow	= RCT2_ADDRESS(0x0141FC46, uint8)[colour * 8];
			fill	= RCT2_ADDRESS(0x0141FC48, uint8)[colour * 8];
			hilight	= RCT2_ADDRESS(0x0141FC4A, uint8)[colour * 8];
		} else {
			shadow	= RCT2_ADDRESS(0x0141FC47, uint8)[colour * 8];
			fill	= RCT2_ADDRESS(0x0141FC49, uint8)[colour * 8];
			hilight	= RCT2_ADDRESS(0x0141FC4B, uint8)[colour * 8];
		}

		if (flags & no_border) {
			gfx_fill_rect(dpi, left, top, right, bottom, fill);
		} else if (flags & pressed) {
			// Draw outline of box
			gfx_fill_rect(dpi, left, top, left, bottom, shadow);
			gfx_fill_rect(dpi, left + 1, top, right, top, shadow);
			gfx_fill_rect(dpi, right, top + 1, right, bottom - 1, hilight);
			gfx_fill_rect(dpi, left + 1, bottom, right, bottom, hilight);

			if (!(flags & no_fill)) {
				if (!(flags & 0x40)) {
					if (flags & 0x04) {
						fill = RCT2_ADDRESS(0x0141FC49, uint8)[0];
					} else {
						fill = RCT2_ADDRESS(0x0141FC4A, uint8)[colour * 8];
					}
				}
				gfx_fill_rect(dpi, left+1, top+1, right-1, bottom-1, fill);
			}
		} else {
			// Draw outline of box
			gfx_fill_rect(dpi, left, top, left, bottom - 1, hilight);
			gfx_fill_rect(dpi, left + 1, top, right - 1, top, hilight);
			gfx_fill_rect(dpi, right, top, right, bottom - 1, shadow);
			gfx_fill_rect(dpi, left, bottom, right, bottom, shadow);

			if (!(flags & no_fill)) {
				if (flags & 0x04) {
					fill = RCT2_ADDRESS(0x0141FC49, uint8)[0];
				}
				gfx_fill_rect(dpi, left+1, top+1, right-1, bottom-1, fill);
			}
		}
	}
}

#define RCT2_Y_RELATED_GLOBAL_1 0x9E3D12 //uint16
#define RCT2_Y_END_POINT_GLOBAL 0x9ABDAC //sint16
#define RCT2_Y_START_POINT_GLOBAL 0xEDF808 //sint16
#define RCT2_X_RELATED_GLOBAL_1 0x9E3D10 //uint16
#define RCT2_X_END_POINT_GLOBAL 0x9ABDA8 //sint16
#define RCT2_X_START_POINT_GLOBAL 0xEDF80C //sint16
#define RCT2_DPI_LINE_LENGTH_GLOBAL 0x9ABDB0 //uint16 width+pitch

/*
* rct2: 0x67A690
* copies a sprite onto the buffer. There is no compression used on the sprite
* image.
*/
void gfx_bmp_sprite_to_buffer(uint8* palette_pointer, uint8* unknown_pointer, uint8* source_pointer, uint8* dest_pointer, rct_g1_element* source_image, rct_drawpixelinfo *dest_dpi, int height, int width, int image_type){
	uint8 zoom_level = dest_dpi->zoom_level;
	uint8 zoom_amount = 1 << zoom_level;
	//Requires use of palette?
	if (image_type & IMAGE_TYPE_USE_PALETTE){

		//Mix with another image?? and colour adjusted
		if (unknown_pointer!= NULL){ //Not tested. I can't actually work out when this code runs.
			unknown_pointer += source_pointer - source_image->offset;// RCT2_GLOBAL(0x9E3CE0, uint32);

			for (; height > 0; height -= zoom_amount){
				uint8* next_source_pointer = source_pointer + (uint32)(source_image->width * zoom_amount);
				uint8* next_unknown_pointer = unknown_pointer + (uint32)(source_image->width * zoom_amount);
				uint8* next_dest_pointer = dest_pointer + (dest_dpi->width / zoom_amount) + dest_dpi->pitch;

				for (int no_pixels = width; no_pixels > 0; no_pixels -= zoom_amount, source_pointer += zoom_amount, unknown_pointer += zoom_amount, dest_pointer++){
					uint8 pixel = *source_pointer;
					pixel = palette_pointer[pixel];
					pixel &= *unknown_pointer;
					if (pixel){
						*dest_pointer = pixel;
					}
				}
				source_pointer = next_source_pointer;
				dest_pointer = next_dest_pointer;
				unknown_pointer = next_unknown_pointer;
			}
			return;
		}

		//image colour adjusted?
		for (; height > 0; height -= zoom_amount){
			uint8* next_source_pointer = source_pointer + (uint32)(source_image->width * zoom_amount);
			uint8* next_dest_pointer = dest_pointer + (dest_dpi->width / zoom_amount) + dest_dpi->pitch;
			for (int no_pixels = width; no_pixels > 0; no_pixels -= zoom_amount, source_pointer += zoom_amount, dest_pointer++){
				uint8 pixel = *source_pointer;
				pixel = palette_pointer[pixel];
				if (pixel){
					*dest_pointer = pixel;
				}
			}

			source_pointer = next_source_pointer;
			dest_pointer = next_dest_pointer;
		}
		return;
	}

	//Mix with background. It only uses source pointer for
	//telling if it needs to be drawn not for colour.
	if (image_type & IMAGE_TYPE_MIX_BACKGROUND){//Not tested
		for (; height > 0; height -= zoom_amount){
			uint8* next_source_pointer = source_pointer + (uint32)(source_image->width * zoom_amount);
			uint8* next_dest_pointer = dest_pointer + (dest_dpi->width / zoom_amount) + dest_dpi->pitch;

			for (int no_pixels = width; no_pixels > 0; no_pixels -= zoom_amount, source_pointer += zoom_amount, dest_pointer++){
				uint8 pixel = *source_pointer;
				if (pixel){
					pixel = *dest_pointer;
					pixel = palette_pointer[pixel];
					*dest_pointer = pixel;
				}
			}

			source_pointer = next_source_pointer;
			dest_pointer = next_dest_pointer;
		}
		return;
	}

	//Basic bitmap no fancy stuff
	if (!(source_image->flags & G1_FLAG_BMP)){//Not tested
		for (; height > 0; height -= zoom_amount){
			uint8* next_source_pointer = source_pointer + (uint32)(source_image->width * zoom_amount);
			uint8* next_dest_pointer = dest_pointer + (dest_dpi->width / zoom_amount) + dest_dpi->pitch;

			for (int no_pixels = width; no_pixels > 0; no_pixels -= zoom_amount, dest_pointer++, source_pointer += zoom_amount){
				*dest_pointer = *source_pointer;
			}

			dest_pointer = next_dest_pointer;
			source_pointer = next_source_pointer;
		}
		return;
	}

	if (RCT2_GLOBAL(0x9E3CDC, uint32) != 0){//Not tested. I can't actually work out when this code runs.
		unknown_pointer += source_pointer - source_image->offset;

		for (; height > 0; height -= zoom_amount){
			uint8* next_source_pointer = source_pointer + (uint32)(source_image->width * zoom_amount);
			uint8* next_unknown_pointer = unknown_pointer + (uint32)(source_image->width * zoom_amount);
			uint8* next_dest_pointer = dest_pointer + (dest_dpi->width / zoom_amount) + dest_dpi->pitch;

			for (int no_pixels = width; no_pixels > 0; no_pixels -= zoom_amount, dest_pointer++, source_pointer += zoom_amount, unknown_pointer += zoom_amount){
				uint8 pixel = *source_pointer;
				pixel &= *unknown_pointer;
				if (pixel){
					*dest_pointer = pixel;
				}
			}
			dest_pointer = next_dest_pointer;
			source_pointer = next_source_pointer;
			unknown_pointer = next_unknown_pointer;
		}
	}

	//Basic bitmap with no draw pixels
	for (; height > 0; height -= zoom_amount){
		uint8* next_source_pointer = source_pointer + (uint32)(source_image->width * zoom_amount);
		uint8* next_dest_pointer = dest_pointer + (dest_dpi->width / zoom_amount) + dest_dpi->pitch;

		for (int no_pixels = width; no_pixels > 0; no_pixels -= zoom_amount, dest_pointer++, source_pointer += zoom_amount){
			uint8 pixel = *source_pointer;
			if (pixel){
				*dest_pointer = pixel;
			}
		}
		dest_pointer = next_dest_pointer;
		source_pointer = next_source_pointer;
	}
	return;
}


/*
* rct2: 0x67AA18 transfers readied images onto buffers
* This function copies the sprite data onto the screen
*/
void gfx_rle_sprite_to_buffer(uint8* source_bits_pointer, uint8* dest_bits_pointer, uint8* palette_pointer, rct_drawpixelinfo *dpi, int image_type, int source_y_start, int height, int source_x_start, int width){
	int zoom_level = dpi->zoom_level;
	int zoom_amount = 1 << zoom_level;
	uint8* next_source_pointer;
	uint8* next_dest_pointer = dest_bits_pointer;

	//For every line in the image
	for (int y = source_y_start; y < (height + source_y_start); y += zoom_amount){

		//The first part of the source pointer is a list of offsets to different lines
		//This will move the pointer to the correct source line.
		next_source_pointer = source_bits_pointer + ((uint16*)source_bits_pointer)[y];

		uint8 last_data_line = 0;

		//For every data section in the line
		while (!last_data_line){
			uint8* source_pointer = next_source_pointer;
			uint8* dest_pointer = next_dest_pointer;

			int no_pixels = *source_pointer++;
			//gap_size is the number of non drawn pixels you require to
			//jump over on your destination
			uint8 gap_size = *source_pointer++;
			//The last bit in no_pixels tells you if you have reached the end of a line
			last_data_line = no_pixels & 0x80;
			//Clear the last data line bit so we have just the no_pixels
			no_pixels &= 0x7f;
			//Have our next source pointer point to the next data section
			next_source_pointer = source_pointer + no_pixels;

			//Calculates the start point of the image
			int x_start = gap_size - source_x_start;

			if (x_start > 0){
				//Since the start is positive
				//We need to move the drawing surface to the correct position
				dest_pointer += x_start / zoom_amount;
			}
			else{
				//If the start is negative we require to remove part of the image.
				//This is done by moving the image pointer to the correct position.
				source_pointer -= x_start;
				//The no_pixels will be reduced in this operation
				no_pixels += x_start;
				//If there are no pixels there is nothing to draw this data section
				if (no_pixels <= 0) continue;
				//Reset the start position to zero as we have taken into account all moves
				x_start = 0;
			}

			int x_end = x_start + no_pixels;
			//If the end position is further out than the whole image
			//end position then we need to shorten the line again
			if (x_end > width){
				//Shorten the line
				no_pixels -= x_end - width;
				//If there are no pixels there is nothing to draw.
				if (no_pixels <= 0) continue;
			}

			//Finally after all those checks, copy the image onto the drawing surface
			//If the image type is not a basic one we require to mix the pixels
			if (image_type & IMAGE_TYPE_USE_PALETTE){//In the .exe these are all unraveled loops
				for (; no_pixels > 0; no_pixels -= zoom_amount, source_pointer += zoom_amount, dest_pointer++){
					uint8 al = *source_pointer;
					uint8 ah = *dest_pointer;
					if (image_type & IMAGE_TYPE_MIX_BACKGROUND)
						al = palette_pointer[(((uint16)al << 8) | ah) - 0x100];
					else
						al = palette_pointer[al];
					*dest_pointer = al;
				}
			}
			else if (image_type & IMAGE_TYPE_MIX_BACKGROUND){//In the .exe these are all unraveled loops
				//Doesnt use source pointer ??? mix with background only?
				//Not Tested
				
				for (; no_pixels > 0; no_pixels -= zoom_amount, dest_pointer++){
					uint8 pixel = *dest_pointer;
					pixel = palette_pointer[pixel];
					*dest_pointer = pixel;
				}
			}
			else
			{
				for (; no_pixels > 0; no_pixels -= zoom_amount, source_pointer += zoom_amount, dest_pointer++){
					*dest_pointer = *source_pointer;
				}
			}
		}

		//Add a line to the drawing surface pointer
		next_dest_pointer += dpi->width / zoom_amount + dpi->pitch;
	}
}

/**
 *
 *  rct2: 0x0067A28E
 * image_id (ebx) 
 * image_id as below
 * 0b_111X_XXXX_XXXX_XXXX_XXXX_XXXX_XXXX_XXXX image_type
 * 0b_XXX1_11XX_XXXX_XXXX_XXXX_XXXX_XXXX_XXXX image_sub_type (unknown pointer)
 * 0b_XXX1_1111_XXXX_XXXX_XXXX_XXXX_XXXX_XXXX secondary_colour
 * 0b_XXXX_XXXX_1111_1XXX_XXXX_XXXX_XXXX_XXXX primary_colour
 * 0b_XXXX_X111_1111_1XXX_XXXX_XXXX_XXXX_XXXX palette_ref
 * 0b_XXXX_XXXX_XXXX_X111_1111_1111_1111_1111 image_id (offset to g1)
 * x (cx)
 * y (dx)
 * dpi (esi)
 * tertiary_colour (ebp)
 */
void gfx_draw_sprite(rct_drawpixelinfo *dpi, int image_id, int x, int y, uint32 tertiary_colour)
{
	//RCT2_CALLPROC_X(0x0067A28E, 0, image_id, x, y, 0, (int)dpi, tertiary_colour);
	//return;

	int image_type = (image_id & 0xE0000000) >> 28;
	int image_sub_type = (image_id & 0x1C000000) >> 26;

	uint8* palette_pointer = NULL;
	uint8 palette[0x100];

	RCT2_GLOBAL(0x00EDF81C, uint32) = image_id & 0xE0000000;

	uint8* unknown_pointer = (uint8*)(RCT2_ADDRESS(0x9E3CE4, uint32*)[image_sub_type]);
	RCT2_GLOBAL(0x009E3CDC, uint32) = (uint32)unknown_pointer;

	if (image_type && !(image_type & IMAGE_TYPE_UNKNOWN)) {
		uint8 palette_ref = (image_id >> 19) & 0xFF;
		if (image_type & IMAGE_TYPE_MIX_BACKGROUND){
			unknown_pointer = NULL;
			RCT2_GLOBAL(0x009E3CDC, uint32) = 0;
		}
		else{
			palette_ref &= 0x7F;
		}

		uint16 palette_offset = palette_to_g1_offset[palette_ref];
		palette_pointer = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[palette_offset].offset;
		RCT2_GLOBAL(0x9ABDA4, uint32) = (uint32)palette_pointer;
	}
	else if (image_type && !(image_type & IMAGE_TYPE_USE_PALETTE)){
		RCT2_GLOBAL(0x9E3CDC, uint32) = 0;
		unknown_pointer = NULL;

		uint32 primary_offset = palette_to_g1_offset[(image_id >> 19) & 0x1F];
		uint32 secondary_offset = palette_to_g1_offset[(image_id >> 24) & 0x1F];
		uint32 tertiary_offset = palette_to_g1_offset[tertiary_colour];

		rct_g1_element* primary_colour = &RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[primary_offset];
		rct_g1_element* secondary_colour = &RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[secondary_offset];
		rct_g1_element* tertiary_colour = &RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[tertiary_offset];

		memcpy((uint8*)0x9ABFFF, &primary_colour->offset[0xF3], 12);
		memcpy((uint8*)0x9ABFD6, &secondary_colour->offset[0xF3], 12);
		memcpy((uint8*)0x9ABF3A, &tertiary_colour->offset[0xF3], 12);

		//image_id
		RCT2_GLOBAL(0xEDF81C, uint32) |= 0x20000000;
		image_id |= IMAGE_TYPE_USE_PALETTE << 28;

		RCT2_GLOBAL(0x9ABDA4, uint32) = 0x9ABF0C;
		palette_pointer = (uint8*)0x9ABF0C;
	}
	else if (image_type){
		RCT2_GLOBAL(0x9E3CDC, uint32) = 0;
		unknown_pointer = NULL;
		//Copy the peep palette into a new palette. 
		//Not really required but its nice to make a copy
		memcpy(palette, peep_palette, 0x100);

		//Top
		int top_type = (image_id >> 19) & 0x1f;
		uint32 top_offset = palette_to_g1_offset[top_type]; //RCT2_ADDRESS(0x97FCBC, uint32)[top_type];
		rct_g1_element top_palette = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[top_offset];
		memcpy(palette + 0xF3, top_palette.offset + 0xF3, 12);
		
		//Trousers
		int trouser_type = (image_id >> 24) & 0x1f;
		uint32 trouser_offset = palette_to_g1_offset[trouser_type]; //RCT2_ADDRESS(0x97FCBC, uint32)[trouser_type];
		rct_g1_element trouser_palette = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[trouser_offset];
		memcpy(palette + 0xCA, trouser_palette.offset + 0xF3, 12);

		//For backwards compatibility until the zooming function is done
		RCT2_GLOBAL(0x9ABDA4, uint8*) = palette;
		palette_pointer = palette;
	}
	gfx_draw_sprite_palette_set(dpi, image_id, x, y, palette_pointer, unknown_pointer);
}

/*
* rct: 0x0067A46E
* image_id (ebx) and also (0x00EDF81C)
* palette_pointer (0x9ABDA4)
* unknown_pointer (0x9E3CDC)
* dpi (edi)
* x (cx)
* y (dx)
*/
void gfx_draw_sprite_palette_set(rct_drawpixelinfo *dpi, int image_id, int x, int y, uint8* palette_pointer, uint8* unknown_pointer){
	int image_element = 0x7FFFF&image_id;
	int image_type = (image_id & 0xE0000000) >> 28;
	
	rct_g1_element* g1_source = &(RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[image_element]);

	//Zooming code has been integrated into main code.
	//if (dpi->zoom_level >= 1){ //These have not been tested
	//	//something to do with zooming
	//	if (dpi->zoom_level == 1){
	//		RCT2_CALLPROC_X(0x0067A28E, 0, image_id, x, y, 0, (int)dpi, 0);
	//		return;
	//	}
	//	if (dpi->zoom_level == 2){
	//		RCT2_CALLPROC_X(0x0067DADA, 0, (int)g1_source, x, y, 0, (int)dpi, 0);
	//		return;
	//	}
	//	RCT2_CALLPROC_X(0x0067FAAE, 0, (int)g1_source, x, y, 0, (int)dpi, 0);
	//	return;
	//}
	if ( dpi->zoom_level && (g1_source->flags & (1<<4)) ){
		rct_drawpixelinfo zoomed_dpi = {
			.bits = dpi->bits,
			.x = dpi->x >> 1,
			.y = dpi->y >> 1,
			.height = dpi->height>>1,
			.width = dpi->width>>1,
			.pitch = dpi->pitch,
			.zoom_level = dpi->zoom_level - 1
		};
		gfx_draw_sprite_palette_set(&zoomed_dpi, (image_type << 28) | (image_element - g1_source->zoomed_offset), x >> 1, y >> 1, palette_pointer, unknown_pointer);
		return;
	}

	if ( dpi->zoom_level && (g1_source->flags & (1<<5)) ){
		return;
	}

	//Its used super often so we will define it to a seperate variable.
	int zoom_level = dpi->zoom_level;
	int zoom_amount = 1 << zoom_level;
	int zoom_mask = 0xFFFFFFFF << zoom_level;

	//This will be the height of the drawn image
	int height = g1_source->height;
	//This is the start y coordinate on the destination
	sint16 dest_start_y = ((y + g1_source->y_offset)&zoom_mask) - dpi->y;
	//This is the start y coordinate on the source
	int source_start_y = 0;

	if (dest_start_y < 0){
		//If the destination y is negative reduce the height of the
		//image as we will cut off the bottom
		height += dest_start_y;
		//If the image is no longer visible nothing to draw
		if (height <= 0){
			return;
		}
		//The source image will start a further up the image
		source_start_y -= dest_start_y;
		//The destination start is now reset to 0
		dest_start_y = 0;
	}

	int dest_end_y = dest_start_y + height;

	if (dest_end_y > dpi->height){
		//If the destination y is outside of the drawing
		//image reduce the height of the image
		height -= dest_end_y - dpi->height;
	}
	//If the image no longer has anything to draw
	if (height <= 0)return;

	dest_start_y /= zoom_amount;
	dest_end_y /= zoom_amount;

	//This will be the width of the drawn image
	int width = g1_source->width;
	//This is the source start x coordinate
	int source_start_x = 0;
	//This is the destination start x coordinate
	sint16 dest_start_x = ((x + g1_source->x_offset) & zoom_mask) - dpi->x;

	if (dest_start_x < 0){
		//If the destination is negative reduce the width
		//image will cut off the side
		width += dest_start_x;
		//If there is no image to draw
		if (width <= 0){
			return;
		}
		//The source start will also need to cut off the side
		source_start_x -= dest_start_x;
		//Reset the destination to 0
		dest_start_x = 0;
	}

	int dest_end_x = dest_start_x + width;

	if (dest_end_x > dpi->width){
		//If the destination x is outside of the drawing area
		//reduce the image width.
		width -= dest_end_x - dpi->width;
		//If there is no image to draw.
		if (width <= 0)return;
	}

	dest_start_x /= zoom_amount;
	dest_end_x /= zoom_amount;

	uint8* dest_pointer = (uint8*)dpi->bits;
	//Move the pointer to the start point of the destination
	dest_pointer += ((dpi->width / zoom_amount) + dpi->pitch)*dest_start_y + dest_start_x;
	
	if (g1_source->flags & G1_FLAG_RLE_COMPRESSION){
		//We have to use a different method to move the source pointer for
		//rle encoded sprites so that will be handled within this function
		gfx_rle_sprite_to_buffer(g1_source->offset, dest_pointer, palette_pointer, dpi, image_type, source_start_y, height, source_start_x, width);
		return;
	}
	uint8* source_pointer = g1_source->offset;
	//Move the pointer to the start point of the source
	source_pointer += g1_source->width*source_start_y + source_start_x;

	if (!(g1_source->flags & 0x02)){
		gfx_bmp_sprite_to_buffer(palette_pointer, unknown_pointer, source_pointer, dest_pointer, g1_source, dpi, height, width, image_type);
		return;
	}
	//0x67A60A Not tested
	int total_no_pixels = g1_source->width*g1_source->height;
	source_pointer = g1_source->offset;
	uint8* new_source_pointer_start = malloc(total_no_pixels);
	uint8* new_source_pointer = new_source_pointer_start;// 0x9E3D28;
	int ebx, ecx;
	while (total_no_pixels>0){
		sint8 no_pixels = *source_pointer;
		if (no_pixels >= 0){
			source_pointer++;
			total_no_pixels -= no_pixels;
			memcpy((char*)new_source_pointer, (char*)source_pointer, no_pixels);
			new_source_pointer += no_pixels;
			source_pointer += no_pixels;
			continue;
		}
		ecx = no_pixels;
		no_pixels &= 0x7;
		ecx >>= 3;//SAR
		int eax = ((int)no_pixels)<<8;
		ecx = -ecx;//Odd
		eax = eax & 0xFF00 + *(source_pointer+1);
		total_no_pixels -= ecx;
		source_pointer += 2;
		ebx = (uint32)new_source_pointer - eax;
		eax = (uint32)source_pointer;
		source_pointer = (uint8*)ebx;
		ebx = eax;
		eax = 0;
		memcpy((char*)new_source_pointer, (char*)source_pointer, ecx);
		new_source_pointer += ecx;
		source_pointer += ecx;
		source_pointer = (uint8*)ebx;
	}
	source_pointer = new_source_pointer_start + g1_source->width*source_start_y + source_start_x;
	gfx_bmp_sprite_to_buffer(palette_pointer, unknown_pointer, source_pointer, dest_pointer, g1_source, dpi, height, width, image_type);
	free(new_source_pointer_start);
	return;
}

/**
 *
 *  rct2: 0x00683854
 * a1 (ebx)
 * product (cl)
 */
void gfx_transpose_palette(int pal, unsigned char product)
{
	rct_g1_element g1 = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[pal];
	int width = g1.width;
	int x = g1.x_offset;  
	uint8* dest_pointer = (uint8*)&(RCT2_ADDRESS(0x014124680,uint8)[x]);
	uint8* source_pointer = g1.offset;

	for (; width > 0; width--) {
		dest_pointer[0] = (source_pointer[0] * product) >> 8;
		dest_pointer[1] = (source_pointer[1] * product) >> 8;
		dest_pointer[2] = (source_pointer[2] * product) >> 8;
		source_pointer += 3;
		dest_pointer += 4;
	}
	osinterface_update_palette((char*)0x01424680, 10, 236);//Odd would have expected dest_pointer
}
/**
 * Draws i formatted text string centred at i specified position.
 *  rct2: 0x006C1D6C
 * dpi (edi)
 * format (bx)
 * x (cx)
 * y (dx)
 * colour (al)
 * args (esi)
 */
void gfx_draw_string_centred(rct_drawpixelinfo *dpi, int format, int x, int y, int colour, void *args)
{
	char* buffer;
	short text_width;

	buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, char);
	format_string(buffer, format, args);

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 0xE0;

	// Measure text width
	text_width = gfx_get_string_width(buffer);

	// Draw the text centred
	if (text_width <= 0xFFFF) {
		x -= text_width / 2;
		gfx_draw_string(dpi, buffer, colour, x, y);
	}
}

/**
 *
 *  rct2: 0x006ED7E5
 */
void gfx_invalidate_screen()
{
	int width = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16);
	int height = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, sint16);
	gfx_set_dirty_blocks(0, 0, width, height);
}

/**
 * 
 *  rct2: 0x006E732D
 * left (ax)
 * top (bx)
 * right (dx)
 * bottom (bp)
 */
void gfx_set_dirty_blocks(int left, int top, int right, int bottom)
{
	int x, y;
	uint8 *screenDirtyBlocks = RCT2_ADDRESS(0x00EDE408, uint8);

	left = max(left, 0);
	top = max(top, 0);
	right = min(right, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16));
	bottom = min(bottom, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, sint16));

	if (left >= right)
		return;
	if (top >= bottom)
		return;

	right--;
	bottom--;

	left >>= RCT2_GLOBAL(0x009ABDF0, sint8);
	right >>= RCT2_GLOBAL(0x009ABDF0, sint8);
	top >>= RCT2_GLOBAL(0x009ABDF1, sint8);
	bottom >>= RCT2_GLOBAL(0x009ABDF1, sint8);

	for (y = top; y <= bottom; y++)
		for (x = left; x <= right; x++)
			screenDirtyBlocks[y * RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_COLUMNS, sint32) + x] = 0xFF;
}

/**
 *
 *  rct2: 0x006E73BE
 */
void gfx_draw_all_dirty_blocks()
{
	int x, y, xx, yy, columns, rows;
	uint8 *screenDirtyBlocks = RCT2_ADDRESS(0x00EDE408, uint8);

	for (x = 0; x < RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_COLUMNS, sint32); x++) {
		for (y = 0; y < RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_ROWS, sint32); y++) {
			if (screenDirtyBlocks[y * RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_COLUMNS, sint32) + x] == 0)
				continue;

			// Determine columns
			for (xx = x; xx < RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_COLUMNS, sint32); xx++)
				if (screenDirtyBlocks[y * RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_COLUMNS, sint32) + xx] == 0)
					break;
			columns = xx - x;

			// Check rows
			for (yy = y; yy < RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_ROWS, sint32); yy++)
				for (xx = x; xx < x + columns; xx++)
					if (screenDirtyBlocks[yy * RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_COLUMNS, sint32) + xx] == 0)
						goto endRowCheck;
			
		endRowCheck:
			rows = yy - y;
			gfx_draw_dirty_blocks(x, y, columns, rows);
		}
	}
}

static void gfx_draw_dirty_blocks(int x, int y, int columns, int rows)
{
	int left, top, right, bottom;
	uint8 *screenDirtyBlocks = RCT2_ADDRESS(0x00EDE408, uint8);

	// Unset dirty blocks
	for (top = y; top < y + rows; top++)
		for (left = x; left < x + columns; left++)
			screenDirtyBlocks[top * RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_COLUMNS, sint32) + left] = 0;

	// Determine region in pixels
	left = max(0, x * RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_WIDTH, sint16));
	top = max(0, y * RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_HEIGHT, sint16));
	right = min(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16), left + (columns * RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_WIDTH, sint16)));
	bottom = min(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, sint16), top + (rows * RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_HEIGHT, sint16)));
	if (right <= left || bottom <= top)
		return;

	// Draw region
	gfx_redraw_screen_rect(left, top, right, bottom);
}

/**
 * 
 *  rct2: 0x006E7499 
 * left (ax)
 * top (bx)
 * right (dx)
 * bottom (bp)
 */
void gfx_redraw_screen_rect(short left, short top, short right, short bottom)
{
	rct_window* w;
	rct_drawpixelinfo *screenDPI = RCT2_ADDRESS(RCT2_ADDRESS_SCREEN_DPI, rct_drawpixelinfo);
	rct_drawpixelinfo *windowDPI = RCT2_ADDRESS(RCT2_ADDRESS_WINDOW_DPI, rct_drawpixelinfo);

	// Unsure what this does
	RCT2_CALLPROC_X(0x00683326, left, top, right - 1, bottom - 1, 0, 0, 0);

	windowDPI->bits = screenDPI->bits + left + ((screenDPI->width + screenDPI->pitch) * top);
	windowDPI->x = left;
	windowDPI->y = top;
	windowDPI->width = right - left;
	windowDPI->height = bottom - top;
	windowDPI->pitch = screenDPI->width + screenDPI->pitch + left - right;

	for (w = g_window_list; w < RCT2_GLOBAL(RCT2_ADDRESS_NEW_WINDOW_PTR, rct_window*); w++) {
		if (w->flags & WF_TRANSPARENT)
			continue;
		if (right <= w->x || bottom <= w->y)
			continue;
		if (left >= w->x + w->width || top >= w->y + w->height)
			continue;
		window_draw(w, left, top, right, bottom);
	}
}

/**
 *  Return the width of the string in buffer
 *
 *  rct2: 0x006C2321
 * buffer (esi)
 */
int gfx_get_string_width(char* buffer)
{
	// Current font sprites
	uint16* current_font_sprite_base;
	// Width of string
	int width;
	rct_g1_element g1_element;
	
	current_font_sprite_base = RCT2_ADDRESS(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16);
	width = 0;

	for (uint8* curr_char = (uint8*)buffer; *curr_char != (uint8)0; curr_char++) {

		if (*curr_char >= 0x20) {
			width += RCT2_ADDRESS(RCT2_ADDRESS_FONT_CHAR_WIDTH, uint8)[*current_font_sprite_base + (*curr_char - 0x20)];
			continue;
		}
		switch(*curr_char) {
		case FORMAT_MOVE_X:
			curr_char++;
			width = *curr_char;
			break;
		case FORMAT_ADJUST_PALETTE:
		case 3:
		case 4:
			curr_char++;
			break;
		case FORMAT_NEWLINE:
		case FORMAT_NEWLINE_SMALLER:
			continue;
		case FORMAT_TINYFONT:
			*current_font_sprite_base = 0x1C0;
			break;
		case FORMAT_BIGFONT:
			*current_font_sprite_base = 0x2A0;
			break;
		case FORMAT_MEDIUMFONT:
			*current_font_sprite_base = 0x0E0;
			break;
		case FORMAT_SMALLFONT:
			*current_font_sprite_base = 0;
			break;
		case FORMAT_OUTLINE:
		case FORMAT_OUTLINE_OFF:
		case FORMAT_WINDOW_COLOUR_1:
		case FORMAT_WINDOW_COLOUR_2:
		case FORMAT_WINDOW_COLOUR_3:
		case 0x10:
			continue;
		case FORMAT_INLINE_SPRITE:
			g1_element = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[*((uint32*)(curr_char+1))&0x7FFFF];
			width += g1_element.width;
			curr_char += 4;
			break;
		default:
			if (*curr_char <= 0x16) { //case 0x11? FORMAT_NEW_LINE_X_Y
				curr_char += 2;
				continue;
			}
			curr_char += 4;//never happens?
			break;
		}
	}
	return width;
}

/**
 *  Clip the text in buffer to width, add ellipsis and return the new width of the clipped string
 *
 *  rct2: 0x006C2460
 * buffer (esi)
 * width (edi)
 */
int gfx_clip_string(char* buffer, int width)
{
	// Location of font sprites
	uint16 current_font_sprite_base;
	// Width the string has to fit into
	unsigned int max_width;
	// Character to change to ellipsis
	unsigned char* last_char;
	// Width of the string, including ellipsis

	unsigned int clipped_width;

	rct_g1_element g1_element;

	if (width < 6) {
		*buffer = 0;
		return 0;
	}
	
	current_font_sprite_base = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16);
	max_width = width - (3 * RCT2_ADDRESS(0x141E9F6, uint8)[current_font_sprite_base]);

	clipped_width = 0;
	last_char = buffer;

	for (unsigned char* curr_char = buffer; *curr_char != (uint8)0; curr_char++) {
		if (*curr_char < 0x20) {
			switch (*curr_char) {
			case FORMAT_MOVE_X:
				curr_char++;
				clipped_width = *curr_char;
				continue;
			case FORMAT_ADJUST_PALETTE:
			case 3:
			case 4:
				curr_char++;
				continue;
			case FORMAT_NEWLINE:
			case FORMAT_NEWLINE_SMALLER:
				continue;
			case FORMAT_TINYFONT:
				current_font_sprite_base = 0x1C0;
				break;
			case FORMAT_BIGFONT:
				current_font_sprite_base = 0x2A0;
				break;
			case FORMAT_MEDIUMFONT:
				current_font_sprite_base = 0x0E0;
				break;
			case FORMAT_SMALLFONT:
				current_font_sprite_base = 0;
				break;
			case FORMAT_OUTLINE:
			case FORMAT_OUTLINE_OFF:
			case FORMAT_WINDOW_COLOUR_1:
			case FORMAT_WINDOW_COLOUR_2:
			case FORMAT_WINDOW_COLOUR_3:
			case 0x10:
				continue;
			case FORMAT_INLINE_SPRITE:
				g1_element = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[*((uint32*)(curr_char+1))&0x7FFFF];
				clipped_width += g1_element.width;
				curr_char += 4;
				continue;
			default:
				if (*curr_char <= 0x16) { //case 0x11? FORMAT_NEW_LINE_X_Y
					curr_char += 2;
					continue;
				}
				curr_char += 4;//never happens?
				continue;
			}
			max_width = width - (3 * RCT2_ADDRESS(0x141E9F6, uint8)[current_font_sprite_base]);
		}

		clipped_width += RCT2_ADDRESS(RCT2_ADDRESS_FONT_CHAR_WIDTH, uint8)[current_font_sprite_base + (*curr_char - 0x20)];

		if ((int)clipped_width > width) {
// 			*((uint32*)last_char) = '...';
			strcpy(last_char-3, "...");
			clipped_width = width;
			return clipped_width;
		}
		if (clipped_width <= max_width) {
			last_char = curr_char+1;
		}
	}
	return clipped_width;
}


/**
 *  Wrap the text in buffer to width, returns width of longest line.
 *
 *  Inserts NULL where line should break (as \n is used for something else),
 *  so the number of lines is returned in num_lines. font_height seems to be
 *  a control character for line height.
 *
 *  rct2: 0x006C21E2
 * buffer (esi)
 * width (edi) - in
 * num_lines (edi) - out
 * font_height (ebx) - out
 */
int gfx_wrap_string(char* buffer, int width, int* num_lines, int* font_height)
{
	unsigned int line_width = 0;
	unsigned int max_width = 0;
	rct_g1_element g1_element;

    *num_lines = 0;
	*font_height = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16);

	// Pointer to the start of the current word
	unsigned char* curr_word = NULL;
	// Width of line up to current word
	unsigned int curr_width;

	for (unsigned char* curr_char = buffer; *curr_char != (uint8)0; curr_char++) {

		// Remember start of current word and line width up to this word
        if (*curr_char == ' ') {
            curr_word = curr_char;
            curr_width = line_width;
        }

		// 5 is RCT2 new line?
        if (*curr_char != 5) {
			if (*curr_char < ' ') {
				switch(*curr_char) {
				case FORMAT_MOVE_X:
				case FORMAT_ADJUST_PALETTE:
				case 3:
				case 4:
					curr_char++;
					continue;
				case FORMAT_NEWLINE:
				case FORMAT_NEWLINE_SMALLER:
					continue;
				case FORMAT_TINYFONT:
					*font_height = 0x1C0;
					continue;
				case FORMAT_BIGFONT:
					*font_height = 0x2A0;
					continue;
				case FORMAT_MEDIUMFONT:
					*font_height = 0xE0;
					continue;
				case FORMAT_SMALLFONT:
					*font_height = 0;
					continue;
				case FORMAT_OUTLINE:
				case FORMAT_OUTLINE_OFF:
				case FORMAT_WINDOW_COLOUR_1:
				case FORMAT_WINDOW_COLOUR_2:
				case FORMAT_WINDOW_COLOUR_3:
				case 0x10:
					continue;
				case FORMAT_INLINE_SPRITE:
					g1_element = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[*((uint32*)(curr_char + 1)) & 0x7FFFF];
					line_width += g1_element.width;
					curr_char += 4;
					break;
				default:
					if (*curr_char <= 0x16) {
						curr_char += 2;
						continue;
					}
					curr_char += 4;
					continue;
				}
			}

			line_width += RCT2_ADDRESS(RCT2_ADDRESS_FONT_CHAR_WIDTH, uint8)[*font_height + (*curr_char - 0x20)];

			if ((int)line_width <= width) {
				continue;
			}
			if (curr_word == 0) {
				curr_char--;
				unsigned char* old_char = curr_char;
				unsigned char swap_char = 0;
				unsigned char temp;
				// Insert NULL at current character
				// Aboslutely no guarantee that this won't overrun!
				do {
					temp = swap_char;
					swap_char = *curr_char;
					*curr_char = temp;
					curr_char++;
				} while(swap_char != 0);

				*curr_char = swap_char;
				curr_char = old_char;
				curr_char++;
				*num_lines += 1;

				if (line_width > max_width) {
					max_width = line_width;
				}
				line_width = 0;
				curr_word = 0;
				continue;
			}
			curr_char = curr_word;
			line_width = curr_width;
        }

        *num_lines += 1;
        *curr_char = 0;

		if (line_width > max_width) {
			max_width = line_width;
		}
		line_width = 0;
		curr_word = 0;
	}
	if (max_width == 0)return line_width;
	return max_width;
}


/**
 * Draws i formatted text string left aligned at i specified position but clips
 * the text with an elipsis if the text width exceeds the specified width.
 *  rct2: 0x006C1B83
 * dpi (edi)
 * format (bx)
 * args (esi)
 * colour (al)
 * x (cx)
 * y (dx)
 * width (bp)
 */
void gfx_draw_string_left_clipped(rct_drawpixelinfo* dpi, int format, void* args, int colour, int x, int y, int width)
{
	char* buffer;

	buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, char);
	format_string(buffer, format, args);

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 0xE0;

	// Clip text - return value is not needed
	gfx_clip_string(buffer, width);

	gfx_draw_string(dpi, buffer, colour, x, y);
}

/**
 * Draws i formatted text string centred at i specified position but clips the
 * text with an elipsis if the text width exceeds the specified width.
 *  rct2: 0x006C1BBA
 * dpi (edi)
 * format (bx)
 * args (esi)
 * colour (al)
 * x (cx)
 * y (dx)
 * width (bp)
 */
void gfx_draw_string_centred_clipped(rct_drawpixelinfo *dpi, int format, void *args, int colour, int x, int y, int width)
{
	char* buffer;
	short text_width;

	buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, char);
	format_string(buffer, format, args);

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 0xE0;

	// Clip text
	text_width = gfx_clip_string(buffer, width);

	// Draw the text centred
	if (text_width <= 0xFFFF) {
		x -= (text_width - 1) / 2;
		gfx_draw_string(dpi, buffer, colour, x, y);
	}
}

/**
 * Draws i formatted text string right aligned.
 *  rct2: 0x006C1BFC
 * dpi (edi)
 * format (bx)
 * args (esi)
 * colour (al)
 * x (cx)
 * y (dx)
 */
void gfx_draw_string_right(rct_drawpixelinfo* dpi, int format, void* args, int colour, int x, int y)
{
	char* buffer;
	short text_width;

	buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, char);
	format_string(buffer, format, args);

	// Measure text width
	text_width = gfx_get_string_width(buffer);

	// Draw the text right aligned
	x -= text_width;
	gfx_draw_string(dpi, buffer, colour, x, y);
}

/**
 * 
 *  rct2: 0x006C1E53
 * dpi (edi)
 * args (esi)
 * x (cx)
 * y (dx)
 * width (bp)
 * colour (al)
 * format (ebx)
 */
int gfx_draw_string_centred_wrapped(rct_drawpixelinfo *dpi, void *args, int x, int y, int width, int format, int colour)
{
	int font_height, line_height, line_width, line_y, num_lines;
	// Location of font sprites
	uint16* current_font_sprite_base;

	char* buffer = RCT2_ADDRESS(0x009C383D, char);

	current_font_sprite_base = RCT2_ADDRESS(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16);
	*current_font_sprite_base = 0xE0;

	gfx_draw_string(dpi, buffer, colour, dpi->x, dpi->y);

	buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, char);

	format_string(buffer, format, args);

	*current_font_sprite_base = 0xE0;

	// line_width unused here
	line_width = gfx_wrap_string(buffer, width, &num_lines, &font_height);

	line_height = 0x0A;

	if (font_height > 0xE0) {
		line_height = 6;
		if (font_height != 0x1C0) {
			line_height = 0x12;
		}
	}

	if (*buffer == 0x0B) {
		line_height = line_height + 1;
	}

	font_height = (line_height / 2) * num_lines;
	line_y = y - font_height;

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_FLAGS, uint16) = 0;

	for (int line = 0; line <= num_lines; ++line) {
		int half_width = gfx_get_string_width(buffer) / 2;
		gfx_draw_string(dpi, buffer, 0xFE, x - half_width, line_y);

		buffer += strlen(buffer) + 1;
        line_y += line_height;
	}

	return line_y - y;
}

/**
 * 
 *  rct2: 0x006C2105
 * dpi (edi)
 * args (esi)
 * x (cx)
 * y (dx)
 * width (bp)
 * format (bx)
 * colour (al)
 */
int gfx_draw_string_left_wrapped(rct_drawpixelinfo *dpi, void *args, int x, int y, int width, int format, int colour)
{
	// font height might actually be something else
	int font_height, line_height, line_width, line_y, num_lines;

	// Location of font sprites
	uint16* current_font_sprite_base = RCT2_ADDRESS(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16);
	*current_font_sprite_base = 0xE0;

	char* buffer = RCT2_ADDRESS(0x009C383D, char);

	gfx_draw_string(dpi, buffer, colour, dpi->x, dpi->y);

	buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, char);

	format_string(buffer, format, args);

	*current_font_sprite_base = 0xE0;

	// Line width unused here
	line_width = gfx_wrap_string(buffer, width, &num_lines, &font_height);

	line_height = 0x0A;

	if (font_height > 0xE0) {
		line_height = 6;
		if (font_height != 0x1C0) {
			line_height = 0x12;
		}
	}

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_FLAGS, uint16) = 0;

	line_y = y;

	for (int line = 0; line <= num_lines; ++line) {
		gfx_draw_string(dpi, buffer, 0xFE, x, line_y);
		buffer += strlen(buffer) + 1;
        line_y += line_height;
	} 

	return line_y - y;
}

/**
 * Draws i formatted text string.
 *  rct2: 0x006C1B2F
 * dpi (edi)
 * format (bx)
 * args (esi)
 * colour (al)
 * x (cx)
 * y (dx)
 */
void gfx_draw_string_left(rct_drawpixelinfo *dpi, int format, void *args, int colour, int x, int y)
{
	char* buffer;

	buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, char);
	format_string(buffer, format, args);
	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 0xE0;
	gfx_draw_string(dpi, buffer, colour, x, y);
}

/* Changes the palette so that the next character changes colour
*/
void colour_char(uint8 colour, uint16* current_font_flags, uint8* palette_pointer) {

	int eax;

	rct_g1_element g1_element = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[0x1332];
	eax = ((uint32*)g1_element.offset)[colour & 0xFF];

	if (!(*current_font_flags & 2)) {
		eax = eax & 0x0FF0000FF;
	}
	// Adjust text palette. Store current colour? 
	palette_pointer[1] = eax & 0xFF;
	palette_pointer[2] = (eax >> 8) & 0xFF;
	palette_pointer[3] = (eax >> 16) & 0xFF;
	palette_pointer[4] = (eax >> 24) & 0xFF;
	RCT2_GLOBAL(0x009ABDA4, uint32) = (uint32)palette_pointer;
}

/* Changes the palette so that the next character changes colour
*  This is specific to changing to a predefined window related colour
*/
void colour_char_window(uint8 colour, uint16* current_font_flags,uint8* palette_pointer) {

	int eax;

	eax = RCT2_ADDRESS(0x0141FD45, uint8)[colour * 8];
	if (*current_font_flags & 2) {
		eax |= 0x0A0A00;
	}
	 //Adjust text palette. Store current colour? 
	palette_pointer[1] = eax & 0xFF;
	palette_pointer[2] = (eax >> 8) & 0xFF;
	palette_pointer[3] = (eax >> 16) & 0xFF;
	palette_pointer[4] = (eax >> 24) & 0xFF;
	RCT2_GLOBAL(0x009ABDA4, uint32) = (uint32)palette_pointer;
}


/**
 * 
 *  rct2: 0x00682702
 * dpi (edi)
 * buffer (esi)
 * colour (al)
 * x (cx)
 * y (dx)
 */
void gfx_draw_string(rct_drawpixelinfo *dpi, char *buffer, int colour, int x, int y)
{

	int eax, ebx, ebp;
	rct_g1_element* g1_element;

	// Maximum length/height of string
	int max_x = x;
	int max_y = y;

	// 
	uint16* current_font_flags = RCT2_ADDRESS(RCT2_ADDRESS_CURRENT_FONT_FLAGS, uint16);
	uint16* current_font_sprite_base = RCT2_ADDRESS(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16);

	uint8* palette_pointer = text_palette;

	// Flag for skipping non-printing characters
	int skip_char = 0;

	if (colour != 0xFE) {

		if (x >= dpi->x + dpi->width)
			return;
		
		if (x + 0x280 <= dpi->x)
			return;

		if (y >= dpi->y + dpi->height)
			return;

		if (y + 0x5A <= dpi->y) {
			return;
		}

		if (colour != 0xFF) {

			// switch_colour:
			*current_font_flags = 0;
			if (*current_font_sprite_base < 0) {
				*current_font_flags |= 4;
				if (*current_font_sprite_base != 0xFFFF) {
					*current_font_flags |= 8;
				}
				*current_font_sprite_base = 0xE0;
			}
			if (colour & (1 << 5)) {
				*current_font_flags |= 2;
			}
			colour &= ~(1 << 5);

			if (!(colour & 0x40)) {
				ebp = colour;
				if (*current_font_flags & 1) {
					if ((y + 0x13 <= dpi->y) || (dpi->y + dpi->height <= y)) {
						skip_char = 1;
					} else {
						skip_char = 0;
					}
				} else {
					colour_char_window(ebp, current_font_flags, palette_pointer);
				}
			} else {
				*current_font_flags |= 1;
				colour &= 0x1F;

				if (*current_font_flags & 4) {
					if (*current_font_flags & 8) {
						eax = RCT2_ADDRESS(0x0141FC48, uint8)[colour * 8];
						eax = eax << 16;
						eax = eax | RCT2_ADDRESS(0x0141FC46, uint8)[colour * 8];
					} else {
						eax = RCT2_ADDRESS(0x0141FC49, uint8)[colour * 8];
						eax = eax << 16;
						eax = eax | RCT2_ADDRESS(0x0141FC47, uint8)[colour * 8];
					}
				} else {
					eax = RCT2_ADDRESS(0x0141FC4A, uint8)[colour * 8];
					eax = eax << 16;
					eax = eax | RCT2_ADDRESS(0x0141FC48, uint8)[colour * 8];
				}
				// Adjust text palette. Store current colour? ;
				palette_pointer[1] = eax & 0xFF;
				palette_pointer[2] = (eax >> 8) & 0xFF;
				palette_pointer[3] = (eax >> 16) & 0xFF;
				palette_pointer[4] = (eax >> 24) & 0xFF;
				RCT2_GLOBAL(0x009ABDA4, uint32) = (uint32)palette_pointer;
				eax = 0;
			}
		}
	}

	if ((y + 0x13 <= dpi->y) || (dpi->y + dpi->height <= y)) {
		skip_char = 1;
	}

	for (uint8 al = *buffer; al > 0; ++buffer, al = *buffer) {

		// Skip to the next printing character
		if (skip_char) {
			if (al < 0x20) {
				// Control codes
				skip_char = 0;
			} else if (al >= FORMAT_COLOUR_CODE_START && al <= FORMAT_COLOUR_CODE_END) {
				// Colour codes
				if (*current_font_flags == 1) {
					if ((y + 0x13 <= dpi->y) || (dpi->y + dpi->height <= y)) {
						skip_char = 1;
					} else {
						skip_char = 0;
					}
					continue;
				}
				colour_char(al - FORMAT_COLOUR_CODE_START, current_font_flags, palette_pointer);
				continue;
			} else {
				continue;
			}
		}

		// Control codes
		switch (al) {
		case FORMAT_MOVE_X://Start New Line at start+buffer x, same y. (Overwrite?)
			max_x = x + *++buffer;
			break;
		case FORMAT_ADJUST_PALETTE:
			al = *++buffer;
			if (*current_font_flags & 1) {
				if ((y + 0x13 <= dpi->y) || (dpi->y + dpi->height <= y)) {
					skip_char = 1;
					break;
				}
			}

			eax = palette_to_g1_offset[al]; //RCT2_ADDRESS(0x097FCBC, uint32)[al * 4];
			g1_element = &(RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[eax]);
			ebx = g1_element->offset[0xF9] + (1 << 8);
			if (!(*current_font_flags & 2)) {
				ebx = ebx & 0xFF;
			}

			palette_pointer[1] = ebx & 0xff;
			palette_pointer[2] = (ebx >> 8) & 0xff;
			//Adjust the text palette
			memcpy(palette_pointer + 3, &(g1_element->offset[0xF7]), 2);
			memcpy(palette_pointer + 5, &(g1_element->offset[0xFA]), 2);
			//Set the palette pointer
			RCT2_GLOBAL(0x009ABDA4, uint32) = (uint32)palette_pointer;


			if ((y + 0x13 <= dpi->y) || (dpi->y + dpi->height <= y)) {
				skip_char = 1;
			}
			break;
		case FORMAT_NEWLINE://Start New Line at set y lower
			max_x = x;
			if (*current_font_sprite_base <= 0xE0) {
				max_y += 10;
				break;
			}
			else if (*current_font_sprite_base == 0x1C0) {
				max_y += 6;
				break;
			}
			max_y += 18;
			break;
		case FORMAT_NEWLINE_SMALLER://Start New Line at set y lower
			max_x = x;
			if (*current_font_sprite_base <= 0xE0) {
				max_y += 5;
				break;
			}
			else if (*current_font_sprite_base == 0x1C0) {
				max_y += 3;
				break;
			}
			max_y += 9;
			break;
		case FORMAT_TINYFONT:
			*current_font_sprite_base = 0x1C0;
			break;
		case FORMAT_BIGFONT:
			*current_font_sprite_base = 0x2A0;
			break;
		case FORMAT_MEDIUMFONT:
			*current_font_sprite_base = 0xE0;
			break;
		case FORMAT_SMALLFONT:
			*current_font_sprite_base = 0;
			break;
		case FORMAT_OUTLINE:
			*current_font_flags |= 2;
			break;
		case FORMAT_OUTLINE_OFF:
			*current_font_flags &= 0x0FFFD;
			break;
		case FORMAT_WINDOW_COLOUR_1:
			ebp = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WINDOW_COLOUR_1, uint8);
			if (*current_font_flags & 1) {
				if ((y + 0x13 <= dpi->y) || (dpi->y + dpi->height <= y)) {
					skip_char = 1;
				}
				else {
					skip_char = 0;
				}
				break;
			}
			colour_char_window(ebp, current_font_flags, palette_pointer);
			break;
		case FORMAT_WINDOW_COLOUR_2:
			ebp = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WINDOW_COLOUR_2, uint8);
			if (*current_font_flags & 1) {
				if ((y + 0x13 <= dpi->y) || (dpi->y + dpi->height <= y)) {
					skip_char = 1;
				}
				else {
					skip_char = 0;
				}
				break;
			}
			colour_char_window(ebp, current_font_flags, palette_pointer);
			break;
		case FORMAT_WINDOW_COLOUR_3:
			ebp = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WINDOW_COLOUR_3, uint8);
			if (*current_font_flags & 1) {
				if ((y + 0x13 <= dpi->y) || (dpi->y + dpi->height <= y)) {
					skip_char = 1;
				}
				else {
					skip_char = 0;
				}
				break;
			}
			colour_char_window(ebp, current_font_flags, palette_pointer);
			break;
		case FORMAT_NEWLINE_X_Y: //Start new line at specified x,y
			max_x = x + *++buffer;
			max_y = y + *++buffer;
			break;
		case FORMAT_INLINE_SPRITE:
			buffer += 4;
			if (max_x >= dpi->x + dpi->width) {
				skip_char = 1;
				break;
			}
			ebx = *((uint16*)(buffer - 3));
			eax = ebx & 0x7FFFF;
			g1_element = &(RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[eax]);

			gfx_draw_sprite(dpi, ebx, max_x, max_y, 0);

			max_x = max_x + g1_element->width;
			break;
		default:
			// Colour codes
			if ((al >= FORMAT_COLOUR_CODE_START) && (al <= FORMAT_COLOUR_CODE_END)){
				
				if (*current_font_flags == 1) {
					if ((y + 0x13 <= dpi->y) || (dpi->y + dpi->height <= y)) {
						skip_char = 1;
					} else {
						skip_char = 0;
					}
					continue;
				}
				colour_char(al - FORMAT_COLOUR_CODE_START, current_font_flags, palette_pointer);
				continue;
			}
				
			// Normal Characters
			if (max_x >= dpi->x + dpi->width) {
				skip_char = 1;
			}
			if (max_x + 0x1A < dpi->x) {
				ebx = al-0x20;
				ebx += *current_font_sprite_base;
				max_x = max_x + (RCT2_ADDRESS(RCT2_ADDRESS_FONT_CHAR_WIDTH, uint8)[ebx] & 0xFF);
				continue;
			}

			uint32 char_offset = al - 0x20 + *current_font_sprite_base;
			RCT2_GLOBAL(0x00EDF81C, uint32) = (IMAGE_TYPE_USE_PALETTE << 28);
				
			gfx_draw_sprite_palette_set(dpi, (IMAGE_TYPE_USE_PALETTE << 28) | char_offset + SPR_CHAR_START, max_x, max_y, palette_pointer, NULL);
			max_x += (RCT2_ADDRESS(RCT2_ADDRESS_FONT_CHAR_WIDTH, uint8)[char_offset] & 0xFF);
			continue;
		} 
	}

	gLastDrawStringX = max_x;
	gLastDrawStringY = max_y;
}

/*
*
* rct2: 0x006EE53B
* left (ax)
* width (bx)
* top (cx)
* height (dx)
* drawpixelinfo (edi)
*/
rct_drawpixelinfo* clip_drawpixelinfo(rct_drawpixelinfo* dpi, int left, int width, int top, int height)
{
	rct_drawpixelinfo* newDrawPixelInfo = rct2_malloc(sizeof(rct_drawpixelinfo));

	int right = left + width;
	int bottom = top + height;

	newDrawPixelInfo->bits = dpi->bits;
	newDrawPixelInfo->x = dpi->x;
	newDrawPixelInfo->y = dpi->y;
	newDrawPixelInfo->width = dpi->width;
	newDrawPixelInfo->height = dpi->height;
	newDrawPixelInfo->pitch = dpi->pitch;
	newDrawPixelInfo->zoom_level = 0;
	newDrawPixelInfo->var_0F = dpi->var_0F;

	if (left > newDrawPixelInfo->x) {
		uint16 newWidth = left - newDrawPixelInfo->x;
		newDrawPixelInfo->width -= newWidth;
		newDrawPixelInfo->x = left;
		newDrawPixelInfo->pitch += newWidth;
		newDrawPixelInfo->bits += newWidth;
	}

	int stickOutWidth = newDrawPixelInfo->x + newDrawPixelInfo->width - right;
	if (stickOutWidth < 0) {
		newDrawPixelInfo->width -= stickOutWidth;
		newDrawPixelInfo->pitch += stickOutWidth;
	}

	if (top > newDrawPixelInfo->y) {
		uint16 newHeight = top - newDrawPixelInfo->y;
		newDrawPixelInfo->height = newHeight;
		newDrawPixelInfo->y = top;
		int bitsPlus = (newDrawPixelInfo->pitch + newDrawPixelInfo->width) * newHeight;
		newDrawPixelInfo->bits += bitsPlus;
	}

	int bp = newDrawPixelInfo->y + newDrawPixelInfo->height - bottom;
	if (bp < 0) {
		newDrawPixelInfo->height -= bp;
	}

	if (newDrawPixelInfo->width > 0 && newDrawPixelInfo->height > 0) {
		newDrawPixelInfo->x -= left;
		newDrawPixelInfo->y -= top;

		return newDrawPixelInfo;
	}

	rct2_free(newDrawPixelInfo);
	return NULL;
}

/***
*
* rct2: 0x00684027
*
* ebp used to be a parameter but it is always zero
* left   : eax
* top    : ebx
* width  : ecx
* height : edx
* x_start: edi
* y_start: esi
*/
void gfx_draw_rain(int left, int top, int width, int height, uint32 x_start, uint32 y_start){
	uint8* pattern = RCT2_GLOBAL(RCT2_ADDRESS_RAIN_PATTERN, uint8*);
	uint8 pattern_x_space = *pattern++;
	uint8 pattern_y_space = *pattern++;

	uint8 pattern_start_x_offset = x_start % pattern_x_space;
	uint8 pattern_start_y_offset = y_start % pattern_y_space;;

	rct_drawpixelinfo* dpi = RCT2_ADDRESS(RCT2_ADDRESS_SCREEN_DPI, rct_drawpixelinfo);
	uint32 pixel_offset = (dpi->pitch + dpi->width)*top + left;
	uint8 pattern_y_pos = pattern_start_y_offset;

	//Stores the colours of changed pixels
	uint32* pixel_store = RCT2_ADDRESS(RCT2_ADDRESS_RAIN_PIXEL_STORE, uint32);
	pixel_store += RCT2_GLOBAL(RCT2_ADDRESS_NO_RAIN_PIXELS, uint32);

	for (; height != 0; height--){
		uint8 pattern_x = pattern[pattern_y_pos * 2];
		if (pattern_x != 0xFF){
			if (RCT2_GLOBAL(0x9AC00C, uint32) <= 0x1F38){

				int final_pixel_offset = width + pixel_offset;

				int x_pixel_offset = pixel_offset;
				x_pixel_offset += ((uint8)(pattern_x - pattern_start_x_offset)) % pattern_x_space;

				uint8 pattern_pixel = pattern[pattern_y_pos * 2 + 1];
				for (; x_pixel_offset < final_pixel_offset; x_pixel_offset += pattern_x_space){
					uint8 current_pixel = dpi->bits[x_pixel_offset];
					dpi->bits[x_pixel_offset] = pattern_pixel;
					RCT2_GLOBAL(RCT2_ADDRESS_NO_RAIN_PIXELS, uint32)++;

					//Store colour and position
					*pixel_store++ = (x_pixel_offset << 8) | current_pixel;
				}
			}
		}

		pixel_offset += dpi->pitch + dpi->width;

		pattern_y_pos++;
		pattern_y_pos %= pattern_y_space;
	}
}

void draw_string_left_underline(rct_drawpixelinfo *dpi, int format, void *args, int colour, int x, int y)
{
	char buffer[128];
	int width;

	format_string(buffer, format, args);
	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 224;
	width = gfx_get_string_width(buffer);
	gfx_draw_string(dpi, buffer, colour, x, y);
	gfx_fill_rect(dpi, x, y + 11, x + width, y + 11, text_palette[1]);
	if (text_palette[2] != 0)
		gfx_fill_rect(dpi, x + 1, y + 12, x + width + 1, y + 12, text_palette[2]);
}

void draw_string_right_underline(rct_drawpixelinfo *dpi, int format, void *args, int colour, int x, int y)
{
	char buffer[128];
	int width;

	format_string(buffer, format, args);
	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 224;
	width = gfx_get_string_width(buffer);
	x -= width;
	gfx_draw_string(dpi, buffer, colour, x, y);
	gfx_fill_rect(dpi, x, y + 11, x + width, y + 11, text_palette[1]);
	if (text_palette[2] != 0)
		gfx_fill_rect(dpi, x + 1, y + 12, x + width + 1, y + 12, text_palette[2]);
}
