/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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

#include "stdafx.h"
#include "BaseSplitterFileEx.h"
#include <MMReg.h>
#include "../../../DSUtil/AudioParser.h"
#include <InitGuid.h>
#include <moreuuids.h>
#include <basestruct.h>

//
// CBaseSplitterFileEx
//

CBaseSplitterFileEx::CBaseSplitterFileEx(IAsyncReader* pReader, HRESULT& hr, bool fRandomAccess, bool fStreaming, bool fStreamingDetect)
	: CBaseSplitterFile(pReader, hr, fRandomAccess, fStreaming, fStreamingDetect)
	, m_tslen(0)
	,m_rtPTSOffset(0)
{
}

CBaseSplitterFileEx::~CBaseSplitterFileEx()
{
}

//

bool CBaseSplitterFileEx::NextMpegStartCode(BYTE& code, __int64 len)
{
	BitByteAlign();
	DWORD dw = (DWORD)-1;
	do {
		if (len-- == 0 || !GetRemaining()) {
			return false;
		}
		dw = (dw << 8) | (BYTE)BitRead(8);
	} while ((dw&0xffffff00) != 0x00000100);
	code = (BYTE)(dw&0xff);
	return true;
}

//

#define MARKER if (BitRead(1) != 1) {ASSERT(0); return false;}

bool CBaseSplitterFileEx::Read(pshdr& h)
{
	memset(&h, 0, sizeof(h));

	BYTE b = (BYTE)BitRead(8, true);

	if ((b&0xf1) == 0x21) {
		h.type = mpeg1;

		EXECUTE_ASSERT(BitRead(4) == 2);

		h.scr = 0;
		h.scr |= BitRead(3) << 30;
		MARKER; // 32..30
		h.scr |= BitRead(15) << 15;
		MARKER; // 29..15
		h.scr |= BitRead(15);
		MARKER;
		MARKER; // 14..0
		h.bitrate = BitRead(22);
		MARKER;
	} else if ((b&0xc4) == 0x44) {
		h.type = mpeg2;

		EXECUTE_ASSERT(BitRead(2) == 1);

		h.scr = 0;
		h.scr |= BitRead(3) << 30;
		MARKER; // 32..30
		h.scr |= BitRead(15) << 15;
		MARKER; // 29..15
		h.scr |= BitRead(15);
		MARKER; // 14..0
		h.scr = (h.scr*300 + BitRead(9)) * 10 / 27;
		MARKER;
		h.bitrate = BitRead(22);
		MARKER;
		MARKER;
		BitRead(5); // reserved
		UINT64 stuffing = BitRead(3);
		while (stuffing-- > 0) {
			EXECUTE_ASSERT(BitRead(8) == 0xff);
		}
	} else {
		return false;
	}

	h.bitrate *= 400;

	return true;
}

bool CBaseSplitterFileEx::Read(pssyshdr& h)
{
	memset(&h, 0, sizeof(h));

	WORD len = (WORD)BitRead(16);
	MARKER;
	h.rate_bound = (DWORD)BitRead(22);
	MARKER;
	h.audio_bound = (BYTE)BitRead(6);
	h.fixed_rate = !!BitRead(1);
	h.csps = !!BitRead(1);
	h.sys_audio_loc_flag = !!BitRead(1);
	h.sys_video_loc_flag = !!BitRead(1);
	MARKER;
	h.video_bound = (BYTE)BitRead(5);

	EXECUTE_ASSERT((BitRead(8)&0x7f) == 0x7f); // reserved (should be 0xff, but not in reality)

	for (len -= 6; len > 3; len -= 3) { // TODO: also store these, somewhere, if needed
		UINT64 stream_id = BitRead(8);
		UNREFERENCED_PARAMETER(stream_id);
		EXECUTE_ASSERT(BitRead(2) == 3);
		UINT64 p_std_buff_size_bound = (BitRead(1)?1024:128)*BitRead(13);
		UNREFERENCED_PARAMETER(p_std_buff_size_bound);
	}

	return true;
}

bool CBaseSplitterFileEx::Read(peshdr& h, BYTE code)
{
	memset(&h, 0, sizeof(h));

	if (!(code >= 0xbd && code < 0xf0 || code == 0xfd)) { // 0xfd => blu-ray (.m2ts)
		return false;
	}

	h.len = (WORD)BitRead(16);

	if (code == 0xbe || code == 0xbf) {
		return true;
	}

	// mpeg1 stuffing (ff ff .. , max 16x)
	for (int i = 0; i < 16 && BitRead(8, true) == 0xff; i++) {
		BitRead(8);
		if (h.len) {
			h.len--;
		}
	}

	h.type = (BYTE)BitRead(2, true) == mpeg2 ? mpeg2 : mpeg1;

	if (h.type == mpeg1) {
		BYTE b = (BYTE)BitRead(2);

		if (b == 1) {
			h.std_buff_size = (BitRead(1)?1024:128)*BitRead(13);
			if (h.len) {
				h.len -= 2;
			}
			b = (BYTE)BitRead(2);
		}

		if (b == 0) {
			h.fpts = (BYTE)BitRead(1);
			h.fdts = (BYTE)BitRead(1);
		}
	} else if (h.type == mpeg2) {
		EXECUTE_ASSERT(BitRead(2) == mpeg2);
		h.scrambling = (BYTE)BitRead(2);
		h.priority = (BYTE)BitRead(1);
		h.alignment = (BYTE)BitRead(1);
		h.copyright = (BYTE)BitRead(1);
		h.original = (BYTE)BitRead(1);
		h.fpts = (BYTE)BitRead(1);
		h.fdts = (BYTE)BitRead(1);
		h.escr = (BYTE)BitRead(1);
		h.esrate = (BYTE)BitRead(1);
		h.dsmtrickmode = (BYTE)BitRead(1);
		h.morecopyright = (BYTE)BitRead(1);
		h.crc = (BYTE)BitRead(1);
		h.extension = (BYTE)BitRead(1);
		h.hdrlen = (BYTE)BitRead(8);
	} else {
		if (h.len) {
			while (h.len-- > 0) {
				BitRead(8);
			}
		}
		goto error;
	}

	if (h.fpts) {
		if (h.type == mpeg2) {
			BYTE b = (BYTE)BitRead(4);
			if (!(h.fdts && b == 3 || !h.fdts && b == 2)) {/*ASSERT(0); */goto error;} // TODO
		}

		h.pts = 0;
		h.pts |= BitRead(3) << 30;
		MARKER; // 32..30
		h.pts |= BitRead(15) << 15;
		MARKER; // 29..15
		h.pts |= BitRead(15);
		MARKER; // 14..0
		h.pts = 10000*h.pts/90 + m_rtPTSOffset;
	}

	if (h.fdts) {
		BYTE b = (BYTE)BitRead(4);
		if (b != 1) {/*return false;*/} // TODO

		h.dts = 0;
		h.dts |= BitRead(3) << 30;
		MARKER; // 32..30
		h.dts |= BitRead(15) << 15;
		MARKER; // 29..15
		h.dts |= BitRead(15);
		MARKER; // 14..0
		h.dts = 10000*h.dts/90 + m_rtPTSOffset;
	}

	// skip to the end of header

	if (h.type == mpeg1) {
		if (!h.fpts && !h.fdts && BitRead(4) != 0xf) {
			/*ASSERT(0);*/ goto error;
		}

		if (h.len) {
			h.len--;
			if (h.pts) {
				h.len -= 4;
			}
			if (h.dts) {
				h.len -= 5;
			}
		}
	}

	if (h.type == mpeg2) {
		if (h.len) {
			h.len -= 3+h.hdrlen;
		}

		int left = h.hdrlen;
		if (h.fpts) {
			left -= 5;
		}
		if (h.fdts) {
			left -= 5;
		}

		if (h.extension) { /* PES extension */
			BYTE pes_ext = (BYTE)BitRead(8);
			left--;
			BYTE skip = (pes_ext >> 4) & 0xb;
			skip += skip & 0x9;
			if (pes_ext & 0x40 || skip > left) {
				TRACE(_T("peshdr read - pes_ext %X is invalid\n"), pes_ext);
				pes_ext = skip = 0;
			}
			for (int i=0; i<skip; i++) {
				BitRead(8);
			}
			left -= skip;
			if (pes_ext & 0x01) { /* PES extension 2 */
				BYTE ext2_len = (BYTE)BitRead(8);
				left--;
				if ((ext2_len & 0x7f) > 0) {
					BYTE id_ext = (BYTE)BitRead(8);
					if ((id_ext & 0x80) == 0) {
						h.id_ext = id_ext;
					}
					left--;
				}
			}
		}
		while (left-- > 0) {
			BitRead(8);
		}
	}

	return true;

error:
	memset(&h, 0, sizeof(h));

	return false;
}

