/*****************************************************************************
 * Copyright (c) 2014 Ted John, Peter Hill
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

#include <windows.h>
#include <string.h>
#include "addresses.h"
#include "climate.h"
#include "date.h"
#include "news_item.h"
#include "park.h"
#include "peep.h"
#include "sprite.h"
#include "sprites.h"
#include "string_ids.h"
#include "widget.h"
#include "window.h"

enum WINDOW_GAME_BOTTOM_TOOLBAR_WIDGET_IDX {
	WIDX_LEFT_OUTSET,
	WIDX_LEFT_INSET,
	WIDX_MONEY,
	WIDX_GUESTS,
	WIDX_PARK_RATING,

	WIDX_MIDDLE_OUTSET,
	WIDX_MIDDLE_INSET,
	WIDX_NEWS_SUBJECT,
	WIDX_NEWS_LOCATE,

	WIDX_RIGHT_OUTSET,
	WIDX_RIGHT_INSET,
	WIDX_DATE
};

rct_widget window_game_bottom_toolbar_widgets[] = {
	{ WWT_IMGBTN,	0,	0x0000,	0x0077,	0,		33,		0xFFFFFFFF,	STR_NONE },	// Left outset panel
	{ WWT_IMGBTN,	0,	0x0002,	0x0075,	2,		31,		0xFFFFFFFF,	STR_NONE },	// Left inset panel
	{ WWT_FLATBTN,	0,	0x0002,	0x0075,	1,		12,		0xFFFFFFFF,	STR_PROFIT_PER_WEEK_AND_PARK_VALUE_TIP },	// Money window
	{ WWT_FLATBTN,	0,	0x0002,	0x0075,	11,		22,		0xFFFFFFFF,	STR_NONE },	// Guests window
	{ WWT_FLATBTN,	0,	0x0002,	0x0075,	21,		31,		0xFFFFFFFF,	STR_PARK_RATING_TIP },	// Park rating window

	{ WWT_IMGBTN,	2,	0x0078,	0x0207,	0,		33,		0xFFFFFFFF,	STR_NONE },	// Middle outset panel
	{ WWT_25,		2,	0x007A,	0x0205,	2,		31,		0xFFFFFFFF,	STR_NONE },	// Middle inset panel
	{ WWT_FLATBTN,	2,	0x007D,	0x0094,	5,		28,		0xFFFFFFFF,	STR_SHOW_SUBJECT_TIP },	// Associated news item window
	{ WWT_FLATBTN,	2,	0x01EB,	0x0202,	5,		28,		SPR_LOCATE,	STR_LOCATE_SUBJECT_TIP },	// Scroll to news item target

	{ WWT_IMGBTN,	0,	0x0208,	0x027F,	0,		33,		0xFFFFFFFF,	STR_NONE },	// Right outset panel
	{ WWT_IMGBTN,	0,	0x020A,	0x027D,	2,		31,		0xFFFFFFFF,	STR_NONE },	// Right inset panel
	{ WWT_FLATBTN,	0,	0x020A,	0x027D,	2,		13,		0xFFFFFFFF,	2290 },	// Date
	{ WIDGETS_END },
};

static void window_game_bottom_toolbar_emptysub() { }
static void window_game_bottom_toolbar_mouseup();
static void window_game_bottom_toolbar_tooltip();
static void window_game_bottom_toolbar_invalidate();
static void window_game_bottom_toolbar_paint();

static void window_game_bottom_toolbar_draw_left_panel(rct_drawpixelinfo *dpi, rct_window *w);
static void window_game_bottom_toolbar_draw_park_rating(rct_drawpixelinfo *dpi, rct_window *w, int colour, int x, int y, uint8 factor);
static void window_game_bottom_toolbar_draw_right_panel(rct_drawpixelinfo *dpi, rct_window *w);
static void window_game_bottom_toolbar_draw_news_item(rct_drawpixelinfo *dpi, rct_window *w);
static void window_game_bottom_toolbar_draw_tutorial_text(rct_drawpixelinfo *dpi, rct_window *w);

static void* window_game_bottom_toolbar_events[] = {
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_mouseup,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_tooltip,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_emptysub,
	window_game_bottom_toolbar_invalidate,
	window_game_bottom_toolbar_paint,
	window_game_bottom_toolbar_emptysub
};

/**
 * Creates the main game bottom toolbar window.
 *  rct2: 0x0066B52F (part of 0x0066B3E8)
 */
