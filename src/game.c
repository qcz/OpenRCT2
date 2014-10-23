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
 
#include "addresses.h"
#include "audio/audio.h"
#include "config.h"
#include "game.h"
#include "input.h"
#include "localisation/localisation.h"
#include "interface/screenshot.h"
#include "interface/viewport.h"
#include "interface/widget.h"
#include "interface/window.h"
#include "management/finance.h"
#include "management/news_item.h"
#include "management/research.h"
#include "object.h"
#include "peep/peep.h"
#include "peep/staff.h"
#include "platform/osinterface.h"
#include "ride/ride.h"
#include "ride/ride_ratings.h"
#include "ride/vehicle.h"
#include "scenario.h"
#include "title.h"
#include "tutorial.h"
#include "util/sawyercoding.h"
#include "windows/error.h"
#include "windows/tooltip.h"
#include "world/climate.h"
#include "world/park.h"
#include "world/sprite.h"

int gGameSpeed = 1;

/**
 * 
 *  rct2: 0x0066B5C0 (part of 0x0066B3E8)
 */
void game_create_windows()
{
	window_main_open();
	window_game_top_toolbar_open();
	window_game_bottom_toolbar_open();
	RCT2_CALLPROC_EBPSAFE(0x0066B905);
}

/**
*
*  rct2: 0x006838BD
*/
void update_palette_effects()
{
	if (RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8) == 1) {
		// change palette to lighter color during lightning
		int palette = 1532;
		if (RCT2_GLOBAL(0x009ADAE0, sint32) != -1) {
			palette = RCT2_GLOBAL(RCT2_GLOBAL(0x009ADAE0, int) + 2, int);
		}
		rct_g1_element g1_element = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[palette];
		int xoffset = g1_element.x_offset;
		xoffset = xoffset * 4;
		for (int i = 0; i < g1_element.width; i++) {
			RCT2_ADDRESS(0x01424680 + xoffset, uint8)[(i * 4) + 0] = -((0xFF - g1_element.offset[(i * 3) + 0]) / 2) - 1;
			RCT2_ADDRESS(0x01424680 + xoffset, uint8)[(i * 4) + 1] = -((0xFF - g1_element.offset[(i * 3) + 1]) / 2) - 1;
			RCT2_ADDRESS(0x01424680 + xoffset, uint8)[(i * 4) + 2] = -((0xFF - g1_element.offset[(i * 3) + 2]) / 2) - 1;
		}
		RCT2_GLOBAL(0x014241BC, uint32) = 2;
		osinterface_update_palette(RCT2_ADDRESS(0x01424680, uint8), 10, 236);
		RCT2_GLOBAL(0x014241BC, uint32) = 0;
		RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8)++;
	} else {
		if (RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8) == 2) {
			// change palette back to normal after lightning
			int palette = 1532;
			if (RCT2_GLOBAL(0x009ADAE0, sint32) != -1) {
				palette = RCT2_GLOBAL(RCT2_GLOBAL(0x009ADAE0, int) + 2, int);
			}
			rct_g1_element g1_element = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[palette];
			int xoffset = g1_element.x_offset;
			xoffset = xoffset * 4;
			for (int i = 0; i < g1_element.width; i++) {
				RCT2_ADDRESS(0x01424680 + xoffset, uint8)[(i * 4) + 0] = g1_element.offset[(i * 3) + 0];
				RCT2_ADDRESS(0x01424680 + xoffset, uint8)[(i * 4) + 1] = g1_element.offset[(i * 3) + 1];
				RCT2_ADDRESS(0x01424680 + xoffset, uint8)[(i * 4) + 2] = g1_element.offset[(i * 3) + 2];
			}
		}

		// animate the water/lava/chain movement palette
		int q = 0;
		int weather_colour = RCT2_ADDRESS(0x98195C, uint32)[RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WEATHER_GLOOM, uint8)];
		if (weather_colour != -1) {
			q = 1;
			if (weather_colour != 0x2000031) {
				q = 2;
			}
		}
		uint32 j = RCT2_GLOBAL(0x009DE584, uint32);
		j = (((uint16)((~j / 2) * 128) * 15) >> 16);
		int p = 1533;
		if (RCT2_GLOBAL(0x009ADAE0, int) != -1) {
			p = RCT2_GLOBAL(RCT2_GLOBAL(0x009ADAE0, int) + 0x6, int);
		}
		rct_g1_element g1_element = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[q + p];
		uint8* vs = &g1_element.offset[j * 3];
		uint8* vd = RCT2_ADDRESS(0x01424A18, uint8);
		int n = 5;
		for (int i = 0; i < n; i++) {
			vd[0] = vs[0];
			vd[1] = vs[1];
			vd[2] = vs[2];
			vs += 9;
			if (vs >= &g1_element.offset[9 * n]) {
				vs -= 9 * n;
			}
			vd += 4;
		}

		p = 1536;
		if (RCT2_GLOBAL(0x009ADAE0, int) != -1) {
			p = RCT2_GLOBAL(RCT2_GLOBAL(0x009ADAE0, int) + 0xA, int);
		}
		g1_element = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[q + p];
		vs = &g1_element.offset[j * 3];
		n = 5;
		for (int i = 0; i < n; i++) {
			vd[0] = vs[0];
			vd[1] = vs[1];
			vd[2] = vs[2];
			vs += 9;
			if (vs >= &g1_element.offset[9 * n]) {
				vs -= 9 * n;
			}
			vd += 4;
		}

		j = ((uint16)(RCT2_GLOBAL(0x009DE584, uint32) * -960) * 3) >> 16;
		p = 1539;
		g1_element = RCT2_ADDRESS(RCT2_ADDRESS_G1_ELEMENTS, rct_g1_element)[q + p];
		vs = &g1_element.offset[j * 3];
		vd += 12;
		n = 3;
		for (int i = 0; i < n; i++) {
			vd[0] = vs[0];
			vd[1] = vs[1];
			vd[2] = vs[2];
			vs += 3;
			if (vs >= &g1_element.offset[3 * n]) {
				vs -= 3 * n;
			}
			vd += 4;
		}

		RCT2_GLOBAL(0x014241BC, uint32) = 2;
		osinterface_update_palette(RCT2_ADDRESS(0x01424680, uint8), 230, 16);
		RCT2_GLOBAL(0x014241BC, uint32) = 0;
		if (RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8) == 2) {
			RCT2_GLOBAL(0x014241BC, uint32) = 2;
			osinterface_update_palette(RCT2_ADDRESS(0x01424680, uint8), 10, 236);
			RCT2_GLOBAL(0x014241BC, uint32) = 0;
			RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8) = 0;
		}
	}
	if (RCT2_GLOBAL(0x009E2C4C, uint32) == 2 || RCT2_GLOBAL(0x009E2C4C, uint32) == 1) {
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_CAP_BPP, uint32) != 8) {
			RCT2_GLOBAL(0x009E2C78, int) = 1;
		}
	}
}

