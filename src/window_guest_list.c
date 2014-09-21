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
#include <assert.h>
#include "addresses.h"
#include "game.h"
#include "peep.h"
#include "string_ids.h"
#include "sprite.h"
#include "sprites.h"
#include "ride.h"
#include "widget.h"
#include "window.h"
#include "window_dropdown.h"

enum {
	PAGE_INDIVIDUAL,
	PAGE_SUMMARISED
};

enum WINDOW_GUEST_LIST_WIDGET_IDX {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_TAB_CONTENT_PANEL,
	WIDX_PAGE_DROPDOWN,
	WIDX_PAGE_DROPDOWN_BUTTON,
	WIDX_INFO_TYPE_DROPDOWN,
	WIDX_INFO_TYPE_DROPDOWN_BUTTON,
	WIDX_MAP,
	WIDX_TAB_1,
	WIDX_TAB_2,
	WIDX_GUEST_LIST
};

enum {
	VIEW_ACTIONS,
	VIEW_THOUGHTS
};

static rct_widget window_guest_list_widgets[] = {
	{ WWT_FRAME,			0,	0,		349,	0,	329,	0x0FFFFFFFF,	STR_NONE },						// panel / background
	{ WWT_CAPTION,			0,	1,		348,	1,	14,		STR_GUESTS,		STR_WINDOW_TITLE_TIP },			// title bar
	{ WWT_CLOSEBOX,			0,	337,	347,	2,	13,		STR_CLOSE_X,	STR_CLOSE_WINDOW_TIP },			// close x button
	{ WWT_RESIZE,			1,	0,		349,	43,	329,	0x0FFFFFFFF,	STR_NONE },						// tab content panel
	{ WWT_DROPDOWN,			1,	5,		84,		59,	70,		STR_PAGE_1,		STR_NONE },						// page dropdown
	{ WWT_DROPDOWN_BUTTON,	1,	73,		83,		60,	69, 	876,			STR_NONE },						// page dropdown button
	{ WWT_DROPDOWN,			1,	126,	301,	59,	70, 	0x0FFFFFFFF,	STR_INFORMATION_TYPE_TIP },		// information type dropdown
	{ WWT_DROPDOWN_BUTTON,	1,	290,	300,	60,	69, 	876,			STR_INFORMATION_TYPE_TIP },		// information type dropdown button
	{ WWT_FLATBTN,			1,	321,	344,	46,	69, 	5192,			STR_SHOW_GUESTS_ON_MAP_TIP },	// map
	{ WWT_TAB,				1,	3,		33,		17,	43, 	0x02000144E,	STR_INDIVIDUAL_GUESTS_TIP },	// tab 1
	{ WWT_TAB,				1,	34,		64,		17,	43, 	0x02000144E,	STR_SUMMARISED_GUESTS_TIP },	// tab 2
	{ WWT_SCROLL,			1,	3,		346,	72,	326,	3,				STR_NONE },						// guest list
	{ WIDGETS_END },
};

static void window_guest_list_emptysub() { }
static void window_guest_list_mouseup();
static void window_guest_list_resize();
static void window_guest_list_mousedown(int widgetIndex, rct_window*w, rct_widget* widget);
static void window_guest_list_dropdown();
static void window_guest_list_update(rct_window *w);
static void window_guest_list_scrollgetsize();
static void window_guest_list_scrollmousedown();
static void window_guest_list_scrollmouseover();
static void window_guest_list_tooltip();
static void window_guest_list_invalidate();
static void window_guest_list_paint();
static void window_guest_list_scrollpaint();

static void* window_guest_list_events[] = {
	window_guest_list_emptysub,
	window_guest_list_mouseup,
	window_guest_list_resize,
	window_guest_list_mousedown,
	window_guest_list_dropdown,
	window_guest_list_emptysub,
	window_guest_list_update,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_scrollgetsize,
	window_guest_list_scrollmousedown,
	window_guest_list_emptysub,
	window_guest_list_scrollmouseover,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_tooltip,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_invalidate,
	window_guest_list_paint,
	window_guest_list_scrollpaint
};