void window_game_bottom_toolbar_open()
{
	rct_window* window;

	window = window_create(
		0, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, sint16) - 32,
		RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16), 32,
		(uint32*)window_game_bottom_toolbar_events,
		WC_BOTTOM_TOOLBAR,
		WF_STICK_TO_FRONT | WF_TRANSPARENT | WF_5
	);
	window->widgets = window_game_bottom_toolbar_widgets;
	window->enabled_widgets |=
		(1 << WIDX_LEFT_OUTSET) |
		(1 << WIDX_MONEY) |
		(1 << WIDX_GUESTS) |
		(1 << WIDX_PARK_RATING) |
		(1 << WIDX_MIDDLE_OUTSET) |
		(1 << WIDX_MIDDLE_INSET) |
		(1 << WIDX_NEWS_SUBJECT) |
		(1 << WIDX_NEWS_LOCATE) |
		(1 << WIDX_RIGHT_OUTSET) |
		(1 << WIDX_DATE);

	window->frame_no = 0;
	window_init_scroll_widgets(window);
	window->colours[0] = 140;
	window->colours[1] = 140;
	window->colours[2] = 0;
}

/**
 * 
 *  rct2: 0x0066C588
 */
static void window_game_bottom_toolbar_mouseup()
{
	short widgetIndex;
	rct_window *w, *mainWindow;
	rct_news_item *newsItem;

	window_widget_get_registers(w, widgetIndex);

	switch (widgetIndex) {
	case WIDX_LEFT_OUTSET:
	case WIDX_MONEY:
		if (!(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY))
			window_finances_open();
		break;
	case WIDX_GUESTS:
		window_park_guests_open();
		break;
	case WIDX_PARK_RATING:
		window_park_rating_open();
		break;
	case WIDX_MIDDLE_INSET:
		news_item_close_current();
		break;
	case WIDX_NEWS_SUBJECT:
		newsItem = &(RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item)[0]);
		news_item_open_subject(newsItem->type, newsItem->assoc);
		break;
	case WIDX_NEWS_LOCATE:
		newsItem = &(RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item)[0]);
		if (newsItem->type == NEWS_ITEM_NULL)
			break;

		{
			int x, y, z;
			int subject = newsItem->assoc;

			news_item_get_subject_location(newsItem->type, subject, &x, &y, &z);

			if ((uint16)x == SPRITE_LOCATION_NULL)
				break;

			if ((mainWindow = window_get_main()) != NULL)
				window_scroll_to_location(mainWindow, x, y, z);
		}
		break;
	case WIDX_RIGHT_OUTSET:
	case WIDX_DATE:
		window_news_open();
		break;
	}
}

static void window_game_bottom_toolbar_tooltip()
{
	int month, day;
	short tool_tip_index;
	rct_window *w;

	#ifdef _MSC_VER
	__asm mov tool_tip_index, ax
	#else
	__asm__ ( "mov %[tool_tip_index], ax " : [tool_tip_index] "+m" (tool_tip_index) );
	#endif

	#ifdef _MSC_VER
	__asm mov w, esi
	#else
	__asm__ ( "mov %[w], esi " : [w] "+m" (w) );
	#endif


	switch (tool_tip_index) {
	case WIDX_MONEY:
		RCT2_GLOBAL(0x013CE952, int) = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_PROFIT, sint32);
		RCT2_GLOBAL(0x013CE956, int) = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_PARK_VALUE, sint32);
		tool_tip_index = 0;
		break;
	case WIDX_PARK_RATING:
		RCT2_GLOBAL(0x013CE952, short) = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_PARK_RATING, sint16);
		tool_tip_index = 0;
		break;
	case WIDX_DATE:
		month = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_YEAR, sint16) & 7;
		day = ((RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_TICKS, uint16) * days_in_month[month]) >> 16) & 0xFF;
		
		RCT2_GLOBAL(0x013CE952, short) = STR_DATE_DAY_1 + day;
		RCT2_GLOBAL(0x013CE954, short) = STR_MONTH_MARCH + month;
		tool_tip_index = 0;
		break;
	}

#ifdef _MSC_VER
	__asm mov  ax, tool_tip_index
#else
	__asm__("mov ax, %[tool_tip_index] " :[tool_tip_index] "+m" (tool_tip_index));