bool CBaseSplitterFileEx::Read(seqhdr& h, int len, CMediaType* pmt, bool find_sync)
{
	__int64 endpos = GetPos() + len; // - sequence header length

	BYTE id = 0;

	while (GetPos() < endpos && id != 0xb3) {
		if (!NextMpegStartCode(id, len)) {
			return false;
		}

		if (!find_sync) {
			break;
		}
	}

	if (id != 0xb3) {
		return false;
	}

	__int64 shpos = GetPos() - 4;

	h.width = (WORD)BitRead(12);
	h.height = (WORD)BitRead(12);
	h.ar = BitRead(4);
	static int ifps[16] = {0, 1126125, 1125000, 1080000, 900900, 900000, 540000, 450450, 450000, 0, 0, 0, 0, 0, 0, 0};
	h.ifps = ifps[BitRead(4)];
	h.bitrate = (DWORD)BitRead(18);
	MARKER;
	h.vbv = (DWORD)BitRead(10);
	h.constrained = BitRead(1);

	h.fiqm = BitRead(1);
	if (h.fiqm)
		for (int i = 0; i < _countof(h.iqm); i++) {
			h.iqm[i] = (BYTE)BitRead(8);
		}

	h.fniqm = BitRead(1);
	if (h.fniqm)
		for (int i = 0; i < _countof(h.niqm); i++) {
			h.niqm[i] = (BYTE)BitRead(8);
		}

	__int64 shlen = GetPos() - shpos;

	static float ar[] = {
		1.0000f,1.0000f,0.6735f,0.7031f,0.7615f,0.8055f,0.8437f,0.8935f,
		0.9157f,0.9815f,1.0255f,1.0695f,1.0950f,1.1575f,1.2015f,1.0000f
	};

	h.arx = (int)((float)h.width / ar[h.ar] + 0.5);
	h.ary = h.height;

	mpeg_t type = mpeg1;

	__int64 shextpos = 0, shextlen = 0;

	if (NextMpegStartCode(id, 8) && id == 0xb5) { // sequence header ext
		shextpos = GetPos() - 4;

		h.startcodeid = BitRead(4);
		h.profile_levelescape = BitRead(1); // reserved, should be 0
		h.profile = BitRead(3);
		h.level = BitRead(4);
		h.progressive = BitRead(1);
		h.chroma = BitRead(2);
		h.width |= (BitRead(2)<<12);
		h.height |= (BitRead(2)<<12);
		h.bitrate |= (BitRead(12)<<18);
		MARKER;
		h.vbv |= (BitRead(8)<<10);
		h.lowdelay = BitRead(1);
		h.ifps = (DWORD)(h.ifps * (BitRead(2)+1) / (BitRead(5)+1));

		shextlen = GetPos() - shextpos;

		struct {
			DWORD x, y;
		} ar[] = {{h.width,h.height},{4,3},{16,9},{221,100},{h.width,h.height}};
		int i = min(max(h.ar, 1), 5)-1;
		h.arx = ar[i].x;
		h.ary = ar[i].y;

		type = mpeg2;

		while (GetPos() < endpos) {
			if (NextMpegStartCode(id, endpos - GetPos())) {
				if (id != 0xb5) {
					continue;
				}

				if ((endpos - GetPos()) >= 5) {

					BYTE startcodeid = BitRead(4);
					if (startcodeid == 0x02) {
						BYTE video_format = BitRead(3);
						BYTE color_description = BitRead(1);
						if (color_description) {
							BYTE color_primaries = BitRead(8);
							BYTE color_trc = BitRead(8);
							BYTE colorspace = BitRead(8);
						}

						WORD panscan_width = (WORD)BitRead(14);
						MARKER;
						WORD panscan_height = (WORD)BitRead(14);

						if (panscan_width && panscan_height) {
							h.arx *= h.width  * panscan_height;
							h.ary *= h.height * panscan_width;
						}

						break;
					}
				}
			}
		}
	}

	h.ifps = 10 * h.ifps / 27;
	h.bitrate = h.bitrate == (1<<30)-1 ? 0 : h.bitrate * 400;

	CSize aspect(h.arx, h.ary);
	ReduceDim(aspect);
	h.arx = aspect.cx;
	h.ary = aspect.cy;

	if (!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Video;

	if (type == mpeg1) {
		pmt->subtype						= MEDIASUBTYPE_MPEG1Payload;
		pmt->formattype						= FORMAT_MPEGVideo;
		int len								= FIELD_OFFSET(MPEG1VIDEOINFO, bSequenceHeader) + int(shlen + shextlen);
		MPEG1VIDEOINFO* vi					= (MPEG1VIDEOINFO*)DNew BYTE[len];
		memset(vi, 0, len);
		vi->hdr.dwBitRate					= h.bitrate;
		vi->hdr.AvgTimePerFrame				= h.ifps;
		vi->hdr.bmiHeader.biSize			= sizeof(vi->hdr.bmiHeader);
		vi->hdr.bmiHeader.biWidth			= h.width;
		vi->hdr.bmiHeader.biHeight			= h.height;
		vi->hdr.bmiHeader.biXPelsPerMeter	= h.width * h.ary;
		vi->hdr.bmiHeader.biYPelsPerMeter	= h.height * h.arx;
		vi->cbSequenceHeader = DWORD(shlen + shextlen);
		Seek(shpos);
		ByteRead((BYTE*)&vi->bSequenceHeader[0], shlen);
		if (shextpos && shextlen) {
			Seek(shextpos);
		}
		ByteRead((BYTE*)&vi->bSequenceHeader[0] + shlen, shextlen);
		pmt->SetFormat((BYTE*)vi, len);
		delete [] vi;
	} else if (type == mpeg2) {
		pmt->subtype				= MEDIASUBTYPE_MPEG2_VIDEO;
		pmt->formattype				= FORMAT_MPEG2_VIDEO;
		int len						= FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + int(shlen + shextlen);
		MPEG2VIDEOINFO* vi			= (MPEG2VIDEOINFO*)DNew BYTE[len];
		memset(vi, 0, len);
		vi->hdr.dwBitRate			= h.bitrate;
		vi->hdr.AvgTimePerFrame		= h.ifps;
		vi->hdr.dwPictAspectRatioX	= h.arx;
		vi->hdr.dwPictAspectRatioY	= h.ary;
		vi->hdr.bmiHeader.biSize	= sizeof(vi->hdr.bmiHeader);
		vi->hdr.bmiHeader.biWidth	= h.width;
		vi->hdr.bmiHeader.biHeight	= h.height;
		vi->dwProfile				= h.profile;
		vi->dwLevel					= h.level;
		vi->cbSequenceHeader		= DWORD(shlen + shextlen);
		Seek(shpos);
		ByteRead((BYTE*)&vi->dwSequenceHeader[0], shlen);
		if (shextpos && shextlen) {
			Seek(shextpos);
		}
		ByteRead((BYTE*)&vi->dwSequenceHeader[0] + shlen, shextlen);
		pmt->SetFormat((BYTE*)vi, len);
		delete [] vi;
	} else {
		return false;
	}

	return true;
}

static int NextMpegStartCodeGb(CGolombBuffer& gb, BYTE& code)
{
	gb.BitByteAlign();
	DWORD dw = (DWORD)-1;
	do {
		if (gb.IsEOF()) {
			return false;
		}
		dw = (dw << 8) | (BYTE)gb.BitRead(8);
	} while ((dw&0xffffff00) != 0x00000100);
	code = (BYTE)(dw&0xff);
	return true;
}

#define MARKERGB if (gb.BitRead(1) != 1) {ASSERT(0); return false;}
bool CBaseSplitterFileEx::Read(seqhdr& h, CAtlArray<BYTE>& buf, CMediaType* pmt, bool find_sync)
{
	BYTE id = 0;

	CGolombBuffer gb(buf.GetData(), buf.GetCount());

	while (!gb.IsEOF() && id != 0xb3) {
		if (!NextMpegStartCodeGb(gb, id)) {
			return false;
		}

		if (!find_sync) {
			break;
		}
	}

	if (id != 0xb3) {
		return false;
	}

	__int64 shpos = gb.GetPos() - 4;

	h.width = (WORD)gb.BitRead(12);
	h.height = (WORD)gb.BitRead(12);
	h.ar = gb.BitRead(4);
	static int ifps[16] = {0, 1126125, 1125000, 1080000, 900900, 900000, 540000, 450450, 450000, 0, 0, 0, 0, 0, 0, 0};
	h.ifps = ifps[gb.BitRead(4)];
	h.bitrate = (DWORD)gb.BitRead(18);
	MARKERGB;
	h.vbv = (DWORD)gb.BitRead(10);
	h.constrained = gb.BitRead(1);

	h.fiqm = gb.BitRead(1);
	if (h.fiqm)
		for (int i = 0; i < _countof(h.iqm); i++) {
			h.iqm[i] = (BYTE)gb.BitRead(8);
		}

	h.fniqm = gb.BitRead(1);
	if (h.fniqm)
		for (int i = 0; i < _countof(h.niqm); i++) {
			h.niqm[i] = (BYTE)gb.BitRead(8);
		}

	__int64 shlen = gb.GetPos() - shpos;

	static float ar[] = {
		1.0000f,1.0000f,0.6735f,0.7031f,0.7615f,0.8055f,0.8437f,0.8935f,
		0.9157f,0.9815f,1.0255f,1.0695f,1.0950f,1.1575f,1.2015f,1.0000f
	};

	h.arx = (int)((float)h.width / ar[h.ar] + 0.5);
	h.ary = h.height;

	mpeg_t type = mpeg1;

	__int64 shextpos = 0, shextlen = 0;

	if (NextMpegStartCodeGb(gb, id) && id == 0xb5) { // sequence header ext
		shextpos = gb.GetPos() - 4;

		h.startcodeid = gb.BitRead(4);
		h.profile_levelescape = gb.BitRead(1); // reserved, should be 0
		h.profile = gb.BitRead(3);
		h.level = gb.BitRead(4);
		h.progressive = gb.BitRead(1);
		h.chroma = gb.BitRead(2);
		h.width |= (gb.BitRead(2)<<12);
		h.height |= (gb.BitRead(2)<<12);
		h.bitrate |= (gb.BitRead(12)<<18);
		MARKERGB;
		h.vbv |= (gb.BitRead(8)<<10);
		h.lowdelay = gb.BitRead(1);
		h.ifps = (DWORD)(h.ifps * (gb.BitRead(2)+1) / (gb.BitRead(5)+1));

		shextlen = gb.GetPos() - shextpos;

		struct {
			DWORD x, y;
		} ar[] = {{h.width,h.height},{4,3},{16,9},{221,100},{h.width,h.height}};
		int i = min(max(h.ar, 1), 5)-1;
		h.arx = ar[i].x;
		h.ary = ar[i].y;

		type = mpeg2;

		while (!gb.IsEOF()) {
			if (NextMpegStartCodeGb(gb, id)) {
				if (id != 0xb5) {
					continue;
				}

				if (gb.RemainingSize() >= 5) {

					BYTE startcodeid = gb.BitRead(4);
					if (startcodeid == 0x02) {
						BYTE video_format = gb.BitRead(3);
						BYTE color_description = gb.BitRead(1);
						if (color_description) {
							BYTE color_primaries = gb.BitRead(8);
							BYTE color_trc = gb.BitRead(8);
							BYTE colorspace = gb.BitRead(8);
						}

						WORD panscan_width = (WORD)gb.BitRead(14);
						MARKERGB;
						WORD panscan_height = (WORD)gb.BitRead(14);

						if (panscan_width && panscan_height) {
							h.arx *= h.width  * panscan_height;
							h.ary *= h.height * panscan_width;
						}

						break;
					}
				}
			}
		}
	}

	h.ifps = 10 * h.ifps / 27;
	h.bitrate = h.bitrate == (1<<30)-1 ? 0 : h.bitrate * 400;

	CSize aspect(h.arx, h.ary);
	ReduceDim(aspect);
	h.arx = aspect.cx;
	h.ary = aspect.cy;

	if (!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Video;

	if (type == mpeg1) {
		pmt->subtype						= MEDIASUBTYPE_MPEG1Payload;
		pmt->formattype						= FORMAT_MPEGVideo;
		int len								= FIELD_OFFSET(MPEG1VIDEOINFO, bSequenceHeader) + int(shlen + shextlen);
		MPEG1VIDEOINFO* vi					= (MPEG1VIDEOINFO*)DNew BYTE[len];
		memset(vi, 0, len);
		vi->hdr.dwBitRate					= h.bitrate;
		vi->hdr.AvgTimePerFrame				= h.ifps;
		vi->hdr.bmiHeader.biSize			= sizeof(vi->hdr.bmiHeader);
		vi->hdr.bmiHeader.biWidth			= h.width;
		vi->hdr.bmiHeader.biHeight			= h.height;
		vi->hdr.bmiHeader.biXPelsPerMeter	= h.width * h.ary;
		vi->hdr.bmiHeader.biYPelsPerMeter	= h.height * h.arx;
		vi->cbSequenceHeader				= DWORD(shlen + shextlen);
		gb.Reset();
		gb.SkipBytes(shextpos);
		gb.ReadBuffer((BYTE*)&vi->bSequenceHeader[0], shlen);
		if (shextpos && shextlen) {
			gb.Reset();
			gb.SkipBytes(shextpos);
		}
		gb.ReadBuffer((BYTE*)&vi->bSequenceHeader[0] + shlen, shextlen);
		pmt->SetFormat((BYTE*)vi, len);
		delete [] vi;
	} else if (type == mpeg2) {
		pmt->subtype						= MEDIASUBTYPE_MPEG2_VIDEO;
		pmt->formattype						= FORMAT_MPEG2_VIDEO;
		int len								= FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + int(shlen + shextlen);
		MPEG2VIDEOINFO* vi					= (MPEG2VIDEOINFO*)DNew BYTE[len];
		memset(vi, 0, len);
		vi->hdr.dwBitRate					= h.bitrate;
		vi->hdr.AvgTimePerFrame				= h.ifps;
		vi->hdr.dwPictAspectRatioX			= h.arx;
		vi->hdr.dwPictAspectRatioY			= h.ary;
		vi->hdr.bmiHeader.biSize			= sizeof(vi->hdr.bmiHeader);
		vi->hdr.bmiHeader.biWidth			= h.width;
		vi->hdr.bmiHeader.biHeight			= h.height;
		vi->dwProfile						= h.profile;
		vi->dwLevel							= h.level;
		vi->cbSequenceHeader				= DWORD(shlen + shextlen);
		gb.Reset();
		gb.SkipBytes(shpos);
		ByteRead((BYTE*)&vi->dwSequenceHeader[0], shlen);
		if (shextpos && shextlen) {
			gb.Reset();
			gb.SkipBytes(shextpos);
		}
		gb.ReadBuffer((BYTE*)&vi->dwSequenceHeader[0] + shlen, shextlen);
		pmt->SetFormat((BYTE*)vi, len);
		delete [] vi;
	} else {
		return false;
	}

	return true;
}

#define AGAIN Seek(pos); BitRead(8); continue;
bool CBaseSplitterFileEx::Read(mpahdr& h, int len, CMediaType* pmt, bool fAllowV25)
{
	memset(&h, 0, sizeof(h));

	int syncbits = fAllowV25 ? 11 : 12;
	int bitrate = 0;

	for (;;) {
		for (; len >= 4 && BitRead(syncbits, true) != (1<<syncbits) - 1; len--) {
			BitRead(8);
		}

		if (len < 4) {
			return false;
		}

		__int64 pos = GetPos();

		h.sync = BitRead(11);
		h.version = BitRead(2);
		h.layer = BitRead(2);
		h.crc = BitRead(1);
		h.bitrate = BitRead(4);
		h.freq = BitRead(2);
		h.padding = BitRead(1);
		h.privatebit = BitRead(1);
		h.channels = BitRead(2);
		h.modeext = BitRead(2);
		h.copyright = BitRead(1);
		h.original = BitRead(1);
		h.emphasis = BitRead(2);

		if (h.version == 1 || h.layer == 0 || h.freq == 3 || h.bitrate == 15 || h.emphasis == 2) {
			AGAIN
		}

		if (h.version == 3 && h.layer == 2) {
			if (h.channels == 3) {
				if (h.bitrate >= 11 && h.bitrate <= 14) {
					AGAIN
				}
			} else {
				if (h.bitrate == 1 || h.bitrate == 2 || h.bitrate == 3 || h.bitrate == 5) {
					AGAIN
				}
			}
		}

		h.layer = 4 - h.layer;

		static int brtbl[][5] = {
			{  0,   0,   0,   0,   0},
			{ 32,  32,  32,  32,   8},
			{ 64,  48,  40,  48,  16},
			{ 96,  56,  48,  56,  24},
			{128,  64,  56,  64,  32},
			{160,  80,  64,  80,  40},
			{192,  96,  80,  96,  48},
			{224, 112,  96, 112,  56},
			{256, 128, 112, 128,  64},
			{288, 160, 128, 144,  80},
			{320, 192, 160, 160,  96},
			{352, 224, 192, 176, 112},
			{384, 256, 224, 192, 128},
			{416, 320, 256, 224, 144},
			{448, 384, 320, 256, 160},
			{  0,   0,   0,   0,   0},
		};

		static int brtblcol[][4] = {
			{0, 3, 4, 4},
			{0, 0, 1, 2}
		};
		bitrate = 1000 * brtbl[h.bitrate][brtblcol[h.version&1][h.layer]];
		if (bitrate == 0) {
			AGAIN
		}

		break;
	}

	static int freq[][4] = {
		{11025, 0, 22050, 44100},
		{12000, 0, 24000, 48000},
		{ 8000, 0, 16000, 32000}
	};

	bool l3ext = h.layer == 3 && !(h.version&1);

	h.nSamplesPerSec = freq[h.freq][h.version];
	h.FrameSize = h.layer == 1
				  ? (12 * bitrate / h.nSamplesPerSec + h.padding) * 4
				  : (l3ext ? 72 : 144) * bitrate / h.nSamplesPerSec + h.padding;
	h.rtDuration = 10000000i64 * (h.layer == 1 ? 384 : l3ext ? 576 : 1152) / h.nSamplesPerSec;// / (h.channels == 3 ? 1 : 2);
	h.nBytesPerSec = bitrate / 8;

	if (!pmt) {
		return true;
	}

	size_t size = h.layer == 3
				  ? sizeof(WAVEFORMATEX/*MPEGLAYER3WAVEFORMAT*/) // no need to overcomplicate this...
				  : sizeof(MPEG1WAVEFORMAT);
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)DNew BYTE[size];
	memset(wfe, 0, size);
	wfe->cbSize = WORD(size - sizeof(WAVEFORMATEX));

	if (h.layer == 3) {
		wfe->wFormatTag = WAVE_FORMAT_MPEGLAYER3;

		/*		MPEGLAYER3WAVEFORMAT* f = (MPEGLAYER3WAVEFORMAT*)wfe;
				f->wfx.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
				f->wID = MPEGLAYER3_ID_UNKNOWN;
				f->fdwFlags = h.padding ? MPEGLAYER3_FLAG_PADDING_ON : MPEGLAYER3_FLAG_PADDING_OFF; // _OFF or _ISO ?
		*/
	} else {
		MPEG1WAVEFORMAT* f	= (MPEG1WAVEFORMAT*)wfe;
		f->wfx.wFormatTag	= WAVE_FORMAT_MPEG;
		f->fwHeadMode		= 1 << h.channels;
		f->fwHeadModeExt	= 1 << h.modeext;
		f->wHeadEmphasis	= h.emphasis+1;
		if (h.privatebit) {
			f->fwHeadFlags	|= ACM_MPEG_PRIVATEBIT;
		}
		if (h.copyright) {
			f->fwHeadFlags	|= ACM_MPEG_COPYRIGHT;
		}
		if (h.original) {
			f->fwHeadFlags	|= ACM_MPEG_ORIGINALHOME;
		}
		if (h.crc == 0) {
			f->fwHeadFlags	|= ACM_MPEG_PROTECTIONBIT;
		}
		if (h.version == 3) {
			f->fwHeadFlags	|= ACM_MPEG_ID_MPEG1;
		}
		f->fwHeadLayer		= 1 << (h.layer-1);
		f->dwHeadBitrate	= bitrate;
	}

	wfe->nChannels = h.channels == 3 ? 1 : 2;
	wfe->nSamplesPerSec = h.nSamplesPerSec;
	wfe->nBlockAlign = h.FrameSize;
	wfe->nAvgBytesPerSec = h.nBytesPerSec;

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = FOURCCMap(wfe->wFormatTag);
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)wfe, sizeof(WAVEFORMATEX) + wfe->cbSize);

	delete [] wfe;

	return true;
}