static int _window_guest_list_highlighted_index;
static int _window_guest_list_selected_tab;
static int _window_guest_list_selected_filter;
static int _window_guest_list_selected_page;
static int _window_guest_list_selected_view;
static int _window_guest_list_num_pages;
static int _window_guest_list_num_groups;

static uint16 _window_guest_list_groups_num_guests[240];
static uint32 _window_guest_list_groups_argument_1[240];
static uint32 _window_guest_list_groups_argument_2[240];
static uint8 _window_guest_list_groups_guest_faces[240 * 58];

static int window_guest_list_is_peep_in_filter(rct_peep* peep);
static void window_guest_list_find_groups();

static void get_arguments_from_peep(rct_peep *peep, uint32 *argument_1, uint32* argument_2);

/**
 * 
 *  rct2: 0x006992E3
 */
void window_guest_list_open()
{
	rct_window* window;

	// Check if window is already open
	window = window_bring_to_front_by_id(WC_GUEST_LIST, 0);
	if (window != NULL)
		return;

	window = window_create_auto_pos(350, 330, (uint32*)window_guest_list_events, WC_GUEST_LIST, 0x0400);
	window->widgets = window_guest_list_widgets;
	window->enabled_widgets =
		(1 << WIDX_CLOSE) |
		(1 << WIDX_PAGE_DROPDOWN) |
		(1 << WIDX_PAGE_DROPDOWN_BUTTON) |
		(1 << WIDX_INFO_TYPE_DROPDOWN) |
		(1 << WIDX_INFO_TYPE_DROPDOWN_BUTTON) |
		(1 << WIDX_MAP) |
		(1 << WIDX_TAB_1) |
		(1 << WIDX_TAB_2);

	window_init_scroll_widgets(window);
	_window_guest_list_highlighted_index = -1;
	window->list_information_type = 0;
	_window_guest_list_selected_tab = PAGE_INDIVIDUAL;
	_window_guest_list_selected_filter = -1;
	_window_guest_list_selected_page = 0;
	_window_guest_list_num_pages = 1;
	window_guest_list_widgets[WIDX_PAGE_DROPDOWN].type = WWT_EMPTY;
	window_guest_list_widgets[WIDX_PAGE_DROPDOWN_BUTTON].type = WWT_EMPTY;
	window->var_492 = 0;
	window->min_width = 350;
	window->min_height = 330;
	window->max_width = 500;
	window->max_height = 450;
	window->flags |= WF_RESIZABLE;
	window->colours[0] = 1;
	window->colours[1] = 15;
	window->colours[2] = 15;
}

/**
 * 
 *  rct2: 0x00699AAF
 */
static void window_guest_list_mouseup()
{
	short widgetIndex;
	rct_window *w;

	window_widget_get_registers(w, widgetIndex);

	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	case WIDX_MAP:
		window_map_open();
		break;
	}
}

/**
 * 
 *  rct2: 0x00699EA3
 */
static void window_guest_list_resize()
{
	rct_window *w;

	window_get_register(w);

	w->min_width = 350;
	w->min_height = 330;
	if (w->width < w->min_width) {
		window_invalidate(w);
		w->width = w->min_width;
	}
	if (w->height < w->min_height) {
		window_invalidate(w);
		w->height = w->min_height;
	}
}

/**
 * 
 *  rct2: 0x00699AC4
 */
