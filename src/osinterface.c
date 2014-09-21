/*****************************************************************************
 * Copyright (c) 2014 Ted John, Alexander Overvoorde
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

#include <stdio.h>
#include <shlobj.h>
#include <tchar.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <windows.h>

#include "addresses.h"
#include "config.h"
#include "gfx.h"
#include "input.h"
#include "osinterface.h"
#include "screenshot.h"
#include "window.h"
#include "rct2.h"
#include "cursors.h"

typedef void(*update_palette_func)(char*, int, int);

openrct2_cursor gCursorState;
const unsigned char *gKeysState;
unsigned char *gKeysPressed;
unsigned int gLastKeyPressed;

static void osinterface_create_window();
static void osinterface_close_window();
static void osinterface_resize(int width, int height);

static SDL_Window *_window;
static SDL_Surface *_surface;
static SDL_Palette *_palette;

static int _screenBufferSize;
static void *_screenBuffer;

static SDL_Cursor* _cursors[NO_CURSORS];

static const int _fullscreen_modes[] = { 0, SDL_WINDOW_FULLSCREEN, SDL_WINDOW_FULLSCREEN_DESKTOP };

void osinterface_init()
{
	osinterface_create_window();

	gKeysPressed = malloc(sizeof(unsigned char) * 256);
	memset(gKeysPressed, 0, sizeof(unsigned char) * 256);

	// RCT2_CALLPROC(0x00404584); // dinput_init()
}

/**
 *  This is not quite the same as the below function as we don't want to
 *  derfererence the cursor before the function.
 *  rct2: 0x0407956
 */
void osinterface_set_cursor(char cursor){
	//HCURSOR hCurs = RCT2_ADDRESS(RCT2_ADDRESS_HCURSOR_START, HCURSOR)[cursor];
	//SetCursor((HCURSOR)hCurs);
	SDL_SetCursor(_cursors[cursor]);
}
/**
 *rct2: 0x0068352C
 */