void game_update()
{
	int i, numUpdates, tmp;

	// Handles picked-up peep and rain redraw
	redraw_peep_and_rain();

	// 0x006E3AEC // screen_game_process_mouse_input();
	// RCT2_CALLPROC_EBPSAFE(0x006E3AEC); // screen_game_process_keyboard_input();
	screenshot_check();
	game_handle_keyboard_input();

	// Determine how many times we need to update the game
	if (gGameSpeed > 1) {
		numUpdates = 1 << (gGameSpeed - 1);
	} else {
		numUpdates = RCT2_GLOBAL(0x009DE588, uint16) / 31;
		numUpdates = clamp(1, numUpdates, 4);
	}

	// Update the game one or more times
	if (RCT2_GLOBAL(0x009DEA6E, uint8) == 0) {
		for (i = 0; i < numUpdates; i++) {
			game_logic_update();
			start_title_music();

			if (gGameSpeed > 1)
				continue;

			// Possibly smooths viewport scrolling, I don't see a difference though
			if (RCT2_GLOBAL(0x009E2D74, uint32) == 1) {
				RCT2_GLOBAL(0x009E2D74, uint32) = 0;
				break;
			} else {
				if (RCT2_GLOBAL(RCT2_ADDRESS_INPUT_STATE, uint8) == INPUT_STATE_RESET ||
					RCT2_GLOBAL(RCT2_ADDRESS_INPUT_STATE, uint8) == INPUT_STATE_NORMAL
				) {
					if (RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32)) {
						RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) &= ~INPUT_FLAG_VIEWPORT_SCROLLING;
						break;
					}
				} else {
					break;
				}
			}
		}
	}


	RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) &= ~INPUT_FLAG_VIEWPORT_SCROLLING;
	RCT2_GLOBAL(0x009AC861, uint16) ^= (1 << 15);
	RCT2_GLOBAL(0x009AC861, uint16) &= ~(1 << 1);
	tmp = RCT2_GLOBAL(0x009AC861, uint16) & (1 << 0);
	RCT2_GLOBAL(0x009AC861, uint16) &= ~(1 << 0);
	if (!tmp)
		RCT2_GLOBAL(0x009AC861, uint16) |= (1 << 1);
	RCT2_GLOBAL(0x009AC861, uint16) &= ~(1 << 3);
	tmp = RCT2_GLOBAL(0x009AC861, uint16) & (1 << 2);
	RCT2_GLOBAL(0x009AC861, uint16) &= ~(1 << 2);
	if (!tmp)
		RCT2_GLOBAL(0x009AC861, uint16) |= (1 << 2);

	window_map_tooltip_update_visibility();
	window_update_all();

	RCT2_GLOBAL(0x01388698, uint16)++;

	// Input
	RCT2_GLOBAL(0x0141F568, uint8) = RCT2_GLOBAL(0x0013CA740, uint8);
	game_handle_input();

	update_palette_effects();
	update_rain_animation();

	if (RCT2_GLOBAL(0x009AAC73, uint8) != 255) {
		RCT2_GLOBAL(0x009AAC73, uint8)++;
		if (RCT2_GLOBAL(0x009AAC73, uint8) == 255)
			config_save();
	}
}

