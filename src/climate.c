/*****************************************************************************
 * Copyright (c) 2014 Ted John, Matthias Lanzinger
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
#include "audio.h"
#include "climate.h"
#include "date.h"
#include "gfx.h"
#include "rct2.h"
#include "scenario.h"

enum {
	THUNDER_STATUS_NULL = 0,
	THUNDER_STATUS_PLAYING = 1,

	MAX_THUNDER_INSTANCES = 2
};

typedef struct {
	sint8 base_temperature;
	sint8 distribution_size;
	sint8 distribution[24];
} rct_weather_transition;

int gClimateNextWeather;

static int _climateCurrentWeatherEffect;

static int _climateNextTemperature;
static int _climateNextWeatherEffect;
static int _climateNextWeatherGloom;
static int _climateNextRainLevel;

static const rct_weather_transition* climate_transitions[4];

// Sound data
static int _rainVolume = 1;
static rct_sound _rainSoundInstance;
static unsigned int _lightningTimer, _thunderTimer;
static rct_sound _thunderSoundInstance[MAX_THUNDER_INSTANCES];
static int _thunderStatus[MAX_THUNDER_INSTANCES] = { THUNDER_STATUS_NULL, THUNDER_STATUS_NULL };
static unsigned int _thunderSoundId;
static int _thunderVolume;
static int _thunderStereoEcho = 0;

static void climate_determine_future_weather();

static void climate_update_rain_sound();
static void climate_update_thunder_sound();
static void climate_update_lightning();
static void climate_update_thunder();
static int climate_play_thunder(int instanceIndex, int soundId, int volume, int pan);

int climate_celsius_to_fahrenheit(int celsius)
{
	return (celsius * 29) / 16 + 32;
}

/**
 *  Set climate and determine start weather.
 *  rct2: 0x006C45ED
 */
void climate_reset(int climate)
{
	RCT2_GLOBAL(RCT2_ADDRESS_CLIMATE, sint8) = climate;
	climate_determine_future_weather();
}

sint8 step_weather_level(sint8 cur_weather_level, sint8 next_weather_level) {
	if (next_weather_level > cur_weather_level) {
		return cur_weather_level + 1;
	} else {
		return cur_weather_level - 1;
	}
}


//for cheats
void toggle_climate_lock()
{
	g_climate_locked = !g_climate_locked;
}

/**
 * Weather & climate update iteration.
 * Gradually changes the weather parameters towards their determined next values.
 * 
 * rct2: 0x006C46B1
 */
void climate_update()
{
	uint8 screen_flags = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8);
	sint8 temperature = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TEMPERATURE, sint8),
		target_temperature = _climateNextTemperature,
		cur_gloom = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WEATHER_GLOOM, sint8),
		next_gloom = _climateNextWeatherGloom,
		cur_rain = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_RAIN_LEVEL, sint8),
		next_rain = _climateNextRainLevel;

	if (g_climate_locked) //for cheats
		return;

	if (screen_flags & (~SCREEN_FLAGS_PLAYING)) // only normal play mode gets climate
		return;

	if (RCT2_GLOBAL(RCT2_ADDRESS_CLIMATE_UPDATE_TIMER, sint16))	{

		if (RCT2_GLOBAL(RCT2_ADDRESS_CLIMATE_UPDATE_TIMER, sint16) == 960)
			RCT2_GLOBAL(0x009A9804, uint32) |= 8; // climate dirty flag?

		RCT2_GLOBAL(RCT2_ADDRESS_CLIMATE_UPDATE_TIMER, sint16)--;

	} else if (!(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, sint32) & 0x7F)) {
		
		if (temperature == target_temperature) {
			if (cur_gloom == next_gloom) {
				_climateCurrentWeatherEffect = _climateNextWeatherEffect;

				if (cur_rain == next_rain) {
					RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WEATHER, sint8) = gClimateNextWeather;
					climate_determine_future_weather();
					RCT2_GLOBAL(0x009A9804, uint32) |= 8; // climate dirty flag?
				} else if (next_rain <= 2) { // Safe-guard
					RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_RAIN_LEVEL, sint8) = step_weather_level(cur_rain, next_rain);
				}
			} else {
				RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WEATHER_GLOOM, sint8) = step_weather_level(cur_gloom, next_gloom);
				gfx_invalidate_screen();
			}

		} else {
			RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TEMPERATURE, sint8) = step_weather_level(temperature, target_temperature);
			RCT2_GLOBAL(0x009A9804, uint32) |= 8; // climate dirty flag?
		}
	}
}


/**
 * Calculates future weather development.
 * RCT2 implements this as discrete probability distributions dependant on month and climate 
 * for next_weather. The other weather parameters are then looked up depending only on the
 * next weather.
 *
 * rct2: 0x006C461C
 */
