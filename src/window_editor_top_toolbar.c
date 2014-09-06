/*****************************************************************************
* Copyright (c) 2014 Dániel Tar
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

#include "addresses.h"
#include "string_ids.h"
#include "sprites.h"
#include "viewport.h"
#include "widget.h"
#include "window.h"

enum WINDOW_EDITOR_TOP_TOOLBAR_WIDGET_IDX {
	WIDX_PAUSE,					// 1
	WIDX_FILE_MENU,				// 2
	WIDX_ZOOM_OUT,				// 4
	WIDX_ZOOM_IN,				// 8
	WIDX_ROTATE,				// 10
	WIDX_VIEW_MENU,				// 20
	WIDX_MAP,					// 40
	WIDX_LAND,					// 80
	WIDX_WATER,					// 100
	WIDX_SCENERY,				// 200
	WIDX_PATH,					// 400
	WIDX_CONSTRUCT_RIDE,		// 800
	WIDX_RIDES,					// 1000
	WIDX_PARK,					// 2000
	WIDX_STAFF,					// 4000
	WIDX_GUESTS,				// 8000
	WIDX_CLEAR_SCENERY,			// 10000
};

static rct_widget window_editor_top_toolbar_widgets[] = {
	{ WWT_EMPTY, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFF },
	{ WWT_TRNBTN, 0, 0, 29, 0, 27, 0x20000000 | SPR_TOOLBAR_FILE, STR_DISC_AND_GAME_OPTIONS_TIP },
	{ WWT_TRNBTN, 1, 40, 69, 0, 27, 0x20000000 | SPR_TOOLBAR_ZOOM_OUT, STR_ZOOM_OUT_TIP },
	{ WWT_TRNBTN, 1, 70, 99, 0, 27, 0x20000000 | SPR_TOOLBAR_ZOOM_IN, STR_ZOOM_IN_TIP },
	{ WWT_TRNBTN, 1, 100, 129, 0, 27, 0x20000000 | SPR_TOOLBAR_ROTATE, STR_ROTATE_TIP },
	{ WWT_TRNBTN, 1, 130, 159, 0, 27, 0x20000000 | SPR_TOOLBAR_VIEW, STR_VIEW_OPTIONS_TIP },
	{ WWT_TRNBTN, 1, 160, 189, 0, 27, 0x20000000 | SPR_TOOLBAR_MAP, STR_SHOW_MAP_TIP },
	{ WWT_TRNBTN, 2, 267, 296, 0, 27, 0x20000000 | SPR_TOOLBAR_LAND, STR_ADJUST_LAND_TIP },
	{ WWT_TRNBTN, 2, 297, 326, 0, 27, 0x20000000 | SPR_TOOLBAR_WATER, STR_ADJUST_WATER_TIP },
	{ WWT_TRNBTN, 2, 327, 356, 0, 27, 0x20000000 | SPR_TOOLBAR_SCENERY, STR_PLACE_SCENERY_TIP },
	{ WWT_TRNBTN, 2, 357, 386, 0, 27, 0x20000000 | SPR_TOOLBAR_FOOTPATH, STR_BUILD_FOOTPATH_TIP },
	{ WWT_TRNBTN, 2, 387, 416, 0, 27, 0x20000000 | SPR_TOOLBAR_CONSTRUCT_RIDE, STR_BUILD_RIDE_TIP },
	{ WWT_EMPTY, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFF },
	{ WWT_EMPTY, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFF },
	{ WWT_EMPTY, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFF },
	{ WWT_EMPTY, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFF },
	{ WWT_TRNBTN, 2, 560, 589, 0, 27, 0x20000000 | SPR_TOOLBAR_CLEAR_SCENERY, STR_CLEAR_SCENERY_TIP },
	{ WIDGETS_END },
};

static void window_editor_top_toolbar_emptysub() { }

static void* window_editor_top_toolbar_events[] = {
	window_editor_top_toolbar_emptysub,
	0x0066f9d7,
	0x0066fada,
	0x0066fa57,
	0x0066fa38,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	0x0066fb0e,
	0x0066fb5c,
	0x0066fb37,
	0x0066fc44,
	0x0066fa74,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	0x0066f87d,
	0x0066f9d1,
	window_editor_top_toolbar_emptysub
};

/**
* Creates the main editor top toolbar window.
* rct2: 0x0066B485 (part of 0x0066B3E8)
*/
void window_editor_top_toolbar_open()
{
	rct_window* window;

	window = window_create(0, 0, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16), 28,
		(uint32*)window_editor_top_toolbar_events,
		WC_TOP_TOOLBAR, WF_STICK_TO_FRONT | WF_TRANSPARENT | WF_5);
	window->widgets = window_editor_top_toolbar_widgets;

	window->enabled_widgets |=
		(1 << WIDX_ZOOM_IN) |
		(1 << WIDX_ZOOM_OUT) |
		(1 << WIDX_ROTATE) |
		(1 << WIDX_FILE_MENU) |
		(1 << WIDX_LAND) |
		(1 << WIDX_VIEW_MENU) |
		(1 << WIDX_SCENERY) |
		(1 << WIDX_WATER) |
		(1 << WIDX_PATH) |
		(1 << WIDX_MAP) |
		(1 << WIDX_CONSTRUCT_RIDE) |
		(1 << WIDX_CLEAR_SCENERY);

	window_init_scroll_widgets(window);
	window->colours[0] = 7;
	window->colours[1] = 12;
	window->colours[2] = 24;
	window->colours[3] = 1;
}