static void osinterface_load_cursors(){

	RCT2_GLOBAL(0x14241BC, uint32) = 2;
	HINSTANCE hInst = RCT2_GLOBAL(RCT2_ADDRESS_HINSTANCE, HINSTANCE);
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_ARROW,				HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x74));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_BLANK,				HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0xA1));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_UP_ARROW,			HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x6D));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_UP_DOWN_ARROW,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x6E));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_HAND_POINT,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x70));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_ZZZ,				HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x78));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_DIAGONAL_ARROWS,	HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x77));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_PICKER,			HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x7C));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_TREE_DOWN,			HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x83));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_FOUNTAIN_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x7F));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_STATUE_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x80));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_BENCH_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x81));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_CROSS_HAIR,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x82));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_BIN_DOWN,			HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x84));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_LAMPPOST_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x85));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_FENCE_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x8A));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_FLOWER_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x89));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_PATH_DOWN,			HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x8B));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_DIG_DOWN,			HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x8D));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_WATER_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x8E));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_HOUSE_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x8F));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_VOLCANO_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x90));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_WALK_DOWN,			HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x91));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_PAINT_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x9E));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_ENTRANCE_DOWN,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0x9F));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_HAND_OPEN,			HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0xA6));
	RCT2_GLOBAL(RCT2_ADDRESS_HCURSOR_HAND_CLOSED,		HCURSOR) = LoadCursor(hInst, MAKEINTRESOURCE(0xA5));

	_cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	_cursors[1] = SDL_CreateCursor(blank_cursor_data, blank_cursor_mask, 32, 32, BLANK_CURSOR_HOTX, BLANK_CURSOR_HOTY);
	_cursors[2] = SDL_CreateCursor(up_arrow_cursor_data, up_arrow_cursor_mask, 32, 32, UP_ARROW_CURSOR_HOTX, UP_ARROW_CURSOR_HOTY);
	_cursors[3] = SDL_CreateCursor(up_down_arrow_cursor_data, up_down_arrow_cursor_mask, 32, 32, UP_DOWN_ARROW_CURSOR_HOTX, UP_DOWN_ARROW_CURSOR_HOTY);
	_cursors[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	_cursors[5] = SDL_CreateCursor(zzz_cursor_data, zzz_cursor_mask, 32, 32, ZZZ_CURSOR_HOTX, ZZZ_CURSOR_HOTY);
	_cursors[6] = SDL_CreateCursor(diagonal_arrow_cursor_data, diagonal_arrow_cursor_mask, 32, 32, DIAGONAL_ARROW_CURSOR_HOTX, DIAGONAL_ARROW_CURSOR_HOTY);
	_cursors[7] = SDL_CreateCursor(picker_cursor_data, picker_cursor_mask, 32, 32, PICKER_CURSOR_HOTX, PICKER_CURSOR_HOTY);
	_cursors[8] = SDL_CreateCursor(tree_down_cursor_data, tree_down_cursor_mask, 32, 32, TREE_DOWN_CURSOR_HOTX, TREE_DOWN_CURSOR_HOTY);
	_cursors[9] = SDL_CreateCursor(fountain_down_cursor_data, fountain_down_cursor_mask, 32, 32, FOUNTAIN_DOWN_CURSOR_HOTX, FOUNTAIN_DOWN_CURSOR_HOTY);
	_cursors[10] = SDL_CreateCursor(statue_down_cursor_data, statue_down_cursor_mask, 32, 32, STATUE_DOWN_CURSOR_HOTX, STATUE_DOWN_CURSOR_HOTY);
	_cursors[11] = SDL_CreateCursor(bench_down_cursor_data, bench_down_cursor_mask, 32, 32, BENCH_DOWN_CURSOR_HOTX, BENCH_DOWN_CURSOR_HOTY);
	_cursors[12] = SDL_CreateCursor(cross_hair_cursor_data, cross_hair_cursor_mask, 32, 32, CROSS_HAIR_CURSOR_HOTX, CROSS_HAIR_CURSOR_HOTY);
	_cursors[13] = SDL_CreateCursor(bin_down_cursor_data, bin_down_cursor_mask, 32, 32, BIN_DOWN_CURSOR_HOTX, BIN_DOWN_CURSOR_HOTY);
	_cursors[14] = SDL_CreateCursor(lamppost_down_cursor_data, lamppost_down_cursor_mask, 32, 32, LAMPPOST_DOWN_CURSOR_HOTX, LAMPPOST_DOWN_CURSOR_HOTY);
	_cursors[15] = SDL_CreateCursor(fence_down_cursor_data, fence_down_cursor_mask, 32, 32, FENCE_DOWN_CURSOR_HOTX, FENCE_DOWN_CURSOR_HOTY);
	_cursors[16] = SDL_CreateCursor(flower_down_cursor_data, flower_down_cursor_mask, 32, 32, FLOWER_DOWN_CURSOR_HOTX, FLOWER_DOWN_CURSOR_HOTY);
	_cursors[17] = SDL_CreateCursor(path_down_cursor_data, path_down_cursor_mask, 32, 32, PATH_DOWN_CURSOR_HOTX, PATH_DOWN_CURSOR_HOTY);
	_cursors[18] = SDL_CreateCursor(dig_down_cursor_data, dig_down_cursor_mask, 32, 32, DIG_DOWN_CURSOR_HOTX, DIG_DOWN_CURSOR_HOTY);
	_cursors[19] = SDL_CreateCursor(water_down_cursor_data, water_down_cursor_mask, 32, 32, WATER_DOWN_CURSOR_HOTX, WATER_DOWN_CURSOR_HOTY);
	_cursors[20] = SDL_CreateCursor(house_down_cursor_data, house_down_cursor_mask, 32, 32, HOUSE_DOWN_CURSOR_HOTX, HOUSE_DOWN_CURSOR_HOTY);
	_cursors[21] = SDL_CreateCursor(volcano_down_cursor_data, volcano_down_cursor_mask, 32, 32, VOLCANO_DOWN_CURSOR_HOTX, VOLCANO_DOWN_CURSOR_HOTY);
	_cursors[22] = SDL_CreateCursor(walk_down_cursor_data, walk_down_cursor_mask, 32, 32, WALK_DOWN_CURSOR_HOTX, WALK_DOWN_CURSOR_HOTY);
	_cursors[23] = SDL_CreateCursor(paint_down_cursor_data, paint_down_cursor_mask, 32, 32, PAINT_DOWN_CURSOR_HOTX, PAINT_DOWN_CURSOR_HOTY);
	_cursors[24] = SDL_CreateCursor(entrance_down_cursor_data, entrance_down_cursor_mask, 32, 32, ENTRANCE_DOWN_CURSOR_HOTX, ENTRANCE_DOWN_CURSOR_HOTY);
	_cursors[25] = SDL_CreateCursor(hand_open_cursor_data, hand_open_cursor_mask, 32, 32, HAND_OPEN_CURSOR_HOTX, HAND_OPEN_CURSOR_HOTY);
	_cursors[26] = SDL_CreateCursor(hand_closed_cursor_data, hand_closed_cursor_mask, 32, 32, HAND_CLOSED_CURSOR_HOTX, HAND_CLOSED_CURSOR_HOTY);
	osinterface_set_cursor(CURSOR_ARROW);
	RCT2_GLOBAL(0x14241BC, uint32) = 0;
}

