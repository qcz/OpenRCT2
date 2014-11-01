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
#include "../game.h"
#include "../object.h"
#include "../sprites.h"
#include "../audio/audio.h"
#include "../localisation/string_ids.h"
#include "../localisation/format_codes.h"
#include "../interface/viewport.h"
#include "../interface/widget.h"
#include "../interface/window.h"

enum WINDOW_STAFF_LIST_WIDGET_IDX {
	WIDX_EDITOR_OBJECT_SELECTION_BACKGROUND,			// 0, 1
	WIDX_EDITOR_OBJECT_SELECTION_TITLE,					// 1, 2
	WIDX_EDITOR_OBJECT_SELECTION_CLOSE,					// 2, 4
	WIDX_EDITOR_OBJECT_SELECTION_TAB_CONTENT_PANEL,		// 3, 8
	WIDX_EDITOR_OBJECT_SELECTION_TAB_1,					// 4, 10
	WIDX_EDITOR_OBJECT_SELECTION_TAB_2,					// 5, 20
	WIDX_EDITOR_OBJECT_SELECTION_TAB_3,					// 6, 40
	WIDX_EDITOR_OBJECT_SELECTION_TAB_4,					// 7, 80
	WIDX_EDITOR_OBJECT_SELECTION_TAB_5,					// 8, 100
	WIDX_EDITOR_OBJECT_SELECTION_TAB_6,					// 9, 200
	WIDX_EDITOR_OBJECT_SELECTION_TAB_7,					// 10, 400
	WIDX_EDITOR_OBJECT_SELECTION_TAB_8,					// 11, 800
	WIDX_EDITOR_OBJECT_SELECTION_TAB_9,					// 12, 1000
	WIDX_EDITOR_OBJECT_SELECTION_TAB_10,				// 13, 2000
	WIDX_EDITOR_OBJECT_SELECTION_TAB_11,				// 14, 4000
	WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN1,				// 15, 8000
	WIDX_EDITOR_OBJECT_SELECTION_LIST,					// 16, 10000
	WIDX_EDITOR_OBJECT_SELECTION_FLATBTN,				// 17, 20000
	WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN2,				// 18, 40000
};

static rct_widget window_editor_object_selection_widgets[] = {
	{ WWT_FRAME,			0, 0, 599, 0, 399,		0xFFFFFFFF,	STR_NONE },		// 09ADB00
	{ WWT_CAPTION,			0, 1, 598, 1, 14,		3181,		829 },			// 09ADB10
	{ WWT_CLOSEBOX,			0, 587, 597, 2, 13,		824,		828 },			// 09ADB20
	{ WWT_RESIZE,			1, 0, 599, 43, 399,		0xFFFFFFFF, STR_NONE },		// 09ADB30
	{ WWT_TAB,				1, 3, 33, 17, 43,		0x20000000 | 5198, 1812 },	// 09ADB40
	{ WWT_TAB,				1, 34, 64, 17, 43,		0x20000000 | 5198, 1812 },	// 09ADB50
	{ WWT_TAB,				1, 65, 95, 17, 43,		0x20000000 | 5198, 1812 },	// 09ADB60
	{ WWT_TAB,				1, 96, 126, 17, 43,		0x20000000 | 5198, 1812 },	// 09ADB70
	{ WWT_TAB,				1, 127, 157, 17, 43,	0x20000000 | 5198, 1812 },	// 09ADB80
	{ WWT_TAB,				1, 158, 188, 17, 43,	0x20000000 | 5198, 1812 },	// 09ADB90
	{ WWT_TAB,				1, 189, 219, 17, 43,	0x20000000 | 5198, 1812 },	// 09ADBA0
	{ WWT_TAB,				1, 220, 250, 17, 43,	0x20000000 | 5198, 1812 },	// 09ADBB0
	{ WWT_TAB,				1, 251, 281, 17, 43,	0x20000000 | 5198, 1812 },	// 09ADBC0
	{ WWT_TAB,				1, 282, 312, 17, 43,	0x20000000 | 5198, 1812 },	// 09ADBD0
	{ WWT_TAB,				1, 313, 343, 17, 43,	0x20000000 | 5198, 1812 },	// 09ADBE0
	{ WWT_DROPDOWN_BUTTON,	0, 470, 591, 23, 34,	3364,		3365 },			// 09ADBF0
	{ WWT_SCROLL,			1, 4, 291, 46, 386,		2,			STR_NONE },		// 09ADC00
	{ WWT_FLATBTN,			1, 391, 504, 46, 159,	0xFFFFFFFF,	STR_NONE },		// 09ADC10
	{ WWT_DROPDOWN_BUTTON,	0, 384, 595, 24, 35,	3376,		3377 },			// 09ADC20
	{ WIDGETS_END },
};

