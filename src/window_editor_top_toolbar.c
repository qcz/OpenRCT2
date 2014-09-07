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
#include "editor.h"
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
	{ WWT_EMPTY, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFF },														// 1		0x009A9844
	{ WWT_TRNBTN, 0, 0, 29, 0, 27, 0x20000000 | SPR_TOOLBAR_FILE, STR_DISC_AND_GAME_OPTIONS_TIP },			// 2		0x009A9854
	{ WWT_TRNBTN, 1, 40, 69, 0, 27, 0x20000000 | SPR_TOOLBAR_ZOOM_OUT, STR_ZOOM_OUT_TIP },					// 4		0x009A9864
	{ WWT_TRNBTN, 1, 70, 99, 0, 27, 0x20000000 | SPR_TOOLBAR_ZOOM_IN, STR_ZOOM_IN_TIP },					// 8		0x009A9874
	{ WWT_TRNBTN, 1, 100, 129, 0, 27, 0x20000000 | SPR_TOOLBAR_ROTATE, STR_ROTATE_TIP },					// 10		0x009A9884
	{ WWT_TRNBTN, 1, 130, 159, 0, 27, 0x20000000 | SPR_TOOLBAR_VIEW, STR_VIEW_OPTIONS_TIP },				// 20		0x009A9894
	{ WWT_TRNBTN, 1, 160, 189, 0, 27, 0x20000000 | SPR_TOOLBAR_MAP, STR_SHOW_MAP_TIP },						// 40		0x009A98A4
	{ WWT_TRNBTN, 2, 267, 296, 0, 27, 0x20000000 | SPR_TOOLBAR_LAND, STR_ADJUST_LAND_TIP },					// 80		0x009A98B4
	{ WWT_TRNBTN, 2, 297, 326, 0, 27, 0x20000000 | SPR_TOOLBAR_WATER, STR_ADJUST_WATER_TIP },				// 100		0x009A98C4
	{ WWT_TRNBTN, 2, 327, 356, 0, 27, 0x20000000 | SPR_TOOLBAR_SCENERY, STR_PLACE_SCENERY_TIP },			// 200		0x009A98D4
	{ WWT_TRNBTN, 2, 357, 386, 0, 27, 0x20000000 | SPR_TOOLBAR_FOOTPATH, STR_BUILD_FOOTPATH_TIP },			// 400		0x009A98E4
	{ WWT_TRNBTN, 2, 387, 416, 0, 27, 0x20000000 | SPR_TOOLBAR_CONSTRUCT_RIDE, STR_BUILD_RIDE_TIP },		// 800		0x009A98F4
	{ WWT_EMPTY, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFF },														// 1000		0x009A9904
	{ WWT_EMPTY, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFF },														// 2000		0x009A9914
	{ WWT_EMPTY, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFF },														// 4000		0x009A9924
	{ WWT_EMPTY, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0xFFFF },														// 8000		0x009A9934
	{ WWT_TRNBTN, 2, 560, 589, 0, 27, 0x20000000 | SPR_TOOLBAR_CLEAR_SCENERY, STR_CLEAR_SCENERY_TIP },		// 10000	0x009A9944
	{ WIDGETS_END },
};

static void window_editor_top_toolbar_emptysub() { }

static void window_editor_top_toolbar_invalidate();
static void window_editor_top_toolbar_paint();

static void* window_editor_top_toolbar_events[] = {
	window_editor_top_toolbar_emptysub,
	(void*)0x0066f9d7, // mouseup
	(void*)0x0066fada, // resize
	(void*)0x0066fa57, // mousedown
	(void*)0x0066fa38, // dropdown
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_emptysub,
	(void*)0x0066fb0e,
	(void*)0x0066fb5c,
	(void*)0x0066fb37,
	(void*)0x0066fc44,
	(void*)0x0066fa74,
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
	window_editor_top_toolbar_invalidate, // (void*)0x0066f87d, // oninvalidate
	window_editor_top_toolbar_paint, //(void*)0x0066f9d1, // onpaint
	window_editor_top_toolbar_emptysub
};