#endif
}

/**
 * 
 *  rct2: 0x0066BBA0
 */
static void window_game_bottom_toolbar_invalidate()
{
	int x;
	rct_window *w;
	rct_news_item *newsItem;

	window_get_register(w);

	// Anchor the middle and right panel to the right
	x = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16);
	w->width = x;
	x--;
	window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].right = x;
	x -= 2;
	window_game_bottom_toolbar_widgets[WIDX_RIGHT_INSET].right = x;
	x -= 115;
	window_game_bottom_toolbar_widgets[WIDX_RIGHT_INSET].left = x;
	x -= 2;
	window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].left = x;
	x--;
	window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].right = x;
	x -= 2;
	window_game_bottom_toolbar_widgets[WIDX_MIDDLE_INSET].right = x;
	x -= 3;
	window_game_bottom_toolbar_widgets[WIDX_NEWS_LOCATE].right = x;
	x -= 23;
	window_game_bottom_toolbar_widgets[WIDX_NEWS_LOCATE].left = x;
	window_game_bottom_toolbar_widgets[WIDX_DATE].left = window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].left + 2;
	window_game_bottom_toolbar_widgets[WIDX_DATE].right = window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].right - 2;

	window_game_bottom_toolbar_widgets[WIDX_LEFT_INSET].type = WWT_EMPTY;
	window_game_bottom_toolbar_widgets[WIDX_RIGHT_INSET].type = WWT_EMPTY;

	newsItem = &(RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item)[0]);
	if (newsItem->type == 0) {
		window_game_bottom_toolbar_widgets[WIDX_MIDDLE_INSET].type = WWT_EMPTY;
		window_game_bottom_toolbar_widgets[WIDX_NEWS_SUBJECT].type = WWT_EMPTY;
		window_game_bottom_toolbar_widgets[WIDX_NEWS_LOCATE].type = WWT_EMPTY;
	} else {
		window_game_bottom_toolbar_widgets[WIDX_MIDDLE_INSET].type = WWT_25;
		window_game_bottom_toolbar_widgets[WIDX_NEWS_SUBJECT].type = WWT_FLATBTN;
		window_game_bottom_toolbar_widgets[WIDX_NEWS_LOCATE].type = WWT_FLATBTN;
		w->disabled_widgets &= ~(1 << WIDX_NEWS_SUBJECT);
		w->disabled_widgets &= ~(1 << WIDX_NEWS_LOCATE);

		// Find out if the news item is no longer valid
		int x, y, z;
		int subject = newsItem->assoc;
		news_item_get_subject_location(newsItem->type, subject, &x, &y, &z);

		if (x == SPRITE_LOCATION_NULL)
			w->disabled_widgets |= (1 << WIDX_NEWS_LOCATE);

		if (!(((char*)0x0097BE7C)[newsItem->type] & 2)) {
			w->disabled_widgets |= (1 << WIDX_NEWS_SUBJECT);
			window_game_bottom_toolbar_widgets[WIDX_NEWS_SUBJECT].type = WWT_EMPTY;
		}

		if (newsItem->flags & 1) {
			w->disabled_widgets |= (1 << WIDX_NEWS_SUBJECT);
			w->disabled_widgets |= (1 << WIDX_NEWS_LOCATE);
		}
	}

	// Hide money if there is no money
	if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY) {
		window_game_bottom_toolbar_widgets[WIDX_MONEY].type = WWT_EMPTY;
		window_game_bottom_toolbar_widgets[WIDX_GUESTS].top = 1;
		window_game_bottom_toolbar_widgets[WIDX_GUESTS].bottom = 17;
		window_game_bottom_toolbar_widgets[WIDX_PARK_RATING].top = 17;
	} else {
		window_game_bottom_toolbar_widgets[WIDX_MONEY].type = WWT_FLATBTN;
		window_game_bottom_toolbar_widgets[WIDX_GUESTS].top = 11;
		window_game_bottom_toolbar_widgets[WIDX_GUESTS].bottom = 22;
		window_game_bottom_toolbar_widgets[WIDX_PARK_RATING].top = 21;
	}
}

/**
 * 
 *  rct2: 0x0066BB79
 */
void window_game_bottom_toolbar_invalidate_news_item()
{
	window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].type =
		RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item)[0].type == NEWS_ITEM_NULL ? WWT_EMPTY : WWT_IMGBTN;
	window_invalidate_by_id((WIDX_MIDDLE_OUTSET << 8) | 0x80 | WC_BOTTOM_TOOLBAR, 0);
}

