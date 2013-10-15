/*
 * Copyright (C) 2012 Alexandr Vodiannikov aka "Aleksoid1978" (Aleksoid1978@mail.ru)
 *
 * This file is part of MPC-BE.
 *
 * MPC-BE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-BE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "DSUtil.h"
#include "GolombBuffer.h"

static const byte pixel_aspect[17][2]= {
	{0, 1},
	{1, 1},
	{12, 11},
	{10, 11},
	{16, 11},
	{40, 33},
	{24, 11},
	{20, 11},
	{32, 11},
	{80, 33},
	{18, 11},
	{15, 11},
	{64, 33},
	{160,99},
	{4, 3},
	{3, 2},
	{2, 1},
};

struct avc_hdr
{
	BYTE profile, level;
	unsigned int width, height;
	unsigned int crop_left, crop_right, crop_top, crop_bottom;
	UINT64 chroma_format_idc;
	BYTE interlaced;
	__int64 AvgTimePerFrame;

	AV_Rational sar;
};

struct hevc_hdr
{
	DWORD fourcc;
	unsigned int width, height;

	DWORD nal_length_size;
	DWORD profile, level;

	SIZE sar;
};

bool ParseDiracHeader(CGolombBuffer gb, unsigned* width, unsigned* height, REFERENCE_TIME* AvgTimePerFrame);
bool ParseAVCHeader(CGolombBuffer gb, avc_hdr& h, bool fullscan = false);

bool ParseHEVCHeader(BYTE* headerData, int headerSize, hevc_hdr& h);
void CreateHEVCSequenceHeader(BYTE* headerData, int headerSize, DWORD* dwSequenceHeader, DWORD& dwSequenceSize);

////

struct vc_params_t {
	DWORD width, height;

	DWORD nal_length_size;
	DWORD profile, level;

	void clear() {
		memset(this, 0, sizeof(*this));
	}
};

bool ParseHEVCDecoderConfigurationRecord(BYTE* data, int size, vc_params_t& params);
