/*****************************************************************************
 * Copyright (c) 2014 Ted John
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

#include <string.h>
#include "../addresses.h"
#include "../audio/audio.h"
#include "../game.h"
#include "../input.h"
#include "../interface/viewport.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "../localisation/localisation.h"
#include "../sprites.h"
#include "../ride/track.h"

#define TRACK_MINI_PREVIEW_WIDTH	168
#define TRACK_MINI_PREVIEW_HEIGHT	78
#define TRACK_MINI_PREVIEW_SIZE		(TRACK_MINI_PREVIEW_WIDTH * TRACK_MINI_PREVIEW_HEIGHT)

enum {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_ROTATE,
	WIDX_MIRROR,
	WIDX_SELECT_DIFFERENT_DESIGN,
	WIDX_PRICE
};

static rct_widget window_track_place_widgets[] = {
	{ WWT_FRAME,			0,	0,		199,	0,		123,	0xFFFFFFFF,						STR_NONE									},
	{ WWT_CAPTION,			0,	1,		198,	1,		14,		3155,							STR_WINDOW_TITLE_TIP						},
	{ WWT_CLOSEBOX,			0,	187,	197,	2,		13,		STR_CLOSE_X,					STR_CLOSE_WINDOW_TIP						},
	{ WWT_FLATBTN,			0,	173,	196,	83,		106,	SPR_ROTATE_ARROW,				STR_ROTATE_90_TIP							},
	{ WWT_FLATBTN,			0,	173,	196,	59,		82,		5170,							STR_MIRROR_IMAGE_TIP						},
	{ WWT_DROPDOWN_BUTTON,	0,	4,		195,	109,	120,	STR_SELECT_A_DIFFERENT_DESIGN,	STR_GO_BACK_TO_DESIGN_SELECTION_WINDOW_TIP	},
	{ WWT_EMPTY,			0,	0,		0,		0,		0,		0xFFFFFFFF,						STR_NONE									},
	{ WIDGETS_END },
};

static void window_track_place_emptysub() { }
static void window_track_place_close();
static void window_track_place_mouseup();
static void window_track_place_update(rct_window *w);
static void window_track_place_toolupdate();
static void window_track_place_tooldown();
static void window_track_place_toolabort();
static void window_track_place_unknown14();
static void window_track_place_paint();

static void* window_track_place_events[] = {
	window_track_place_close,
	window_track_place_mouseup,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_update,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_toolupdate,
	window_track_place_tooldown,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_toolabort,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_unknown14,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_emptysub,
	window_track_place_paint,
	window_track_place_emptysub
};

static uint8 *_window_track_place_mini_preview;
static sint16 _window_track_place_last_x;
static sint16 _window_track_place_last_y;

static uint8 _window_track_place_last_was_valid;
static sint16 _window_track_place_last_valid_x;
static sint16 _window_track_place_last_valid_y;
static sint16 _window_track_place_last_valid_z;
static money32 _window_track_place_last_cost;

/**
 *
 *  rct2: 0x006D182E
 */
static void window_track_place_clear_mini_preview()
{
	memset(_window_track_place_mini_preview, 220, TRACK_MINI_PREVIEW_SIZE);
}

/**
 * Size: 0x0A
 */
typedef struct {
	uint8 var_00;
	sint16 x;
	sint16 y;
	uint8 pad_05[3];
	uint8 var_08;
	uint8 unk_09;
} rct_preview_track;

/**
 * Size: 0x04
 */
typedef struct {
	union {
		uint32 all;
		struct {
			sint8 x;
			sint8 y;
			uint8 unk_2;
			uint8 type;
		};
	};
} rct_preview_maze;

#define swap(x, y) x = x ^ y; y = x ^ y; x = x ^ y;

/**
 *
 *  rct2: 0x006D1845
 */
