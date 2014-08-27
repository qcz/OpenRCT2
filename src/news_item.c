/*****************************************************************************
 * Copyright (c) 2014 Ted John, Peter Hill, Matthias Lanzinger
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
#include "addresses.h"
#include "audio.h"
#include "date.h"
#include "news_item.h"
#include "rct2.h"
#include "ride.h"
#include "string_ids.h"
#include "sprite.h"
#include "window.h"

void window_game_bottom_toolbar_invalidate_news_item();
static int news_item_get_new_history_slot();

/**
 *
 *  rct2: 0x0066DF32
 */
void news_item_init_queue()
{
	int i;
	rct_news_item *newsItems = RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item);

	newsItems[0].type = NEWS_ITEM_NULL;
	newsItems[11].type = NEWS_ITEM_NULL;
	for (i = 0; i < 16; i++)
		RCT2_ADDRESS(0x01358750, uint8)[i] = 0;

	window_game_bottom_toolbar_invalidate_news_item();
}

/**
 * 
 *  rct2: 0x0066E252
 */
void news_item_update_current()
{
	short ax, bx, remove_time;
	rct_news_item *newsItems = RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item);

	get_system_time();

	ax = RCT2_GLOBAL(RCT2_ADDRESS_OS_TIME_DAY, sint16);
	bx = RCT2_GLOBAL(RCT2_ADDRESS_OS_TIME_MONTH, sint16);
	if (bx != RCT2_GLOBAL(0x009DEA6B, sint16)) {
		bx--;
		if (bx == 0)
			bx = 12;
		if (bx != RCT2_GLOBAL(0x009DEA6B, sint16) || ax == 1) {
			// loc_66E2AE
			RCT2_GLOBAL(0x013573DC, sint32) = 10000;
			if (RCT2_GLOBAL(0x013573DC, sint32) >= 0)
				RCT2_GLOBAL(0x013573DC, sint32) = -RCT2_GLOBAL(0x013573DC, sint32);
		}
	} else {
		if (ax != RCT2_GLOBAL(0x009DEA69, sint16)) {
			ax--;
			if (ax != RCT2_GLOBAL(0x009DEA69, sint16)) {
				// loc_66E2AE
				RCT2_GLOBAL(0x013573DC, sint32) = 10000;
				if (RCT2_GLOBAL(0x013573DC, sint32) >= 0)
					RCT2_GLOBAL(0x013573DC, sint32) = -RCT2_GLOBAL(0x013573DC, sint32);
			}
		}
	}

	RCT2_GLOBAL(0x009DEA69, sint16) = RCT2_GLOBAL(RCT2_ADDRESS_OS_TIME_DAY, sint16);
	RCT2_GLOBAL(0x009DEA6B, sint16) = RCT2_GLOBAL(RCT2_ADDRESS_OS_TIME_MONTH, sint16);

	// Check if there is a current news item
	if (newsItems[0].type == 0)
		return;

	window_game_bottom_toolbar_invalidate_news_item();

	// Update the current news item
	newsItems[0].ticks++;
	if (newsItems[0].ticks == 1 && !(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & 1)) {
		// Play sound
		sound_play_panned(SOUND_NEWS_ITEM, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16) / 2);
	}

	// Removal of current news item
	remove_time = 320;
	if (newsItems[2].type != 0 &&
		newsItems[3].type != 0 &&
		newsItems[4].type != 0 &&
		newsItems[5].type != 0)
		remove_time = 256;

	if (newsItems[0].ticks >= remove_time)
		news_item_close_current();
}

/**
 * 
 *  rct2: 0x0066E377
 */
void news_item_close_current()
{
	int i;
	rct_news_item *newsItems = RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item);

	// Check if there is a current message
	if (newsItems[0].type == NEWS_ITEM_NULL)
		return;

	// Find an available history news item slot for current message
	i = news_item_get_new_history_slot();

	// Set the history news item slot to the current news item
	newsItems[i] = newsItems[0];

	// Set the end of the end of the history list
	if (i < 60)
		newsItems[i + 1].type = NEWS_ITEM_NULL;

	// Invalidate the news window
	window_invalidate_by_id(WC_RECENT_NEWS, 0);

	// Dequeue the current news item, shift news up
	for (i = 0; i < 10; i++)
		newsItems[i] = newsItems[i + 1];
	newsItems[10].type = NEWS_ITEM_NULL;

	// Invalidate current news item bar
	window_game_bottom_toolbar_invalidate_news_item();
}

/**
 * Finds a spare history slot or replaces an existing one if there are no spare
 * slots available.
 */
static int news_item_get_new_history_slot()
{
	int i;
	rct_news_item *newsItems = RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item);

	// Find an available history news item slot
	for (i = 11; i < 61; i++)
		if (newsItems[i].type == NEWS_ITEM_NULL)
			return i;

	// Dequeue the first history news item, shift history up
	for (i = 11; i < 60; i++)
		newsItems[i] = newsItems[i + 1];
	return 60;
}

/**
 * Get the (x,y,z) coordinates of the subject of a news item.
 * If the new item is no longer valid, return SPRITE_LOCATION_NULL in the x-coordinate
 *
 *  rct2: 0x0066BA74
 */
