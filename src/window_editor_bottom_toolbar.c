/*****************************************************************************
* Copyright (c) 2014 D�niel Tar
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

#include <stdbool.h>
#include "addresses.h"
#include "editor.h"
#include "sprites.h"
#include "string_ids.h"
#include "viewport.h"
#include "widget.h"
#include "window.h"

enum WINDOW_EDITOR_TOP_TOOLBAR_WIDGET_IDX {
	WIDX_PREVIOUS_IMAGE,		// 1
	WIDX_PREVIOUS_STEP_BUTTON,	// 2
	WIDX_NEXT_IMAGE,			// 4
	WIDX_NEXT_STEP_BUTTON,		// 8
};

static rct_widget window_editor_bottom_toolbar_widgets[] = {
	{ WWT_IMGBTN, 0, 0, 199, 0, 33, 0xFFFFFFFF, 0xFFFF },			// 1		0x9A9958
	{ WWT_FLATBTN, 0, 2, 197, 2, 31, 0xFFFFFFFF, 0xFFFF },			// 2		0x9A9968
	{ WWT_IMGBTN, 0, 440, 639, 0, 33, 0xFFFFFFFF, 0xFFFF },			// 4		0x9A9978
	{ WWT_FLATBTN, 0, 442, 637, 2, 31, 0xFFFFFFFF, 0xFFFF },		// 8		0x9A9988
	{ WIDGETS_END },
};

static void window_editor_bottom_toolbar_emptysub() { }

static void window_editor_bottom_toolbar_invalidate();
static void window_editor_bottom_toolbar_paint();

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
	window_editor_bottom_toolbar_invalidate, //0x0066f1c9,
	window_editor_bottom_toolbar_paint, //0x0066f25c,
	window_editor_bottom_toolbar_emptysub
};

/**
* Creates the main editor top toolbar window.
* rct2: 0x0066F052 (part of 0x0066EF38)
*/
void window_editor_bottom_toolbar_open()
{
	rct_window* window;

	window = window_create(0, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, sint16) - 32,
		RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16), 32,
		(uint32*)window_editor_bottom_toolbar_events,
		WC_BOTTOM_TOOLBAR, WF_STICK_TO_FRONT | WF_TRANSPARENT | WF_5);
	window->widgets = window_editor_bottom_toolbar_widgets;

	window->enabled_widgets |=
		(1 << WIDX_PREVIOUS_STEP_BUTTON) |
		(1 << WIDX_NEXT_STEP_BUTTON) |
		(1 << WIDX_PREVIOUS_IMAGE) |
		(1 << WIDX_NEXT_IMAGE);

	window_init_scroll_widgets(window);
	window->colours[0] = 150;
	window->colours[1] = 150;
	window->colours[2] = 141;

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & (SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER)) {
		window->colours[0] = 135;
		window->colours[1] = 135;
		window->colours[2] = 135;
	}
}

void hide_previous_step_button() {
	window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_STEP_BUTTON].type = WWT_EMPTY;
	window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].type = WWT_EMPTY;
}

void hide_next_step_button() {
	window_editor_bottom_toolbar_widgets[WIDX_NEXT_STEP_BUTTON].type = WWT_EMPTY;
	window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].type = WWT_EMPTY;
}

/**
*
*  rct2: 0x0066F1C9
*/
void window_editor_bottom_toolbar_invalidate() {
	rct_window* w;

	window_get_register(w);

	window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_STEP_BUTTON].type = WWT_FLATBTN;
	window_editor_bottom_toolbar_widgets[WIDX_NEXT_STEP_BUTTON].type = WWT_FLATBTN;
	window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].type = WWT_IMGBTN;
	window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].type = WWT_IMGBTN;

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) {
		hide_previous_step_button();
		hide_next_step_button();
	} else {
		if (g_editor_step == EDITOR_STEP_OBJECT_SELECTION) {
			hide_previous_step_button();
		} else if (g_editor_step == EDITOR_STEP_ROLLERCOASTER_DESIGNER) {
			hide_next_step_button();
		} else if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER)) {
			if (RCT2_GLOBAL(0x13573C8, uint16) != 0x2710 || RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY_SCENARIO) {
				hide_previous_step_button();
			}
		}
	}
}