static void osinterface_unload_cursors(){
	for (int i = 0; i < NO_CURSORS; ++i){
		if (_cursors[i] != NULL)SDL_FreeCursor(_cursors[i]);
	}
}

static void osinterface_create_window()
{
	SDL_SysWMinfo wmInfo;
	HWND hWnd;
	int width, height;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		RCT2_ERROR("SDL_Init %s", SDL_GetError());
		exit(-1);
	}

	// stuff
	{
		osinterface_load_cursors();
		RCT2_CALLPROC_EBPSAFE(0x0068371D);

		width = RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_RESOLUTION_WIDTH, sint16);
		height = RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_RESOLUTION_HEIGHT, sint16);

		width = 640;
		height = 480;
	}

	RCT2_GLOBAL(0x009E2D8C, sint32) = 0;

	_window = SDL_CreateWindow("OpenRCT2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
		_fullscreen_modes[gGeneral_config.fullscreen_mode] | SDL_WINDOW_RESIZABLE);
	if (!_window) {
		RCT2_ERROR("SDL_CreateWindow failed %s", SDL_GetError());
		exit(-1);
	}

	SDL_VERSION(&wmInfo.version);
	// Get the HWND context
	if (SDL_GetWindowWMInfo(_window, &wmInfo) != SDL_TRUE) {
		RCT2_ERROR("SDL_GetWindowWMInfo failed %s", SDL_GetError());
		exit(-1);
	}
	hWnd = wmInfo.info.win.window;
	RCT2_GLOBAL(0x009E2D70, HWND) = hWnd;

	// Set the update palette function pointer
	RCT2_GLOBAL(0x009E2BE4, update_palette_func) = osinterface_update_palette;

	// Initialise the surface, palette and draw buffer
	osinterface_resize(width, height);
}


