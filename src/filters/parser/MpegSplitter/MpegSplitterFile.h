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

#pragma once

#include <atlbase.h>
#include <atlcoll.h>
#include "../BaseSplitter/BaseSplitter.h"
#include "../../../DSUtil/GolombBuffer.h"

#define NO_SUBTITLE_PID		1 // Fake PID use for the "No subtitle" entry
#define NO_SUBTITLE_NAME	L"No subtitle"

#define ISVALIDPID(pid)		(pid >= 0x10 && pid < 0x1fff)

class CMpegSplitterFile : public CBaseSplitterFileEx
{
	CAtlMap<WORD, BYTE> m_pid2pes;
	CAtlMap<WORD, avchdr> avch;
	CAtlMap<WORD, seqhdr> seqh;

	CAtlMap<DWORD, int> streamPTSCount;

	template<class T>
	class CValidStream {
		BYTE m_nValidStream;
		T m_val;
	public:
		CValidStream() {
			m_nValidStream = 0;
			memset(&m_val, 0, sizeof(m_val));
		}
		void Handle(T& val) {
			if (m_val == val) {
				m_nValidStream++;
			} else {
				m_nValidStream = 0;
			}
			memcpy(&m_val, &val, sizeof(val));
		}
		BOOL IsValid() { return m_nValidStream >= 3; }
	};

	CAtlMap<WORD, CValidStream<latm_aachdr>>	m_aaclatmValid;
	CAtlMap<WORD, CValidStream<aachdr>>			m_aacValid;
	CAtlMap<WORD, CValidStream<mpahdr>>			m_mpaValid;
	CAtlMap<WORD, CValidStream<ac3hdr>>			m_ac3Valid;

	BOOL m_bOpeningCompleted;

	HRESULT Init(IAsyncReader* pAsyncReader);
	void OnComplete(IAsyncReader* pAsyncReader);

public:
	bool m_bIsBD;
	CHdmvClipInfo &m_ClipInfo;
	CMpegSplitterFile(IAsyncReader* pAsyncReader, HRESULT& hr, CHdmvClipInfo &ClipInfo, bool bIsBD, bool ForcedSub, int AC3CoreOnly, bool m_AlternativeDuration, bool SubEmptyPin);

	REFERENCE_TIME NextPTS(DWORD TrackNum);

	CCritSec m_csProps;

	MPEG_TYPES m_type;

	REFERENCE_TIME m_rtMin, m_rtMax;
	__int64 m_posMin, m_posMax;
	int m_rate; // byte/sec

	int m_AC3CoreOnly;
	bool m_ForcedSub, m_AlternativeDuration, m_SubEmptyPin;

	struct stream {
		CMediaType mt;
		std::vector<CMediaType> mts;
		WORD pid;
		BYTE pesid, ps1id;
		bool lang_set;
		char lang[4];

		stream() {
			pid			= 0;
			pesid		= 0;
			ps1id		= 0;
			lang_set	= false;
			mt.InitMediaType();
			memset(lang, 0, _countof(lang));
		}

		operator DWORD() const {
			return pid ? pid : ((pesid<<8)|ps1id);
		}

		bool operator == (const struct stream& s) const {
			return (DWORD)*this == (DWORD)s;
		}
	};

	enum stream_type { video, audio, subpic, stereo, unknown };

	class CStreamList : public CAtlList<stream>
	{
	public:
		void Insert(stream& s, int type) {
			if (type == stream_type::subpic) {
				if (s.pid == NO_SUBTITLE_PID) {
					AddTail(s);
					return;
				}
				for (POSITION pos = GetHeadPosition(); pos; GetNext(pos)) {
					stream& s2 = GetAt(pos);
					if (s.pid < s2.pid || s2.pid == NO_SUBTITLE_PID) {
						InsertBefore(pos, s);
						return;
					}
				}
				AddTail(s);
			} else {
				Insert(s);
			}
		}

		void Insert(stream& s) {
			AddTail(s);
			if (GetCount() > 1) {
				for (size_t j = 0; j < GetCount(); j++) {
					for (size_t i = 0; i < GetCount() - 1; i++) {
						if (GetAt(FindIndex(i)) > GetAt(FindIndex(i+1))) {
							SwapElements(FindIndex(i), FindIndex(i+1));
						}
					}
				}
			}
		}

		void Replace(stream& source, stream& dest) {
			for (POSITION pos = GetHeadPosition(); pos; GetNext(pos)) {
				stream& s = GetAt(pos);
				if (source == s) {
					SetAt(pos, dest);
					return;
				}
			}
		}

		static CStringW ToString(int type) {
			return
				type == video	? L"Video" :
				type == audio	? L"Audio" :
				type == subpic	? L"Subtitle" :
				type == stereo	? L"Stereo" :
				L"Unknown";
		}

		const stream* FindStream(int pid) {
			for (POSITION pos = GetHeadPosition(); pos; GetNext(pos)) {
				const stream& s = GetAt(pos);
				if (s.pid == pid) {
					return &s;
				}
			}

			return NULL;
		}

	} m_streams[unknown];

	void SearchStreams(__int64 start, __int64 stop, IAsyncReader* pAsyncReader, BOOL CalcDuration = FALSE);
	DWORD AddStream(WORD pid, BYTE pesid, BYTE ps1id, DWORD len);
	void  AddHdmvPGStream(WORD pid, const char* language_code);
	CAtlList<stream>* GetMasterStream();
	bool IsHdmv() {
		return m_ClipInfo.IsHdmv();
	};

	// program map table - mpeg-ts
	struct program {
		WORD program_number;
		struct stream {
			WORD			pid;
			PES_STREAM_TYPE	type;

		};
		stream streams[64];

		BYTE ts_buffer[1024];
		WORD ts_len_cur, ts_len_packet;

		struct program() {
			memset(this, 0, sizeof(*this));
		}
	};
	CAtlMap<WORD, program> m_programs;

	void SearchPrograms(__int64 start, __int64 stop);
	void UpdatePrograms(const trhdr& h, bool UpdateLang = true);
	void UpdatePrograms(CGolombBuffer gb, WORD pid, bool UpdateLang = true);
	const program* FindProgram(WORD pid, int &iStream, const CHdmvClipInfo::Stream * &_pClipInfo);

	// program stream map - mpeg-ps
	PES_STREAM_TYPE m_psm[256];
	void UpdatePSM();

	CAtlMap<DWORD, CStringA> m_pPMT_Lang;

	bool GetStreamType(WORD pid, PES_STREAM_TYPE &stream_type);

	BOOL bIsBadPacked;
};
