/* -*- Mode: C++; c-basic-offset: 2; tab-width: 2; indent-tabs-mode: nil -*-
 * 
 * Quadra, an action puzzle game
 * Copyright (C) 1998-2000  Ludus Design
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdint.h>
#include <stdio.h>

#include "types.h"
#include "video.h"
#include "palette.h"

Palette noir;

void Palette::set() {
  video->dosetpal(pal, size);
}

void Palette::load(const Image& raw) {
	size=raw.palettesize();
	int j=0;
	for(int i(0); i<size; i++) {
		pal[i].peRed = raw.pal()[j++];
		pal[i].peGreen = raw.pal()[j++];
		pal[i].peBlue = raw.pal()[j++];
	}
}

Remap::Remap(const Palette& d, Palette* src): dst(d) {
	if(src) {
		for(int i=0; i<src->size; i++)
			findrgb((uint8_t) i, src->r((uint8_t) i), src->g((uint8_t) i), src->b((uint8_t) i));
	}
}

void Remap::findrgb(const uint8_t m, const uint8_t r, const uint8_t g, const uint8_t b) {
	int best_diff=9999999, diff;
	uint8_t best_i=0;
	for(int i=1; i<dst.size; i++) {
		diff=(int) ((dst.pal[i].peRed-r)*(dst.pal[i].peRed-r)*2 + (dst.pal[i].peGreen-g)*(dst.pal[i].peGreen-g)*3 + (dst.pal[i].peBlue-b)*(dst.pal[i].peBlue-b));
		if(diff == 0) {
			map[m] = (uint8_t) i;
			return;
		}
		if(diff < best_diff) {
			best_i = (uint8_t) i;
			best_diff = diff;
		}
	}
	map[m] = best_i;
}

Fade::Fade(const Palette& dst, const Palette& src, int frame) {
	int j=0;
	for(int i(0); i<256; i++) {
		current[j++]=(short) (src.pal[i].peRed<<7);
		current[j++]=(short) (src.pal[i].peGreen<<7);
		current[j++]=(short) (src.pal[i].peBlue<<7);
	}
	newdest(dst, frame);
}

void Fade::setdest(const Palette& dst) {
	dest=dst;
	int j=0;
	for(int i(0); i<256; i++) {
		current[j++]=(short) (dest.pal[i].peRed<<7);
		current[j++]=(short) (dest.pal[i].peGreen<<7);
		current[j++]=(short) (dest.pal[i].peBlue<<7);
	}
	video->setpal(dest);
	currentframe=destframe;
}

void Fade::newdest(const Palette& dst, int frame) {
	dest=dst;
	int j=0;
	for(int i(0); i<256; i++) {
		delta[j]=(short) (((dest.pal[i].peRed<<7)-current[j++])/frame);
		delta[j]=(short) (((dest.pal[i].peGreen<<7)-current[j++])/frame);
		delta[j]=(short) (((dest.pal[i].peBlue<<7)-current[j++])/frame);
	}
	currentframe=0;
	destframe=frame;
}

int Fade::step() {
	if(currentframe==destframe)
		return 1;
	else {
		for(int i(0); i<768; i++)
			current[i] = (short) (current[i] + delta[i]);
		currentframe++;
		return 0;
	}
}

void Fade::set() {
	if(currentframe==destframe)
		return;
	if(currentframe==destframe-1) {
		video->setpal(dest);
	} else {
		video->pal.set_size(256);
		for(int i(0); i<256; i++)
			video->pal.setcolor((uint8_t) i, (uint8_t) (current[i*3]>>7), (uint8_t) (current[i*3+1]>>7), (uint8_t) (current[i*3+2]>>7));
		video->newpal = true;
	}
}