bool CBaseSplitterFileEx::Read(latm_aachdr& h, int len, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	__int64 pos		= GetPos();
	int len_start	= len;

	for (; len >= 7 && BitRead(11, true) != 0x2b7; len--) {
		BitRead(8);
	}

	if (len < 7) {
		return false;
	}

	BYTE buffer[64] = { 0 };
	ByteRead(buffer, min(len, 64));

	BYTE extra[64] = { 0 };
	unsigned int extralen;
	if (!ParseAACLatmHeader(buffer, min(len, 64), h.samplerate, h.channels, extra, extralen)) {
		return false;
	}

	if (!pmt) {
		return true;
	}

	// try detect ADTS header
	aachdr aac_h;
	Seek(pos);
	if (Read(aac_h, len_start, pmt, false)) {
		memset(&h, 0, sizeof(h));
		return true;
	}

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)DNew BYTE[sizeof(WAVEFORMATEX) + extralen];
	memset(wfe, 0, sizeof(WAVEFORMATEX));
	wfe->wFormatTag = WAVE_FORMAT_LATM_AAC;
	wfe->nChannels = h.channels;
	wfe->nSamplesPerSec = h.samplerate;
	wfe->nBlockAlign = 1;
	wfe->nAvgBytesPerSec = 0;
	wfe->cbSize = extralen;
	if(extralen) {
		memcpy((BYTE*)(wfe + 1), extra, extralen);
	}

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = MEDIASUBTYPE_LATM_AAC;
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)wfe, sizeof(WAVEFORMATEX) + wfe->cbSize);

	delete [] wfe;

	return true;
}

