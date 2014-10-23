/*****************************************************************************
 * Copyright (c) 2014 Ted John, Dennis Devriendt
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
#include "../game.h"
#include "../config.h"
#include "../localisation/localisation.h"
#include "../interface/viewport.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "../world/map.h"
#include "../world/banner.h"
#include "../world/scenery.h"
#include "error.h"
#include "dropdown.h"
#include "../drawing/drawing.h"

#define WW 113
#define WH 96

enum WINDOW_BANNER_WIDGET_IDX {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_VIEWPORT,
	WIDX_BANNER_TEXT,
	WIDX_BANNER_NO_ENTRY,
	WIDX_BANNER_DEMOLISH,
	WIDX_MAIN_COLOR,
	WIDX_TEXT_COLOR_DROPDOWN,
	WIDX_TEXT_COLOR_DROPDOWN_BUTTON
};

rct_widget window_banner_widgets[] = {
	{ WWT_FRAME,			0,	0,			WW - 1,	0,		WH - 1,		0x0FFFFFFFF,	65535},								// panel / background
	{ WWT_CAPTION,			0,	1,			WW - 2,	1,		14,			0xBA9,			STR_WINDOW_TITLE_TIP},				// title bar
	{ WWT_CLOSEBOX,			0,	WW - 13,	WW - 3,	2,		13,			0x338,			STR_CLOSE_WINDOW_TIP},				// close x button
	{ WWT_VIEWPORT,			1,	3,			WW - 26,17,		WH - 20,	0x0FFFFFFFE,	65535},								// tab content panel
	{ WWT_FLATBTN,			1,	WW - 25,	WW - 2,	19,		42,			0x1430,			STR_CHANGE_BANNER_TEXT_TIP},		// change banner button
	{ WWT_FLATBTN,			1,	WW - 25,	WW - 2,	43,		66,			0x143A,			STR_SET_AS_NO_ENTRY_BANNER_TIP},	// no entry button
	{ WWT_FLATBTN,			1,	WW - 25,	WW - 2,	67,		90,			0x142D,			STR_DEMOLISH_BANNER_TIP},			// demolish button
	{ WWT_COLORBTN,			1,	5,			16,		WH - 16,WH - 5,		0x0FFFFFFFF,	STR_SELECT_MAIN_COLOR_TIP},			// high money
	{ WWT_DROPDOWN,			1,	43,			81,		WH - 16,WH - 5,		0x0FFFFFFFF,	65535},								// high money
	{ WWT_DROPDOWN_BUTTON,	1,	70,			80,		WH - 15,WH - 6,		0x36C,			STR_SELECT_TEXT_COLOR_TIP},			// high money
	{ WIDGETS_END },
};

static void window_banner_emptysub() { }
static void window_banner_mouseup();
static void window_banner_mousedown(int widgetIndex, rct_window*w, rct_widget* widget);
static void window_banner_dropdown();
static void window_banner_textinput();
static void window_banner_invalidate();
static void window_banner_paint();
static void window_banner_unknown_14();

static void* window_banner_events[] = {
	window_banner_emptysub,
	window_banner_mouseup,
	window_banner_emptysub,
	window_banner_mousedown, 
	window_banner_dropdown, 
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_textinput,
	window_banner_unknown_14,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_emptysub,
	window_banner_invalidate,
	window_banner_paint, 
	window_banner_emptysub
};

/**
*
*  rct2: 0x006BA305
*/
void window_banner_open(rct_windownumber number)
{
	rct_window* w;
	rct_widget *viewportWidget;


	// Check if window is already open
	w = window_bring_to_front_by_number(WC_BANNER, number);
	if (w != NULL)
		return;

	w = window_create_auto_pos(WW, WH, (uint32*)window_banner_events, WC_BANNER, 0);
	w->widgets = window_banner_widgets;
	w->enabled_widgets =
		(1 << WIDX_CLOSE) |
		(1 << WIDX_BANNER_TEXT) |
		(1 << WIDX_BANNER_NO_ENTRY) |
		(1 << WIDX_BANNER_DEMOLISH) |
		(1 << WIDX_MAIN_COLOR) |
		(1 << WIDX_TEXT_COLOR_DROPDOWN) |
		(1 << WIDX_TEXT_COLOR_DROPDOWN_BUTTON);

	w->number = number;
	window_init_scroll_widgets(w);
	w->colours[0] = 24;
	w->colours[1] = 24;
	w->colours[2] = 24;

	int view_x = gBanners[w->number].x << 5;
	int view_y = gBanners[w->number].y << 5;
	int ebp = ((view_y << 8) | view_x) >> 5;
	
	rct_map_element* map_element = TILE_MAP_ELEMENT_POINTER(ebp);

	while(1){
		if (((map_element->type & MAP_ELEMENT_TYPE_MASK) == MAP_ELEMENT_TYPE_BANNER) &&
			(map_element->properties.banner.index == w->number)) break;

		map_element++;
	}

	int view_z = map_element->base_height<<3;
	w->frame_no = view_z;

	view_x += 16;
	view_y += 16;

	// Create viewport
	viewportWidget = &window_banner_widgets[WIDX_VIEWPORT];
	viewport_create(
		w,
		w->x + viewportWidget->left + 1,
		w->y + viewportWidget->top + 1,
		(viewportWidget->right - viewportWidget->left) - 2,
		(viewportWidget->bottom - viewportWidget->top) - 2,
		0,
		view_x,
		view_y,
		view_z,
		0,
		-1
	);

	w->viewport->flags = (RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_FLAGS, uint8) & CONFIG_FLAG_ALWAYS_SHOW_GRIDLINES) ? VIEWPORT_FLAG_GRIDLINES : 0;
	w->flags |= WF_2;
	window_invalidate(w);
}