void game_logic_update()
{
	short stringId, _dx;

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, sint32)++;
	RCT2_GLOBAL(RCT2_ADDRESS_SCENARIO_TICKS, sint32)++;
	RCT2_GLOBAL(0x009DEA66, sint16)++;
	if (RCT2_GLOBAL(0x009DEA66, sint16) == 0)
		RCT2_GLOBAL(0x009DEA66, sint16)--;

	sub_68B089();
	scenario_update();
	climate_update();
	fountain_update_all();
	sub_6A876D();
	peep_update_all();
	vehicle_update_all();
	texteffect_update_all();
	ride_update_all();
	park_update();
	research_update();
	ride_ratings_update_all();
	ride_measurements_update();
	map_invalidate_animations();
	vehicle_sounds_update();
	peep_update_crowd_noise();
	climate_update_sound();
	news_item_update_current();
	RCT2_CALLPROC_EBPSAFE(0x0067009A);	// scenario editor opening of windows for a phase

	stop_completed_sounds(); // removes other sounds that are no longer playing, this is normally called somewhere in rct2_init

	// Update windows
	window_dispatch_update_all();

	if (RCT2_GLOBAL(0x009AC31B, uint8) != 0) {
		stringId = STR_UNABLE_TO_LOAD_FILE;
		_dx = RCT2_GLOBAL(0x009AC31C, uint16);
		if (RCT2_GLOBAL(0x009AC31B, uint8) != 254) {
			stringId = RCT2_GLOBAL(0x009AC31C, uint16);
			_dx = 0xFFFF;
		}
		RCT2_GLOBAL(0x009AC31B, uint8) = 0;

		window_error_open(stringId, _dx);
	}
}

/**
 * 
 *  rct2: 0x0069C62C
 *
 * @param cost (ebp)
 */
static int game_check_affordability(int cost)
{
	if (cost <= 0)return cost;
	if (RCT2_GLOBAL(0x141F568, uint8) & 0xF0)return cost;
	
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32)&(1 << 8))){
		if (cost <= (sint32)(DECRYPT_MONEY(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONEY_ENCRYPTED, sint32))))return cost;
	}
	RCT2_GLOBAL(0x13CE952, uint32) = cost;
	RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = 827;
	return 0x80000000;
}

static uint32 game_do_command_table[58];
static GAME_COMMAND_POINTER* new_game_command_table[58];

/**
 * 
 *  rct2: 0x006677F2
 *
 * @param flags (ebx)
 * @param command (esi)
 */