bool CBaseSplitterFileEx::Read(aachdr& h, int len, CMediaType* pmt, bool find_sync)
{
	memset(&h, 0, sizeof(h));

	if (!find_sync && (BitRead(12, true) != 0xfff)) {
		return false;
	}

	for (; len >= 7 && BitRead(12, true) != 0xfff; len--) {
		BitRead(8);
	}

	if (len < 7) {
		return false;
	}

	h.sync = BitRead(12);
	h.version = BitRead(1);
	h.layer = BitRead(2);
	h.fcrc = BitRead(1);
	h.profile = BitRead(2);
	h.freq = BitRead(4);
	h.privatebit = BitRead(1);
	h.channels = BitRead(3);
	h.original = BitRead(1);
	h.home = BitRead(1);

	h.copyright_id_bit = BitRead(1);
	h.copyright_id_start = BitRead(1);
	h.aac_frame_length = BitRead(13);
	h.adts_buffer_fullness = BitRead(11);
	h.no_raw_data_blocks_in_frame = BitRead(2);

	if (h.fcrc == 0) {
		h.crc = (WORD)BitRead(16);
	}

	if (h.layer != 0 || h.freq > 12 || h.aac_frame_length <= (h.fcrc == 0 ? 9 : 7) || h.channels < 1) {
		return false;
	}

	h.FrameSize = h.aac_frame_length - (h.fcrc == 0 ? 9 : 7);
	static int freq[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350};
	h.nBytesPerSec = h.aac_frame_length * freq[h.freq] / 1024; // ok?
	h.rtDuration = 10000000i64 * 1024 / freq[h.freq]; // ok?

	if (!pmt) {
		return true;
	}

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)DNew BYTE[sizeof(WAVEFORMATEX)+5];
	memset(wfe, 0, sizeof(WAVEFORMATEX)+5);
	wfe->wFormatTag = WAVE_FORMAT_RAW_AAC1;
	wfe->nChannels = h.channels <= 6 ? h.channels : 2;
	wfe->nSamplesPerSec = freq[h.freq];
	wfe->nBlockAlign = h.aac_frame_length;
	wfe->nAvgBytesPerSec = h.nBytesPerSec;
	wfe->cbSize = MakeAACInitData((BYTE*)(wfe+1), h.profile, wfe->nSamplesPerSec, wfe->nChannels);

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = MEDIASUBTYPE_RAW_AAC1;
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)wfe, sizeof(WAVEFORMATEX)+wfe->cbSize);

	delete [] wfe;

	return true;
}