static void window_editor_object_selection_emptysub() { }

static void window_editor_object_selection_close();
static void window_editor_object_selection_mouseup();
static void window_editor_object_selection_scrollgetsize();
static void window_editor_object_selection_scrollmousedown();
static void window_editor_object_selection_scrollmouseover();
static void window_editor_object_selection_tooltip();
static void window_editor_object_selection_invalidate();
static void window_editor_object_selection_paint();
static void window_editor_object_selection_scrollpaint();

static void* window_editor_object_selection_events[] = {
	window_editor_object_selection_close, //0x006ab199, // close
	window_editor_object_selection_mouseup, //0x006aafab, // mouseup
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
	window_editor_object_selection_scrollgetsize, //0x006ab031, // scrollgetsize
	window_editor_object_selection_scrollmousedown, //0x006ab0b6, // scrollmousedown
	window_editor_object_selection_emptysub,
	window_editor_object_selection_scrollmouseover, //0x006ab079, // scrollmouseover
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_tooltip, //0x006ab058, // tooltip
	window_editor_object_selection_emptysub,
	window_editor_object_selection_emptysub,
	window_editor_object_selection_invalidate, //0x006aa9fd, // invalidate
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

	RCT2_CALLPROC_EBPSAFE(0x006AB211);
	RCT2_CALLPROC_EBPSAFE(0x006AA770);

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
		window->enabled_widgets |= (1 << i);
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
*  rct2: 0x006ab199
*/
void window_editor_object_selection_close() {
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_EDITOR))
		return;

	RCT2_CALLPROC_EBPSAFE(0x006ABB66);
	RCT2_CALLPROC_EBPSAFE(0x006ABBBE);
	RCT2_CALLPROC_EBPSAFE(0x006A9FC0);
	object_free_scenario_text();
	RCT2_CALLPROC_EBPSAFE(0x006AB316);
	RCT2_CALLPROC_EBPSAFE(0x00685675);
	RCT2_CALLPROC_EBPSAFE(0x0068585B);
	window_new_ride_init_vars();
}

/**
*
*  rct2: 0x006aafab
*/
void window_editor_object_selection_mouseup() {
	short widgetIndex;
	rct_window *w;

	window_widget_get_registers(w, widgetIndex);

	switch (widgetIndex) {
	case WIDX_EDITOR_OBJECT_SELECTION_CLOSE:
		game_do_command(0, 1, 0, 0, GAME_COMMAND_LOAD_OR_QUIT, 1, 0);
		break;
	case WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN1:
		w->list_information_type ^= 1;
		window_invalidate(w);
		break;
	case WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN2:
		if (w->selected_list_item != -1) {
			w->selected_list_item = -1;
			object_free_scenario_text();
		}
		window_invalidate(w);

		int eax, ebx, ecx, edx, esi, edi, ebp;
		RCT2_CALLFUNC_X(0x00674FCE, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
		if (eax == 1) {
			window_close(w);
			RCT2_CALLPROC_EBPSAFE(0x006D386D);
		}
		break;
	}

	if (widgetIndex >= WIDX_EDITOR_OBJECT_SELECTION_TAB_1 &&
		widgetIndex <= WIDX_EDITOR_OBJECT_SELECTION_TAB_11 &&
		w->selected_tab != widgetIndex - WIDX_EDITOR_OBJECT_SELECTION_TAB_1) {
		w->selected_tab = widgetIndex - WIDX_EDITOR_OBJECT_SELECTION_TAB_1;
		w->selected_list_item = -1;
		w->var_494 = 0xFFFFFFFF;
		w->scrolls[0].v_top = 0;
		object_free_scenario_text();
		window_invalidate(w);
	}
}

/**
*
*  rct2: 0x006ab031
*/
void window_editor_object_selection_scrollgetsize() {
	rct_window *w;

	window_get_register(w);

	int scrollHeight = 0;

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) &
		(SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER)) {
		scrollHeight = RCT2_GLOBAL(0x00F43412, uint16) * 13;
	} else {
		scrollHeight = RCT2_ADDRESS(0x00F433E1, uint16)[w->selected_tab] * 13;
	}