static void osinterface_resize(int width, int height)
{
	rct_drawpixelinfo *screenDPI;
	int newScreenBufferSize;
	void *newScreenBuffer;

	if (_surface != NULL)
		SDL_FreeSurface(_surface);
	if (_palette != NULL)
		SDL_FreePalette(_palette);

	_surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
	_palette = SDL_AllocPalette(256);

	if (!_surface || !_palette) {
		RCT2_ERROR("%p || %p == NULL %s", _surface, _palette, SDL_GetError());
		exit(-1);
	}

	if (SDL_SetSurfacePalette(_surface, _palette)) {
		RCT2_ERROR("SDL_SetSurfacePalette failed %s", SDL_GetError());
		exit(-1);
	}

	newScreenBufferSize = _surface->pitch * _surface->h;
	newScreenBuffer = malloc(newScreenBufferSize);
	if (_screenBuffer == NULL) {
		memset(newScreenBuffer, 0, newScreenBufferSize);
	} else {
		memcpy(newScreenBuffer, _screenBuffer, min(_screenBufferSize, newScreenBufferSize));
		if (newScreenBufferSize - _screenBufferSize > 0)
			memset((uint8*)newScreenBuffer + _screenBufferSize, 0, newScreenBufferSize - _screenBufferSize);
		free(_screenBuffer);
	}

	_screenBuffer = newScreenBuffer;
	_screenBufferSize = newScreenBufferSize;

	RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16) = width;
	RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, sint16) = height;

	screenDPI = RCT2_ADDRESS(RCT2_ADDRESS_SCREEN_DPI, rct_drawpixelinfo);
	screenDPI->bits = _screenBuffer;
	screenDPI->x = 0;
	screenDPI->y = 0;
	screenDPI->width = width;
	screenDPI->height = height;
	screenDPI->pitch = _surface->pitch - _surface->w;

	RCT2_GLOBAL(0x009ABDF0, uint8) = 6;
	RCT2_GLOBAL(0x009ABDF1, uint8) = 3;
	RCT2_GLOBAL(0x009ABDF2, uint8) = 1;
	RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_WIDTH, sint16) = 64;
	RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_HEIGHT, sint16) = 8;
	RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_COLUMNS, sint32) = (width >> 6) + 1;
	RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_ROWS, sint32) = (height >> 3) + 1;

	window_resize_gui(width, height);
	//RCT2_CALLPROC_EBPSAFE(0x0066B905); // resize_gui()

	gfx_invalidate_screen();
}

void osinterface_update_palette(char* colours, int start_index, int num_colours)
{
	SDL_Color base[256];
	SDL_Surface *surface;
	int i;

	surface = SDL_GetWindowSurface(_window);
	if (!surface) {
		RCT2_ERROR("SDL_GetWindowSurface failed %s", SDL_GetError());
		exit(1);
	}

	for (i = 0; i < 256; i++) {
		base[i].r = colours[2];
		base[i].g = colours[1];
		base[i].b = colours[0];
		base[i].a = 0;
		colours += 4;
	}

	if (SDL_SetPaletteColors(_palette, base, 0, 256)) {
		RCT2_ERROR("SDL_SetPaletteColors failed %s", SDL_GetError());
		exit(1);
	}
}

void osinterface_draw()
{
	// Lock the surface before setting its pixels
	if (SDL_MUSTLOCK(_surface))
		if (SDL_LockSurface(_surface) < 0) {
			RCT2_ERROR("locking failed %s", SDL_GetError());
			return;
		}

	// Copy pixels from the virtual screen buffer to the surface
	memcpy(_surface->pixels, _screenBuffer, _surface->pitch * _surface->h);

	// Unlock the surface
	if (SDL_MUSTLOCK(_surface))
		SDL_UnlockSurface(_surface);

	// Copy the surface to the window
	if (SDL_BlitSurface(_surface, NULL, SDL_GetWindowSurface(_window), NULL)) {
		RCT2_ERROR("SDL_BlitSurface %s", SDL_GetError());
		exit(1);
	}
	if (SDL_UpdateWindowSurface(_window)) {
		RCT2_ERROR("SDL_UpdateWindowSurface %s", SDL_GetError());
		exit(1);
	}
}