int game_do_command(int eax, int ebx, int ecx, int edx, int esi, int edi, int ebp)
{
	return game_do_command_p(esi, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
}

/**
*
*  rct2: 0x006677F2 with pointers as arguments
*
* @param flags (ebx)
* @param command (esi)
*/
int game_do_command_p(int command, int *eax, int *ebx, int *ecx, int *edx, int *esi, int *edi, int *ebp)
{
	int cost, flags, insufficientFunds;
	int original_ebx, original_edx, original_esi, original_edi, original_ebp;

	*esi = command;
	original_ebx = *ebx;
	original_edx = *edx;
	original_esi = *esi;
	original_edi = *edi;
	original_ebp = *ebp;

	flags = *ebx;
	RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = 0xFFFF;

	// Increment nest count
	RCT2_GLOBAL(0x009A8C28, uint8)++;

	*ebx &= ~1;
	
	// Primary command
	if (game_do_command_table[command] == 0) {
		new_game_command_table[command](eax, ebx, ecx, edx, esi, edi, ebp);
	} else {
		RCT2_CALLFUNC_X(game_do_command_table[command], eax, ebx, ecx, edx, esi, edi, ebp);
	}
	cost = *ebx;

	if (cost != 0x80000000) {
		// Check funds
		insufficientFunds = 0;
		if (RCT2_GLOBAL(0x009A8C28, uint8) == 1 && !(flags & 4) && !(flags & 0x20) && cost != 0)
			insufficientFunds = game_check_affordability(cost);

		if (insufficientFunds != 0x80000000) {
			*ebx = original_ebx;
			*edx = original_edx;
			*esi = original_esi;
			*edi = original_edi;
			*ebp = original_ebp;

			if (!(flags & 1)) {
				// Decrement nest count
				RCT2_GLOBAL(0x009A8C28, uint8)--;
				return cost;
			}

			// Secondary command
			if (game_do_command_table[command] == 0) {
				new_game_command_table[command](eax, ebx, ecx, edx, esi, edi, ebp);
			} else {
				RCT2_CALLFUNC_X(game_do_command_table[command], eax, ebx, ecx, edx, esi, edi, ebp);
			}
			*edx = *ebx;

			if (*edx != 0x80000000 && *edx < cost)
				cost = *edx;

			// Decrement nest count
			RCT2_GLOBAL(0x009A8C28, uint8)--;
			if (RCT2_GLOBAL(0x009A8C28, uint8) != 0)
				return cost;

			// 
			if (!(flags & 0x20)) {
				// Update money balance
				finance_payment(cost, RCT2_GLOBAL(0x0141F56C, uint8));
				if (RCT2_GLOBAL(0x0141F568, uint8) == RCT2_GLOBAL(0x013CA740, uint8)) {
					// Create a +/- money text effect
					if (cost != 0)
						RCT2_CALLPROC_X(0x0069C5D0, 0, cost, 0, 0, 0, 0, 0);
				}
			}

			return cost;
		}
	}

	// Error occured

	// Decrement nest count
	RCT2_GLOBAL(0x009A8C28, uint8)--;

	// Show error window
	if (RCT2_GLOBAL(0x009A8C28, uint8) == 0 && (flags & 1) && RCT2_GLOBAL(0x0141F568, uint8) == RCT2_GLOBAL(0x013CA740, uint8) && !(flags & 8))
		window_error_open(RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, uint16), RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16));

	return 0x80000000;
}


/**
 * 
 *  rct2: 0x00667C15
 */
void game_pause_toggle()
{
	char input_bl;

	#ifdef _MSC_VER
	__asm mov input_bl, bl
	#else
	__asm__ ( "mov %[input_bl], bl " : [input_bl] "+m" (input_bl) );
	#endif


	if (input_bl & 1) {
		RCT2_GLOBAL(0x009DEA6E, uint32) ^= 1;
		window_invalidate_by_class(WC_TOP_TOOLBAR);
		if (RCT2_GLOBAL(0x009DEA6E, uint32) & 1)
			pause_sounds();
		else
			unpause_sounds();
	}

	#ifdef _MSC_VER
	__asm mov ebx, 0
	#else
	__asm__ ( "mov ebx, 0 "  );
	#endif

}

/**
 * 
 *  rct2: 0x0066DB5F
 */