static void climate_determine_future_weather()
{
	sint8 climate = RCT2_GLOBAL(RCT2_ADDRESS_CLIMATE, sint8);
	const rct_weather_transition* climate_table = climate_transitions[climate];
	sint8 month = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_YEAR, sint16) & 7;
	rct_weather_transition transition = climate_table[month];
	
	// Generate a random variable with values 0 upto distribution_size-1 and chose weather from the distribution table accordingly
	sint8 next_weather = transition.distribution[ ((rand() & 0xFF) * transition.distribution_size) >> 8 ];
	gClimateNextWeather = next_weather;

	_climateNextTemperature = transition.base_temperature + climate_weather_data[next_weather].temp_delta;
	_climateNextWeatherEffect = climate_weather_data[next_weather].effect_level;
	_climateNextWeatherGloom = climate_weather_data[next_weather].gloom_level;
	_climateNextRainLevel = climate_weather_data[next_weather].rain_level;
	
	RCT2_GLOBAL(RCT2_ADDRESS_CLIMATE_UPDATE_TIMER, sint16) = 1920;
}

/**
 * 
 * rct2: 0x006BCB91
 */
void climate_update_sound()
{
	if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_SOUND_DEVICE, uint32) == 0xFFFFFFFF)
		return;
	if (RCT2_GLOBAL(0x009AF59C, uint8) != 0)
		return;
	if (!(RCT2_GLOBAL(0x009AF59D, uint8) & 1))
		return;
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & 1)
		return;

	climate_update_rain_sound();
	climate_update_thunder_sound();
}

static void climate_update_rain_sound()
{
	if (_climateCurrentWeatherEffect == 1 || _climateCurrentWeatherEffect == 2) {
		if (_rainVolume == 1) {
			// Start playing the rain sound
			if (sound_prepare(SOUND_RAIN_1, &_rainSoundInstance, 1, RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_SOUND_SW_BUFFER, uint32)))
				sound_play(&_rainSoundInstance, 1, -4000, 0, 0);
			_rainVolume = -4000;
		} else {
			// Increase rain sound
			_rainVolume = min(-1400, _rainVolume + 80);
			sound_set_volume(&_rainSoundInstance, _rainVolume);
		}
	} else if (_rainVolume != 1) {
		// Decrease rain sound
		_rainVolume -= 80;
		if (_rainVolume > -4000) {
			sound_set_volume(&_rainSoundInstance, _rainVolume);
		} else {
			sound_stop(&_rainSoundInstance);
			_rainVolume = 1;
		}
	}
}

static void climate_update_thunder_sound()
{
	if (_thunderStereoEcho) {
		// Play thunder on right side
		_thunderStereoEcho = 0;
		climate_play_thunder(1, _thunderSoundId, _thunderVolume, 10000);
	} else if (_thunderTimer != 0) {
		climate_update_lightning();
		climate_update_thunder();
	} else if (_climateCurrentWeatherEffect == 2) {
		// Create new thunder and lightning
		unsigned int randomNumber = scenario_rand();
		if ((randomNumber & 0xFFFF) <= 0x1B4) {
			randomNumber >>= 16;
			_thunderTimer = 43 + (randomNumber % 64);
			_lightningTimer = randomNumber % 32;
		}
	}

	// Stop thunder sounds if they have finished
	for (int i = 0; i < MAX_THUNDER_INSTANCES; i++) {
		if (_thunderStatus[i] == THUNDER_STATUS_NULL)
			continue;

		if (!sound_is_playing(&_thunderSoundInstance[i])) {
			sound_stop(&_thunderSoundInstance[i]);
			_thunderStatus[i] = THUNDER_STATUS_NULL;
		}
	}
}

static void climate_update_lightning()
{
	if (_lightningTimer == 0)
		return;

	_lightningTimer--;
	if (RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint16) == 0)
		if ((scenario_rand() & 0xFFFF) <= 0x2000)
			RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint16) = 1;
}

static void climate_update_thunder()
{
	_thunderTimer--;
	if (_thunderTimer != 0)
		return;

	unsigned int randomNumber = scenario_rand();
	if (randomNumber & 0x10000) {
		if (_thunderStatus[0] == THUNDER_STATUS_NULL && _thunderStatus[1] == THUNDER_STATUS_NULL) {
			// Play thunder on left side
			_thunderSoundId = (randomNumber & 0x20000) ? SOUND_THUNDER_1 : SOUND_THUNDER_2;
			_thunderVolume = (-((int)((randomNumber >> 18) & 0xFF))) << 3;
			climate_play_thunder(0, _thunderSoundId, _thunderVolume, -10000);

			// Let thunder play on right side
			_thunderStereoEcho = 1;
		}
	} else {
		if (_thunderStatus[0] == THUNDER_STATUS_NULL){
			_thunderSoundId = (randomNumber & 0x20000) ? SOUND_THUNDER_1 : SOUND_THUNDER_2;
			int pan = (((randomNumber >> 18) & 0xFF) - 128) * 16;
			climate_play_thunder(0, _thunderSoundId, 0, pan);
		}
	}
}