bool CBaseSplitterFileEx::Read(ac3hdr& h, int len, CMediaType* pmt, bool find_sync, bool AC3CoreOnly)
{
	static int freq[] = {48000, 44100, 32000, 0};

	bool e_ac3 = false;

	__int64 startpos = GetPos();

	memset(&h, 0, sizeof(h));

	// Parse TrueHD and MLP header
	if (!AC3CoreOnly) {
		BYTE buf[20];

		int fsize = 0;
		ByteRead(buf, 20);

		audioframe_t aframe;
		fsize = ParseMLPHeader(buf, &aframe);
		if (fsize) {

			if (!pmt) {
				return true;
			}

			int bitrate   = (int)(fsize * 8i64 * aframe.samplerate / aframe.samples); // inaccurate, because fsize is not constant

			pmt->majortype		= MEDIATYPE_Audio;
			pmt->subtype		= aframe.param2 ? MEDIASUBTYPE_DOLBY_TRUEHD : MEDIASUBTYPE_MLP;
			pmt->formattype		= FORMAT_WaveFormatEx;

			WAVEFORMATEX* wfe	= (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
			memset(wfe, 0, sizeof(WAVEFORMATEX));
			wfe->wFormatTag      = WAVE_FORMAT_UNKNOWN;
			wfe->nChannels       = aframe.channels;
			wfe->nSamplesPerSec  = aframe.samplerate;
			wfe->nAvgBytesPerSec = (bitrate + 4) /8;
			wfe->nBlockAlign     = fsize < WORD_MAX ? fsize : WORD_MAX;
			wfe->wBitsPerSample  = aframe.param1;

			pmt->SetSampleSize(0);

			return true;
		}
	}

	Seek(startpos);

	if (find_sync) {
		for (; len >= 7 && BitRead(16, true) != 0x0b77; len--) {
			BitRead(8);
		}
	}

	if (len < 7) {
		return false;
	}

	h.sync = (WORD)BitRead(16);
	if (h.sync != 0x0B77) {
		return false;
	}

	_int64 pos = GetPos();
	h.crc1 = (WORD)BitRead(16);
	h.fscod = BitRead(2);
	h.frmsizecod = BitRead(6);
	h.bsid = BitRead(5);
	if (h.bsid > 16) {
		return false;
	}
	if (h.bsid <= 10) {
		/* Normal AC-3 */
		if (h.fscod == 3) {
			return false;
		}
		if (h.frmsizecod > 37) {
			return false;
		}
		h.bsmod = BitRead(3);
		h.acmod = BitRead(3);
		if (h.acmod == 2) {
			h.dsurmod = BitRead(2);
		}
		if ((h.acmod & 1) && h.acmod != 1) {
			h.cmixlev = BitRead(2);
		}
		if (h.acmod & 4) {
			h.surmixlev = BitRead(2);
		}
		h.lfeon = BitRead(1);
		h.sr_shift = max(h.bsid, 8) - 8;
	} else {
		/* Enhanced AC-3 */
		e_ac3 = true;
		Seek(pos);
		h.frame_type = (BYTE)BitRead(2);
		h.substreamid = (BYTE)BitRead(3);
		if (h.frame_type || h.substreamid) {
			return false;
		}
		h.frame_size = ((WORD)BitRead(11) + 1) << 1;
		if (h.frame_size < 7) {
			return false;
		}
		h.sr_code = (BYTE)BitRead(2);
		if (h.sr_code == 3) {
			BYTE sr_code2 = (BYTE)BitRead(2);
			if (sr_code2 == 3) {
				return false;
			}
			h.sample_rate = freq[sr_code2] / 2;
			h.sr_shift = 1;
		} else {
			static int eac3_blocks[4] = {1, 2, 3, 6};
			h.num_blocks = eac3_blocks[BitRead(2)];
			h.sample_rate = freq[h.sr_code];
			h.sr_shift = 0;
		}
		h.acmod = BitRead(3);
		h.lfeon = BitRead(1);
	}

	if (!pmt) {
		return true;
	}

	WAVEFORMATEX wfe;
	memset(&wfe, 0, sizeof(wfe));
	wfe.wFormatTag = WAVE_FORMAT_DOLBY_AC3;

	static int channels[] = {2, 1, 2, 3, 3, 4, 4, 5};
	wfe.nChannels = channels[h.acmod] + h.lfeon;

	if (!e_ac3) {
		wfe.nSamplesPerSec = freq[h.fscod] >> h.sr_shift;
		static int rate[] = {32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 576, 640, 768, 896, 1024, 1152, 1280};
		wfe.nAvgBytesPerSec = ((rate[h.frmsizecod>>1] * 1000) >> h.sr_shift) / 8;
	} else {
		wfe.nSamplesPerSec = h.sample_rate;
		wfe.nAvgBytesPerSec = h.frame_size * h.sample_rate / (h.num_blocks * 256);
	}

	wfe.nBlockAlign = (WORD)(1536 * wfe.nAvgBytesPerSec / wfe.nSamplesPerSec);

	pmt->majortype = MEDIATYPE_Audio;
	if (e_ac3) {
		pmt->subtype = MEDIASUBTYPE_DOLBY_DDPLUS;
	} else {
		pmt->subtype = MEDIASUBTYPE_DOLBY_AC3;
	}
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	return true;
}

bool CBaseSplitterFileEx::Read(dtshdr& h, int len, CMediaType* pmt, bool find_sync)
{
	memset(&h, 0, sizeof(h));

	if (find_sync) {
		for (; len >= 10 && BitRead(32, true) != 0x7ffe8001; len--) {
			BitRead(8);
		}
	}

	if (len < 10) {
		return false;
	}

	h.sync = (DWORD)BitRead(32);
	if (h.sync != 0x7ffe8001) {
		return false;
	}

	h.frametype = BitRead(1);
	h.deficitsamplecount = BitRead(5);
	h.fcrc = BitRead(1);
	h.nblocks = BitRead(7)+1;
	h.framebytes = (WORD)BitRead(14)+1;
	h.amode = BitRead(6);
	h.sfreq = BitRead(4);
	h.rate = BitRead(5);

	h.downmix = BitRead(1);
	h.dynrange = BitRead(1);
	h.timestamp = BitRead(1);
	h.aux_data = BitRead(1);
	h.hdcd = BitRead(1);
	h.ext_descr = BitRead(3);
	h.ext_coding = BitRead(1);
	h.aspf = BitRead(1);
	h.lfe = BitRead(2);
	h.predictor_history = BitRead(1);


	if (!pmt) {
		return true;
	}

	WAVEFORMATEX wfe;
	memset(&wfe, 0, sizeof(wfe));
	wfe.wFormatTag = WAVE_FORMAT_DTS2;

	static int channels[] = {1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6, 6, 7, 8, 8};

	if (h.amode < _countof(channels)) {
		wfe.nChannels = channels[h.amode];
		if (h.lfe > 0) {
			++wfe.nChannels;
		}
	}

	static int freq[] = {0,8000,16000,32000,0,0,11025,22050,44100,0,0,12000,24000,48000,0,0};
	wfe.nSamplesPerSec = freq[h.sfreq];

	/*static int rate[] = {
		  32000,   56000,   64000,   96000,
		 112000,  128000,  192000,  224000,
		 256000,  320000,  384000,  448000,
		 512000,  576000,  640000,  768000,
		 960000, 1024000, 1152000, 1280000,
		1344000, 1408000, 1411200, 1472000,
		1536000, 1920000, 2048000, 3072000,
		3840000, 0, 0, 0 //open, variable, lossless
	};
	int nom_bitrate = rate[h.rate];*/

	unsigned int bitrate = (unsigned int)(8ui64 * h.framebytes * wfe.nSamplesPerSec / (h.nblocks*32));

	wfe.nAvgBytesPerSec = (bitrate + 4) / 8;
	wfe.nBlockAlign = h.framebytes;

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = MEDIASUBTYPE_DTS;
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	return true;
}

bool CBaseSplitterFileEx::Read(lpcmhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	h.emphasis = BitRead(1);
	h.mute = BitRead(1);
	h.reserved1 = BitRead(1);
	h.framenum = BitRead(5);
	h.quantwordlen = BitRead(2);
	h.freq = BitRead(2);
	h.reserved2 = BitRead(1);
	h.channels = BitRead(3);
	h.drc = (BYTE)BitRead(8);

	if (h.quantwordlen == 3 || h.reserved1 || h.reserved2) {
		return false;
	}

	if (!pmt) {
		return true;
	}

	WAVEFORMATEX wfe;
	memset(&wfe, 0, sizeof(wfe));
	wfe.wFormatTag = WAVE_FORMAT_PCM;
	wfe.nChannels = h.channels+1;
	static int freq[] = {48000, 96000, 44100, 32000};
	wfe.nSamplesPerSec = freq[h.freq];
	switch (h.quantwordlen) {
		case 0:
			wfe.wBitsPerSample = 16;
			break;
		case 1:
			wfe.wBitsPerSample = 20;
			break;
		case 2:
			wfe.wBitsPerSample = 24;
			break;
	}
	wfe.nBlockAlign = (wfe.nChannels*2*wfe.wBitsPerSample) / 8;
	wfe.nAvgBytesPerSec = (wfe.nBlockAlign*wfe.nSamplesPerSec) / 2;

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = MEDIASUBTYPE_DVD_LPCM_AUDIO;
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	return true;
}

bool CBaseSplitterFileEx::Read(dvdalpcmhdr& h, int len, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));
	if (len < 8) return false;

	h.firstaudioframe = (WORD)BitRead(16);// Byte pointer to start of first audio frame.
	h.unknown1        = (BYTE)BitRead(8); // Unknown - e.g. 0x10 for stereo, 0x00 for surround
	if (h.unknown1!= 0x10 && h.unknown1!= 0x00) 
		return false; // this is not the aob. maybe this is a vob.

	h.bitpersample1   = (BYTE)BitRead(4);
	h.bitpersample2   = (BYTE)BitRead(4);
	h.samplerate1     = (BYTE)BitRead(4);
	h.samplerate2     = (BYTE)BitRead(4);
	h.unknown2        = (BYTE)BitRead(8); // Unknown - e.g. 0x00
	h.groupassignment = (BYTE)BitRead(8); // Channel Group Assignment
	h.unknown3        = (BYTE)BitRead(8); // Unknown - e.g. 0x80

	if (h.bitpersample1 > 2 || (h.samplerate1&7) > 2 || h.groupassignment > 20 ||
		h.unknown2 != 0x00 || h.unknown3 != 0x80) {
			return false; // poor parameters or this is a vob.
	}
	// decoder limitations
	if (h.groupassignment > 1 && (h.bitpersample2 != h.bitpersample1 || h.samplerate2 != h.samplerate1)) {
		return false; // decoder does not support different bit depth and frequency.
	}

	if (!pmt) {
		return true;
	}

	WAVEFORMATEX wfe;
	memset(&wfe, 0, sizeof(wfe));
	wfe.wFormatTag = WAVE_FORMAT_UNKNOWN;
	static const WORD depth[] = {16, 20, 24};
	static const DWORD freq[] = {48000, 96000, 192000, 0, 0, 0, 0, 0, 44100, 88200, 1764000};
	                              // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
	static const WORD channels1[] = {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4};
	static const WORD channels2[] = {0, 0, 1, 2, 1, 2, 3, 1, 2, 3, 2, 3, 4, 1, 2, 1, 2, 3, 1, 1, 2};
	wfe.wBitsPerSample = depth[h.bitpersample1];
	wfe.nSamplesPerSec = freq[h.samplerate1];
	wfe.nChannels = channels1[h.groupassignment]+channels2[h.groupassignment];
	
	if (wfe.nChannels > 2) {
		wfe.nBlockAlign = (depth[h.bitpersample1] * channels1[h.groupassignment] * (WORD)(freq[h.samplerate1] / freq[h.samplerate2]) +
		                   depth[h.bitpersample2] * channels2[h.groupassignment]) * 2 / 8;
	} else {
		wfe.nBlockAlign = depth[h.bitpersample1] * channels1[h.groupassignment] * 2 / 8;
	}
	wfe.nAvgBytesPerSec = wfe.nBlockAlign * wfe.nSamplesPerSec / 2;

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = MEDIASUBTYPE_DVD_LPCM_AUDIO;
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	return true;
}

bool CBaseSplitterFileEx::Read(hdmvlpcmhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	h.size			= (WORD)BitRead(16);
	h.channels		= (BYTE)BitRead(4);
	h.samplerate	= (BYTE)BitRead(4);
	h.bitpersample	= (BYTE)BitRead(2);

	if (h.channels > 11
			|| h.samplerate > 5
			|| h.bitpersample > 3) {
		return false;
	}

	if (!pmt) {
		return true;
	}

	WAVEFORMATEX_HDMV_LPCM wfe;
	wfe.wFormatTag = WAVE_FORMAT_PCM;

	static int channels[] = {0, 1, 0, 2, 3, 3, 4, 4, 5, 6, 7, 8};
	wfe.nChannels	 = channels[h.channels];
	if (wfe.nChannels == 0) {
		return false;
	}
	wfe.channel_conf = h.channels;

	static int freq[] = {0, 48000, 0, 0, 96000, 192000};
	wfe.nSamplesPerSec = freq[h.samplerate];
	if (wfe.nSamplesPerSec == 0) {
		return false;
	}

	static int bitspersample[] = {0, 16, 20, 24};
	wfe.wBitsPerSample = bitspersample[h.bitpersample];
	if (wfe.wBitsPerSample == 0) {
		return false;
	}

	wfe.nBlockAlign		= wfe.nChannels*wfe.wBitsPerSample>>3;
	wfe.nAvgBytesPerSec = wfe.nBlockAlign*wfe.nSamplesPerSec;

	pmt->majortype	= MEDIATYPE_Audio;
	pmt->subtype	= MEDIASUBTYPE_HDMV_LPCM_AUDIO;
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	return true;
}