void osinterface_process_messages()
{
	SDL_Event e;

	gLastKeyPressed = 0;
	// gCursorState.wheel = 0;
	gCursorState.left &= ~CURSOR_CHANGED;
	gCursorState.middle &= ~CURSOR_CHANGED;
	gCursorState.right &= ~CURSOR_CHANGED;
	gCursorState.old = 0;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
// 			rct2_finish();
			rct2_quit();
			break;
		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				osinterface_resize(e.window.data1, e.window.data2);
			break;
		case SDL_MOUSEMOTION:
			RCT2_GLOBAL(0x0142406C, int) = e.motion.x;
			RCT2_GLOBAL(0x01424070, int) = e.motion.y;

			gCursorState.x = e.motion.x;
			gCursorState.y = e.motion.y;
			break;
		case SDL_MOUSEWHEEL:
			gCursorState.wheel += e.wheel.y * 128;
			break;
		case SDL_MOUSEBUTTONDOWN:
			RCT2_GLOBAL(0x01424318, int) = e.button.x;
			RCT2_GLOBAL(0x0142431C, int) = e.button.y;
			switch (e.button.button) {
			case SDL_BUTTON_LEFT:
				store_mouse_input(1);
				gCursorState.left = CURSOR_PRESSED;
				gCursorState.old = 1;
				break;
			case SDL_BUTTON_MIDDLE:
				gCursorState.middle = CURSOR_PRESSED;
				break;
			case SDL_BUTTON_RIGHT:
				store_mouse_input(3);
				gCursorState.right = CURSOR_PRESSED;
				gCursorState.old = 2;
				break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			RCT2_GLOBAL(0x01424318, int) = e.button.x;
			RCT2_GLOBAL(0x0142431C, int) = e.button.y;
			switch (e.button.button) {
			case SDL_BUTTON_LEFT:
				store_mouse_input(2);
				gCursorState.left = CURSOR_RELEASED;
				gCursorState.old = 3;
				break;
			case SDL_BUTTON_MIDDLE:
				gCursorState.middle = CURSOR_RELEASED;
				break;
			case SDL_BUTTON_RIGHT:
				store_mouse_input(4);
				gCursorState.right = CURSOR_RELEASED;
				gCursorState.old = 4;
				break;
			}
			break;
		case SDL_KEYDOWN:
			gLastKeyPressed = e.key.keysym.sym;
			gKeysPressed[e.key.keysym.scancode] = 1;
			if (e.key.keysym.sym == SDLK_RETURN && e.key.keysym.mod & KMOD_ALT)
				osinterface_set_fullscreen_mode(!gGeneral_config.fullscreen_mode);
			if (e.key.keysym.sym == SDLK_PRINTSCREEN){
				RCT2_GLOBAL(RCT2_ADDRESS_SCREENSHOT_COUNTDOWN, sint8) = 1;
				//this function is normally called only in-game (in game_update)
				//calling it here will save screenshots even while in main menu
				screenshot_check();
			}
			break;
		default:
			break;
		}
	}

	gCursorState.any = gCursorState.left | gCursorState.middle | gCursorState.right;

	// Updates the state of the keys
	int numKeys = 256;
	gKeysState = SDL_GetKeyboardState(&numKeys);
}

static void osinterface_close_window()
{
	if (_window != NULL)
		SDL_DestroyWindow(_window);
	if (_surface != NULL)
		SDL_FreeSurface(_surface);
	if (_palette != NULL)
		SDL_FreePalette(_palette);
	osinterface_unload_cursors();
}

void osinterface_free()
{
	free(gKeysPressed);

	osinterface_close_window();
	SDL_Quit();
}

void osinterface_set_fullscreen_mode(int mode){
	if (mode == gGeneral_config.fullscreen_mode)
		return;

	if (SDL_SetWindowFullscreen(_window, _fullscreen_modes[mode])){
		RCT2_ERROR("SDL_SetWindowFullscreen %s", SDL_GetError());
		exit(1);
	}
	//SDL automatically resizes the fullscreen window to the nearest allowed screen resolution
	//No need to call osinterface_resize() here, SDL_WINDOWEVENT_SIZE_CHANGED event will be triggered anyway

	gGeneral_config.fullscreen_mode = mode;

	config_save();
}

/**
 * 
 *  rct2: 0x00407978
 */