static void window_track_place_draw_mini_preview()
{
	rct_track_design *design = (rct_track_design*)0x009D8178;
	uint8 *pixel, colour, *trackPtr, bits;
	int i, rotation, pass, x, y, pixelX, pixelY, originX, originY, minX, minY, maxX, maxY;
	rct_preview_maze *mazeBlock;
	rct_preview_track *trackBlock;

	window_track_place_clear_mini_preview();

	minX = 0;
	minY = 0;
	maxX = 0;
	maxY = 0;

	// First pass is used to determine the width and height of the image so it can centre it
	for (pass = 0; pass < 2; pass++) {
		originX = 0;
		originY = 0;
		if (pass == 1) {
			originX -= ((maxX + minX) >> 6) << 5;
			originY -= ((maxY + minY) >> 6) << 5;
		}

		if (design->type != RIDE_TYPE_MAZE) {
			#pragma region Track

			rotation = RCT2_GLOBAL(0x00F440AE, uint8) + RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_ROTATION, uint32);
			trackPtr = design->preview[0];

			while (*trackPtr != 255) {
				int trackType = *trackPtr;
				if (trackType == 101)
					trackType = 255;

				// Station track is a lighter colour
				colour = RCT2_ADDRESS(0x0099BA64, uint8)[trackType * 16] & 0x10 ? 222 : 218;

				// Follow a single track piece shape
				trackBlock = RCT2_ADDRESS(0x00994638, rct_preview_track*)[trackType];
				while (trackBlock->var_00 != 255) {
					x = originX;
					y = originY;
					
					switch (rotation & 3) {
					case 0:
						x += trackBlock->x;
						y += trackBlock->y;
						break;
					case 1:
						x += trackBlock->y;
						y -= trackBlock->x;
						break;
					case 2:
						x -= trackBlock->x;
						y -= trackBlock->y;
						break;
					case 3:
						x -= trackBlock->y;
						y += trackBlock->x;
						break;
					}

					if (pass == 0) {
						minX = min(minX, x);
						maxX = max(maxX, x);
						minY = min(minY, y);
						maxY = max(maxY, y);
					} else {
						pixelX = 80 + ((y / 32) - (x / 32)) * 4;
						pixelY = 38 + ((y / 32) + (x / 32)) * 2;
						if (pixelX <= 160 && pixelY <= 75) {
							pixel = &_window_track_place_mini_preview[pixelY * TRACK_MINI_PREVIEW_WIDTH + pixelX];

							bits = trackBlock->var_08 << (rotation & 3);
							bits = (bits & 0x0F) | ((bits & 0xF0) >> 4);

							for (i = 0; i < 4; i++) {
								if (bits & 1) pixel[338 + i] = colour;
								if (bits & 2) pixel[168 + i] = colour;
								if (bits & 4) pixel[  2 + i] = colour;
								if (bits & 8) pixel[172 + i] = colour;
							}
						}
					}
					trackBlock++;
				}

				// Change rotation and next position based on track curvature
				rotation &= 3;
				trackType *= 10;
				switch (rotation) {
				case 0:
					originX += RCT2_GLOBAL(0x009968C1 + trackType, sint16);
					originY += RCT2_GLOBAL(0x009968C3 + trackType, sint16);
					break;
				case 1:
					originX += RCT2_GLOBAL(0x009968C3 + trackType, sint16);
					originY -= RCT2_GLOBAL(0x009968C1 + trackType, sint16);
					break;
				case 2:
					originX -= RCT2_GLOBAL(0x009968C1 + trackType, sint16);
					originY -= RCT2_GLOBAL(0x009968C3 + trackType, sint16);
					break;
				case 3:
					originX -= RCT2_GLOBAL(0x009968C3 + trackType, sint16);
					originY += RCT2_GLOBAL(0x009968C1 + trackType, sint16);
					break;
				}
				rotation += RCT2_ADDRESS(0x009968BC, uint8)[trackType] - RCT2_ADDRESS(0x009968BB, uint8)[trackType];
				rotation &= 3;
				if (RCT2_ADDRESS(0x009968BC, uint8)[trackType] & 4)
					rotation |= 4;
				if (!(rotation & 4)) {
					originX += RCT2_GLOBAL(0x00993CCC + (rotation * 4), sint16);
					originY += RCT2_GLOBAL(0x00993CCE + (rotation * 4), sint16);
				}
				trackPtr += 2;
			}

			#pragma endregion
		} else {
			#pragma region Maze

			rotation = (RCT2_GLOBAL(0x00F440AE, uint8) + RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_ROTATION, uint32)) & 3;
			mazeBlock = (rct_preview_maze*)design->preview[0];
			while (mazeBlock->all != 0) {
				x = mazeBlock->x * 32;
				y = mazeBlock->y * 32;
				switch (rotation) {
				case 1:
					x = -x;
					swap(x, y);
					break;
				case 2:
					x = -x;
					y = -y;
					break;
				case 3:
					x = -x;
					swap(x, y);
					break;
				}
				x += originX;
				y += originY;

				// Entrance or exit is a lighter colour
				colour = mazeBlock->type == 8 || mazeBlock->type == 128 ? 222 : 218;

				if (pass == 0) {
					minX = min(minX, x);
					maxX = max(maxX, x);
					minY = min(minY, y);
					maxY = max(maxY, y);
				} else {
					pixelX = 80 + ((y / 32) - (x / 32)) * 4;
					pixelY = 38 + ((y / 32) + (x / 32)) * 2;
					if (pixelX <= 160 && pixelY <= 75) {
						pixel = &_window_track_place_mini_preview[pixelY * TRACK_MINI_PREVIEW_WIDTH + pixelX];

						for (i = 0; i < 4; i++) {
							pixel[338 + i] = colour;
							pixel[168 + i] = colour;
							pixel[  2 + i] = colour;
							pixel[172 + i] = colour;
						}
					}
				}
				mazeBlock++;
			}

			#pragma endregion
		}
	}
}

