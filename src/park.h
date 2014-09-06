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

#ifndef _PARK_H_
#define _PARK_H_

#include "rct2.h"

#define DECRYPT_MONEY(money) rol32((money) ^ 0xF4EC9621, 13)
#define ENCRYPT_MONEY(money) (ror32((money), 13) ^ 0xF4EC9621)

enum {
	PARK_FLAGS_PARK_OPEN = (1 << 0),
	PARK_FLAGS_FORBID_LANDSCAPE_CHANGES = (1 << 2),
	PARK_FLAGS_FORBID_TREE_REMOVAL = (1 << 3),
	PARK_FLAGS_SHOW_REAL_GUEST_NAMES = (1 << 4),
	PARK_FLAGS_FORBID_HIGH_CONSTRUCTION = (1 << 5), // below tree height
	PARK_FLAGS_PREF_LESS_INTENSE_RIDES = (1 << 6),
	PARK_FLAGS_FORBID_MARKETING_CAMPAIGN = (1 << 7),
	PARK_FLAGS_PREF_MORE_INTENSE_RIDES = (1 << 8),
	PARK_FLAGS_NO_MONEY = (1 << 11),
	PARK_FLAGS_DIFFICULT_GUEST_GENERATION = (1 << 12),
	PARK_FLAGS_PARK_FREE_ENTRY = (1 << 13),
	PARK_FLAGS_DIFFICULT_PARK_RATING = (1 << 14),
	PARK_FLAGS_NO_MONEY_SCENARIO = (1 << 17),  // equivalent to PARK_FLAGS_NO_MONEY, but used in scenario editor
	PARK_FLAGS_18 = (1 << 18)
};

int park_is_open();
void park_init();
void park_reset_history();
int park_calculate_size();

int calculate_park_rating();
money32 calculate_park_value();
money32 calculate_company_value();
void reset_park_entrances();
void generate_new_guest();

void park_update();
void park_update_histories();

uint8 calculate_guest_initial_happiness(uint8 percentage);

void game_command_set_park_entrance_fee();

#endif