static void window_guest_list_mousedown(int widgetIndex, rct_window*w, rct_widget* widget)
{
	int i;
	switch (widgetIndex) {
	case WIDX_TAB_1:
	case WIDX_TAB_2:
		if (_window_guest_list_selected_filter == -1)
			if (_window_guest_list_selected_tab == widgetIndex - WIDX_TAB_1)
				break;
		_window_guest_list_selected_tab = widgetIndex - WIDX_TAB_1;
		_window_guest_list_selected_page = 0;
		_window_guest_list_num_pages = 1;
		window_guest_list_widgets[WIDX_PAGE_DROPDOWN].type = WWT_EMPTY;
		window_guest_list_widgets[WIDX_PAGE_DROPDOWN_BUTTON].type = WWT_EMPTY;
		w->list_information_type = 0;
		_window_guest_list_selected_filter = -1;
		window_invalidate(w);
		w->scrolls[0].v_top = 0;
		break;
	case WIDX_PAGE_DROPDOWN_BUTTON:
		widget = &w->widgets[widgetIndex - 1];

		window_dropdown_show_text_custom_width(
			w->x + widget->left,
			w->y + widget->top,
			widget->bottom - widget->top + 1,
			w->colours[1],
			0x80,
			_window_guest_list_num_pages,
			widget->right - widget->left - 3
		);

		for (i = 0; i < 2; i++) {
			gDropdownItemsFormat[i] = 1142;
			gDropdownItemsArgs[i] = STR_PAGE_1 + i;
		}
		gDropdownItemsChecked = (1 << _window_guest_list_selected_view);
		break;
	case WIDX_INFO_TYPE_DROPDOWN_BUTTON:
		widget = &w->widgets[widgetIndex - 1];

		for (i = 0; i < 2; i++) {
			gDropdownItemsFormat[i] = 1142;
			gDropdownItemsArgs[i] = STR_ACTIONS + i;
		}

		window_dropdown_show_text_custom_width(
			w->x + widget->left,
			w->y + widget->top,
			widget->bottom - widget->top + 1,
			w->colours[1],
			0x80,
			2,
			widget->right - widget->left - 3
		);

		gDropdownItemsChecked = (1 << _window_guest_list_selected_view);
		break;
	}
}

/**
 * 
 *  rct2: 0x00699AE1
 */
static void window_guest_list_dropdown()
{
	short dropdownIndex, widgetIndex;
	rct_window *w;

	window_dropdown_get_registers(w, widgetIndex, dropdownIndex);

	switch (widgetIndex) {
	case WIDX_PAGE_DROPDOWN_BUTTON:
		if (dropdownIndex == -1)
			break;
		_window_guest_list_selected_page = dropdownIndex;
		window_invalidate(w);
		break;
	case WIDX_INFO_TYPE_DROPDOWN_BUTTON:
		if (dropdownIndex == -1)
			break;
		_window_guest_list_selected_view = dropdownIndex;
		window_invalidate(w);
		break;
	}
}

/**
 * 
 *  rct2: 0x00699E54
 */
static void window_guest_list_update(rct_window *w)
{
	if (RCT2_GLOBAL(0x00F1AF20, uint16) != 0)
		RCT2_GLOBAL(0x00F1AF20, uint16)--;
	w->list_information_type++;
	if (w->list_information_type >= (_window_guest_list_selected_tab == PAGE_INDIVIDUAL ? 24 : 32))
		w->list_information_type = 0;
	widget_invalidate(WC_GUEST_LIST, 0, WIDX_TAB_1 + _window_guest_list_selected_tab);
}

/**
 * 
 *  rct2: 0x00699C55
 */