/**
 *
 *  rct2: 0x0068A15E
 */
static void sub_68A15E(int x, int y, short *ax, short *bx)
{
	int eax, ebx, ecx, edx, esi, edi, ebp;
	eax = x;
	ebx = y;
	RCT2_CALLFUNC_X(0x0068A15E, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
	*ax = *((short*)&eax);
	*bx = *((short*)&ebx);
}

/**
 * Seems to highlight the surface tiles to match the track layout at the given position but also returns some Z value.
 *  rct2: 0x006D01B3
 */
static int sub_6D01B3(int bl, int x, int y, int z)
{
	int eax, ebx, ecx, edx, esi, edi, ebp;
	eax = x;
	ebx = bl;
	ecx = y;
	edx = z;
	esi = 0;
	edi = 0;
	ebp = 0;
	RCT2_CALLFUNC_X(0x006D01B3,  &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
	return *((short*)&ebx);
}

/**
 *
 *  rct2: 0x006D017F
 */
static void window_track_place_clear_provisional()
{
	if (_window_track_place_last_was_valid) {
		sub_6D01B3(
			(RCT2_GLOBAL(0x00F440EB, uint8) << 8) | 6,
			_window_track_place_last_valid_x,
			_window_track_place_last_valid_y,
			_window_track_place_last_valid_z
		);
		_window_track_place_last_was_valid = 0;
	}
}

/**
 *
 *  rct2: 0x006D17C6
 */
static int window_track_place_get_base_z(int x, int y)
{
	rct_map_element *mapElement;
	int z;
	
	mapElement = map_get_surface_element_at(x >> 5, y >> 5);
	z = mapElement->base_height * 8;

	// Increase Z above slope
	if (mapElement->properties.surface.slope & 0x0F) {
		z += 16;

		// Increase Z above double slope
		if (mapElement->properties.surface.slope & 0x10)
			z += 16;
	}

	// Increase Z above water
	if (mapElement->properties.surface.terrain & 0x1F)
		z = max(z, (mapElement->properties.surface.terrain & 0x1F) << 4);
	
	return z + sub_6D01B3(3, x, y, z);
}

static void window_track_place_attempt_placement(int x, int y, int z, int bl, money32 *cost, uint8 *rideIndex)
{
	int eax, ebx, ecx, edx, esi, edi, ebp;
	money32 result;

	eax = x;
	ebx = bl;
	ecx = y;
	edi = z;
	result = game_do_command_p(GAME_COMMAND_47, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);

	if (cost != NULL) *cost = result;
	if (rideIndex != NULL) *rideIndex = edi & 0xFF;
}

/**
 *
 *  rct2: 0x006CFCA0
 */
void window_track_place_open()
{
	rct_window *w;

	window_close_construction_windows();

	_window_track_place_mini_preview = malloc(TRACK_MINI_PREVIEW_SIZE);
	window_track_place_clear_mini_preview();

	w = window_create(0, 29, 200, 124, (uint32*)window_track_place_events, WC_TRACK_DESIGN_PLACE, 0);
	w->widgets = window_track_place_widgets;
	w->enabled_widgets = 4 | 8 | 0x10 | 0x20;
	window_init_scroll_widgets(w);
	w->colours[0] = 24;
	w->colours[1] = 24;
	w->colours[2] = 24;
	tool_set(w, 6, 12);
	RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) |= 6;
	window_push_others_right(w);
	show_gridlines();
	_window_track_place_last_cost = MONEY32_UNDEFINED;
	_window_track_place_last_x = 0xFFFF;
	RCT2_GLOBAL(0x00F440AE, uint8) = (-RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_ROTATION, uint8)) & 3;
	window_track_place_draw_mini_preview();
}