bool CBaseSplitterFileEx::Read(mlphdr& h, int len, CMediaType* pmt, bool find_sync)
{
	memset(&h, 0, sizeof(h));
	if (len < 20) return false;

	__int64 startpos = GetPos();

	audioframe_t aframe;
	int fsize = 0;

	BYTE buf[20];
	int k = find_sync ? len - 20 : 1;
	int i = 0;
	while (i < k) {
		Seek(startpos+i);
		ByteRead(buf, 20);
		fsize = ParseMLPHeader(buf, &aframe);
		if (fsize) {
			break;
		}
		++i;
	}

	if (fsize && aframe.param2 == 0) {
		h.size = fsize;

		if (!pmt) {
			return true;
		}

		int bitrate   = (int)(fsize * 8i64 * aframe.samplerate / aframe.samples); // inaccurate, because fsize is not constant
		pmt->majortype			= MEDIATYPE_Audio;
		pmt->subtype			= MEDIASUBTYPE_MLP;
		pmt->formattype			= FORMAT_WaveFormatEx;

		WAVEFORMATEX* wfe		= (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
		memset(wfe, 0, sizeof(WAVEFORMATEX));
		wfe->wFormatTag			= WAVE_FORMAT_UNKNOWN;
		wfe->nChannels			= aframe.channels;
		wfe->nSamplesPerSec		= aframe.samplerate;
		wfe->nAvgBytesPerSec	= (bitrate + 4) /8;
		wfe->nBlockAlign		= fsize < WORD_MAX ? fsize : WORD_MAX;
		wfe->wBitsPerSample		= aframe.param1;

		pmt->SetSampleSize(0);

		Seek(startpos+i);
		return true;
	}

	return false;
}

bool CBaseSplitterFileEx::Read(dvdspuhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if (!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = MEDIASUBTYPE_DVD_SUBPICTURE;
	pmt->formattype = FORMAT_None;

	return true;
}

bool CBaseSplitterFileEx::Read(hdmvsubhdr& h, CMediaType* pmt, const char* language_code)
{
	memset(&h, 0, sizeof(h));

	if (!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Subtitle;
	pmt->subtype = MEDIASUBTYPE_HDMVSUB;
	pmt->formattype = FORMAT_None;

	SUBTITLEINFO* psi = (SUBTITLEINFO*)pmt->AllocFormatBuffer(sizeof(SUBTITLEINFO));
	if (psi) {
		memset(psi, 0, pmt->FormatLength());
		strcpy_s(psi->IsoLang, language_code ? language_code : "eng");
	}

	return true;
}

bool CBaseSplitterFileEx::Read(svcdspuhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if (!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = MEDIASUBTYPE_SVCD_SUBPICTURE;
	pmt->formattype = FORMAT_None;

	return true;
}

bool CBaseSplitterFileEx::Read(cvdspuhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if (!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = MEDIASUBTYPE_CVD_SUBPICTURE;
	pmt->formattype = FORMAT_None;

	return true;
}

bool CBaseSplitterFileEx::Read(ps2audhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if (BitRead(16, true) != 'SS') {
		return false;
	}

	__int64 pos = GetPos();

	while (BitRead(16, true) == 'SS') {
		DWORD tag = (DWORD)BitRead(32, true);
		DWORD size = 0;

		if (tag == 'SShd') {
			BitRead(32);
			ByteRead((BYTE*)&size, sizeof(size));
			ASSERT(size == 0x18);
			Seek(GetPos());
			ByteRead((BYTE*)&h, sizeof(h));
		} else if (tag == 'SSbd') {
			BitRead(32);
			ByteRead((BYTE*)&size, sizeof(size));
			break;
		}
	}

	Seek(pos);

	if (!pmt) {
		return true;
	}

	WAVEFORMATEXPS2 wfe;
	wfe.wFormatTag =
		h.unk1 == 0x01 ? WAVE_FORMAT_PS2_PCM :
		h.unk1 == 0x10 ? WAVE_FORMAT_PS2_ADPCM :
		WAVE_FORMAT_UNKNOWN;
	wfe.nChannels = (WORD)h.channels;
	wfe.nSamplesPerSec = h.freq;
	wfe.wBitsPerSample = 16; // always?
	wfe.nBlockAlign = wfe.nChannels*wfe.wBitsPerSample>>3;
	wfe.nAvgBytesPerSec = wfe.nBlockAlign*wfe.nSamplesPerSec;
	wfe.dwInterleave = h.interleave;

	pmt->majortype = MEDIATYPE_Audio;
	pmt->subtype = FOURCCMap(wfe.wFormatTag);
	pmt->formattype = FORMAT_WaveFormatEx;
	pmt->SetFormat((BYTE*)&wfe, sizeof(wfe));

	return true;
}

bool CBaseSplitterFileEx::Read(ps2subhdr& h, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if (!pmt) {
		return true;
	}

	pmt->majortype = MEDIATYPE_Subtitle;
	pmt->subtype = MEDIASUBTYPE_PS2_SUB;
	pmt->formattype = FORMAT_None;

	return true;
}

__int64 CBaseSplitterFileEx::Read(trhdr& h, bool fSync)
{
	memset(&h, 0, sizeof(h));

	BitByteAlign();

	if (m_tslen == 0) {
		__int64 pos = GetPos();
		int count	= 0;

		for (int i = 0; i < 192; i++) {
			if (BitRead(8, true) == 0x47) {
				__int64 pos = GetPos();
				Seek(pos + 188);
				if (BitRead(8, true) == 0x47) {
					if (m_tslen != 188) {
						count = 0;
					}
					m_tslen = 188;    // TS stream
					if (count > 1) {
						break;
					}
					count++;
				} else {
					Seek(pos + 192);
					if (BitRead(8, true) == 0x47) {
						if (m_tslen != 192) {
							count = 0;
						}
						m_tslen = 192;    // M2TS stream
						if (count > 1) {
							break;
						}
						count++;
					}
				}
			} else {
				BitRead(8);
			}
		}

		Seek(pos);

		if (m_tslen == 0) {
			return -1;
		}
	}

	if (fSync) {
		for (int i = 0; i < m_tslen; i++) {
			if (BitRead(8, true) == 0x47) {
				if (i == 0) {
					break;
				}
				Seek(GetPos()+m_tslen);
				if (BitRead(8, true) == 0x47) {
					Seek(GetPos()-m_tslen);
					break;
				}
			}

			BitRead(8);

			if (i == m_tslen-1) {
				return -1;
			}
		}
	}

	if (BitRead(8, true) != 0x47) {
		return -1;
	}

	h.next = GetPos() + m_tslen;

	h.sync = (BYTE)BitRead(8);
	h.error = BitRead(1);
	h.payloadstart = BitRead(1);
	h.transportpriority = BitRead(1);
	h.pid = BitRead(13);
	h.scrambling = BitRead(2);
	h.adapfield = BitRead(1);
	h.payload = BitRead(1);
	h.counter = BitRead(4);

	h.bytes = 188 - 4;

	if (h.adapfield) {
		h.length = (BYTE)BitRead(8);

		if (h.length > 0) {
			h.discontinuity = BitRead(1);
			h.randomaccess = BitRead(1);
			h.priority = BitRead(1);
			h.fPCR = BitRead(1);
			h.OPCR = BitRead(1);
			h.splicingpoint = BitRead(1);
			h.privatedata = BitRead(1);
			h.extension = BitRead(1);

			int i = 1;

			if (h.fPCR && h.length>6) {
				h.PCR = BitRead(33);
				BitRead(6);
				UINT64 PCRExt = BitRead(9);
				h.PCR = (h.PCR*300 + PCRExt) * 10 / 27;
				i += 6;
			}

			h.length = min(h.length, h.bytes-1);
			for (; i < h.length; i++) {
				BitRead(8);
			}
		}

		h.bytes -= h.length+1;

		if (h.bytes < 0) {
			ASSERT(0);
			return -1;
		}
	}

	return h.next - m_tslen;
}

bool CBaseSplitterFileEx::Read(trsechdr& h)
{
	memset(&h, 0, sizeof(h));

	BYTE pointer_field = (BYTE)BitRead(8);
	while (pointer_field-- > 0) {
		BitRead(8);
	}
	h.table_id                 = (BYTE)BitRead(8);
	h.section_syntax_indicator = (WORD)BitRead(1);
	h.zero                     = (WORD)BitRead(1);
	h.reserved1                = (WORD)BitRead(2);
	h.section_length           = (WORD)BitRead(12);
	h.transport_stream_id      = (WORD)BitRead(16);
	h.reserved2                = (BYTE)BitRead(2);
	h.version_number           = (BYTE)BitRead(5);
	h.current_next_indicator   = (BYTE)BitRead(1);
	h.section_number           = (BYTE)BitRead(8);
	h.last_section_number      = (BYTE)BitRead(8);

	return h.section_syntax_indicator == 1 && h.zero == 0;
}

bool CBaseSplitterFileEx::Read(pvahdr& h, bool fSync)
{
	memset(&h, 0, sizeof(h));

	BitByteAlign();

	if (fSync) {
		for (int i = 0; i < 65536; i++) {
			if ((BitRead(64, true)&0xfffffc00ffe00000i64) == 0x4156000055000000i64) {
				break;
			}
			BitRead(8);
		}
	}

	if ((BitRead(64, true)&0xfffffc00ffe00000i64) != 0x4156000055000000i64) {
		return false;
	}

	h.sync = (WORD)BitRead(16);
	h.streamid = (BYTE)BitRead(8);
	h.counter = (BYTE)BitRead(8);
	h.res1 = (BYTE)BitRead(8);
	h.res2 = BitRead(3);
	h.fpts = BitRead(1);
	h.postbytes = BitRead(2);
	h.prebytes = BitRead(2);
	h.length = (WORD)BitRead(16);

	if (h.length > 6136) {
		return false;
	}

	__int64 pos = GetPos();

	if (h.streamid == 1 && h.fpts) {
		h.pts = 10000*BitRead(32)/90 + m_rtPTSOffset;
	} else if (h.streamid == 2 && (h.fpts || (BitRead(32, true)&0xffffffe0) == 0x000001c0)) {
		BYTE b;
		if (!NextMpegStartCode(b, 4)) {
			return false;
		}
		peshdr h2;
		if (!Read(h2, b)) {
			return false;
		}
		// Maybe bug, code before: if (h.fpts = h2.fpts) h.pts = h2.pts;
		h.fpts = h2.fpts;
		if (h.fpts) {
			h.pts = h2.pts;
		}
	}

	BitRead(8*h.prebytes);

	h.length -= (WORD)(GetPos() - pos);

	return true;
}

bool CBaseSplitterFileEx::Read(avchdr& h, int len, CMediaType* pmt)
{
	__int64 endpos		= min(GetPos() + len + 4, GetAvailable());
	__int64 nalstartpos	= GetPos();
	bool repeat			= false;

	// First try search for the start code
	DWORD _dwStartCode = (DWORD)BitRead(32, true);
	while (GetPos() < endpos &&
			(_dwStartCode & 0xFFFFFF1F) != 0x101 &&		// Coded slide of a non-IDR
			(_dwStartCode & 0xFFFFFF1F) != 0x105 &&		// Coded slide of an IDR
			(_dwStartCode & 0xFFFFFF1F) != 0x107 &&		// Sequence Parameter Set
			(_dwStartCode & 0xFFFFFF1F) != 0x108 &&		// Picture Parameter Set
			(_dwStartCode & 0xFFFFFF1F) != 0x109 &&		// Access Unit Delimiter
			(_dwStartCode & 0xFFFFFF1F) != 0x10f		// Subset Sequence Parameter Set (MVC)
		  ) {
		BitRead(8);
		_dwStartCode = (DWORD)BitRead(32, true);
	}
	if (GetPos() >= endpos) {
		return false;
	}

	// At least a SPS (normal or subset) and a PPS is required
	while (GetPos() < endpos && (!(h.spspps[index_sps].complete || h.spspps[index_subsetsps].complete) || !h.spspps[index_pps1].complete || repeat))
	{
		BYTE id = h.lastid;
		repeat = false;

		// Get array index from NAL unit type
		spsppsindex index = index_unknown;

		if ((id&0x60) != 0) {
			if ((id&0x9f) == 0x07) {
				index = index_sps;
			} else if ((id&0x9f) == 0x0F) {
				index = index_subsetsps;
			} else if ((id&0x9f) == 0x08) {
				index = h.spspps[index_pps1].complete ? index_pps2 : index_pps1;
			}
		}

		// Search for next start code
		DWORD dwStartCode = (DWORD)BitRead(32, true);
		while (GetPos() < endpos && (dwStartCode != 0x00000001) && (dwStartCode & 0xFFFFFF00) != 0x00000100) {
			BitRead(8);
			dwStartCode = (DWORD)BitRead(32, true);
		}

		// Skip start code
		__int64 pos;
		if (GetPos() < endpos) {
			if (dwStartCode == 0x00000001)
				BitRead(32);
			else
				BitRead(24);

			pos = GetPos();
			h.lastid = (BYTE)BitRead(8);
		} else {
			pos = GetPos()-4;
		}

		// The SPS or PPS might be fragmented, copy data into buffer until NAL is complete
		if (index >= 0) {
			if (h.spspps[index].complete) {
				// Don't handle SPS/PPS twice
				continue;
			} else if (pos > nalstartpos) {
				// Copy into buffer
				Seek(nalstartpos);
				unsigned int bufsize = _countof(h.spspps[index].buffer);
				int len = min(int(bufsize - h.spspps[index].size), int(pos - nalstartpos));
				ByteRead(h.spspps[index].buffer+h.spspps[index].size, len);
				Seek(pos);
				h.spspps[index].size += len;

				//ASSERT(h.spspps[index].size < bufsize); // disable for better debug ...

				if (h.spspps[index].size >= bufsize || dwStartCode == 0x00000001 || (dwStartCode & 0xFFFFFF00) == 0x00000100) {
					if (Read(h, index)) {
						h.spspps[index].complete = true;
						h.spspps[index].size -= 4;
					} else {
						h.spspps[index].size = 0;
					}
				}

				repeat = true;
			}
		}

		nalstartpos = pos;
	}

	// Exit and wait for next packet if there is no SPS and PPS yet
	if ((!h.spspps[index_sps].complete && !h.spspps[index_subsetsps].complete) || !h.spspps[index_pps1].complete || repeat) {

		return false;
	}

	if (!pmt) {
		return true;
	}

	{
		// Calculate size of extra data
		int extra = 0;
		for (int i = 0; i < 4; i++) {
			if (h.spspps[i].complete) {
				extra += 2 + (h.spspps[i].size);
			}
		}

		CSize aspect(h.hdr.width * h.hdr.sar.num, h.hdr.height * h.hdr.sar.den);
		ReduceDim(aspect);
		if (aspect.cx * 2 < aspect.cy) {
			return false;
		}

		pmt->majortype				= MEDIATYPE_Video;
		pmt->formattype				= FORMAT_MPEG2_VIDEO;
		int len = FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + extra;
		MPEG2VIDEOINFO* vi			= (MPEG2VIDEOINFO*)DNew BYTE[len];
		memset(vi, 0, len);
		// vi->hdr.dwBitRate = ;
		
		vi->hdr.AvgTimePerFrame		= h.hdr.AvgTimePerFrame;
		vi->hdr.dwPictAspectRatioX	= aspect.cx;
		vi->hdr.dwPictAspectRatioY	= aspect.cy;
		vi->hdr.bmiHeader.biSize = sizeof(vi->hdr.bmiHeader);
		vi->hdr.bmiHeader.biWidth	= h.hdr.width;
		vi->hdr.bmiHeader.biHeight	= h.hdr.height;
		if (h.spspps[index_subsetsps].complete && !h.spspps[index_sps].complete) {
			pmt->subtype			= FOURCCMap(vi->hdr.bmiHeader.biCompression = 'CVME');	// MVC stream without base view
		} else if (h.spspps[index_subsetsps].complete && h.spspps[index_sps].complete) {
			pmt->subtype			= FOURCCMap(vi->hdr.bmiHeader.biCompression = 'CVMA');	// MVC stream with base view
		} else {
			pmt->subtype			= FOURCCMap(vi->hdr.bmiHeader.biCompression = '1CVA');	// AVC stream
		}
		vi->dwProfile				= h.hdr.profile;
		vi->dwFlags					= 4; // ?
		vi->dwLevel					= h.hdr.level;
		vi->cbSequenceHeader		= extra;

		// Copy extra data
		BYTE* p = (BYTE*)&vi->dwSequenceHeader[0];
		for (int i = 0; i < 4; i++) {
			if (h.spspps[i].complete) {
				*p++ = (h.spspps[i].size) >> 8;
				*p++ = (h.spspps[i].size) & 0xff;
				memcpy(p, h.spspps[i].buffer, h.spspps[i].size);
				p += h.spspps[i].size;
			}
		}

		pmt->SetFormat((BYTE*)vi, len);
		delete [] vi;
	}

	return true;
}

static void RemoveMpegEscapeCode(BYTE* dst, BYTE* src, int length)
{
	int		si=0;
	int		di=0;
	while (si+2<length) {
		//remove escapes (very rare 1:2^22)
		if (src[si+2]>3) {
			dst[di++]= src[si++];
			dst[di++]= src[si++];
		} else if (src[si]==0 && src[si+1]==0) {
			if (src[si+2]==3) { //escape
				dst[di++]= 0;
				dst[di++]= 0;
				si+=3;
				continue;
			} else { //next start code
				return;
			}
		}

		dst[di++]= src[si++];
	}
}

bool CBaseSplitterFileEx::Read(avchdr& h, spsppsindex index)
{
	// Only care about SPS and subset SPS
	if (index != index_sps && index != index_subsetsps)
		return true;

	// Manage escape codes
	BYTE buffer[MAX_SPSPPS];
	RemoveMpegEscapeCode(buffer, h.spspps[index].buffer, MAX_SPSPPS);
	CGolombBuffer gb(buffer, MAX_SPSPPS);

	gb.BitRead(8);	// nal_unit_type
	if (!ParseAVCHeader(gb, h.hdr, true)) {
		return false;
	}

	if (index == index_subsetsps) {
		if (h.hdr.profile == 83 || h.hdr.profile == 86) {
			// TODO: SVC extensions
			return false;
		} else if (h.hdr.profile == 118 || h.hdr.profile == 128) {
			gb.BitRead(1); // bit_equal_to_one

			// seq_parameter_set_mvc_extension
			h.views = (unsigned int) gb.UExpGolombRead()+1;
		}
	}

	return true;
}

bool CBaseSplitterFileEx::Read(vc1hdr& h, int len, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	__int64 endpos = GetPos() + len; // - sequence header length
	__int64 extrapos = 0, extralen = 0;
	int		nFrameRateNum = 0, nFrameRateDen = 1;

	if (GetPos() < endpos+4 && BitRead(32, true) == 0x0000010F) {
		extrapos = GetPos();

		BitRead(32);

		h.profile			= (BYTE)BitRead(2);

		// Check if advanced profile
		if (h.profile != 3) {
			return false;
		}

		h.level				= (BYTE)BitRead(3);
		h.chromaformat		= (BYTE)BitRead(2);

		// (fps-2)/4 (->30)
		h.frmrtq_postproc	= (BYTE)BitRead(3); //common
		// (bitrate-32kbps)/64kbps
		h.bitrtq_postproc	= (BYTE)BitRead(5); //common
		h.postprocflag		= (BYTE)BitRead(1); //common

		h.width				= ((unsigned int)BitRead(12) + 1) << 1;
		h.height			= ((unsigned int)BitRead(12) + 1) << 1;

		h.broadcast			= (BYTE)BitRead(1);
		h.interlace			= (BYTE)BitRead(1);
		h.tfcntrflag		= (BYTE)BitRead(1);
		h.finterpflag		= (BYTE)BitRead(1);
		BitRead(1); // reserved
		h.psf				= (BYTE)BitRead(1);
		if (BitRead(1)) {
			int ar = 0;
			BitRead(14);
			BitRead(14);
			if (BitRead(1)) {
				ar = (int)BitRead(4);
			}
			if (ar && ar < 14) {
				h.sar.num = pixel_aspect[ar][0];
				h.sar.den = pixel_aspect[ar][1];
			} else if (ar == 15) {
				h.sar.num = (BYTE)BitRead(8);
				h.sar.den = (BYTE)BitRead(8);
			}

			// Read framerate
			const int ff_vc1_fps_nr[5] = { 24, 25, 30, 50, 60 };
			const int ff_vc1_fps_dr[2] = { 1000, 1001 };

			if (BitRead(1)) {
				if (BitRead(1)) {
					nFrameRateNum = 32;
					nFrameRateDen = (int)BitRead(16) + 1;
				} else {
					int nr, dr;
					nr = (int)BitRead(8);
					dr = (int)BitRead(4);
					if (nr && nr < 8 && dr && dr < 3) {
						nFrameRateNum = ff_vc1_fps_dr[dr - 1];
						nFrameRateDen = ff_vc1_fps_nr[nr - 1] * 1000;
					}
				}
			}

		}

		Seek(extrapos + 4);
		extralen	= 0;
		int parse	= 0; // really need a signed type? may be unsigned will be better

		while (GetPos() < endpos+4 && ((parse == 0x0000010E) || (parse & 0xFFFFFF00) != 0x00000100)) {
			parse = (parse<<8) | (int)BitRead(8);
			extralen++;
		}
	}

	if (!extralen) {
		return false;
	}

	if (!pmt) {
		return true;
	}

	{
		if (!h.sar.num) {
			h.sar.num = 1;
		}
		if (!h.sar.den) {
			h.sar.den = 1;
		}
		CSize aspect = CSize(h.width * h.sar.num, h.height * h.sar.den);
		if (h.width == h.sar.num && h.height == h.sar.den) {
			aspect = CSize(h.width, h.height);
		}
		ReduceDim(aspect);

		pmt->majortype				= MEDIATYPE_Video;
		pmt->subtype				= FOURCCMap('1CVW');
		pmt->formattype				= FORMAT_VIDEOINFO2;
		int vi_len					= sizeof(VIDEOINFOHEADER2) + (int)extralen + 1;
		VIDEOINFOHEADER2* vi		= (VIDEOINFOHEADER2*)DNew BYTE[vi_len];
		memset(vi, 0, vi_len);
		vi->AvgTimePerFrame			= (10000000I64*nFrameRateNum)/nFrameRateDen;

		vi->dwPictAspectRatioX		= aspect.cx;
		vi->dwPictAspectRatioY		= aspect.cy;
		vi->bmiHeader.biSize		= sizeof(vi->bmiHeader);
		vi->bmiHeader.biWidth		= h.width;
		vi->bmiHeader.biHeight		= h.height;
		vi->bmiHeader.biCompression	= '1CVW';
		BYTE* p = (BYTE*)vi + sizeof(VIDEOINFOHEADER2);
		*p++ = 0;
		Seek(extrapos);
		ByteRead(p, extralen);
		pmt->SetFormat((BYTE*)vi, vi_len);
		delete [] vi;
	}
	return true;
}

#define pc_seq_header 0x00
bool CBaseSplitterFileEx::Read(dirachdr& h, int len, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if (len < 13) {
		return false;
	}

	if (BitRead(32, true) == 'BBCD') {
		BYTE buffer[1024];
		ByteRead(buffer, min(len, 1024));
		if (buffer[4] != pc_seq_header) {
			return false;
		}

		CGolombBuffer gb(buffer+13, min(len - 13, 1024));
		gb.BitByteAlign();

		if (!ParseDiracHeader(gb, &h.width, &h.height, &h.AvgTimePerFrame)) {
			return false;
		}
		
		if (!pmt) {
			return true;
		}

		{
			pmt->majortype = MEDIATYPE_Video;
			pmt->formattype = FORMAT_VideoInfo;
			pmt->subtype = FOURCCMap('card');

			VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
			memset(pmt->Format(), 0, pmt->FormatLength());

			pvih->AvgTimePerFrame = h.AvgTimePerFrame;
			pvih->bmiHeader.biSize = sizeof(pvih->bmiHeader);
			pvih->bmiHeader.biWidth = h.width;
			pvih->bmiHeader.biHeight = h.height;
			pvih->bmiHeader.biPlanes = 1;
			pvih->bmiHeader.biBitCount = 12;
			pvih->bmiHeader.biCompression = 'card';
			pvih->bmiHeader.biSizeImage = DIBSIZE(pvih->bmiHeader);
		}
	
		return true;
	}

	return false;
}

bool CBaseSplitterFileEx::Read(dvbsub& h, int len, CMediaType* pmt)
{
	memset(&h, 0, sizeof(h));

	if ((BitRead(32, true) & 0xFFFFFF00) == 0x20000f00) {
		static const SUBTITLEINFO SubFormat = { 0, "", L"" };

		pmt->majortype		= MEDIATYPE_Subtitle;
		pmt->subtype		= MEDIASUBTYPE_DVB_SUBTITLES;
		pmt->formattype		= FORMAT_None;
		pmt->SetFormat ((BYTE*)&SubFormat, sizeof(SUBTITLEINFO));

		return true;
	}

	return false;
}

bool CBaseSplitterFileEx::Read(hevchdr& h, int len, CMediaType* pmt, bool find_sync)
{
	__int64 startpos	= GetPos();
	__int64 endpos		= startpos + len;

	int NAL_unit_type	= -1;
	while (GetPos() < endpos && NAL_unit_type != NAL_UNIT_SPS) {
		BYTE id = 0;
		if (!NextMpegStartCode(id, len)) {
			return false;
		}
		NAL_unit_type = (id >> 1) & 0x3F;
		if (!find_sync && NAL_unit_type != NAL_UNIT_VPS) {
			break;
		}
	}

	if (NAL_unit_type != NAL_UNIT_SPS) {
		return false;
	}

	__int64 size = endpos - GetPos();
	BYTE* buf = DNew BYTE[size];
	memset(buf, 0, size);
	ByteRead(buf, size);

	vc_params_t params = {0};
	if (ParseSequenceParameterSet(buf + 1, size - 1, params)) {

		BITMAPINFOHEADER pbmi;
		memset(&pbmi, 0, sizeof(BITMAPINFOHEADER));
		pbmi.biSize			= sizeof(pbmi);
		pbmi.biWidth		= params.width;
		pbmi.biHeight		= params.height;
		pbmi.biCompression	= FCC('HEVC');
		pbmi.biPlanes		= 1;
		pbmi.biBitCount		= 24;

		CSize aspect(pbmi.biWidth, pbmi.biHeight);
		ReduceDim(aspect);

		if (pmt) {
			BYTE* extradata		= NULL;
			size_t extrasize	= 0;

#if (0)
			extrasize			= len;
			extradata			= (BYTE*)malloc(extrasize);
			Seek(startpos);
			ByteRead(extradata, extrasize);
#else
			{
				// fill extradata with VPS/SPS/PPS Nal units
				Seek(startpos);
				__int64 nal_pos	= startpos;
				NAL_unit_type	= -1;

				int vps_present = 0;
				int sps_present = 0;
				int pps_present = 0;
				while (GetPos() < endpos) {
					BYTE id = 0;
					if (!NextMpegStartCode(id, len)) {
						break;
					}
					int nat = (id >> 1) & 0x3F;
					__int64 tmppos = GetPos();

					switch (NAL_unit_type) {
						case NAL_UNIT_VPS:
						case NAL_UNIT_SPS:
						case NAL_UNIT_PPS:
							if (NAL_unit_type == NAL_UNIT_VPS) {
								vps_present++;
							} else if (NAL_unit_type == NAL_UNIT_SPS) {
								sps_present++;
							} else if (NAL_unit_type == NAL_UNIT_PPS) {
								pps_present++;
							}
							
							Seek(nal_pos);
							int size	= tmppos - nal_pos - 4;
							extradata	= (BYTE*)realloc(extradata, extrasize + size);
							ByteRead(extradata + extrasize, size);
							extrasize	+= size;

							break;
					}

					Seek(tmppos);
					nal_pos			= GetPos() - 4;
					NAL_unit_type	= nat;

					if (vps_present && sps_present && pps_present) {
						break;
					}
				}
			}
#endif

			CreateMPEG2VISimple(pmt, &pbmi, 0, aspect, extradata, extrasize);
			free(extradata);
		}

		delete[] buf;
		return true;
	}

	delete[] buf;
	return false;
}

/*

To see working buffer in debugger, look :
	- m_pCache.m_p	 for the cached buffer
	- m_pos			 for current read position

*/
