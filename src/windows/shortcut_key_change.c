/*****************************************************************************
* Copyright (c) 2014 Ted John, Duncan Frost
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

#include "../addresses.h"
#include "../config.h"
#include "../interface/window.h"
#include "../interface/widget.h"
#include "../localisation/localisation.h"

#define WW 250
#define WH 60

enum WINDOW_SHORTCUT_CHANGE_WIDGET_IDX {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
};

// 0x9DE4E0
static rct_widget window_shortcut_change_widgets[] = {
	{ WWT_FRAME,			0,	0,			WW - 1,	0,		WH - 1,		STR_NONE,		STR_NONE },
	{ WWT_CAPTION,			0,	1,			WW - 2,	1,		14,			STR_OPTIONS,	STR_WINDOW_TITLE_TIP },
	{ WWT_CLOSEBOX,			0,	WW-13,		WW - 3,	2,		13,			STR_CLOSE_X,	STR_CLOSE_WINDOW_TIP },
	{ WIDGETS_END }
};

static void window_shortcut_change_emptysub(){}
static void window_shortcut_change_mouseup();
static void window_shortcut_change_paint();

//0x9A3F7C
static void* window_shortcut_change_events[] = {
	window_shortcut_change_emptysub,
	window_shortcut_change_mouseup,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_emptysub,
	window_shortcut_change_paint,
	window_shortcut_change_emptysub
};

void window_shortcut_change_open(int selected_key){
	// Move this to window_shortcut_change_open
	window_close_by_class(WC_CHANGE_KEYBOARD_SHORTCUT);
	// Save the item we are selecting for new window
	RCT2_GLOBAL(0x9DE511, uint8) = selected_key;
	rct_window* w = window_create_auto_pos(WW, WH, (uint32*)window_shortcut_change_events, WC_CHANGE_KEYBOARD_SHORTCUT, 0);

	w->widgets = window_shortcut_change_widgets;
	w->enabled_widgets = (1 << 2);
	window_init_scroll_widgets(w);
	w->colours[0] = 7;
	w->colours[1] = 7;
	w->colours[2] = 7;
}

/**
*
*  rct2: 0x006E3AE0
*/
static void window_shortcut_change_mouseup(){
	short widgetIndex;
	rct_window *w;

	window_widget_get_registers(w, widgetIndex);

	switch (widgetIndex){
	case WIDX_CLOSE:
		window_close(w);
	}
}

/**
*
*  rct2: 0x006E3A9F
*/
static void window_shortcut_change_paint(){
	rct_window *w;
	rct_drawpixelinfo *dpi;

	window_paint_get_registers(w, dpi);

	window_draw_widgets(w, dpi);

	int x = w->x + 125;
	int y = w->y + 30;

	RCT2_GLOBAL(0x13CE952, uint16) = 2493 + RCT2_GLOBAL(0x9DE511, uint8);
	gfx_draw_string_centred_wrapped(dpi, (void*)0x13CE952, x, y, 242, 2785, RCT2_GLOBAL(0x9DEB8D, uint8));
}