int osinterface_407978(rct2_install_info* install_info, char* source, char* font, uint8 charset)
{
	char subkey[MAX_PATH];
	char subkey2[MAX_PATH];
	strcpy(subkey, "Software\\Infogrames\\");
	strcat(subkey, source);
	strcpy(subkey2, "Software\\Fish Technology Group\\");
	strcat(subkey2, source);
	LOGFONTA lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfCharSet = charset;
	lf.lfHeight = 12;
	lf.lfWeight = 400;
	strcpy(lf.lfFaceName, font);
	RCT2_GLOBAL(RCT2_ADDRESS_HFONT, HFONT) = CreateFontIndirectA(&lf);
	HKEY hkey;
	if (RegOpenKeyA(HKEY_LOCAL_MACHINE, subkey, &hkey) != ERROR_SUCCESS && RegOpenKeyA(HKEY_LOCAL_MACHINE, subkey2, &hkey) != ERROR_SUCCESS) {
		return 0;
	} else {
		DWORD type;
		DWORD size = 260;
		RegQueryValueExA(hkey, "Title", 0, &type, install_info->title, &size);
		size = 260;
		RegQueryValueExA(hkey, "Path", 0, &type, install_info->path, &size);
		install_info->var_20C = 235960;
		size = 4;
		RegQueryValueExA(hkey, "InstallLevel", 0, &type, (LPBYTE)&install_info->installlevel, &size);
		for (int i = 0; i <= 15; i++) {
			char name[100];
			sprintf(name, "AddonPack%d", i);
			size = sizeof(install_info->addon[i]);
			if (RegQueryValueExA(hkey, name, 0, &type, install_info->addon[i], &size) == ERROR_SUCCESS) {
				install_info->addons |= (1 << i);
			}
		}
		RegCloseKey(hkey);
		return 1;
	}
}

/**
 * 
 *  rct2: 0x00407D80
 */
int osinterface_get_cursor_pos(int* x, int* y)
{
	POINT point;
	GetCursorPos(&point);
	*x = point.x;
	*y = point.y;
}

/**
 * 
 *  rct2: 0x00407E15
 */
int osinterface_print_window_message(UINT msg, WPARAM wparam, LPARAM lparam)
{
	const char* msgname = "NULL";
	// get the string representation of the msg id, from 190 different values in 0x009A61D8 - 0x009A8873
	// not going to bother reading those since this function is going to be unused and taken out anyways
	char temp[1024];
	sprintf(temp, "Message id = %s (%i), wParam = 0x%x, lParam = 0x%x\n", msgname, msg, wparam, lparam);
	OutputDebugStringA(temp);
	return 1;
}

/**
 * 
 *  rct2: 0x00407E6E
 */
int osinterface_progressbar_create(char* title, int a2)
{
	DWORD style = WS_VISIBLE | WS_BORDER | WS_DLGFRAME;
	if (a2) {
		style = WS_VISIBLE | WS_BORDER | WS_DLGFRAME | PBS_SMOOTH;
	}
	int width = 340;
	int height = GetSystemMetrics(SM_CYCAPTION) + 24;
	HWND hwnd = CreateWindowExA(WS_EX_TOPMOST | WS_EX_DLGMODALFRAME, "msctls_progress32", title, style, (RCT2_GLOBAL(0x01423C08, sint32) - width) / 2, (RCT2_GLOBAL(0x01423C0C, sint32) - height) / 2, width, height, 0, 0, RCT2_GLOBAL(RCT2_ADDRESS_HINSTANCE, HINSTANCE), 0);
	RCT2_GLOBAL(RCT2_ADDRESS_PROGRESSBAR_HWND, HWND) = hwnd;
	if (hwnd) {
		RCT2_GLOBAL(0x009E2DFC, uint32) = 1;
		if (RCT2_GLOBAL(RCT2_ADDRESS_HFONT, HFONT)) {
			SendMessageA(hwnd, WM_SETFONT, (WPARAM)RCT2_GLOBAL(RCT2_ADDRESS_HFONT, HFONT), 1);
		}
		SetWindowTextA(hwnd, title);
		osinterface_progressbar_setmax(0xFF);
		osinterface_progressbar_setpos(0);
		return 1;
	} else {
		return 0;
	}
}

/**
 * 
 *  rct2: 0x00407F16
 */