static void window_guest_list_scrollgetsize()
{
	int i, y, numGuests, spriteIndex;
	rct_window *w;
	rct_peep *peep;

	window_get_register(w);

	switch (_window_guest_list_selected_tab) {
	case PAGE_INDIVIDUAL:
		// Count the number of guests
		numGuests = 0;

		FOR_ALL_GUESTS(spriteIndex, peep) {
			if (peep->var_2A != 0)
				continue;
			if (_window_guest_list_selected_filter != -1)
				if (window_guest_list_is_peep_in_filter(peep))
					continue;
			numGuests++;
		}
		w->var_492 = numGuests;
		y = numGuests * 10;
		break;
	case PAGE_SUMMARISED:
		// Find the groups
		window_guest_list_find_groups();
		w->var_492 = _window_guest_list_num_groups;
		y = _window_guest_list_num_groups * 21;
		break;
	}

	RCT2_GLOBAL(0x00F1EE09, uint32) = numGuests;
	i = _window_guest_list_selected_page;
	for (i = _window_guest_list_selected_page - 1; i >= 0; i--)
		y -= 0x7BF2;
	if (y < 0)
		y = 0;
	if (y > 0x7BF2)
		y = 0x7BF2;
	if (_window_guest_list_highlighted_index != -1) {
		_window_guest_list_highlighted_index = -1;
		window_invalidate(w);
	}

	i = y - window_guest_list_widgets[WIDX_GUEST_LIST].bottom + window_guest_list_widgets[WIDX_GUEST_LIST].top + 21;
	if (i < 0)
		i = 0;
	if (i < w->scrolls[0].v_top) {
		w->scrolls[0].v_top = i;
		window_invalidate(w);
	}

	#ifdef _MSC_VER
	__asm mov ecx, 447
	#else
	__asm__ ( "mov ecx, 447 "  );
	#endif

	#ifdef _MSC_VER
	__asm mov edx, y
	#else
	__asm__ ( "mov edx, %[y] " : [y] "+m" (y) );
	#endif

}

/**
 * 
 *  rct2: 0x00699D7D
 */
static void window_guest_list_scrollmousedown()
{
	int i, spriteIndex;
	short x, y;
	rct_window *w;
	rct_peep *peep;

	window_scrollmouse_get_registers(w, x, y);

	switch (_window_guest_list_selected_tab) {
	case PAGE_INDIVIDUAL:
		i = y / 10;
		i += _window_guest_list_selected_page * 3173;
		FOR_ALL_GUESTS(spriteIndex, peep) {
			if (peep->var_2A != 0)
				continue;
			if (_window_guest_list_selected_filter != -1)
				if (window_guest_list_is_peep_in_filter(peep))
					continue;

			if (i == 0) {
				// Open guest window
				window_peep_open(peep);
				
				break;
			} else {
				i--;
			}
		}
		break;
	case PAGE_SUMMARISED:
		i = y / 21;
		if (i < _window_guest_list_num_groups) {
			RCT2_GLOBAL(0x00F1EDF6, uint32) = _window_guest_list_groups_argument_1[i];
			RCT2_GLOBAL(0x00F1EDFA, uint32) = _window_guest_list_groups_argument_2[i];
			_window_guest_list_selected_filter = _window_guest_list_selected_view;
			_window_guest_list_selected_tab = PAGE_INDIVIDUAL;
			window_invalidate(w);
			w->scrolls[0].v_top = 0;
		}
		break;
	}
}

/**
 * 
 *  rct2: 0x00699D3B
 */
static void window_guest_list_scrollmouseover()
{
	int i;
	short x, y;
	rct_window *w;

	window_scrollmouse_get_registers(w, x, y);

	i = y / (_window_guest_list_selected_tab == PAGE_INDIVIDUAL ? 10 : 21);
	i += _window_guest_list_selected_page * 3173;
	if (i != _window_guest_list_highlighted_index) {
		_window_guest_list_highlighted_index = i;
		window_invalidate(w);
	}
}

/**
 * 
 *  rct2: 0x00699E4A
 */
static void window_guest_list_tooltip()
{
	RCT2_GLOBAL(0x013CE952, uint16) = STR_LIST;
}

/**
 * 
 *  rct2: 0x00699511
 */
