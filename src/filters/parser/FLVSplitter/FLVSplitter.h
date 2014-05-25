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

#include "../BaseSplitter/BaseSplitter.h"

#define FlvSplitterName L"MPC FLV Splitter"
#define FlvSourceName   L"MPC FLV Source"

class __declspec(uuid("47E792CF-0BBE-4F7A-859C-194B0768650A"))
	CFLVSplitterFilter : public CBaseSplitterFilter
{
	UINT32	m_DataOffset;
	bool	m_IgnorePrevSizes;

	UINT32	m_TimeStampOffset;
	bool	m_DetectWrongTimeStamp;

	bool Sync(__int64& pos);

	struct VideoTweak {
		BYTE x;
		BYTE y;
	};

	bool ReadTag(VideoTweak& t);

	struct Tag {
		UINT32 PreviousTagSize;
		BYTE TagType;
		UINT32 DataSize;
		UINT32 TimeStamp;
		UINT32 StreamID;
	};

	bool ReadTag(Tag& t);

	struct AudioTag {
		BYTE SoundFormat;
		BYTE SoundRate;
		BYTE SoundSize;
		BYTE SoundType;
	};

	bool ReadTag(AudioTag& at);

	struct VideoTag {
		BYTE	FrameType;
		BYTE	CodecID;
		BYTE	AVCPacketType;
		UINT32	tsOffset;
	};

	bool ReadTag(VideoTag& vt);

	//struct MetaInfo {
	//	bool    parsed;
	//	double  duration;
	//	double  videodatarate;
	//	double  audiodatarate;
	//	double  videocodecid;
	//	double  audiocodecid;
	//	double  audiosamplerate;
	//	double  audiosamplesize;
	//	bool    stereo;
	//	double  width;
	//	double  height;
	//	int     HM_compatibility;
	//	double  *times;
	//	__int64 *filepositions;
	//	int     keyframenum;
	//};
	//MetaInfo meta;

	void NormalSeek(REFERENCE_TIME rt);
	void AlternateSeek(REFERENCE_TIME rt);

	enum AMF_DATA_TYPE {
		AMF_DATA_TYPE_EMPTY			= -1,
		AMF_DATA_TYPE_NUMBER		= 0x00,
		AMF_DATA_TYPE_BOOL			= 0x01,
		AMF_DATA_TYPE_STRING		= 0x02,
		AMF_DATA_TYPE_OBJECT		= 0x03,
		AMF_DATA_TYPE_NULL			= 0x05,
		AMF_DATA_TYPE_UNDEFINED		= 0x06,
		AMF_DATA_TYPE_REFERENCE		= 0x07,
		AMF_DATA_TYPE_MIXEDARRAY	= 0x08,
		AMF_DATA_TYPE_ARRAY			= 0x0a,
		AMF_DATA_TYPE_DATE			= 0x0b,
		AMF_DATA_TYPE_LONG_STRING	= 0x0c,
		AMF_DATA_TYPE_UNSUPPORTED	= 0x0d,
	};

	struct AMF0 {
		AMF_DATA_TYPE type;
		CString	name;
		CString value_s;
		double	value_d;
		bool	value_b;

		AMF0() {
			type	= AMF_DATA_TYPE_EMPTY;
			value_d	= 0;
			value_b	= 0;
		}

		operator CString() const {
			return value_s;
		}
		operator double() const {
			return value_d;
		}
		operator bool() const {
			return value_b;
		}
	};

	CAtlArray<SyncPoint> m_sps;

	CString AMF0GetString(UINT64 end);
	bool ParseAMF0(UINT64 end, const CString key, CAtlArray<AMF0> &AMF0Array);

protected:
	CAutoPtr<CBaseSplitterFileEx> m_pFile;
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

public:
	CFLVSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);

	// CBaseFilter
	STDMETHODIMP_(HRESULT) QueryFilterInfo(FILTER_INFO* pInfo);

	// IKeyFrameInfo
	STDMETHODIMP_(HRESULT) GetKeyFrameCount(UINT& nKFs);
	STDMETHODIMP_(HRESULT) GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs);
};

class __declspec(uuid("C9ECE7B3-1D8E-41F5-9F24-B255DF16C087"))
	CFLVSourceFilter : public CFLVSplitterFilter
{
public:
	CFLVSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};