#ifdef _MSC_VER
__asm mov edx, scrollHeight
#else
__asm__("mov edx, %[scrollHeight] " : [scrollHeight] "+m" (scrollHeight));
#endif
}

/**
*
*  rct2: 0x006AA703
*/
rct_object_entry* find_object_at_cursor(rct_window *w, int y, uint16* itemIndex, int* edi) {
	uint32 objectCount = RCT2_GLOBAL(0x00F42B6C, uint32);

	*edi = 0x9ADAEC;

	if (objectCount == 0) {
		// TODO
	} else {
		rct_object_entry* object = RCT2_GLOBAL(RCT2_ADDRESS_INSTALLED_OBJECT_LIST, rct_object_entry*);
		for (*itemIndex = 0; *itemIndex < objectCount; (*itemIndex)++, object = object_get_next(object), (*edi)++) {
			if ((object->flags & 0x0F) != w->selected_tab ||
				(RCT2_GLOBAL(*edi, uint8) & 0x20))
				continue;

			y -= 13;

			if (y < 0)
				return object;
		}

		*itemIndex = 0xFFFF;
	}

	return NULL;
}

/**
*
*  rct2: 0x006ab0b6
*/
void window_editor_object_selection_scrollmousedown() {
	short x, y;
	rct_window *w;

	window_scrollmouse_get_registers(w, x, y);

	sint16 itemIndex;
	int edi;
	rct_object_entry* obj = find_object_at_cursor(w, y, &itemIndex, &edi);

	if (itemIndex == -1)
		return;

	if (RCT2_GLOBAL(edi, uint8) & 0x20)
		return;

	window_invalidate(w);
	int ebp = RCT2_GLOBAL(0x0142406C, int);
	sound_play_panned(4, ebp, 0, 0, RCT2_GLOBAL(0x0142406C, int));

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) {
		int bx = 0;
		if (!(RCT2_GLOBAL(edi, uint8) & 1)) {
			bx |= 1;
		}
		bx |= 6;
		RCT2_GLOBAL(0xF43411, uint8) = 0;
	} else {

	}
}

/**
*
*  rct2: 0x006ab079
*/
void window_editor_object_selection_scrollmouseover() {
	short x, y;
	rct_window *w;

	window_scrollmouse_get_registers(w, x, y);

	sint16 itemIndex;
	int edi;
	rct_object_entry* obj = find_object_at_cursor(w, y, &itemIndex, &edi);
	
	if (RCT2_GLOBAL(edi, uint8) & 0x20) {
		itemIndex = -1;
	}
	
	if (itemIndex == w->selected_list_item)
		return;

	w->selected_list_item = itemIndex;
	w->var_494 = obj;
	object_free_scenario_text();
	if (itemIndex != -1) {
		object_get_scenario_text(obj);
	}
	window_invalidate(w);
}