/**
 * 
 *  rct2: 0x0066BC87
 */
static void window_game_bottom_toolbar_paint()
{
	rct_window *w;
	rct_drawpixelinfo *dpi;

	window_paint_get_registers(w, dpi);

	// Draw panel grey backgrounds
	gfx_fill_rect(
		dpi,
		w->x + window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].left,
		w->y + window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].top,
		w->x + window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].right,
		w->y + window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].bottom,
		0x02000033
	);
	gfx_fill_rect(
		dpi,
		w->x + window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].left,
		w->y + window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].top,
		w->x + window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].right,
		w->y + window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].bottom,
		0x02000033
	);

	window_draw_widgets(w, dpi);

	window_game_bottom_toolbar_draw_left_panel(dpi, w);
	window_game_bottom_toolbar_draw_right_panel(dpi, w);

	if (RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item)[0].type != 0)
		window_game_bottom_toolbar_draw_news_item(dpi, w);
	else if (RCT2_GLOBAL(RCT2_ADDRESS_ON_TUTORIAL, uint8))
		window_game_bottom_toolbar_draw_tutorial_text(dpi, w);
}

static void window_game_bottom_toolbar_draw_left_panel(rct_drawpixelinfo *dpi, rct_window *w)
{
	int x, y;

	// Draw green inset rectangle on panel
	gfx_fill_rect_inset(
		dpi,
		w->x + window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].left + 1,
		w->y + window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].top + 1,
		w->x + window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].right - 1,
		w->y + window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].bottom - 1,
		w->colours[1],
		48
	);

	x = (window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].left + window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].right) / 2 + w->x;
	y = window_game_bottom_toolbar_widgets[WIDX_LEFT_OUTSET].top + w->y + 4;

	// Draw money
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY)) {
		RCT2_GLOBAL(0x013CE952, int) = DECRYPT_MONEY(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONEY_ENCRYPTED, sint32));
		gfx_draw_string_centred(
			dpi,
			(RCT2_GLOBAL(0x013CE952, int) < 0 ? 1391 : 1390),
			x, y - 3,
			(RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_OVER_WINDOWCLASS, rct_windowclass) == 2 && RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_OVER_WIDGETINDEX, sint32) == WIDX_MONEY ? 2 : w->colours[0] & 0x7F),
			(void*)0x013CE952
			);
		y += 7;
	}

	// Draw guests
	gfx_draw_string_centred(
		dpi,
		STR_NUM_GUESTS + RCT2_GLOBAL(0x013573FE, uint8),
		x, y,
		(RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_OVER_WINDOWCLASS, rct_windowclass) == 2 && RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_OVER_WIDGETINDEX, sint32) == WIDX_GUESTS ? 2 : w->colours[0] & 0x7F),
		(void*)RCT2_ADDRESS_GUESTS_IN_PARK
	);

	// Draw park rating
	window_game_bottom_toolbar_draw_park_rating(
		dpi,
		w,
		14,
		w->x + window_game_bottom_toolbar_widgets[WIDX_PARK_RATING].left + 11,
		w->y + window_game_bottom_toolbar_widgets[WIDX_PARK_RATING].top,
		max(10, ((RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_PARK_RATING, sint16) / 4) * 263) / 256)
	);
}

/**
 * 
 *  rct2: 0x0066C76C
 */
static void window_game_bottom_toolbar_draw_park_rating(rct_drawpixelinfo *dpi, rct_window *w, int colour, int x, int y, uint8 factor)
{
	short bar_width;

	bar_width = (factor * 90) / 256;
	gfx_fill_rect_inset(dpi, x, y + 1, x + 93, y + 9, w->colours[1], 48);
	if (!(colour & 0x80000000) || RCT2_GLOBAL(0x009DEA6E, uint8) != 0 || (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint8) & 8)) {
		if (bar_width > 2)
			gfx_fill_rect_inset(dpi, x + 2, y + 2, x + bar_width - 1, y + 8, colour & 0x7FFFFFFF, 0);
	}

	// Draw thumbs on the sides
	gfx_draw_sprite(dpi, SPR_RATING_LOW, x - 14, y, 0);
	gfx_draw_sprite(dpi, SPR_RATING_HIGH, x + 92, y, 0);
}