void news_item_get_subject_location(int type, int subject, int *x, int *y, int *z)
{
	int i;
	rct_ride *ride;
	rct_peep *peep;
	rct_vehicle *vehicle;

	switch (type) {
	case NEWS_ITEM_RIDE:
		ride = &g_ride_list[subject];
		if (ride->overall_view == 0xFFFF) {
			*x = SPRITE_LOCATION_NULL;
			break;
		}
		*x = (ride->overall_view & 0xFF) * 32 + 16;
		*y = (ride->overall_view >> 8) * 32 + 16;
		*z = map_element_height(*x, *y);
		break;
	case NEWS_ITEM_PEEP_ON_RIDE:
		peep = GET_PEEP(subject);
		*x = peep->x;
		*y = peep->y;
		*z = peep->z;
		if (*((uint16*)x) != SPRITE_LOCATION_NULL)
			break;

		if (peep->state != 3 && peep->state != 7) {
			*x = SPRITE_LOCATION_NULL;
			break;
		}

		// Find which ride peep is on
		ride = &g_ride_list[peep->current_ride];
		if (!(ride->lifecycle_flags & RIDE_LIFECYCLE_ON_TRACK)) {
			*x = SPRITE_LOCATION_NULL;
			break;
		}

		// Find the first car of the train peep is on
		vehicle = &(g_sprite_list[ride->train_car_map[peep->current_train]]).vehicle;
		// Find the actual car peep is on
		for (i = 0; i < peep->current_car; i++)
			vehicle = &(g_sprite_list[vehicle->next_vehicle_on_train]).vehicle;
		*x = vehicle->x;
		*y = vehicle->y;
		*z = vehicle->z;
		break;
	case NEWS_ITEM_PEEP:
		peep = GET_PEEP(subject);
		*x = peep->x;
		*y = peep->y;
		*z = peep->z;
		break;
	case NEWS_ITEM_BLANK:
		*x = subject;
		*y = subject >> 16;
		*z = map_element_height(*x, *y);
		 break;
	default:
		*x = SPRITE_LOCATION_NULL;
		break;
	}
}


/**
 * rct2: 0x0066DF55
 *
 * @param  a (al)
 * @param string_id (ebx)
 * @param c (ecx)
 **/
void news_item_add_to_queue(uint8 type, rct_string_id string_id, uint32 assoc)
{
	int i = 0;
	rct_news_item *newsItem = RCT2_ADDRESS(RCT2_ADDRESS_NEWS_ITEM_LIST, rct_news_item);

	// find first open slot
	while (newsItem->type != NEWS_ITEM_NULL) {
		if (newsItem + sizeof(newsItem) >= (rct_news_item*)0x13CB1CC)
			news_item_close_current();
		else
			newsItem++;
	}

	//now we have found an item slot to place the new news in
	newsItem->type = type;
	newsItem->flags = 0;
	newsItem->assoc = assoc;
	newsItem->ticks = 0;
	newsItem->month_year = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_YEAR, uint16);
	newsItem->day = ((days_in_month[(newsItem->month_year & 7)] * RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_TICKS, uint16)) >> 16) + 1;

	format_string((char*)0x0141EF68, string_id, (void*)0x013CE952); // overflows possible?
	newsItem->colour = ((char*)0x0141EF68)[0];
	strncpy(newsItem->text, (char*)0x0141EF68, 255);
	newsItem->text[254] = 0;

	// blatant disregard for what happens on the last element.
	// Change this when we implement the queue ourselves.
	newsItem++;
	newsItem->type = 0;
}

/**
 * Opens the window/tab for the subject of the news item
 *
 * rct2: 0x0066EBE6
 *
 **/
void news_item_open_subject(int type, int subject) {

	int eax;
	rct_peep* peep;
	rct_window* window;

	switch (type) {
	case NEWS_ITEM_RIDE:
		RCT2_CALLPROC_X(0x006ACC28, subject, 0, 0, 0, 0, 0, 0);
		break;
	case NEWS_ITEM_PEEP_ON_RIDE:
	case NEWS_ITEM_PEEP:
		peep = GET_PEEP(subject);
		RCT2_CALLPROC_X(0x006989E9, 0, 0, 0, (int)peep, 0, 0, 0);
		break;
	case NEWS_ITEM_MONEY:
		window_finances_open();
		break;
	case NEWS_ITEM_RESEARCH:

		if (subject >= 0x10000) {
			// Open ride list window
			RCT2_CALLPROC_EBPSAFE(0x006B3CFF);
			eax = (subject & 0xFF00) >> 8;
			eax += (subject & 0xFF) << 8;
			// Switch to right tab and scroll to ride location
			RCT2_CALLPROC_X(0x006B3EBA, eax, 0, subject, 0, 0, 0, 0);
			break;
		}

		// Check if window is already open
		window = window_bring_to_front_by_id(WC_SCENERY, 0);
		if (window == NULL) {
			window = window_find_by_id(WC_TOP_TOOLBAR, 0);
			if (window != NULL) {
				window_invalidate(window);
				if (tool_set(window, 9, 0)){
					RCT2_CALLPROC_X(0x006E1172, (subject & 0xFFFF), 0, subject, 0, 0, 0, 0);
				}
				RCT2_GLOBAL(0x009DE518, uint32) |= (1 << 6);
				// Open scenery window
				RCT2_CALLPROC_EBPSAFE(0x006E0FEF);
			}
		}
		// Switch to new scenery tab
		RCT2_CALLPROC_X(0x006E1172, (subject & 0xFFFF), 0, subject, 0, 0, 0, 0);

		break;
	case NEWS_ITEM_PEEPS:
		// Open guest list to right tab
		RCT2_CALLPROC_X(0x006993BA, 3, subject, 0, 0, 0, 0, 0);
		break;
	case NEWS_ITEM_AWARD:
		window_park_awards_open();
		break;
	case NEWS_ITEM_GRAPH:
		window_park_rating_open();
		break;
	}
}
