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

#include "../addresses.h"
#include "../localisation/string_ids.h"
#include "../sprites.h"
#include "../interface/viewport.h"
#include "../interface/widget.h"
#include "../interface/window.h"

enum WINDOW_STAFF_LIST_WIDGET_IDX {
	WIDX_EDITOR_OBJECT_SELECTION_BACKGROUND,			// 1
	WIDX_EDITOR_OBJECT_SELECTION_TITLE,					// 2
	WIDX_EDITOR_OBJECT_SELECTION_CLOSE,					// 4
	WIDX_EDITOR_OBJECT_SELECTION_TAB_CONTENT_PANEL,		// 8
	WIDX_EDITOR_OBJECT_SELECTION_TAB_1,					// 10
	WIDX_EDITOR_OBJECT_SELECTION_TAB_2,					// 20
	WIDX_EDITOR_OBJECT_SELECTION_TAB_3,					// 40
	WIDX_EDITOR_OBJECT_SELECTION_TAB_4,					// 80
	WIDX_EDITOR_OBJECT_SELECTION_TAB_5,					// 100
	WIDX_EDITOR_OBJECT_SELECTION_TAB_6,					// 200
	WIDX_EDITOR_OBJECT_SELECTION_TAB_7,					// 400
	WIDX_EDITOR_OBJECT_SELECTION_TAB_8,					// 800
	WIDX_EDITOR_OBJECT_SELECTION_TAB_9,					// 1000
	WIDX_EDITOR_OBJECT_SELECTION_TAB_10,				// 2000
	WIDX_EDITOR_OBJECT_SELECTION_TAB_11,				// 4000
	WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN1,				// 8000
	WIDX_EDITOR_OBJECT_SELECTION_LIST,					// 10000
	WIDX_EDITOR_OBJECT_SELECTION_FLATBTN,				// 20000
	WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN2,				// 40000
};

static rct_widget window_editor_object_selection_widgets[] = {
	{ WWT_FRAME,			0, 0, 599, 0, 399,		0xFFFFFFFF,	STR_NONE },
	{ WWT_CAPTION,			0, 1, 598, 1, 14,		3181,		829 },
	{ WWT_CLOSEBOX,			0, 587, 597, 2, 13,		824,		828 },
	{ WWT_RESIZE,			1, 0, 599, 43, 399,		0xFFFFFFFF, STR_NONE },
	{ WWT_TAB,				1, 3, 33, 17, 43,		0x20000000 | 5198, 1812 },
	{ WWT_TAB,				1, 34, 64, 17, 43,		0x20000000 | 5198, 1812 },
	{ WWT_TAB,				1, 65, 95, 17, 43,		0x20000000 | 5198, 1812 },
	{ WWT_TAB,				1, 96, 126, 17, 43,		0x20000000 | 5198, 1812 },
	{ WWT_TAB,				1, 127, 157, 17, 43,	0x20000000 | 5198, 1812 },
	{ WWT_TAB,				1, 158, 188, 17, 43,	0x20000000 | 5198, 1812 },
	{ WWT_TAB,				1, 189, 219, 17, 43,	0x20000000 | 5198, 1812 },
	{ WWT_TAB,				1, 220, 250, 17, 43,	0x20000000 | 5198, 1812 },
	{ WWT_TAB,				1, 251, 281, 17, 43,	0x20000000 | 5198, 1812 },
	{ WWT_TAB,				1, 282, 312, 17, 43,	0x20000000 | 5198, 1812 },
	{ WWT_TAB,				1, 313, 343, 17, 43,	0x20000000 | 5198, 1812 },
	{ WWT_DROPDOWN_BUTTON,	0, 470, 591, 23, 34,	3364,		3365 },
	{ WWT_SCROLL,			1, 4, 291, 46, 386,		2,			STR_NONE },
	{ WWT_FLATBTN,			1, 391, 504, 46, 159,	0xFFFFFFFF,	STR_NONE },
	{ WWT_DROPDOWN_BUTTON,	0, 384, 595, 24, 35,	3376,		3377 },
	{ WIDGETS_END },
};

static void window_editor_object_selection_emptysub() { }

static void window_editor_object_selection_paint();
static void window_editor_object_selection_scrollpaint();

static void* window_editor_object_selection_events[] = {
	0x006ab199, // close
	0x006aafab, // mouseup
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	0x006ab031, // scrollgetsize
	0x006ab0b6, // scrollmousedown
	window_editor_object_selection_emptysub,
	0x006ab079, // scrollmouseup
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	0x006ab058, // tooltip
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	0x006aa9fd, // invalidate
	window_editor_object_selection_paint, //0x006aab56, // paint
	window_editor_object_selection_scrollpaint, //0x006aada3, // scrollpaint
};

/**
*
*  rct2: 0x006AA64E
*/
void window_editor_object_selection_open() {
	rct_window* window;

	window = window_bring_to_front_by_class(WC_EDITOR_OBJECT_SELECTION);
	if (window != NULL)
		return;

	int top = max(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16) / 2 - 200, 28);
	int left = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16) / 2 - 300;

	window = window_create(top, left, 600, 400, (uint32*)window_editor_object_selection_events,
		WC_EDITOR_OBJECT_SELECTION, WF_STICK_TO_FRONT);
	window->widgets = window_editor_object_selection_widgets;

	window->enabled_widgets =
		(1 << WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN1) |
		(1 << WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN2) |
		(1 << WIDX_EDITOR_OBJECT_SELECTION_CLOSE);

	for (int i = WIDX_EDITOR_OBJECT_SELECTION_TAB_1; i <= WIDX_EDITOR_OBJECT_SELECTION_TAB_11; i++)
		window->enabled_widgets |= i;

	window_init_scroll_widgets(window);

	window->var_4AE = 0;
	window->selected_tab = 0;
	window->selected_list_item = -1;
	window->var_494 = 0xFFFFFFFF;
	window->colours[0] = 4;
	window->colours[1] = 1;
	window->colours[2] = 1;
}

/**
*
*  rct2: 0x006aab56
*/
void window_editor_object_selection_paint() {
	rct_window *w;
	rct_drawpixelinfo *dpi;

	window_paint_get_registers(w, dpi);

	window_draw_widgets(w, dpi);
	// TODO
}

/**
*
*  rct2: 0x006aada3
*/
void window_editor_object_selection_scrollpaint()
{
	rct_window *w;
	rct_drawpixelinfo *dpi;

	window_paint_get_registers(w, dpi);

	gfx_clear(dpi, ((char*)0x0141FC48)[w->colours[1] * 8] * 0x1010101);
	// TODO
}