/**
*
*  rct2: 0x0066F25C
*/
void window_editor_bottom_toolbar_paint() {
	rct_window *w;
	rct_drawpixelinfo *dpi;

	window_paint_get_registers(w, dpi);

	bool drawPreviousButton = false;
	bool drawNextButton = false;

	if (g_editor_step == EDITOR_STEP_OBJECT_SELECTION) {
		drawNextButton = true;
	} else if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER) {
		drawPreviousButton = true;
	} else if (RCT2_GLOBAL(0x13573C8, uint16) != 0x2710) {
		drawNextButton = true;
	} else if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY_SCENARIO) {
		drawNextButton = true;
	} else {
		drawPreviousButton = true;
	}

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER)) {
		if (drawPreviousButton) {
			gfx_fill_rect(dpi,
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].left + w->x,
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].top + w->y,
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].right + w->x,
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].bottom + w->y, 0x2000033);
		}

		if ((drawPreviousButton || drawNextButton) && g_editor_step != EDITOR_STEP_ROLLERCOASTER_DESIGNER) {
			gfx_fill_rect(dpi,
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].left + w->x,
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].top + w->y,
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].right + w->x,
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].bottom + w->y, 0x2000033);
		}
	}

	window_draw_widgets(w, dpi);

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER)) {
		

		if (drawPreviousButton) {
			gfx_fill_rect_inset(dpi,
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].left + 1 + w->x,
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].top + 1 + w->y,
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].right - 1 + w->x,
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].bottom - 1 + w->y,
				w->colours[1], 0x30);
		}

		if ((drawPreviousButton || drawNextButton) && g_editor_step != EDITOR_STEP_ROLLERCOASTER_DESIGNER) {
			gfx_fill_rect_inset(dpi,
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].left + 1 + w->x,
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].top + 1 + w->y,
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].right - 1 + w->x,
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].bottom - 1 + w->y,
				w->colours[1], 0x30);
		}

		short stateX =
			(window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].right +
			window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].left) / 2 + w->x;
		short stateY = w->height - 0x0C + w->y;
		gfx_draw_string_centred(dpi, STR_OBJECT_SELECTION_STEP + g_editor_step,
			stateX, stateY, (w->colours[2] & 0x7F) | 0x20, 0);

		if (drawPreviousButton) {
			gfx_draw_sprite(dpi, SPR_PREVIOUS,
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].left + 6 + w->x,
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].top + 6 + w->y, 0);

			int textColour = w->colours[1] & 0x7F;

			if (RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_OVER_WINDOWCLASS, uint8) == WC_BOTTOM_TOOLBAR &&
				RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_OVER_WIDGETINDEX, uint8) == WIDX_PREVIOUS_STEP_BUTTON)
				textColour = 2;

			short textX = (window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].left + 30 +
				window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].right) / 2 + w->x;
			short textY = window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].top + 6 + w->y;

			short stringId = STR_OBJECT_SELECTION_STEP + g_editor_step - 1;
			if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER)
				stringId = STR_OBJECT_SELECTION_STEP;

			gfx_draw_string_centred(dpi, STR_BACK_TO_PREVIOUS_STEP, textX, textY, textColour, 0);
			gfx_draw_string_centred(dpi, stringId, textX, textY + 10, textColour, 0);
		}

		if ((drawPreviousButton || drawNextButton) && g_editor_step != EDITOR_STEP_ROLLERCOASTER_DESIGNER) {
			gfx_draw_sprite(dpi, SPR_NEXT,
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].right - 29 + w->x,
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].top + 6 + w->y, 0);

			int textColour = w->colours[1] & 0x7F;

			if (RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_OVER_WINDOWCLASS, uint8) == WC_BOTTOM_TOOLBAR &&
				RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_OVER_WIDGETINDEX, uint8) == WIDX_NEXT_STEP_BUTTON)
				textColour = 2;

			short textX = (window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].left +
				window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].right - 30) / 2 + w->x;
			short textY = window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].top + 6 + w->y;
			
			short stringId = STR_OBJECT_SELECTION_STEP + g_editor_step + 1;
			if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER)
				stringId = STR_ROLLERCOASTER_DESIGNER_STEP;

			gfx_draw_string_centred(dpi, STR_FORWARD_TO_NEXT_STEP, textX, textY, textColour, 0);
			gfx_draw_string_centred(dpi, stringId, textX, textY + 10, textColour, 0);

		}
	}
}