int osinterface_progressbar_destroy()
{
	if (DestroyWindow(RCT2_GLOBAL(RCT2_ADDRESS_PROGRESSBAR_HWND, HWND))) {
		RCT2_GLOBAL(0x009E2DFC, uint32) = 0;
		return 1;
	} else {
		return 0;
	}
}

/**
 * 
 *  rct2: 0x00407F2E
 */
void osinterface_progressbar_setmax(int max)
{
	SendMessageA(RCT2_GLOBAL(RCT2_ADDRESS_PROGRESSBAR_HWND, HWND), PBM_SETRANGE, MAKEWPARAM(0, max), 0);
	SendMessageA(RCT2_GLOBAL(RCT2_ADDRESS_PROGRESSBAR_HWND, HWND), PBM_SETSTEP, 1, 0);
}

/**
 * 
 *  rct2: 0x00407F60
 */
void osinterface_progressbar_setpos(int pos)
{
	SendMessageA(RCT2_GLOBAL(RCT2_ADDRESS_PROGRESSBAR_HWND, HWND), PBM_SETPOS, MAKEWPARAM(pos, 0), 0);
}

/**
 * 
 *  rct2: 0x00407F78
 */
int osinterface_file_seek_from_begin(HANDLE handle, int offset)
{
	return SetFilePointer(handle, offset, 0, FILE_BEGIN);
}

/**
 * 
 *  rct2: 0x00407F8B
 */
int osinterface_file_seek_from_current(HANDLE handle, int offset)
{
	return SetFilePointer(handle, offset, 0, FILE_CURRENT);
}

/**
 * 
 *  rct2: 0x00407F9E
 */
int osinterface_file_seek_from_end(HANDLE handle, int offset)
{
	return SetFilePointer(handle, offset, 0, FILE_END);
}

/**
 * 
 *  rct2: 0x00407FB1
 */
int osinterface_file_read(HANDLE handle, void* data, int size)
{
	DWORD read;
	BOOL result;
	if (size == -1) {
		DWORD current = SetFilePointer(handle, 0, 0, FILE_CURRENT);
		DWORD remaining = SetFilePointer(handle, 0, 0, FILE_END) - current;
		result = ReadFile(handle, data, remaining, &read, 0);
	} else {
		result = ReadFile(handle, data, size, &read, 0);
	}
	if (result) {
		return read;
	} else {
		return -1;
	}
}

/**
 * 
 *  rct2: 0x00408024
 */
int osinterface_file_write(HANDLE handle, const void* data, int size)
{
	DWORD written;
	if (WriteFile(handle, data, size, &written, 0)) {
		return written;
	} else {
		return -1;
	}
}

/**
 * 
 *  rct2: 0x0040804A
 */
int osinterface_file_close(HANDLE handle)
{
	if (handle) {
		return CloseHandle(handle);
	} else {
		return 1;
	}
}

/**
 * 
 *  rct2: 0x00408060
 */
HANDLE osinterface_file_open(const char* filename)
{
	return CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL, 0);
}

/**
 * 
 *  rct2: 0x0040807D
 */
HANDLE osinterface_file_create(const char* filename)
{
	return CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
}

/**
 * 
 *  rct2: 0x00408099
 */
int osinterface_file_move(const char* srcfilename, const char* dstfilename)
{
	return (MoveFileA(srcfilename, dstfilename) != 0) - 1;
}

/**
 * 
 *  rct2: 0x004080AF
 */
int osinterface_file_delete(const char* filename)
{
	return (DeleteFileA(filename) != 0) - 1;
}

/**
 * 
 *  rct2: 0x004080EA
 */
