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

#include <math.h>
#include <SDL.h>
#include <string.h>

extern "C" {
#include "audio.h"
#include "config.h"
}
#include "mixer.h"

Mixer gMixer;

Sample::Sample()
{
	data = 0;
	length = 0;
	issdlwav = false;
}

Sample::~Sample()
{
	Unload();
}

bool Sample::Load(const char* filename)
{
	Unload();
	SDL_RWops* rw = SDL_RWFromFile(filename, "rb");
	if (!rw) {
		SDL_RWclose(rw);
		return false;
	}
	SDL_AudioSpec audiospec;
	memset(&audiospec, 0, sizeof(audiospec));
	SDL_AudioSpec* spec = SDL_LoadWAV_RW(rw, false, &audiospec, &data, (Uint32*)&length);
	if (spec != NULL) {
		format.freq = spec->freq;
		format.format = spec->format;
		format.channels = spec->channels;
		issdlwav = true;
	} else {
		return false;
	}
	return true;
}

bool Sample::LoadCSS1(const char* filename, unsigned int offset)
{
	Unload();
	SDL_RWops* rw = SDL_RWFromFile(filename, "rb");
	if (!rw) {
		return false;
	}
	Uint32 numsounds;
	SDL_RWread(rw, &numsounds, sizeof(numsounds), 1);
	if (offset > numsounds) {
		SDL_RWclose(rw);
		return false;
	}
	SDL_RWseek(rw, offset * 4, RW_SEEK_CUR);
	Uint32 soundoffset;
	SDL_RWread(rw, &soundoffset, sizeof(soundoffset), 1);
	SDL_RWseek(rw, soundoffset, RW_SEEK_SET);
	Uint32 soundsize;
	SDL_RWread(rw, &soundsize, sizeof(soundsize), 1);
	length = soundsize;
	WAVEFORMATEX waveformat;
	SDL_RWread(rw, &waveformat, sizeof(waveformat), 1);
	format.freq = waveformat.nSamplesPerSec;
	format.format = AUDIO_S16LSB;
	format.channels = waveformat.nChannels;
	data = new uint8[length];
	SDL_RWread(rw, data, length, 1);
	SDL_RWclose(rw);
	return true;
}

void Sample::Unload()
{
	if (data) {
		if (issdlwav) {
			SDL_FreeWAV(data);
		} else {
			delete[] data;
		}
		data = 0;
	}
	issdlwav = false;
	length = 0;
}

bool Sample::Convert(AudioFormat format)
{
	if(Sample::format.format != format.format || Sample::format.channels != format.channels || Sample::format.freq != format.freq){
		SDL_AudioCVT cvt;
		if (SDL_BuildAudioCVT(&cvt, Sample::format.format, Sample::format.channels, Sample::format.freq, format.format, format.channels, format.freq) < 0) {
			return false;
		}
		cvt.len = length;
		cvt.buf = (Uint8*)new uint8[cvt.len * cvt.len_mult];
		memcpy(cvt.buf, data, length);
		if (SDL_ConvertAudio(&cvt) < 0) {
			delete[] cvt.buf;
			return false;
		}
		Unload();
		data = cvt.buf;
		length = cvt.len_cvt;
		Sample::format = format;
	}
	return true;
}

const uint8* Sample::Data()
{
	return data;
}

unsigned long Sample::Length()
{
	return length;
}

Stream::Stream()
{
	sourcetype = SOURCE_NONE;
}

unsigned long Stream::GetSome(unsigned long offset, const uint8** data, unsigned long length)
{
	unsigned long size = length;
	switch(sourcetype) {
		case SOURCE_SAMPLE:
			if (offset >= sample->Length()) {
				return 0;
			}
			if (offset + length > sample->Length()) {
				size = sample->Length() - offset;
			}
			*data = &sample->Data()[offset];
			return size;
		break;
	}
	return 0;
}

unsigned long Stream::Length()
{
	switch(sourcetype) {
		case SOURCE_SAMPLE:
			return sample->Length();
		break;
	}
	return 0;
}