static int climate_play_thunder(int instanceIndex, int soundId, int volume, int pan)
{
	if (sound_prepare(soundId, &_thunderSoundInstance[instanceIndex], 1, RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_SOUND_SW_BUFFER, uint32))) {
		sound_play(&_thunderSoundInstance[instanceIndex], 0, volume, pan, 0);

		_thunderStatus[instanceIndex] = THUNDER_STATUS_PLAYING;
		return 1;
	}

	return 0;
}

#pragma region Climate / Weather data tables

// rct2: 0x00993C94
// There is actually a sprite at 0x5A9C for snow but only these weather types seem to be fully implemented
const rct_weather climate_weather_data[6] = {
	{ .temp_delta = 10, .effect_level = 0, .gloom_level = 0, .rain_level = 0, .sprite_id = 0x5A96 }, // Sunny
	{ .temp_delta = 5, .effect_level = 0, .gloom_level = 0, .rain_level = 0, .sprite_id = 0x5A97 }, // Partially Cloudy
	{ .temp_delta = 0, .effect_level = 0, .gloom_level = 0, .rain_level = 0, .sprite_id = 0x5A98 }, // Cloudy
	{ .temp_delta = -2, .effect_level = 1, .gloom_level = 1, .rain_level = 1, .sprite_id = 0x5A99 }, // Rain
	{ .temp_delta = -4, .effect_level = 1, .gloom_level = 2, .rain_level = 2, .sprite_id = 0x5A9A }, // Heavy Rain
	{ .temp_delta = 2, .effect_level = 2, .gloom_level = 2, .rain_level = 2, .sprite_id = 0x5A9B }, // Thunderstorm
};


// rct2: 00993998
static const rct_weather_transition climate_cool_and_wet_transitions[] = {
	{ .base_temperature = 8, .distribution_size = 18,
	.distribution = { 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 10, .distribution_size = 21,
	.distribution = { 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 0, 0 } },
	{ .base_temperature = 14, .distribution_size = 17,
	.distribution = { 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 17, .distribution_size = 17,
	.distribution = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 19, .distribution_size = 23,
	.distribution = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 4 } },
	{ .base_temperature = 20, .distribution_size = 23,
	.distribution = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 4, 4, 5 } },
	{ .base_temperature = 16, .distribution_size = 19,
	.distribution = { 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 4, 4, 5, 0, 0, 0, 0 } },
	{ .base_temperature = 13, .distribution_size = 16,
	.distribution = { 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0 } }
};
static const rct_weather_transition climate_warm_transitions[] = {
	{ .base_temperature = 12, .distribution_size = 21,
	.distribution = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 4, 0, 0 } },
	{ .base_temperature = 13, .distribution_size = 22,
	.distribution = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 5, 0 } },
	{ .base_temperature = 16, .distribution_size = 17,
	.distribution = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 19, .distribution_size = 18,
	.distribution = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 21, .distribution_size = 22,
	.distribution = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 0 } },
	{ .base_temperature = 22, .distribution_size = 17,
	.distribution = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 5, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 19, .distribution_size = 17,
	.distribution = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 16, .distribution_size = 17,
	.distribution = { 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 0, 0, 0, 0, 0, 0 } }
};
static const rct_weather_transition climate_hot_and_dry_transitions[] = {
	{ .base_temperature = 12, .distribution_size = 15,
	.distribution = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 14, .distribution_size = 12,
	.distribution = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 16, .distribution_size = 11,
	.distribution = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 19, .distribution_size = 9,
	.distribution = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 21, .distribution_size = 13,
	.distribution = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 22, .distribution_size = 11,
	.distribution = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 21, .distribution_size = 12,
	.distribution = { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 16, .distribution_size = 13,
	.distribution = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};
static const rct_weather_transition climate_cold_transitions[] = {
	{ .base_temperature = 4, .distribution_size = 18,
	.distribution = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 4, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 5, .distribution_size = 21,
	.distribution = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 0, 0 } },
	{ .base_temperature = 7, .distribution_size = 17,
	.distribution = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 9, .distribution_size = 17,
	.distribution = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 0, 0, 0, 0, 0, 0 } },
	{ .base_temperature = 10, .distribution_size = 23,
	.distribution = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 4 } },
	{ .base_temperature = 11, .distribution_size = 23,
	.distribution = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 5 } },
	{ .base_temperature = 9, .distribution_size = 19,
	.distribution = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 4, 5, 0, 0, 0, 0 } },
	{ .base_temperature = 6, .distribution_size = 16,
	.distribution = { 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0 } }
};

static const rct_weather_transition* climate_transitions[] = {
	climate_cool_and_wet_transitions,
	climate_warm_transitions,
	climate_hot_and_dry_transitions,
	climate_cold_transitions
};

#pragma endregion