/**
*
*  rct2: 0x006AB058
*/
void window_editor_object_selection_tooltip() {
	uint16 tooltipIndex;

#ifdef _MSC_VER
	__asm mov tooltipIndex, ax
#else
	__asm__("mov %[tooltipIndex], ax " : [tooltipIndex] "+m" (tooltipIndex));
#endif

	if (tooltipIndex >= WIDX_EDITOR_OBJECT_SELECTION_TAB_1 &&
		tooltipIndex <= WIDX_EDITOR_OBJECT_SELECTION_TAB_11) {
		RCT2_GLOBAL(0x013CE952, uint16) = tooltipIndex - 4 + 0xC70;
	} else {
		RCT2_GLOBAL(0x013CE952, uint16) = STR_LIST;
	}
}

/**
*
*  rct2: 0x006aa9fd
*/
void window_editor_object_selection_invalidate() {
	rct_window* w;

	window_get_register(w);

	int selectedTab = w->selected_tab;

	w->pressed_widgets |= (1 << WIDX_EDITOR_OBJECT_SELECTION_FLATBTN);
	uint32 pw = w->pressed_widgets & 0xFFFF800F;
	pw |= (1 << (selectedTab + 4));
	pw &= ~(1 << WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN1);
	if (w->list_information_type == 1)
		pw |= (1 << WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN1);
	w->pressed_widgets = pw;

	RCT2_GLOBAL(0x013CE952, uint16) = 0xC70 + selectedTab;

	window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN2].type = WWT_EMPTY;
	window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_TITLE].image = 0xC6D;
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER) {
		window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_TITLE].image = 0xD07;
	}
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) {
		window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_TITLE].image = 0xD08;
		window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN2].type = WWT_DROPDOWN_BUTTON;
	}
	
	window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_CLOSE].type = WWT_CLOSEBOX;
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) &
		(SCREEN_FLAGS_SCENARIO_EDITOR | SCREEN_FLAGS_TRACK_MANAGER)) {
		window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_CLOSE].type = WWT_EMPTY;
	}

	int left = 3;
	for (int i = WIDX_EDITOR_OBJECT_SELECTION_TAB_1; i <= WIDX_EDITOR_OBJECT_SELECTION_TAB_11; i++) {
		int tabIndex = i - WIDX_EDITOR_OBJECT_SELECTION_TAB_1;

		if (!w->list_information_type && ((tabIndex >= 1 && tabIndex <= 4) || tabIndex == 6)) {
			window_editor_object_selection_widgets[i].type = WWT_EMPTY;
		} else {
			window_editor_object_selection_widgets[i].type = WWT_TAB;
			window_editor_object_selection_widgets[i].left = left;
			window_editor_object_selection_widgets[i].right = left + 30;
			left += 31;
		}
	}

	window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN1].type = WWT_DROPDOWN_BUTTON;
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) &
		(SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER)) {
		
		for (int i = WIDX_EDITOR_OBJECT_SELECTION_TAB_2; i <= WIDX_EDITOR_OBJECT_SELECTION_TAB_11; i++)
			window_editor_object_selection_widgets[i].type = WWT_EMPTY;

		window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_DROPDOWN1].type = WWT_EMPTY;
	}

	window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_TAB_11].type = WWT_EMPTY;
	int dx = 300;
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) &
		(SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER))) {
		dx = 150;
	}

	window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_LIST].right = 587 - dx;
	window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_FLATBTN].left = -(dx / 2) + 537;
	window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_FLATBTN].right = -(dx / 2) + 537 + 113;
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

	for (int i = WIDX_EDITOR_OBJECT_SELECTION_TAB_1; i <= WIDX_EDITOR_OBJECT_SELECTION_TAB_11; i++) {
		rct_widget* currentWidget = &window_editor_object_selection_widgets[i];
		if (currentWidget->type == WWT_EMPTY)
			continue;

		int tabIndex = i - WIDX_EDITOR_OBJECT_SELECTION_TAB_1;
		
		gfx_draw_sprite(dpi, 0x1552 + tabIndex, currentWidget->left + w->x, currentWidget->top + w->y, 0);
	}
	
	int colour = RCT2_GLOBAL(0x0141FC44 + (w->colours[0] * 8), uint8);
	rct_widget* flatBtn = &window_editor_object_selection_widgets[WIDX_EDITOR_OBJECT_SELECTION_FLATBTN];
	gfx_fill_rect(dpi, flatBtn->left + w->x, flatBtn->top + w->y, flatBtn->right + w->x, flatBtn->bottom + w->y, colour);

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER)) {
		uint16 selectedOnTab = RCT2_ADDRESS(0x00F433F7, uint16)[w->selected_tab];
		RCT2_GLOBAL(0x013CE952, uint16) = selectedOnTab;

		uint16 maximumSelectableOnTab = object_entry_group_counts[w->selected_tab];
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER)
			maximumSelectableOnTab = 4;
		RCT2_GLOBAL(0x013CE952 + 2, uint16) = maximumSelectableOnTab;

		gfx_draw_string_left(dpi, 0xC5C, (void*)0x013CE952, 0, w->x + 3, w->y + w->height - 12);
	}

	if (w->selected_list_item == -1)
		return;
	if (RCT2_GLOBAL(0x009ADAF8, uint32) == 0xFFFFFFFF)
		return;

	/*int objectText = RCT2_GLOBAL(0x009ADAF8, uint32);
	rct_object_entry* obj = (rct_object_entry*)w->var_494;
	object_paint(obj->flags & 0x0F, 3, obj->flags & 0x0F,
		((flatBtn->left + flatBtn->right) / 2) + w->x,
		((flatBtn->top + flatBtn->bottom) / 2) + w->y, 
		(int)w, 0, RCT2_GLOBAL(0x009ADAF8, uint32));*/


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

	int esi = 0x9ADAEC;
	int dx = 0; // height
	uint32 objectCount = RCT2_GLOBAL(0x00F42B6C, uint32);
	rct_object_entry* object = RCT2_GLOBAL(RCT2_ADDRESS_INSTALLED_OBJECT_LIST, rct_object_entry*);
	for	(int itemIndex = 0;
		itemIndex < objectCount;
		itemIndex++, object = object_get_next(object), esi++)
	{
		if ((object->flags & 0x0F) != w->selected_tab)
		//	(RCT2_GLOBAL(esi, uint8) & 0x20))
			continue;

		if (dx + 13 < dpi->y || dpi->y + dpi->height < dx) {
			dx += 13;
			continue;
		}

		if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER)) {
			gfx_fill_rect_inset(dpi, 2, dx, 11, dx + 10,  w->colours[1], 0xE0);
		}

		int textFormat = FORMAT_BLACK;

		if (object == w->var_494) {
			gfx_fill_rect(dpi, 0, dx, w->width, dx + 11, 0x2000031);
			textFormat = FORMAT_WINDOW_COLOUR_2;
		}

		if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) &&
			(RCT2_GLOBAL(esi, uint8) & 1)) {

			RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 224;

			int colour = w->colours[1] & 0x7F;

			if (object->flags & 0x1C)
				colour |= 0x40;

			gfx_draw_string(dpi, (char*)0x009DED72, colour, 2, dx);
		}


		char *pos = (char*)object;
		// Skip sizeof(rct_object_entry)
		pos += 16;
		// Skip filename
		do {
			pos++;
		} while (*(pos - 1) != 0);

		pos += 4;
				
		uint8* text_buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, uint8);

		if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER)) {
			text_buffer[0] = textFormat;
			strncpy(text_buffer + 1, (char*)pos, 255);
		} else {
			// TODO
		}

		RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 0xE0;
		int left = 15;
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER)
			left = 0;
		
		gfx_draw_string(dpi, text_buffer, (w->colours[1] & 0x7F), left, dx);

		dx += 13;
	}
}