void Stream::SetSource_Sample(Sample& sample)
{
	sourcetype = SOURCE_SAMPLE;
	Stream::sample = &sample;
}

const AudioFormat* Stream::Format()
{
	switch(sourcetype) {
		case SOURCE_SAMPLE:
			return &sample->format;
		break;
	}
	return 0;
}

Channel::Channel()
{
	rate = 1;
	resampler = 0;
	SetVolume(SDL_MIX_MAXVOLUME);
}

Channel::~Channel()
{
	if (resampler) {
		speex_resampler_destroy(resampler);
		resampler = 0;
	}
}

void Channel::Play(Stream& stream, int loop = MIXER_LOOP_NONE)
{
	Channel::stream = &stream;
	Channel::loop = loop;
	offset = 0;
}

void Channel::SetRate(double rate)
{
	Channel::rate = rate;
	if (Channel::rate < 0.001) {
		Channel::rate = 0.001;
	}
}

void Channel::SetVolume(int volume)
{
	Channel::volume = volume;
	if (volume > SDL_MIX_MAXVOLUME) {
		Channel::volume = SDL_MIX_MAXVOLUME;
	}
	if (volume < 0) {
		Channel::volume = 0;
	}
}

void Channel::SetPan(float pan)
{
	Channel::pan = pan;
	if (pan > 1) {
		Channel::pan = 1;
	}
	if (pan < 0) {
		Channel::pan = 0;
	}
	volume_l = (float)sin((1.0 - Channel::pan) * M_PI / 2.0);
	volume_r = (float)sin(Channel::pan * M_PI / 2.0);
}

void Mixer::Init(const char* device)
{
	Close();
	SDL_AudioSpec want, have;
	SDL_zero(want);
	want.freq = 44100;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = 1024;
	want.callback = Callback;
	want.userdata = this;
	deviceid = SDL_OpenAudioDevice(device, 0, &want, &have, 0);
	format.format = have.format;
	format.channels = have.channels;
	format.freq = have.freq;
	const char* filename = get_file_path(PATH_ID_CSS1);
	for (int i = 0; i < SOUND_MAXID; i++) {
		css1samples[i].LoadCSS1(filename, i);
		css1samples[i].Convert(format); // convert to audio output format, saves some cpu usage but requires a bit more memory, optional
		css1streams[i].SetSource_Sample(css1samples[i]);
	}
	effectbuffer = new uint8[(have.samples * format.BytesPerSample() * format.channels) + 200];
	SDL_PauseAudioDevice(deviceid, 0);
}

void Mixer::Close()
{
	SDL_CloseAudioDevice(deviceid);
	delete[] effectbuffer;
}

void SDLCALL Mixer::Callback(void* arg, uint8* stream, int length)
{
	Mixer* mixer = (Mixer*)arg;
	memset(stream, 0, length);
	for (int i = 0; i < 10; i++) {
		mixer->MixChannel(mixer->channels[i], stream, length);
	}
}

