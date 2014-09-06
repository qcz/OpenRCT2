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
#include "viewport.h"
#include "widget.h"
#include "window.h"

enum WINDOW_EDITOR_TOP_TOOLBAR_WIDGET_IDX {
	WIDX_IMGBUTTON1,		// 1
	WIDX_FLATBUTTON1,		// 2
	WIDX_IMGBUTTON2,		// 4
	WIDX_FLATBUTTON2,		// 8
};

static rct_widget window_editor_bottom_toolbar_widgets[] = {
	{ WWT_IMGBTN, 0, 0, 199, 0, 33, 0xFFFFFFFF, 0xFFFF },
	{ WWT_FLATBTN, 0, 2, 197, 2, 31, 0xFFFFFFFF, 0xFFFF },
	{ WWT_IMGBTN, 0, 440, 639, 0, 33, 0xFFFFFFFF, 0xFFFF },
	{ WWT_FLATBTN, 0, 442, 637, 2, 31, 0xFFFFFFFF, 0xFFFF },
	{ WIDGETS_END },
};

static void window_editor_bottom_toolbar_emptysub() { }

static void* window_editor_bottom_toolbar_events[] = {
	window_editor_bottom_toolbar_emptysub,
	0x0066f5ae,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	window_editor_bottom_toolbar_emptysub,
	0x0066f1c9,
	0x0066f25c,
	window_editor_bottom_toolbar_emptysub
};

void window_editor_bottom_toolbar_open()
{
	rct_window* window;

	window = window_create(0, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16) - 32,
		RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16), 32,
		(uint32*)window_editor_bottom_toolbar_events,
		WC_BOTTOM_TOOLBAR, WF_STICK_TO_FRONT | WF_TRANSPARENT | WF_5);
	window->widgets = window_editor_bottom_toolbar_widgets;

	window->enabled_widgets |=
		(1 << WIDX_FLATBUTTON1) |
		(1 << WIDX_FLATBUTTON2) |
		(1 << WIDX_IMGBUTTON1) |
		(1 << WIDX_IMGBUTTON2);

	window_init_scroll_widgets(window);
	window->colours[0] = 7;
	window->colours[1] = 12;
	window->colours[2] = 24;
	window->colours[3] = 1;
}