static void game_load_or_quit()
{
	char input_bl, input_dl;
	short input_di;

	#ifdef _MSC_VER
	__asm mov input_bl, bl
	#else
	__asm__ ( "mov %[input_bl], bl " : [input_bl] "+m" (input_bl) );
	#endif

	#ifdef _MSC_VER
	__asm mov input_dl, dl
	#else
	__asm__ ( "mov %[input_dl], dl " : [input_dl] "+m" (input_dl) );
	#endif

	#ifdef _MSC_VER
	__asm mov input_di, di
	#else
	__asm__ ( "mov %[input_di], di " : [input_di] "+m" (input_di) );
	#endif

	if (!(input_bl & 1))
		return; // 0;

	switch (input_dl) {
	case 0:
		RCT2_GLOBAL(RCT2_ADDRESS_SAVE_PROMPT_MODE, uint16) = input_di;
		window_save_prompt_open();
		break;
	case 1:
		window_close_by_class(WC_SAVE_PROMPT);
		break;
	default:
		game_load_or_quit_no_save_prompt();
		break;
	}

	#ifdef _MSC_VER
	__asm mov ebx, 0
	#else
	__asm__ ( "mov ebx, 0 "  );
	#endif

}

/**
 * 
 *  rct2: 0x00674F40
 */
static int open_landscape_file_dialog()
{
	int result;
	format_string((char*)RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, STR_LOAD_LANDSCAPE_DIALOG_TITLE, 0);
	strcpy((char*)0x0141EF68, (char*)RCT2_ADDRESS_LANDSCAPES_PATH);
	format_string((char*)0x0141EE68, STR_RCT2_LANDSCAPE_FILE, 0);
	pause_sounds();
	result = osinterface_open_common_file_dialog(1, (char*)RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, (char*)0x0141EF68, "*.SV6;*.SV4;*.SC6", (char*)0x0141EE68);
	unpause_sounds();
	// window_proc
	return result;
}

/**
 * 
 *  rct2: 0x00674EB6
 */
static int open_load_game_dialog()
{
	int result;
	format_string((char*)RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, STR_LOAD_GAME_DIALOG_TITLE, 0);
	strcpy((char*)0x0141EF68, (char*)RCT2_ADDRESS_SAVED_GAMES_PATH);
	format_string((char*)0x0141EE68, STR_RCT2_SAVED_GAME, 0);
	pause_sounds();
	result = osinterface_open_common_file_dialog(1, (char*)RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, (char*)0x0141EF68, "*.SV6", (char*)0x0141EE68);
	unpause_sounds();
	// window_proc
	return result;
}

/**
 * 
 *  rct2: 0x0066DC0F
 */
static void load_landscape()
{
	if (open_landscape_file_dialog() == 0) {
		gfx_invalidate_screen();
	} else {
		// Set default filename
		char *esi = (char*)0x0141EF67;
		while (1) {
			esi++;
			if (*esi == '.')
				break;
			if (*esi != 0)
				continue;
			strcpy(esi, ".SC6");
			break;
		}
		strcpy((char*)RCT2_ADDRESS_SAVED_GAMES_PATH_2, (char*)0x0141EF68);

		RCT2_CALLPROC_EBPSAFE(0x006758C0); // landscape_load
		if (1) {
			gfx_invalidate_screen();
			rct2_endupdate();
		} else {
			RCT2_GLOBAL(0x009DEA66, uint16) = 0;
			rct2_endupdate();
		}
	}
}

/**
 * 
 *  rct2: 0x00675E1B
 */