void Mixer::MixChannel(Channel& channel, uint8* data, int length)
{
	if (channel.stream) {
		if (!channel.resampler) {
			channel.resampler = speex_resampler_init(format.channels, format.freq, format.freq, 0, 0);
		}
		AudioFormat channelformat = *channel.stream->Format();
		int loaded = 0;
		SDL_AudioCVT cvt;
		cvt.len_ratio = 1;
		do {
			int samplesize = format.channels * format.BytesPerSample();
			int samples = length / samplesize;
			int samplesloaded = loaded / samplesize;
			int samplestoread = (int)ceil((samples - samplesloaded) * channel.rate);
			int lengthloaded = 0;
			if (channel.offset < channel.stream->Length()) {
				bool mustconvert = false;
				if (MustConvert(*channel.stream)) {
					if (SDL_BuildAudioCVT(&cvt, channelformat.format, channelformat.channels, channelformat.freq, Mixer::format.format, Mixer::format.channels, Mixer::format.freq) == -1) {
						break;
					}
					mustconvert = true;
				}

				const uint8* datastream = 0;
				int readfromstream = (channel.stream->GetSome(channel.offset, &datastream, (int)(((samplestoread) * samplesize) / cvt.len_ratio)) / channelformat.BytesPerSample()) * channelformat.BytesPerSample();
				if (readfromstream == 0) {
					break;
				}

				int volume = channel.volume;
				uint8* dataconverted = 0;
				const uint8* tomix = 0;

				if (mustconvert) {
					if (Convert(cvt, datastream, readfromstream, &dataconverted)) {
						tomix = dataconverted;
						lengthloaded = (cvt.len_cvt / samplesize) * samplesize;
					} else {
						break;
					}
				} else {
					tomix = datastream;
					lengthloaded = readfromstream;
				}

				bool effectbufferloaded = false;

				if (channel.rate != 1 && format.format == AUDIO_S16SYS) {
					int in_len = (int)(ceil((double)lengthloaded / samplesize));
					int out_len = samples + 20; // needs some extra, otherwise resampler sometimes doesn't process all the input samples
					speex_resampler_set_rate(channel.resampler, format.freq, (int)(format.freq * (1 / channel.rate)));
					speex_resampler_process_interleaved_int(channel.resampler, (const spx_int16_t*)tomix, (spx_uint32_t*)&in_len, (spx_int16_t*)effectbuffer, (spx_uint32_t*)&out_len);
					effectbufferloaded = true;
					tomix = effectbuffer;
					lengthloaded = (out_len * samplesize);
				}

				if (channel.pan != 0.5f && format.channels == 2) {
					if (!effectbufferloaded) {
						memcpy(effectbuffer, tomix, lengthloaded);
						effectbufferloaded = true;
						tomix = effectbuffer;
					}
					switch (format.format) {
						case AUDIO_S16SYS:
							EffectPanS16(channel, (sint16*)effectbuffer, lengthloaded / samplesize);
							break;
						case AUDIO_U8:
							EffectPanU8(channel, (uint8*)effectbuffer, lengthloaded / samplesize);
						break;
					}
				}

				int mixlength = lengthloaded;
				if (loaded + mixlength > length) {
					mixlength = length - loaded;
				}

				SDL_MixAudioFormat(&data[loaded], tomix, format.format, mixlength, volume);

				if (dataconverted) {
					delete[] dataconverted;
				}

				channel.offset += readfromstream;

			}

			loaded += lengthloaded;

			if (channel.loop != 0 && channel.offset >= channel.stream->Length()) {
				if (channel.loop != -1) {
					channel.loop--;
				}
				channel.offset = 0;
			}
		} while(loaded < length && channel.loop != 0);
	}
}

void Mixer::EffectPanS16(Channel& channel, sint16* data, int length)
{
	float left = channel.volume_l;
	float right = channel.volume_r;
	for (int i = 0; i < length * 2; i += 2) {
		data[i] = (sint16)(data[i] * left);
		data[i + 1] = (sint16)(data[i + 1] * right);
	}
}

void Mixer::EffectPanU8(Channel& channel, uint8* data, int length)
{
	float left = channel.volume_l;
	float right = channel.volume_r;
	for (int i = 0; i < length * 2; i += 2) {
		data[i] = (uint8)(data[i] * left);
		data[i + 1] = (uint8)(data[i + 1] * right);
	}
}

bool Mixer::MustConvert(Stream& stream)
{
	const AudioFormat* streamformat = stream.Format();
	if (!streamformat) {
		return false;
	}
	if (streamformat->format != format.format || streamformat->channels != format.channels || streamformat->freq != format.freq) {
		return true;
	}
	return false;
}

bool Mixer::Convert(SDL_AudioCVT& cvt, const uint8* data, unsigned long length, uint8** dataout)
{
	if (length == 0 || cvt.len_mult == 0) {
		return false;
	}
	cvt.len = length;
	cvt.buf = (Uint8*)new uint8[cvt.len * cvt.len_mult];
	memcpy(cvt.buf, data, length);
	if (SDL_ConvertAudio(&cvt) < 0) {
		delete[] cvt.buf;
		return false;
	}
	*dataout = cvt.buf;
	return true;
}

void Mixer_Init(const char* device)
{
	gMixer.Init(device);
}