static void window_guest_list_invalidate()
{
	rct_window *w;

	window_get_register(w);

	w->pressed_widgets &= ~(1 << WIDX_TAB_1);
	w->pressed_widgets &= ~(1 << WIDX_TAB_2);
	w->pressed_widgets |= (1LL << (_window_guest_list_selected_tab + WIDX_TAB_1));

	window_guest_list_widgets[WIDX_INFO_TYPE_DROPDOWN].image = STR_ACTIONS + _window_guest_list_selected_view;
	window_guest_list_widgets[WIDX_MAP].type = WWT_EMPTY;
	if (_window_guest_list_selected_tab == PAGE_INDIVIDUAL && _window_guest_list_selected_filter != -1)
		window_guest_list_widgets[WIDX_MAP].type = WWT_FLATBTN;

	window_guest_list_widgets[WIDX_BACKGROUND].right = w->width - 1;
	window_guest_list_widgets[WIDX_BACKGROUND].bottom = w->height - 1;
	window_guest_list_widgets[WIDX_TAB_CONTENT_PANEL].right = w->width - 1;
	window_guest_list_widgets[WIDX_TAB_CONTENT_PANEL].bottom = w->height - 1;
	window_guest_list_widgets[WIDX_TITLE].right = w->width - 2;
	window_guest_list_widgets[WIDX_CLOSE].left = w->width - 13;
	window_guest_list_widgets[WIDX_CLOSE].right = w->width - 3;

	window_guest_list_widgets[WIDX_GUEST_LIST].right = w->width - 4;
	window_guest_list_widgets[WIDX_GUEST_LIST].bottom = w->height - 15;
	window_guest_list_widgets[WIDX_PAGE_DROPDOWN].image = _window_guest_list_selected_page + 3440;
}

/**
 * 
 *  rct2: 0x006995CC
 */
static void window_guest_list_paint()
{
	int i, x, y, format;
	rct_window *w;
	rct_drawpixelinfo *dpi;

	window_paint_get_registers(w, dpi);

	// Widgets
	window_draw_widgets(w, dpi);
	// Tab 1 image
	i = (_window_guest_list_selected_tab == 0 ? w->list_information_type & 0x0FFFFFFFC : 0);
	i += RCT2_ADDRESS(RCT2_GLOBAL(0x00982708, int), int)[0] + 1;
	i |= 0xA1600000;
	gfx_draw_sprite(
		dpi,
		i,
		(window_guest_list_widgets[WIDX_TAB_1].left + window_guest_list_widgets[WIDX_TAB_1].right) / 2 + w->x,
		window_guest_list_widgets[WIDX_TAB_1].bottom - 6 + w->y, 0
	);

	// Tab 2 image
	i = (_window_guest_list_selected_tab == 1 ? w->list_information_type / 4 : 0);
	gfx_draw_sprite(
		dpi,
		5568 + i,
		window_guest_list_widgets[WIDX_TAB_2].left + w->x,
		window_guest_list_widgets[WIDX_TAB_2].top + w->y, 0
	);

	// Filter description
	x = w->x + 6;
	y = w->y + window_guest_list_widgets[WIDX_TAB_CONTENT_PANEL].top + 3;
	if (_window_guest_list_selected_tab == PAGE_INDIVIDUAL) {
		if (_window_guest_list_selected_filter != -1) {
			if (RCT2_GLOBAL(0x00F1EDF6, sint16) != -1)
				format = STR_GUESTS_FILTER + _window_guest_list_selected_filter;
			else
				format = STR_GUESTS_FILTER_THINKING_ABOUT;
		} else {
			format = STR_ALL_GUESTS;
		}
	} else {
		format = STR_ALL_GUESTS_SUMMARISED;
	}
	gfx_draw_string_left_clipped(dpi, format, (void*)0x00F1EDF6, 0, x, y, 310);

	// Number of guests (list items)
	if (_window_guest_list_selected_tab == PAGE_INDIVIDUAL) {
		x = w->x + 4;
		y = w->y + window_guest_list_widgets[WIDX_GUEST_LIST].bottom + 2;
		RCT2_GLOBAL(0x013CE952, sint16) = w->var_492;
		gfx_draw_string_left(dpi, (w->var_492 == 1 ? 1755 : 1754), (void*)0x013CE952, 0, x, y);
	}
}

/**
 * 
 *  rct2: 0x00699701
 */