int game_load_save(const char *path)
{
	rct_window *mainWindow;
	FILE *file;
	int i, j;

	strcpy((char*)0x0141EF68, path);
	file = fopen(path, "rb");
	if (file == NULL) {
		RCT2_GLOBAL(0x009AC31B, uint8) = 255;
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_STRING_ID, uint16) = STR_FILE_CONTAINS_INVALID_DATA;
		return 0;
	}

	if (!sawyercoding_validate_checksum(file)) {
		fclose(file);
		RCT2_GLOBAL(0x009AC31B, uint8) = 255;
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_STRING_ID, uint16) = STR_FILE_CONTAINS_INVALID_DATA;
		return 0;
	}

	rct_s6_header *s6Header = (rct_s6_header*)0x009E34E4;
	rct_s6_info *s6Info = (rct_s6_info*)0x0141F570;

	// Read first chunk
	sawyercoding_read_chunk(file, (uint8*)s6Header);
	if (s6Header->type == S6_TYPE_SAVEDGAME) {
		// Read packed objects
		if (s6Header->num_packed_objects > 0) {
			j = 0;
			for (i = 0; i < s6Header->num_packed_objects; i++)
				j += object_load_packed(file);
			if (j > 0)
				object_list_load();
		}
	}

	if (!object_read_and_load_entries(file)){
		fclose(file);
		RCT2_GLOBAL(0x009AC31B, uint8) = 255;
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_STRING_ID, uint16) = STR_FILE_CONTAINS_INVALID_DATA;
		return 0;
	};

	// Read flags (16 bytes)
	sawyercoding_read_chunk(file, (uint8*)RCT2_ADDRESS_CURRENT_MONTH_YEAR);

	// Read map elements
	memset((void*)RCT2_ADDRESS_MAP_ELEMENTS, 0, MAX_MAP_ELEMENTS * sizeof(rct_map_element));
	sawyercoding_read_chunk(file, (uint8*)RCT2_ADDRESS_MAP_ELEMENTS);

	// Read game data, including sprites
	sawyercoding_read_chunk(file, (uint8*)0x010E63B8);

	fclose(file);

	// Check expansion pack
	// RCT2_CALLPROC_EBPSAFE(0x006757E6);

	// The rest is the same as in scenario load and play
	RCT2_CALLPROC_EBPSAFE(0x006A9FC0);
	map_update_tile_pointers();
	reset_0x69EBE4();// RCT2_CALLPROC_EBPSAFE(0x0069EBE4);
	RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) = SCREEN_FLAGS_PLAYING;
	viewport_init_all();
	game_create_windows();
	mainWindow = window_get_main();

	mainWindow->viewport_target_sprite = -1;
	mainWindow->saved_view_x = RCT2_GLOBAL(RCT2_ADDRESS_SAVED_VIEW_X, sint16);
	mainWindow->saved_view_y = RCT2_GLOBAL(RCT2_ADDRESS_SAVED_VIEW_Y, sint16);
	uint8 _cl = (RCT2_GLOBAL(0x0138869E, sint16) & 0xFF) - mainWindow->viewport->zoom;
	mainWindow->viewport->zoom = RCT2_GLOBAL(0x0138869E, sint16) & 0xFF;
	*((char*)(&RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_ROTATION, sint32))) = RCT2_GLOBAL(0x0138869E, sint16) >> 8;
	if (_cl != 0) {
		if (_cl < 0) {
			_cl = -_cl;
			mainWindow->viewport->view_width >>= _cl;
			mainWindow->viewport->view_height >>= _cl;
		} else {
			mainWindow->viewport->view_width <<= _cl;
			mainWindow->viewport->view_height <<= _cl;
		}
	}
	mainWindow->saved_view_x -= mainWindow->viewport->view_width >> 1;
	mainWindow->saved_view_y -= mainWindow->viewport->view_height >> 1;
	window_invalidate(mainWindow);

	sub_0x0069E9A7(); 
	RCT2_CALLPROC_EBPSAFE(0x006DFEE4);
	window_new_ride_init_vars();
	RCT2_GLOBAL(0x009DEB7C, uint16) = 0;
	if (RCT2_GLOBAL(0x0013587C4, uint32) == 0)		// this check is not in scenario play
		sub_69E869();

	RCT2_CALLPROC_EBPSAFE(0x006837E3); // (palette related)
	gfx_invalidate_screen();
	return 1;
}

/*
 *
 * rct2: 0x0069E9A7
 */
void sub_0x0069E9A7(){
	//RCT2_CALLPROC_EBPSAFE(0x0069E9A7);
	//return;
	for (rct_sprite* spr = g_sprite_list; spr < (rct_sprite*)RCT2_ADDRESS_SPRITES_NEXT_INDEX; ++spr){
		if (spr->unknown.sprite_identifier != 0xFF){
			RCT2_CALLPROC_X(0x0069E9D3, spr->unknown.x, 0, spr->unknown.y, spr->unknown.z, (int)spr, 0, 0);
		}
	}
}

/**
 * 
 *  rct2: 0x0066DBB7
 */
static void load_game()
{
	if (open_load_game_dialog() == 0) {
		gfx_invalidate_screen();
	} else {
		// Set default filename
		char *esi = (char*)0x0141EF67;
		while (1) {
			esi++;
			if (*esi == '.')
				break;
			if (*esi != 0)
				continue;
			strcpy(esi, ".SV6");
			break;
		}
		strcpy((char*)RCT2_ADDRESS_SAVED_GAMES_PATH_2, (char*)0x0141EF68);

		if (game_load_save((char *)0x0141EF68)) {
			gfx_invalidate_screen();
			rct2_endupdate();
		} else {
			RCT2_GLOBAL(0x009DEA66, uint16) = 0;
			rct2_endupdate();
		}
	}
}

