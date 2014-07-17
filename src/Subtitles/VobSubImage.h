/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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

#include <atlcoll.h>

struct COutline {
	CAtlArray<CPoint> pa;
	CAtlArray<int> da;
	void RemoveAll() {
		pa.RemoveAll();
		da.RemoveAll();
	}
	void Add(CPoint p, int d) {
		pa.Add(p);
		da.Add(d);
	}
};

class CVobSubImage
{
	friend class CVobSubFile;

private:
	CSize org;
	RGBQUAD* lpTemp1;
	RGBQUAD* lpTemp2;

	WORD nOffset[2], nPlane;
	bool fCustomPal;
	char fAligned; // we are also using this for calculations, that's why it is char instead of bool...
	int tridx;
	RGBQUAD* orgpal /*[16]*/,* cuspal /*[4]*/;

	bool Alloc(int w, int h);
	void Free();

	BYTE GetNibble(const BYTE* lpData);
	void DrawPixels(CPoint p, int length, int colorid);
	void TrimSubImage();

public:
	int iLang, iIdx;
	bool fForced, bAnimated;
	int tCurrent;
	__int64 start, delay;
	CRect rect;
	struct SubPal {
		BYTE pal: 4, tr: 4;
	};
	SubPal pal[4];
	RGBQUAD* lpPixels;

	CVobSubImage();
	virtual ~CVobSubImage();

	void Invalidate() { iLang = iIdx = -1; }

	void GetPacketInfo(const BYTE* lpData, int packetsize, int datasize, int t = INT_MAX);
	bool Decode(BYTE* lpData, int packetsize, int datasize, int t,
				bool fCustomPal,
				int tridx,
				RGBQUAD* orgpal /*[16]*/, RGBQUAD* cuspal /*[4]*/,
				bool fTrim);

	/////////

private:
	CAutoPtrList<COutline>* GetOutlineList(CPoint& topleft);
	int GrabSegment(int start, const COutline& o, COutline& ret);
	void SplitOutline(const COutline& o, COutline& o1, COutline& o2);
	void AddSegment(COutline& o, CAtlArray<BYTE>& pathTypes, CAtlArray<CPoint>& pathPoints);

public:
	bool Polygonize(CAtlArray<BYTE>& pathTypes, CAtlArray<CPoint>& pathPoints, bool fSmooth, int scale);
	bool Polygonize(CStringW& assstr, bool fSmooth = true, int scale = 3);

	void Scale2x();
};