static void window_guest_list_scrollpaint()
{
	int spriteIndex, format, numGuests, i, j, y;
	rct_window *w;
	rct_drawpixelinfo *dpi;
	rct_peep *peep;
	rct_peep_thought *thought;
	uint32 argument_1, argument_2;

	window_paint_get_registers(w, dpi);

	// Background fill
	gfx_fill_rect(dpi, dpi->x, dpi->y, dpi->x + dpi->width - 1, dpi->y + dpi->height - 1, ((char*)0x0141FC48)[w->colours[1] * 8]);

	switch (_window_guest_list_selected_tab) {
	case PAGE_INDIVIDUAL:
		i = 0;
		y = _window_guest_list_selected_page * -0x7BF2;

		// For each guest
		FOR_ALL_GUESTS(spriteIndex, peep) {
			peep->var_0C &= ~0x200;
			if (peep->var_2A != 0)
				continue;
			if (_window_guest_list_selected_filter != -1) {
				if (window_guest_list_is_peep_in_filter(peep))
					continue;
				RCT2_GLOBAL(0x009AC861, uint16) |= 1;
				peep->var_0C |= 0x200;
			}

			// 
			if (y + 11 >= -0x7FFF && y + 11 > dpi->y && y < 0x7FFF) {
				// Check if y is beyond the scroll control
				if (y > dpi->y + dpi->height)
					break;

				// Highlight backcolour and text colour (format)
				format = 1191;
				if (i == _window_guest_list_highlighted_index) {
					gfx_fill_rect(dpi, 0, y, 800, y + 9, 0x02000031);
					format = 1193;
				}

				// Guest name
				RCT2_GLOBAL(0x013CE952, uint16) = peep->name_string_idx;
				RCT2_GLOBAL(0x013CE954, uint32) = peep->id;
				gfx_draw_string_left_clipped(dpi, format, (void*)0x013CE952, 0, 0, y - 1, 113);

				switch (_window_guest_list_selected_view) {
				case VIEW_ACTIONS:
					// Guest face
					gfx_draw_sprite(dpi, get_peep_face_sprite_small(peep), 118, y, 0);

					// Tracking icon
					if (peep->flags & PEEP_FLAGS_TRACKING)
						gfx_draw_sprite(dpi, 5129, 112, y, 0);
					
					// Action
					
					get_arguments_from_action(peep, &argument_1, &argument_2);

					RCT2_GLOBAL(0x013CE952, uint32) = argument_1;
					RCT2_GLOBAL(0x013CE952 + 4, uint32) = argument_2;
					gfx_draw_string_left_clipped(dpi, format, (void*)0x013CE952, 0, 133, y - 1, 314);
					break;
				case VIEW_THOUGHTS:
					// For each thought
					for (j = 0; j < PEEP_MAX_THOUGHTS; j++) {
						thought = &peep->thoughts[j];
						if (thought->type == PEEP_THOUGHT_TYPE_NONE)
							break;
						if (thought->var_2 == 0)
							continue;
						if (thought->var_2 > 5)
							break;
						
						get_arguments_from_thought(peep->thoughts[j], &argument_1, &argument_2);

						RCT2_GLOBAL(0x013CE952, uint32) = argument_1;
						RCT2_GLOBAL(0x013CE952 + 4, uint32) = argument_2;
						gfx_draw_string_left_clipped(dpi, format, (void*)0x013CE952, 0, 118, y - 1, 329);
						break;
					}
					break;
				}
			}

			// Increment list item index and y
			i++;
			y += 10;
		}
		break;
	case PAGE_SUMMARISED:
		y = 0;

		// For each group of guests
		for (i = 0; i < _window_guest_list_num_groups; i++) {
			// Check if y is beyond the scroll control
			if (y + 22 >= dpi->y) {
				// Check if y is beyond the scroll control
				if (y >= dpi->y + dpi->height)
					break;

				// Highlight backcolour and text colour (format)
				format = 1191;
				if (i == _window_guest_list_highlighted_index) {
					gfx_fill_rect(dpi, 0, y, 800, y + 20, 0x02000031);
					format = 1193;
				}

				// Draw guest faces
				numGuests = _window_guest_list_groups_num_guests[i];
				for (j = 0; j < 56 && j < numGuests; j++)
					gfx_draw_sprite(dpi, _window_guest_list_groups_guest_faces[i * 56 + j] + 5486, j * 8, y + 9, 0);

				// Draw action
				RCT2_GLOBAL(0x013CE952, uint32) = _window_guest_list_groups_argument_1[i];
				RCT2_GLOBAL(0x013CE952 + 4, uint32) = _window_guest_list_groups_argument_2[i];
				RCT2_GLOBAL(0x013CE952 + 10, uint32) = numGuests;
				gfx_draw_string_left_clipped(dpi, format, (void*)0x013CE952, 0, 0, y - 1, 414);

				// Draw guest count
				RCT2_GLOBAL(0x013CE95A, uint16) = STR_GUESTS_COUNT_COMMA_SEP;
				gfx_draw_string_right(dpi, format, (void*)0x0013CE95A, 0, 326, y - 1);
			}
			y += 21;
		}
		break;
	}
}