/**
 *
 *  rct2: 0x006D0119
 */
static void window_track_place_close()
{
	window_track_place_clear_provisional();
	viewport_set_visibility(0);
	RCT2_CALLPROC_EBPSAFE(0x0068AB1B);
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_FLAGS, uint16) &= ~6;
	hide_gridlines();
	free(_window_track_place_mini_preview);
}

/**
 *
 *  rct2: 0x006CFEAC
 */
static void window_track_place_mouseup()
{
	rct_window *w;
	short widgetIndex;

	window_widget_get_registers(w, widgetIndex);

	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	case WIDX_ROTATE:
		window_track_place_clear_provisional();
		RCT2_GLOBAL(0x00F440AE, uint16) = (RCT2_GLOBAL(0x00F440AE, uint16) + 1) & 3;
		window_invalidate(w);
		_window_track_place_last_x = 0xFFFF;
		window_track_place_draw_mini_preview();
		break;
	case WIDX_MIRROR:
		RCT2_CALLPROC_EBPSAFE(0x006D2436);
		RCT2_GLOBAL(0x00F440AE, uint16) = (-RCT2_GLOBAL(0x00F440AE, uint16)) & 3;
		window_invalidate(w);
		_window_track_place_last_x = 0xFFFF;
		window_track_place_draw_mini_preview();
		break;
	case WIDX_SELECT_DIFFERENT_DESIGN:
		window_close(w);
		window_track_list_open(_window_track_list_item);
		break;
	}
}

/**
 *
 *  rct2: 0x006CFCA0
 */
static void window_track_place_update(rct_window *w)
{
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) & INPUT_FLAG_TOOL_ACTIVE))
		if (RCT2_GLOBAL(RCT2_ADDRESS_TOOL_WINDOWCLASS, rct_windowclass) != WC_TRACK_DESIGN_PLACE)
			window_close(w);
}

/**
 *
 *  rct2: 0x006CFF2D
 */
static void window_track_place_toolupdate()
{
	rct_window *w;
	short widgetIndex, x, y;
	int i, z;
	money32 cost;
	uint8 rideIndex;

	window_tool_get_registers(w, widgetIndex, x, y);

	RCT2_CALLPROC_EBPSAFE(0x0068AB1B);
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_FLAGS, uint16) &= ~7;

	// Get the tool map position
	sub_68A15E(x, y, &x, &y);
	if (x == (short)0x8000) {
		window_track_place_clear_provisional();
		return;
	}

	// Check if tool map position has changed since last update
	if (x == _window_track_place_last_x && y == _window_track_place_last_y) {
		sub_6D01B3(0, x, y, 0);
		return;
	}

	cost = MONEY32_UNDEFINED;

	// Get base Z position
	z = window_track_place_get_base_z(x, y);
	if (RCT2_GLOBAL(0x009DEA6E, uint8) == 0) {
		window_track_place_clear_provisional();
		
		// Try increasing Z until a feasible placement is found
		for (i = 0; i < 7; i++) {
			window_track_place_attempt_placement(x, y, z, 105, &cost, &rideIndex);
			if (cost != MONEY32_UNDEFINED) {
				RCT2_GLOBAL(0x00F440EB, uint16) = rideIndex;
				_window_track_place_last_valid_x = x;
				_window_track_place_last_valid_y = y;
				_window_track_place_last_valid_z = z;
				_window_track_place_last_was_valid = 1;
				break;
			}
			z += 8;
		}
	}

	_window_track_place_last_x = x;
	_window_track_place_last_y = y;
	if (cost != _window_track_place_last_cost) {
		_window_track_place_last_cost = cost;
		widget_invalidate(w, WIDX_PRICE);
	}
	
	sub_6D01B3(0, x, y, z);
}

