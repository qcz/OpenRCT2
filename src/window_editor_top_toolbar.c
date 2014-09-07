/*****************************************************************************
* Copyright (c) 2014 Dániel Tar, Ted John
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
#include "toolbar.h"
#include "viewport.h"
#include "widget.h"
#include "window.h"
#include "window_dropdown.h"

enum WINDOW_EDITOR_TOP_TOOLBAR_WIDGET_IDX {
	WIDX_PAUSE,					// 0, 1
	WIDX_FILE_MENU,				// 1, 2
	WIDX_ZOOM_OUT,				// 2, 4
	WIDX_ZOOM_IN,				// 3, 8
	WIDX_ROTATE,				// 4, 10
	WIDX_VIEW_MENU,				// 5, 20
	WIDX_MAP,					// 6, 40
	WIDX_LAND,					// 7, 80
	WIDX_WATER,					// 8, 100
	WIDX_SCENERY,				// 9, 200
	WIDX_PATH,					// 10, 400
	WIDX_CONSTRUCT_RIDE,		// 11, 800
	WIDX_RIDES,					// 12, 1000
	WIDX_PARK,					// 13, 2000
	WIDX_STAFF,					// 14, 4000
	WIDX_GUESTS,				// 15, 8000
	WIDX_CLEAR_SCENERY,			// 16, 10000
};

typedef enum {
	DDIDX_LOAD_GAME = 0,
	DDIDX_SAVE_GAME = 1,
	DDIDX_ABOUT = 3,
	DDIDX_OPTIONS = 4,
	DDIDX_SCREENSHOT = 5,
	DDIDX_QUIT_GAME = 7,
} FILE_MENU_DDIDX;

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

static void window_editor_top_toolbar_mouseup();
static void window_editor_top_toolbar_resize();
static void window_editor_top_toolbar_mousedown(int widgetIndex, rct_window*w, rct_widget* widget);
static void window_editor_top_toolbar_dropdown();
static void window_editor_top_toolbar_invalidate();
static void window_editor_top_toolbar_paint();

static void* window_editor_top_toolbar_events[] = {
	window_editor_top_toolbar_emptysub,
	window_editor_top_toolbar_mouseup, //(void*)0x0066f9d7, // mouseup
	window_editor_top_toolbar_resize, //(void*)0x0066fada, // resize
	window_editor_top_toolbar_mousedown, //(void*)0x0066fa57, // mousedown
	window_editor_top_toolbar_dropdown, //(void*)0x0066fa38, // dropdown
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
*  rct2: 0x0066C957
*/
static void window_editor_top_toolbar_mouseup()
{
	short widgetIndex;
	rct_window *w, *mainWindow;

	window_widget_get_registers(w, widgetIndex);

	switch (widgetIndex) {
	case WIDX_ZOOM_IN:
		if ((mainWindow = window_get_main()) != NULL)
			window_zoom_in(mainWindow);
		break;
	case WIDX_ZOOM_OUT:
		if ((mainWindow = window_get_main()) != NULL)
			window_zoom_out(mainWindow);
		break;
	case WIDX_ROTATE:
		if ((mainWindow = window_get_main()) != NULL)
			window_rotate_camera(mainWindow);
		break;
	case WIDX_SCENERY:
		if (!tool_set(w, WIDX_SCENERY, 0)) {
			RCT2_GLOBAL(0x009DE518, uint32) |= (1 << 6);
			window_scenery_open();
		}
		break;
	case WIDX_PATH:
		if (window_find_by_id(WC_FOOTPATH, 0) == NULL) {
			window_footpath_open();
		} else {
			tool_cancel();
			window_close_by_id(0x80 | WC_FOOTPATH, 0);
		}
		break;
	case WIDX_LAND:
		if ((RCT2_GLOBAL(0x009DE518, uint32) & (1 << 3)) && RCT2_GLOBAL(RCT2_ADDRESS_TOOL_WINDOWCLASS, uint8) == 1 && RCT2_GLOBAL(RCT2_ADDRESS_TOOL_WIDGETINDEX, uint16) == 7) {
			tool_cancel();
		} else {
			show_gridlines();
			tool_set(w, WIDX_LAND, 18);
			RCT2_GLOBAL(0x009DE518, uint32) |= (1 << 6);
			RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16) = 1;
			window_land_open();
		}
		break;
	case WIDX_CLEAR_SCENERY:
		if ((RCT2_GLOBAL(0x009DE518, uint32) & (1 << 3)) && RCT2_GLOBAL(RCT2_ADDRESS_TOOL_WINDOWCLASS, uint8) == 1 && RCT2_GLOBAL(RCT2_ADDRESS_TOOL_WIDGETINDEX, uint16) == 16) {
			tool_cancel();
		} else {
			show_gridlines();
			tool_set(w, WIDX_CLEAR_SCENERY, 12);
			RCT2_GLOBAL(0x009DE518, uint32) |= (1 << 6);
			RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16) = 2;
			window_clear_scenery_open();
		}
		break;
	case WIDX_WATER:
		if ((RCT2_GLOBAL(0x009DE518, uint32) & (1 << 3)) && RCT2_GLOBAL(RCT2_ADDRESS_TOOL_WINDOWCLASS, uint8) == 1 && RCT2_GLOBAL(RCT2_ADDRESS_TOOL_WIDGETINDEX, uint16) == 8) {
			tool_cancel();
		} else {
			show_gridlines();
			tool_set(w, WIDX_WATER, 19);
			RCT2_GLOBAL(0x009DE518, uint32) |= (1 << 6);
			RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16) = 1;
			window_water_open();
		}
		break;
	case WIDX_MAP:
		window_map_open();
		break;
	case WIDX_CONSTRUCT_RIDE:
		window_new_ride_open();
		break;
	}
}