/**
 *  returns 0 for in filter and 1 for not in filter
 *  rct2: 0x0069B865
 */
static int window_guest_list_is_peep_in_filter(rct_peep* peep)
{
	char temp;

	temp = _window_guest_list_selected_view;
	_window_guest_list_selected_view = _window_guest_list_selected_filter;
	uint32 argument1, argument2;
	get_arguments_from_peep(peep, &argument1, &argument2);

	_window_guest_list_selected_view = temp;

	if (((RCT2_GLOBAL(0x00F1EDF6, uint32) >> 16) & 0xFFFF) == 0xFFFF && _window_guest_list_selected_filter == 1)
		argument1 |= 0xFFFF;

	if (argument1 == RCT2_GLOBAL(0x00F1EDF6, uint32) && argument2 == RCT2_GLOBAL(0x00F1EDFA, uint32))
		return 0;
	return 1;
}

/** 
 * rct2:0x0069B7EA
 * Calculates a hash value (arguments) for comparing peep actions/thoughts
 * peep (esi)
 * argument_1 (0x013CE952)
 * argument_2 (0x013CE954)
 */
static void get_arguments_from_peep(rct_peep *peep, uint32 *argument_1, uint32* argument_2)
{
	switch (_window_guest_list_selected_view) {
	case VIEW_ACTIONS:
		get_arguments_from_action(peep, argument_1, argument_2);
		break;
	case VIEW_THOUGHTS:
		if (peep->thoughts[0].var_2 <= 5) {
			if (peep->thoughts[0].type != PEEP_THOUGHT_TYPE_NONE) {
				get_arguments_from_thought(peep->thoughts[0], argument_1, argument_2);
				break;
			}
		}
	default:
		*argument_1 = 0;
		*argument_2 = 0;
	}
	return;
}

/**
 * 
 *  rct2: 0x0069B5AE
 */