int osinterface_open_common_file_dialog(int type, char *title, char *filename, char *filterPattern, char *filterName)
{
	char initialDirectory[MAX_PATH], *dotAddress, *slashAddress;
	OPENFILENAME openFileName;
	BOOL result;
	int tmp;
	DWORD commonFlags;

	// Get directory path from given filename
	strcpy(initialDirectory, filename);
	dotAddress = strrchr(filename, '.');
	if (dotAddress != NULL) {
		slashAddress = strrchr(filename, '\\');
		if (slashAddress < dotAddress)
			*(slashAddress + 1) = 0;
	}

	// Clear filename
	*filename = 0;

	// Set open file name options
	memset(&openFileName, 0, sizeof(OPENFILENAME));
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = RCT2_GLOBAL(0x009E2D70, HWND);
	openFileName.lpstrFile = filename;
	openFileName.nMaxFile = MAX_PATH;
	openFileName.lpstrInitialDir = initialDirectory;
	openFileName.lpstrTitle = title;

	// Copy filter name
	strcpy((char*)0x01423800, filterName);

	// Copy filter pattern
	strcpy((char*)0x01423800 + strlen(filterName) + 1, filterPattern);
	*((char*)(0x01423800 + strlen(filterName) + 1 + strlen(filterPattern) + 1)) = 0;
	openFileName.lpstrFilter = (char*)0x01423800;

	// 
	tmp = RCT2_GLOBAL(0x009E2C74, uint32);
	if (RCT2_GLOBAL(0x009E2BB8, uint32) == 2 && RCT2_GLOBAL(0x009E1AF8, uint32) == 1)
		RCT2_GLOBAL(0x009E2C74, uint32) = 1;

	// Open dialog
	commonFlags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	if (type == 0) {
		openFileName.Flags = commonFlags | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
		result = GetSaveFileName(&openFileName);
	} else if (type == 1) {
		openFileName.Flags = commonFlags | OFN_NONETWORKBUTTON | OFN_FILEMUSTEXIST;
		result = GetOpenFileName(&openFileName);
	}

	// 
	RCT2_GLOBAL(0x009E2C74, uint32) = tmp;

	return result;
}

void osinterface_show_messagebox(char* message){
	MessageBox(NULL, message, "OpenRCT2", MB_OK);
}

char* osinterface_open_directory_browser(char *title) {
	BROWSEINFO      bi;
	char            pszBuffer[MAX_PATH];
	LPITEMIDLIST    pidl;
	LPMALLOC        lpMalloc;

	// Initialize COM
	if (FAILED(CoInitializeEx(0, COINIT_APARTMENTTHREADED))) {
		MessageBox(NULL, _T("Error opening browse window"), _T("ERROR"), MB_OK);
		CoUninitialize();
		return 0;
	}

	// Get a pointer to the shell memory allocator
	if (FAILED(SHGetMalloc(&lpMalloc))) {
		MessageBox(NULL, _T("Error opening browse window"), _T("ERROR"), MB_OK);
		CoUninitialize();
		return 0;
	}

	bi.hwndOwner = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = pszBuffer;
	bi.lpszTitle = _T(title);
	bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;

	char *outPath = "C:\\";

	if (pidl = SHBrowseForFolder(&bi)) {
		// Copy the path directory to the buffer
		if (SHGetPathFromIDList(pidl, pszBuffer)) {
			// Store pszBuffer (and the path) in the outPath
			outPath = (char*) malloc(strlen(pszBuffer)+1);
			strcpy(outPath, pszBuffer);
		}
	}
	CoUninitialize();
	return outPath;
}

char* osinterface_get_orct2_homefolder()
{
	char *path=NULL;
	path = malloc(sizeof(char) * MAX_PATH);
	if (path == NULL){
		osinterface_show_messagebox("Error allocating memory!");
		exit(EXIT_FAILURE);
	}

	path[0] = '\0';

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, path)))
		strcat(path, "\\OpenRCT2");

	return path;
}

char *osinterface_get_orct2_homesubfolder(const char *subFolder)
{
	char *path = osinterface_get_orct2_homefolder();
	strcat(path, "\\");
	strcat(path, subFolder);
	return path;
}

int osinterface_file_exists(const char *path)
{
	return !(GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND);
}

int osinterface_directory_exists(const char *path)
{
	DWORD dwAttrib = GetFileAttributes(path);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

int osinterface_ensure_directory_exists(const char *path)
{
	if (osinterface_directory_exists(path))
		return 1;

	return CreateDirectory(path, NULL);
}

char osinterface_get_path_separator()
{
	return '\\';
}