static void window_game_bottom_toolbar_draw_right_panel(rct_drawpixelinfo *dpi, rct_window *w)
{
	int x, y, temperature, format;

	// Draw green inset rectangle on panel
	gfx_fill_rect_inset(
		dpi,
		w->x + window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].left + 1,
		w->y + window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].top + 1,
		w->x + window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].right - 1,
		w->y + window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].bottom - 1,
		w->colours[1],
		48
	);

	x = (window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].left + window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].right) / 2 + w->x;
	y = window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].top + w->y + 2;

	// Date
	char *freeStr = (char*)0x009BC677;
	freeStr[0] = FORMAT_STRINGID;
	freeStr[1] = ' ';
	freeStr[2] = FORMAT_MONTHYEAR;
	freeStr[3] = 0;

	int month = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_YEAR, sint16) & 7;
	int day = ((RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_TICKS, uint16) * days_in_month[month]) >> 16) & 0xFF;
		
	RCT2_GLOBAL(0x013CE952, short) = STR_DATE_DAY_1 + day;
	RCT2_GLOBAL(0x013CE954, short) = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_YEAR, sint16);
	gfx_draw_string_centred(
		dpi,
		3165,
		x,
		y,
		(RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_OVER_WINDOWCLASS, rct_windowclass) == 2 && RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_OVER_WIDGETINDEX, sint32) == WIDX_DATE ? 2 : w->colours[0] & 0x7F),
		(void*)0x013CE952
	);

	// Temperature
	x = w->x + window_game_bottom_toolbar_widgets[WIDX_RIGHT_OUTSET].left + 15;
	y += 11;

	temperature = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TEMPERATURE, sint8);
	format = STR_CELSIUS_VALUE;
	if (RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_TEMPERATURE, uint8)) {
		temperature = climate_celsius_to_fahrenheit(temperature);
		format = STR_FAHRENHEIT_VALUE;
	}
	RCT2_GLOBAL(0x013CE952, short) = temperature;
	gfx_draw_string_left(dpi, format, (void*)0x013CE952, 0, x, y + 6);
	x += 30;

	// Current weather
	gfx_draw_sprite(dpi, climate_weather_data[RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WEATHER, sint8)].sprite_id, x, y, 0);

	// Next weather
	if (climate_weather_data[RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WEATHER, sint8)].sprite_id != climate_weather_data[gClimateNextWeather].sprite_id) {
		if (RCT2_GLOBAL(RCT2_ADDRESS_CLIMATE_UPDATE_TIMER, sint16) < 960) {
			gfx_draw_sprite(dpi, SPR_NEXT_WEATHER, x + 27, y + 5, 0);
			gfx_draw_sprite(dpi, climate_weather_data[gClimateNextWeather].sprite_id, x + 40, y, 0);
		}
	}
}

/**
 * 
 *  rct2: 0x0066BFA5
 */