/* rct2: 0x6ba4d6*/
static void window_banner_mouseup()
{
	short widgetIndex;
	rct_window *w;

	window_widget_get_registers(w, widgetIndex);

	rct_banner* banner = &gBanners[w->number];
	int x = banner->x << 5;
	int y = banner->y << 5;

	rct_map_element* map_element = TILE_MAP_ELEMENT_POINTER(((y << 8) | x) >> 5);

	while (1){
		if (((map_element->type & MAP_ELEMENT_TYPE_MASK) == MAP_ELEMENT_TYPE_BANNER) &&
			(map_element->properties.banner.index == w->number)) break;
		map_element++;
	}

	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	case WIDX_BANNER_DEMOLISH:
		game_do_command(x, 1, y, map_element->base_height | (map_element->properties.banner.position << 8), GAME_COMMAND_51, 0, 0);
		break;
	case WIDX_BANNER_TEXT:
		window_text_input_open(w, WIDX_BANNER_TEXT, 2982, 2983, gBanners[w->number].string_idx, 0);
		break;
	case WIDX_BANNER_NO_ENTRY:
		RCT2_CALLPROC_EBPSAFE(0x006EE3C3);
		banner->flags ^= BANNER_FLAG_NO_ENTRY;
		window_invalidate(w);

		map_element->properties.banner.flags = 0xFF;
		if (banner->flags & BANNER_FLAG_NO_ENTRY){
			map_element->properties.banner.flags &= ~(1 << map_element->properties.banner.position);
		}
		break;
	}
}

/* rct2: 0x6ba4ff */
static void window_banner_mousedown(int widgetIndex, rct_window*w, rct_widget* widget)
{
	rct_banner* banner = &gBanners[w->number];

	switch (widgetIndex) {
	case WIDX_MAIN_COLOR:
		window_dropdown_show_colour(w, widget, w->colours[1] | 0x80, banner->colour);
		break;
	case WIDX_TEXT_COLOR_DROPDOWN_BUTTON:
	
		for( int i = 0; i < 13; ++i){
			gDropdownItemsFormat[i] = 1142;
			gDropdownItemsArgs[i] = 2997 + i;

		}
		
		//Switch to the dropdown box widget.
		widget--;

		window_dropdown_show_text_custom_width(
			widget->left + w->x, 
			widget->top + w->y, 
			widget->bottom - widget->top + 1,
			w->colours[1], 
			0x80, 
			13, 
			widget->right - widget->left - 3);
		
		gDropdownItemsChecked = 1 << (banner->text_colour - 1);
		break;
	}
}

/* rct2: 0x6ba517 */
static void window_banner_dropdown()
{
	short widgetIndex, dropdownIndex;
	rct_window* w;

	window_dropdown_get_registers(w, widgetIndex, dropdownIndex);
	
	rct_banner* banner = &gBanners[w->number];

	switch(widgetIndex){
	case WIDX_MAIN_COLOR:
		if ( dropdownIndex == 0xFFFF) return;
		banner->colour = (uint8)dropdownIndex;
		window_invalidate(w);
		break;
	case WIDX_TEXT_COLOR_DROPDOWN_BUTTON:
		if ( dropdownIndex == 0xFFFF) return;
		banner->text_colour = dropdownIndex + 1;

		//Can be replaced with a buffer 34 chars wide ( 32 character + 1 colour_format + 1 '\0')
		uint8* text_buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, uint8);
		
		format_string(text_buffer, banner->string_idx, 0);
		
		if (text_buffer[0] < FORMAT_COLOUR_CODE_START 
			|| text_buffer[0] > FORMAT_COLOUR_CODE_END){
			int end_point = strlen(text_buffer) + 1;
			strncpy(text_buffer + 1, text_buffer, 32);
			text_buffer[end_point] = '\0';
		}

		text_buffer[0] = banner->text_colour + FORMAT_COLOUR_CODE_START;

		int string_id = 0, ebx = 0, ecx = 128, edx = 0, ebp = 0, esi = 0;
		// Allocate text_buffer to a new string_id?
		RCT2_CALLFUNC_X(0x6C421D, &string_id, &ebx, &ecx, &edx, &esi, (int*)&text_buffer, &ebp);

		if (string_id){
			rct_string_id prev_string_id = banner->string_idx;
			banner->string_idx = string_id;
			// De-allocate previous string id?
			RCT2_CALLPROC_X(0x6C42AC, prev_string_id, 0, 0, 0, 0, 0, 0);
			window_invalidate(w);
		}
		else{
			window_error_open(2984, RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id));
		}
		break;
	}
}