char save_game()
{
	int eax, ebx, ecx, edx, esi, edi, ebp;
	RCT2_CALLFUNC_X(0x006750E9, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
	if (eax == 0) {
		// user pressed "cancel"
		gfx_invalidate_screen();
		return 0;
	}
	
	char *src = (char*)0x0141EF67;
	do {
		src++;
	} while (*src != '.' && *src != '\0');
	strcpy(src, ".SV6");
	strcpy((char*) RCT2_ADDRESS_SAVED_GAMES_PATH_2, (char*) 0x0141EF68);
	
	eax = 0;
	if (RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_FLAGS, uint8) & 8)
		eax |= 1;
	RCT2_CALLPROC_X(0x006754F5, eax, 0, 0, 0, 0, 0, 0);
	// check success?

	game_do_command(0, 1047, 0, -1, GAME_COMMAND_0, 0, 0);
	gfx_invalidate_screen();
	
	return 1;
}

/**
 * 
 *  rct2: 0x006E3879
 */
void rct2_exit()
{
	RCT2_CALLPROC_EBPSAFE(0x006E3879);
	//Post quit message does not work in 0x6e3879 as its windows only.
}

/**
 * 
 *  rct2: 0x0066DB79
 */
void game_load_or_quit_no_save_prompt()
{
	if (RCT2_GLOBAL(RCT2_ADDRESS_SAVE_PROMPT_MODE, uint16) < 1) {
		game_do_command(0, 1, 0, 1, GAME_COMMAND_LOAD_OR_QUIT, 0, 0);
		tool_cancel();
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & 2)
			load_landscape();
		else
			load_game();
	} else if (RCT2_GLOBAL(RCT2_ADDRESS_SAVE_PROMPT_MODE, uint16) == 1) {
		game_do_command(0, 1, 0, 1, GAME_COMMAND_LOAD_OR_QUIT, 0, 0);
		if (RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) & INPUT_FLAG_5) {
			RCT2_CALLPROC_EBPSAFE(0x0040705E);
			RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) &= ~INPUT_FLAG_5;
		}
		title_load();
		rct2_endupdate();
	} else {
		rct2_exit();
	}
}

#pragma region Game command function table

static uint32 game_do_command_table[58] = {
	0x006B2FC5,
	0x0066397F,
	(uint32)game_pause_toggle,
	0x006C511D,
	0x006C5B69,
	(uint32)game_load_or_quit,
	0x006B3F0F,
	0x006B49D9,
	0x006B4EA6,
	0x006B52D4,
	0x006B578B, // 10
	0x006B5559,
	0x006660A8,
	0x0066640B,
	0x006E0E01,
	0x006E08F4,
	0x006E650F,
	0x006A61DE,
	0x006A68AE,
	0x006A67C0,
	0x00663CCD, // 20
	0x006B53E9,
	0x00698D6C, // text input
	0x0068C542,
	0x0068C6D1,
	0x0068BC01,
	0x006E66A0,
	0x006E6878,
	0x006C5AE9,
	0, // use new_game_command_table, original: 0x006BEFA1, 29
	0x006C09D1, // 30
	0x006C0B83,
	0x006C0BB5,
	0x00669C6D,
	0x00669D4A,
	0x006649BD,
	0x006666E7,
	0x00666A63,
	0x006CD8CE,
	(uint32)game_command_set_park_entrance_fee,
	(uint32)game_command_update_staff_colour, // 40
	0x006E519A,
	0x006E5597,
	0x006B893C,
	0x006B8E1B,
	0x0069DFB3,
	0x00684A7F,
	0x006D13FE,
	0x0069E73C,
	0x006CDEE4,
	0x006B9E6D, // 50
	0x006BA058,
	0x006E0F26,
	0x006E56B5,
	0x006B909A,
	0x006BA16A,
	0x006648E3,
	0x0068DF91
};

void game_command_emptysub(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp) {}

static GAME_COMMAND_POINTER* new_game_command_table[58] = {
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub, // 10
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub, // 20
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_hire_new_staff_member, //game_command_emptysub,
	game_command_emptysub, // 30
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub, // 40
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub, // 50
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub,
	game_command_emptysub
};

#pragma endregion