static void window_guest_list_find_groups()
{
	int spriteIndex, spriteIndex2, groupIndex, faceIndex;
	rct_peep *peep, *peep2;

	int eax = RCT2_GLOBAL(RCT2_ADDRESS_SCENARIO_TICKS, uint32) & 0xFFFFFF00;
	if (_window_guest_list_selected_view == RCT2_GLOBAL(0x00F1EE02, uint32))
		if (RCT2_GLOBAL(0x00F1AF20, uint16) != 0 || eax == RCT2_GLOBAL(0x00F1AF1C, uint32))
			return;

	RCT2_GLOBAL(0x00F1AF1C, uint32) = eax;
	RCT2_GLOBAL(0x00F1EE02, uint32) = _window_guest_list_selected_view;
	RCT2_GLOBAL(0x00F1AF20, uint16) = 320;
	_window_guest_list_num_groups = 0;

	// Set all guests to unassigned
	FOR_ALL_GUESTS(spriteIndex, peep)
		if (peep->var_2A == 0)
			peep->var_0C |= (1 << 8);

	// For each guest / group
	FOR_ALL_GUESTS(spriteIndex, peep) {
		if (peep->var_2A != 0 || !(peep->var_0C & (1 << 8)))
			continue;

		// New group, cap at 240 though
		groupIndex = _window_guest_list_num_groups;
		if (groupIndex >= 240)
			break;

		int ax = peep->sprite_index;
		_window_guest_list_num_groups++;
		_window_guest_list_groups_num_guests[groupIndex] = 1;
		peep->var_0C &= ~(1 << 8);
		
		get_arguments_from_peep( peep, &_window_guest_list_groups_argument_1[groupIndex], &_window_guest_list_groups_argument_2[groupIndex]);
		RCT2_GLOBAL(0x00F1EDF6, uint32) = _window_guest_list_groups_argument_1[groupIndex];
		RCT2_GLOBAL(0x00F1EDFA, uint32) = _window_guest_list_groups_argument_2[groupIndex];
		
		RCT2_ADDRESS(0x00F1AF26, uint8)[groupIndex] = groupIndex;
		faceIndex = groupIndex * 56;
		_window_guest_list_groups_guest_faces[faceIndex++] = get_peep_face_sprite_small(peep) - 5486;

		// Find more peeps that belong to same group
		FOR_ALL_GUESTS(spriteIndex2, peep2) {
			if (peep2->var_2A != 0 || !(peep2->var_0C & (1 << 8)))
				continue;

			uint32 argument1, argument2;
			// Get and check if in same group
			get_arguments_from_peep(peep2, &argument1, &argument2);
			if (argument1 != _window_guest_list_groups_argument_1[groupIndex] || argument2 != _window_guest_list_groups_argument_2[groupIndex] )
				continue;

			// Assign guest
			_window_guest_list_groups_num_guests[groupIndex]++;
			peep2->var_0C &= ~(1 << 8);

			// Add face sprite, cap at 56 though
			if (_window_guest_list_groups_num_guests[groupIndex] >= 56)
				continue;
			_window_guest_list_groups_guest_faces[faceIndex++] = get_peep_face_sprite_small(peep2) - 5486;
		}

		if (RCT2_GLOBAL(0x00F1EDF6, uint16) == 0) {
			_window_guest_list_num_groups--;
			continue;
		}

		int curr_num_guests = _window_guest_list_groups_num_guests[groupIndex];
		int swap_position = 0;
		//This section places the groups in size order.
		while (1) {
			if (swap_position >= groupIndex)
				goto nextPeep;
			if (curr_num_guests > _window_guest_list_groups_num_guests[swap_position])
				break;
			swap_position++;
		}

		int argument_1 = _window_guest_list_groups_argument_1[groupIndex];
		int argument_2 = _window_guest_list_groups_argument_2[groupIndex];
		int bl = RCT2_ADDRESS(0x00F1AF26, uint8)[groupIndex];
		int temp;

		do {
			temp = curr_num_guests;
			curr_num_guests = _window_guest_list_groups_num_guests[swap_position];
			_window_guest_list_groups_num_guests[swap_position] = temp;

			temp = argument_1;
			argument_1 = _window_guest_list_groups_argument_1[swap_position];
			_window_guest_list_groups_argument_1[swap_position] = temp;

			temp = argument_2;
			argument_2 = _window_guest_list_groups_argument_2[swap_position];
			_window_guest_list_groups_argument_2[swap_position] = temp;

			uint8 temp_faces[56];
			memcpy(temp_faces, &(_window_guest_list_groups_guest_faces[groupIndex*56]), 56);
			memcpy(&(_window_guest_list_groups_guest_faces[groupIndex * 56]), &(_window_guest_list_groups_guest_faces[swap_position * 56]), 56);
			memcpy(&(_window_guest_list_groups_guest_faces[swap_position * 56]), temp_faces, 56);

			temp = RCT2_ADDRESS(0x00F1AF26, uint8)[swap_position];
			RCT2_ADDRESS(0x00F1AF26, uint8)[swap_position] = bl;
			bl = temp;
		} while (++swap_position <= groupIndex);

	nextPeep:
		;
	}
}