static void window_game_bottom_toolbar_draw_news_item(rct_drawpixelinfo *dpi, rct_window *w)
{
	int x, y;
	rct_news_item *newsItem;

	newsItem = &(RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item)[0]);

	// Current news item
	gfx_fill_rect_inset(
		dpi,
		w->x + window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].left + 1,
		w->y + window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].top + 1,
		w->x + window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].right - 1,
		w->y + window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].bottom - 1,
		w->colours[2],
		48
	);

	// Text
	memcpy((void*)0x009B5F2C, &newsItem->colour, 256);
	RCT2_CALLPROC_X(
		0x006C1F57,
		14,
		1926,
		(window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].left + window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].right) / 2 + w->x,
		w->y + window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].top + 11,
		0,
		(int)dpi,
		(newsItem->ticks << 16) | (window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].right - window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].left - 62)
	);

	x = w->x + window_game_bottom_toolbar_widgets[WIDX_NEWS_SUBJECT].left;
	y = w->y + window_game_bottom_toolbar_widgets[WIDX_NEWS_SUBJECT].top;
	switch (newsItem->type) {
	case NEWS_ITEM_RIDE:
		gfx_draw_sprite(dpi, SPR_RIDE, x, y, 0);
		break;
	case NEWS_ITEM_PEEP_ON_RIDE:
	case NEWS_ITEM_PEEP:
		if (newsItem->flags & 1)
			break;

		/*
		_edi = (int)e->paint.dpi;
		_cx = x;
		_dx = y;
		#ifdef _MSC_VER
	__asm {
		mov cx, _cx
		mov dx, _dx
		mov esi, w
		mov edi, _edi
		push ebp
		mov ebp, 0066C3B8h
		push after
		push esi
		jmp ebp
		after:
		pop ebp
		}
	#else
	__asm__ ( "\
	\n\
		mov cx, %[_cx] 	\n\
		mov dx, %[_dx] 	\n\
		mov esi, %[w] 	\n\
		mov edi, %[_edi] 	\n\
		push ebp 	\n\
		mov ebp, 0x0066C3B8 	\n\
		push %[after] 	\n\
		push esi 	\n\
		jmp ebp 	\n\
		%[after]: 	\n\
		pop ebp 	\n\
		 " : [_cx] "+m" (_cx), [_dx] "+m" (_dx), [w] "+m" (w), [_edi] "+m" (_edi), [after] "+m" (after) );
	#endif
		break;
		*/

		//_ax = x + 1;
		//_cx = y + 1;
		//_edi = (int)dpi;
		//__asm {
		//	mov edi, _edi
		//		mov ax, _ax
		//		mov cx, _cx
		//		mov bx, 22
		//		mov dx, 22
		//		push ebp
		//}
		//sub_6EE53B();
		//__asm {
		//	pop ebp
		//		mov dpi, edi
		//}
		//if (dpi == NULL)
		//	break;

		//x = 10;
		//y = 19;

		//peep = &rctmem->sprites[*((short*)&newsItem->assoc)].peep;
		//if (peep->type == PEEP_TYPE_STAFF && peep->var_2F == 3)
		//	y += 3;

		//_eax = *((int*)(0x00982708 + (peep->sprite_type * 8)));
		//_ebx = w->frame_no & 0xFFFFFFFC;
		//_ebx += *((int*)_eax);
		//_ebx++;

		//gfx_draw_sprite(dpi, _ebx | (peep->var_30 << 19) | (peep->var_31 << 24) | 0xA0000000, x, y);

		//if (_ebx >= 0x2A1D && _ebx >= 0x2A3D) {
		//	_ebx += 32;
		//	_ebx |= 0x20000000;
		//	_ebx |= peep->balloon_colour << 19;
		//	gfx_draw_sprite(dpi, _ebx, x, y);
		//} else if (_ebx >= 0x2BBD && _ebx >= 0x2BDD) {
		//	_ebx += 32;
		//	_ebx |= 0x20000000;
		//	_ebx |= peep->umbrella_colour << 19;
		//	gfx_draw_sprite(dpi, _ebx, x, y);
		//} else if (_ebx >= 0x29DD && _ebx >= 0x29FD) {
		//	_ebx += 32;
		//	_ebx |= 0x20000000;
		//	_ebx |= peep->hat_colour << 19;
		//	gfx_draw_sprite(dpi, _ebx, x, y);
		//} else {

		//}

		break;
	case NEWS_ITEM_MONEY:
		gfx_draw_sprite(dpi, SPR_FINANCE, x, y, 0);
	case NEWS_ITEM_RESEARCH:
		gfx_draw_sprite(dpi, (newsItem->assoc < 0x10000 ? SPR_NEW_RIDE : SPR_SCENERY), x, y, 0);
		break;
	case NEWS_ITEM_PEEPS:
		gfx_draw_sprite(dpi, SPR_GUESTS, x, y, 0);
	case NEWS_ITEM_AWARD:
		gfx_draw_sprite(dpi, SPR_AWARD, x, y, 0);
	case NEWS_ITEM_GRAPH:
		gfx_draw_sprite(dpi, SPR_GRAPH, x, y, 0);
		break;
	}
}

static void window_game_bottom_toolbar_draw_tutorial_text(rct_drawpixelinfo *dpi, rct_window *w)
{
	int x, y;

	x = (window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].left + window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].right) / 2 + w->x;
	y = window_game_bottom_toolbar_widgets[WIDX_MIDDLE_OUTSET].top + w->y + 2;
	gfx_draw_string_centred(dpi, STR_TUTORIAL, x, y, 32, 0);
	gfx_draw_string_centred(dpi, STR_PRESS_KEY_OR_MOUSE_BUTTON_FOR_CONTROL, x, y + 10, 32, 0);
}