/**
*
* rct2: 0x0066FADA
*/
static void window_editor_top_toolbar_resize() {
	rct_window *mainWindow = window_get_main();
	rct_window *w;

	window_get_register(w);

	int eax = 0;

	if ((w->disabled_widgets & 0xFF) == 0)
		eax |= (1 << 3);

	if ((w->disabled_widgets & 0xFF) == 3)
		eax |= (1 << 2);

	RCT2_CALLPROC_X(0x006ECE14, 0, 0, 0, 0, (int)w, 0, 0);
}

/**
*
*  rct2: 0x0066FA57
*/
static void window_editor_top_toolbar_mousedown(int widgetIndex, rct_window*w, rct_widget* widget)
{
	rct_viewport *mainViewport;

	if (widgetIndex == WIDX_FILE_MENU) {
		short dropdownItemCount = 8;
		gDropdownItemsFormat[0] = 884;
		gDropdownItemsFormat[1] = 885;
		gDropdownItemsFormat[2] = 0;
		gDropdownItemsFormat[3] = 847;
		gDropdownItemsFormat[4] = 2327;
		gDropdownItemsFormat[5] = 891;
		gDropdownItemsFormat[6] = 0;
		gDropdownItemsFormat[7] = 887;

		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & (SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER)) {
			dropdownItemCount = 5;
			gDropdownItemsFormat[0] = 847;
			gDropdownItemsFormat[1] = 2327;
			gDropdownItemsFormat[2] = 891;
			gDropdownItemsFormat[3] = 0;
			gDropdownItemsFormat[4] = 888;

			if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER) {
				gDropdownItemsFormat[4] = 889;
			}
		}

		window_dropdown_show_text(w->x + widget->left, w->y + widget->top,
			widget->bottom - widget->top + 1, w->colours[0] | 0x80, 0x80, dropdownItemCount);
	} else if (widgetIndex == WIDX_VIEW_MENU) {
		top_toolbar_init_view_menu(w, widget);
	}
}

/**
*
*  rct2: 0x0066FA38
*/
void window_editor_top_toolbar_dropdown() {
	short widgetIndex, dropdownIndex;
	rct_window* w;

	window_dropdown_get_registers(w, widgetIndex, dropdownIndex);

	if (widgetIndex == WIDX_FILE_MENU) {
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & (SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER)) {
			// TODO
		}
	} else if (widgetIndex == WIDX_VIEW_MENU) {
		top_toolbar_view_menu_dropdown(dropdownIndex);
	} else if (widgetIndex == WIDX_PATH) {
		// TODO
	}
}

/**
*
*  rct2: 0x0066F87D
*/
void window_editor_top_toolbar_invalidate()
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