/* rct2: 0x6ba50c */
static void window_banner_textinput()
{
	short widgetIndex;
	rct_window *w;
	uint8 result;
	uint8* text;

	window_text_input_get_registers(w, widgetIndex, result, text);


	if (widgetIndex == WIDX_BANNER_TEXT && result) {

		rct_banner* banner = &gBanners[w->number];

		uint8* text_buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, uint8);

		text_buffer[0] = banner->text_colour + FORMAT_COLOUR_CODE_START;
		strncpy(text_buffer + 1, text, 32);

		int string_id = 0, ebx = 0, ecx = 128, edx = 0, ebp = 0, esi = 0;
		RCT2_CALLFUNC_X(0x6C421D, &string_id, &ebx, &ecx, &edx, &esi, (int*)&text_buffer, &ebp);

		if (string_id){
			rct_string_id prev_string_id = banner->string_idx;
			banner->string_idx = string_id;
			// De-allocate previous string id?
			RCT2_CALLPROC_X(0x6C42AC, prev_string_id, 0, 0, 0, 0, 0, 0);
			window_invalidate(w);
		}
		else{
			window_error_open(2984, RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id));
		}
	}
}

/* rct2: 0x006BA44D */
static void window_banner_invalidate()
{
	rct_window* w;

	window_get_register(w);

	rct_banner* banner = &gBanners[w->number];
	rct_widget* colour_btn = &window_banner_widgets[WIDX_MAIN_COLOR];
	colour_btn->type = WWT_EMPTY;

	//sceneray item not sure why we use this instead of banner?
	rct_scenery_entry* sceneryEntry = g_bannerSceneryEntries[banner->var_00];

	if (sceneryEntry->banner.flags & 1) colour_btn->type = WWT_COLORBTN;
	
	w->pressed_widgets &= ~(1ULL<<WIDX_BANNER_NO_ENTRY);
	w->disabled_widgets &= ~(
		(1ULL<<WIDX_BANNER_TEXT)|
		(1ULL<<WIDX_TEXT_COLOR_DROPDOWN)|
		(1ULL<<WIDX_TEXT_COLOR_DROPDOWN_BUTTON));
	
	if (banner->flags & BANNER_FLAG_NO_ENTRY){
		w->pressed_widgets |= (1ULL<<WIDX_BANNER_NO_ENTRY);
		w->disabled_widgets |= 	
			(1ULL<<WIDX_BANNER_TEXT)|
			(1ULL<<WIDX_TEXT_COLOR_DROPDOWN)|
			(1ULL<<WIDX_TEXT_COLOR_DROPDOWN_BUTTON);
	}

	colour_btn->image = (banner->colour << 19) + 0x600013C3;

	rct_widget* drop_down_widget = &window_banner_widgets[WIDX_TEXT_COLOR_DROPDOWN];
	drop_down_widget->image = banner->text_colour + 2996;
}

/* rct2:0x006BA4C5 */
static void window_banner_paint()
{
	rct_window *w;
	rct_drawpixelinfo *dpi;

	window_paint_get_registers(w, dpi);

	window_draw_widgets(w, dpi);

	// Draw viewport
	if (w->viewport != NULL) {
		window_draw_viewport(dpi, w);
	}
}

/* rct2: 0x6BA7B5 */
static void window_banner_unknown_14()
{
	rct_window* w;
	window_get_register(w);
	
	rct_viewport* view = w->viewport;
	w->viewport = 0;

	view->width = 0;
	viewport_update_pointers();

	rct_banner* banner = &gBanners[w->number];

	int view_x = (banner->x << 5) + 16;
	int view_y = (banner->y << 5) + 16;
	int view_z = w->frame_no;

	// Create viewport
	rct_widget* viewportWidget = &window_banner_widgets[WIDX_VIEWPORT];
	viewport_create(
		w,
		w->x + viewportWidget->left + 1,
		w->y + viewportWidget->top + 1,
		(viewportWidget->right - viewportWidget->left) - 1,
		(viewportWidget->bottom - viewportWidget->top) - 1,
		0,
		view_x,
		view_y,
		view_z,
		0,
		-1
	);

	w->viewport->flags = (RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_FLAGS, uint8) & CONFIG_FLAG_ALWAYS_SHOW_GRIDLINES) ? VIEWPORT_FLAG_GRIDLINES : 0;
	window_invalidate(w);
}