/**
 *
 *  rct2: 0x006CFF34
 */
static void window_track_place_tooldown()
{
	rct_window *w;
	short widgetIndex, x, y, z;
	int i;
	money32 cost;
	uint8 rideIndex;

	window_tool_get_registers(w, widgetIndex, x, y);

	window_track_place_clear_provisional();
	RCT2_CALLPROC_EBPSAFE(0x0068AB1B);
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_FLAGS, uint16) &= ~7;

	sub_68A15E(x, y, &x, &y);
	if (x == (short)0x8000)
		return;
	
	// Try increasing Z until a feasible placement is found
	z = window_track_place_get_base_z(x, y);
	for (i = 0; i < 7; i++) {
		RCT2_GLOBAL(0x009A8C29, uint8) |= 1;
		window_track_place_attempt_placement(x, y, z, 1, &cost, &rideIndex);
		RCT2_GLOBAL(0x009A8C29, uint8) &= ~1;

		if (cost != MONEY32_UNDEFINED) {
			window_close_by_class(WC_ERROR);
			sound_play_panned(SOUND_PLACE_ITEM, 0x8001, x, y, z);

			RCT2_GLOBAL(0x00F440A7, uint8) = rideIndex;
			if (RCT2_GLOBAL(0x00F4414E, uint8) & 1) {
				window_ride_main_open(rideIndex);
				window_close(w);
			} else {
				RCT2_CALLPROC_X(0x006CC3FB, 0, 0, 0, rideIndex, 0, 0, 0);
				w = window_find_by_class(WC_RIDE_CONSTRUCTION);
				window_event_helper(w, 29, WE_MOUSE_UP);
			}
			return;
		}

		// Check if player did not have enough funds
		if (RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) == 827)
			break;

		z += 8;
	}

	// Unable to build track
	sound_play_panned(SOUND_ERROR, 0x8001, x, y, z);
}

/**
 *
 *  rct2: 0x006D015C
 */
static void window_track_place_toolabort()
{
	window_track_place_clear_provisional();
}

/**
 *
 *  rct2: 0x006CFF01
 */
static void window_track_place_unknown14()
{
	window_track_place_draw_mini_preview();
}

/**
 *
 *  rct2: 0x006CFD9D
 */
static void window_track_place_paint()
{
	rct_window *w;
	rct_drawpixelinfo *dpi, *clippedDpi;
	rct_g1_element tmpElement, *subsituteElement, *g1Elements = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element);
	
	window_paint_get_registers(w, dpi);

	window_draw_widgets(w, dpi);

	// Draw mini tile preview
	clippedDpi = clip_drawpixelinfo(dpi, w->x + 4, 168, w->y + 18, 78);
	if (clippedDpi != NULL) {
		subsituteElement = &g1Elements[0];
		tmpElement = *subsituteElement;
		subsituteElement->offset = _window_track_place_mini_preview;
		subsituteElement->width = TRACK_MINI_PREVIEW_WIDTH;
		subsituteElement->height = TRACK_MINI_PREVIEW_HEIGHT;
		subsituteElement->x_offset = 0;
		subsituteElement->y_offset = 0;
		subsituteElement->flags = 0;
		gfx_draw_sprite(clippedDpi, 0, 0, 0, 0);
		*subsituteElement = tmpElement;
	}

	// Price
	if (_window_track_place_last_cost != MONEY32_UNDEFINED)
		if (!(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY))
			gfx_draw_string_centred(dpi, STR_COST_LABEL, w->x + 88, w->y + 94, 0, &_window_track_place_last_cost);
}