/**
* Creates the main editor top toolbar window.
* rct2: 0x0066EFC8 (part of 0x0066EF38)
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

/**
*
*  rct2: 0x0066F87D
*/
static void window_editor_top_toolbar_invalidate()
{
	rct_window *w;

	window_get_register(w);

	sint16 screenWidth = max(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16), 640);

	window_editor_top_toolbar_widgets[WIDX_CONSTRUCT_RIDE].left = screenWidth - 30;
	window_editor_top_toolbar_widgets[WIDX_CONSTRUCT_RIDE].right =
		window_editor_top_toolbar_widgets[WIDX_CONSTRUCT_RIDE].left + 29;

	window_editor_top_toolbar_widgets[WIDX_PATH].left = screenWidth - 30;
	window_editor_top_toolbar_widgets[WIDX_PATH].right =
		window_editor_top_toolbar_widgets[WIDX_PATH].left + 29;

	window_editor_top_toolbar_widgets[WIDX_SCENERY].left =
		window_editor_top_toolbar_widgets[WIDX_PATH].left - 30;
	window_editor_top_toolbar_widgets[WIDX_SCENERY].right =
		window_editor_top_toolbar_widgets[WIDX_SCENERY].left + 29;

	window_editor_top_toolbar_widgets[WIDX_WATER].left =
		window_editor_top_toolbar_widgets[WIDX_SCENERY].left - 30;
	window_editor_top_toolbar_widgets[WIDX_WATER].right =
		window_editor_top_toolbar_widgets[WIDX_WATER].left + 29;

	window_editor_top_toolbar_widgets[WIDX_LAND].left =
		window_editor_top_toolbar_widgets[WIDX_WATER].left - 30;
	window_editor_top_toolbar_widgets[WIDX_LAND].right =
		window_editor_top_toolbar_widgets[WIDX_LAND].left + 29;

	window_editor_top_toolbar_widgets[WIDX_CLEAR_SCENERY].left =
		window_editor_top_toolbar_widgets[WIDX_LAND].left - 30;
	window_editor_top_toolbar_widgets[WIDX_CLEAR_SCENERY].right =
		window_editor_top_toolbar_widgets[WIDX_CLEAR_SCENERY].left + 29;

	window_editor_top_toolbar_widgets[WIDX_ZOOM_OUT].type = WWT_EMPTY;
	window_editor_top_toolbar_widgets[WIDX_ZOOM_IN].type = WWT_EMPTY;
	window_editor_top_toolbar_widgets[WIDX_ROTATE].type = WWT_EMPTY;
	window_editor_top_toolbar_widgets[WIDX_VIEW_MENU].type = WWT_EMPTY;
	window_editor_top_toolbar_widgets[WIDX_MAP].type = WWT_EMPTY;
	window_editor_top_toolbar_widgets[WIDX_CLEAR_SCENERY].type = WWT_EMPTY;
	window_editor_top_toolbar_widgets[WIDX_LAND].type = WWT_EMPTY;
	window_editor_top_toolbar_widgets[WIDX_WATER].type = WWT_EMPTY;
	window_editor_top_toolbar_widgets[WIDX_SCENERY].type = WWT_EMPTY;
	window_editor_top_toolbar_widgets[WIDX_PATH].type = WWT_EMPTY;
	window_editor_top_toolbar_widgets[WIDX_CONSTRUCT_RIDE].type = WWT_EMPTY;
	

	if (g_editor_step == EDITOR_STEP_LANDSCAPE_EDITOR) {
		window_editor_top_toolbar_widgets[WIDX_ZOOM_OUT].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_ZOOM_IN].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_ROTATE].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_VIEW_MENU].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_MAP].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_CLEAR_SCENERY].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_LAND].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_WATER].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_SCENERY].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_PATH].type = WWT_TRNBTN;
	} else if (g_editor_step == EDITOR_STEP_ROLLERCOASTER_DESIGNER) {
		window_editor_top_toolbar_widgets[WIDX_ZOOM_OUT].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_ZOOM_IN].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_ROTATE].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_VIEW_MENU].type = WWT_TRNBTN;
		window_editor_top_toolbar_widgets[WIDX_CONSTRUCT_RIDE].type = WWT_TRNBTN;
	}

	if (window_find_by_id(0x94, 0) == NULL)
		w->pressed_widgets &= ~(1 << WIDX_PATH);
	else
		w->pressed_widgets |= (1 << WIDX_PATH);
}

/**
*
*  rct2: 0x0066F9D1
*/
static void window_editor_top_toolbar_paint()
{
	rct_window *w;
	rct_drawpixelinfo *dpi;

	window_paint_get_registers(w, dpi);

	window_draw_widgets(w, dpi);
}