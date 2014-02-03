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

#include "stdafx.h"
#include <MMReg.h>
#include "AviFile.h"
#include "AviSplitter.h"

// option names
#define OPT_REGKEY_AVISplit  _T("Software\\MPC-BE Filters\\AVI Splitter")
#define OPT_SECTION_AVISplit _T("Filters\\AVI Splitter")
#define OPT_BadInterleaved   _T("BadInterleavedSuport")
#define OPT_NeededReindex    _T("NeededReindex")

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_Avi},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CAviSplitterFilter), AviSplitterName, MERIT_NORMAL+1, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CAviSourceFilter), AviSourceName, MERIT_NORMAL+1, 0, NULL, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CAviSplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CAviSourceFilter>, NULL, &sudFilter[1]},
	{L"CAviSplitterPropertyPage", &__uuidof(CAviSplitterSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CAviSplitterSettingsWnd>>},
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
	CAtlList<CString> chkbytes;
	chkbytes.AddTail(_T("0,4,,52494646,8,4,,41564920")); // 'RIFF....AVI '
	chkbytes.AddTail(_T("0,4,,52494646,8,4,,41564958")); // 'RIFF....AVIX'
	chkbytes.AddTail(_T("0,4,,52494646,8,4,,414D5620")); // 'RIFF....AMV '

	RegisterSourceFilter(
		CLSID_AsyncReader,
		MEDIASUBTYPE_Avi,
		chkbytes,
		NULL);

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	UnRegisterSourceFilter(MEDIASUBTYPE_Avi);

	return AMovieDllRegisterServer2(FALSE);
}

#include "../../filters/Filters.h"

CFilterApp theApp;

#endif

//
// CAviSplitterFilter
//

CAviSplitterFilter::CAviSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CAviSplitterFilter"), pUnk, phr, __uuidof(this))
	, m_timeformat(TIME_FORMAT_MEDIA_TIME)
	, m_maxTimeStamp(INVALID_TIME)
	, m_bBadInterleavedSuport(true)
	, m_bSetReindex(true)
{
#ifdef REGISTER_FILTER
	CRegKey key;

	if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, OPT_REGKEY_AVISplit, KEY_READ)) {
		DWORD dw;

		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_BadInterleaved, dw)) {
			m_bBadInterleavedSuport = !!dw;
		}

		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_NeededReindex, dw)) {
			m_bSetReindex = !!dw;
		}
	}
#else
	m_bBadInterleavedSuport	= !!AfxGetApp()->GetProfileInt(OPT_SECTION_AVISplit, OPT_BadInterleaved, m_bBadInterleavedSuport);
	m_bSetReindex			= !!AfxGetApp()->GetProfileInt(OPT_SECTION_AVISplit, OPT_NeededReindex, m_bSetReindex);
#endif
}

STDMETHODIMP CAviSplitterFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	*ppv = NULL;

	return
		QI(IAviSplitterFilter)
		QI(ISpecifyPropertyPages)
		QI(ISpecifyPropertyPages2)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CAviSplitterFilter::QueryFilterInfo(FILTER_INFO* pInfo)
{
	CheckPointer(pInfo, E_POINTER);
	ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));

	if (m_pName && m_pName[0]==L'M' && m_pName[1]==L'P' && m_pName[2]==L'C') {
		(void)StringCchCopyW(pInfo->achName, NUMELMS(pInfo->achName), m_pName);
	} else {
		wcscpy_s(pInfo->achName, AviSourceName);
	}
	pInfo->pGraph = m_pGraph;
	if (m_pGraph) {
		m_pGraph->AddRef();
	}

	return S_OK;
}

HRESULT CAviSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pFile.Free();
	m_tFrame.Free();

	m_pFile.Attach(DNew CAviFile(pAsyncReader, hr));
	if (!m_pFile) {
		return E_OUTOFMEMORY;
	}

	if (!m_bBadInterleavedSuport && !m_pFile->IsInterleaved(!!(::GetKeyState(VK_SHIFT)&0x8000))) {
		hr = E_FAIL;
	}

	if (FAILED(hr)) {
		m_pFile.Free();
		return hr;
	}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = m_pFile->GetTotalTime();

	{
		// reindex if needed

		int fReIndex = 0;

		for (DWORD track = 0; track < m_pFile->m_avih.dwStreams; track++) {
			if (!m_pFile->m_strms[track]->cs.GetCount()) {
				fReIndex++;
			}
		}

		if (fReIndex && m_bSetReindex) {
			if (fReIndex == m_pFile->m_avih.dwStreams) {
				m_rtDuration = 0;
			}

			for (DWORD track = 0; track < m_pFile->m_avih.dwStreams; track++) {
				if (!m_pFile->m_strms[track]->cs.GetCount()) {

					m_pFile->EmptyIndex(track);

					m_fAbort = false;

					__int64 pos = m_pFile->GetPos();
					m_pFile->Seek(0);
					UINT64 Size = 0;
					ReIndex(m_pFile->GetLength(), Size, track);

					if (m_fAbort) {
						m_pFile->EmptyIndex(track);
					}
					m_pFile->Seek(pos);

					m_fAbort = false;
					m_nOpenProgress = 100;
				}
			}
		}
	}

	bool fHasIndex = false;

	for (size_t i = 0; !fHasIndex && i < m_pFile->m_strms.GetCount(); i++)
		if (m_pFile->m_strms[i]->cs.GetCount() > 0) {
			fHasIndex = true;
		}

	for (size_t i = 0; i < m_pFile->m_strms.GetCount(); i++) {
		CAviFile::strm_t* s = m_pFile->m_strms[i];

		if (fHasIndex && s->cs.GetCount() == 0) {
			continue;
		}

		CMediaType mt;
		CAtlArray<CMediaType> mts;

		CStringW name, label;

		if (s->strh.fccType == FCC('vids')) {
			label = L"Video";

			ASSERT(s->strf.GetCount() >= sizeof(BITMAPINFOHEADER));

			BITMAPINFOHEADER* pbmi = &((BITMAPINFO*)s->strf.GetData())->bmiHeader;
			RECT rc = {0, 0, pbmi->biWidth, abs(pbmi->biHeight)};
			
			REFERENCE_TIME AvgTimePerFrame = s->strh.dwRate > 0 ? 10000000ui64 * s->strh.dwScale / s->strh.dwRate : 0;
			
			DWORD dwBitRate = 0;
			if (s->cs.GetCount() && AvgTimePerFrame > 0) {
				UINT64 size = 0;
				for (size_t j = 0; j < s->cs.GetCount(); j++) {
					size += s->cs[j].orgsize;
				}
				dwBitRate = (DWORD)(10000000.0 * size * 8 / (s->cs.GetCount() * AvgTimePerFrame) + 0.5);
				// need calculate in double, because the (10000000ui64 * size * 8) can give overflow
			}

			// building a basic media type
			mt.majortype = MEDIATYPE_Video;
			switch (pbmi->biCompression) {
				case BI_RGB:
				case BI_BITFIELDS:
					mt.subtype =
						pbmi->biBitCount == 1 ? MEDIASUBTYPE_RGB1 :
						pbmi->biBitCount == 4 ? MEDIASUBTYPE_RGB4 :
						pbmi->biBitCount == 8 ? MEDIASUBTYPE_RGB8 :
						pbmi->biBitCount == 16 ? MEDIASUBTYPE_RGB565 :
						pbmi->biBitCount == 24 ? MEDIASUBTYPE_RGB24 :
						pbmi->biBitCount == 32 ? MEDIASUBTYPE_ARGB32 :
						MEDIASUBTYPE_NULL;
					break;
				//case BI_RLE8: mt.subtype = MEDIASUBTYPE_RGB8; break;
				//case BI_RLE4: mt.subtype = MEDIASUBTYPE_RGB4; break;
				case FCC('Y8  '): // uncommon fourcc
					mt.subtype = FOURCCMap(pbmi->biCompression = FCC('Y800'));
					break;
				case FCC('V422'): // uncommon fourcc
					mt.subtype = FOURCCMap(pbmi->biCompression = FCC('YUY2'));
					break;
				case FCC('HDYC'): // UYVY with BT709
				case FCC('UYNV'): // uncommon fourcc
				case FCC('UYNY'): // uncommon fourcc
					mt.subtype = FOURCCMap(pbmi->biCompression = FCC('UYVY'));
					break;
				case FCC('P422'):
					mt.subtype = FOURCCMap(pbmi->biCompression = FCC('Y42B'));
					break;
				case FCC('RV24'): // uncommon fourcc
					pbmi->biCompression = BI_RGB;
					mt.subtype = MEDIASUBTYPE_RGB24;
					pbmi->biHeight = -pbmi->biHeight;
					break;
				case FCC('AVRn'): // uncommon fourcc
				case FCC('JPGL'): // uncommon fourcc
					mt.subtype = MEDIASUBTYPE_MJPG;
					break;
				case FCC('mpg2'):
				case FCC('MPG2'):
				case FCC('MMES'):
					mt.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
					break;
				case FCC('MPEG'):
					mt.subtype = MEDIASUBTYPE_MPEG1Payload;
					break;
				case FCC('DXSB'):
				case FCC('DXSA'):
					label = L"XSub";
				default:
					mt.subtype = FOURCCMap(pbmi->biCompression);
			}

			mt.formattype			= FORMAT_VideoInfo;
			VIDEOINFOHEADER* pvih	= (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + (ULONG)s->strf.GetCount() - sizeof(BITMAPINFOHEADER));
			memset(mt.Format(), 0, mt.FormatLength());
			memcpy(&pvih->bmiHeader, s->strf.GetData(), s->strf.GetCount());
			pvih->AvgTimePerFrame	= AvgTimePerFrame;
			pvih->dwBitRate			= dwBitRate;
			pvih->rcSource			= pvih->rcTarget = rc;

			mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0
							 ? s->strh.dwSuggestedBufferSize*3/2
							 : (pvih->bmiHeader.biWidth*pvih->bmiHeader.biHeight*4));
			mts.Add(mt);

			// building a special media type
			switch (pbmi->biCompression) {
				case FCC('HM10'):
					if (s->cs.GetCount()) {
						__int64 cur_pos = m_pFile->GetPos();
						for (size_t i = 0; i < s->cs.GetCount() - 1; i++) {
							if (s->cs[i].orgsize) {
								m_pFile->Seek(s->cs[i].filepos);
								CBaseSplitterFileEx::hevchdr h;
								CMediaType mt2;
								if (m_pFile->Read(h, s->cs[i].orgsize, &mt2)) {
									mts.InsertAt(0, mt2);
								}
							
								break;
							}
						}

						m_pFile->Seek(cur_pos);
					}
				case FCC('mpg2'):
				case FCC('MPG2'):
				case FCC('MMES'):
					if (s->cs.GetCount()) {
						__int64 cur_pos = m_pFile->GetPos();

						m_pFile->Seek(s->cs[0].filepos);
						CBaseSplitterFileEx::seqhdr h;
						CMediaType mt2;
						if (m_pFile->Read(h, s->cs[0].size, &mt2)) {
							mts.InsertAt(0, mt2);
						}
						m_pFile->Seek(cur_pos);
					}
					break;
				case FCC('H264'):
				case FCC('avc1'):
					if (s->strf.GetCount() > sizeof(BITMAPINFOHEADER)) {
						size_t extralen	= s->strf.GetCount() - sizeof(BITMAPINFOHEADER);
						BYTE* extra		= s->strf.GetData() + (s->strf.GetCount() - extralen);

						CSize aspect(pbmi->biWidth, pbmi->biHeight);
						ReduceDim(aspect);

						pbmi->biCompression = FCC('AVC1');

						CMediaType mt2;
						if (SUCCEEDED(CreateMPEG2VIfromAVC(&mt2, pbmi, AvgTimePerFrame, aspect, extra, extralen))) {
							mts.InsertAt(0, mt2);
						}
					}
					break;
			}
		} else if (s->strh.fccType == FCC('auds') || s->strh.fccType == FCC('amva')) {
			label = L"Audio";

			ASSERT(s->strf.GetCount() >= sizeof(WAVEFORMATEX)
				   || s->strf.GetCount() == sizeof(PCMWAVEFORMAT));

			WAVEFORMATEX* pwfe = (WAVEFORMATEX*)s->strf.GetData();

			if (pwfe->nBlockAlign == 0) {
				continue;
			}

			mt.majortype = MEDIATYPE_Audio;
			if (m_pFile->m_isamv) {
				mt.subtype = FOURCCMap(MAKEFOURCC('A','M','V','A'));
			} else {
				mt.subtype = FOURCCMap(pwfe->wFormatTag);
			}
			mt.formattype = FORMAT_WaveFormatEx;
			if (NULL == mt.AllocFormatBuffer(max((ULONG)s->strf.GetCount(), sizeof(WAVEFORMATEX)))) {
				continue;
			}
			memcpy(mt.Format(), s->strf.GetData(), s->strf.GetCount());
			pwfe = (WAVEFORMATEX*)mt.Format();
			if (s->strf.GetCount() == sizeof(PCMWAVEFORMAT)) {
				pwfe->cbSize = 0;
			}
			if (pwfe->wFormatTag == WAVE_FORMAT_PCM) {
				pwfe->nBlockAlign = pwfe->nChannels*pwfe->wBitsPerSample>>3;
			}
			if (pwfe->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
				mt.subtype = ((WAVEFORMATEXTENSIBLE*)pwfe)->SubFormat;
			}
			if (!pwfe->nChannels) {
				pwfe->nChannels = 2;
			}

			mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0
							 ? s->strh.dwSuggestedBufferSize*3/2
							 : (pwfe->nChannels*pwfe->nSamplesPerSec*32>>3));
			mts.Add(mt);
		} else if (s->strh.fccType == FCC('mids')) {
			label = L"Midi";

			mt.majortype = MEDIATYPE_Midi;
			mt.subtype = MEDIASUBTYPE_NULL;
			mt.formattype = FORMAT_None;
			mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0
							 ? s->strh.dwSuggestedBufferSize*3/2
							 : (1024*1024));
			mts.Add(mt);
		} else if (s->strh.fccType == FCC('txts')) {
			label = L"Text";

			mt.majortype = MEDIATYPE_Text;
			mt.subtype = MEDIASUBTYPE_NULL;
			mt.formattype = FORMAT_None;
			mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0
							 ? s->strh.dwSuggestedBufferSize*3/2
							 : (1024*1024));
			mts.Add(mt);
		} else if (s->strh.fccType == FCC('iavs')) {
			label = L"Interleaved";

			ASSERT(s->strh.fccHandler == FCC('dvsd'));

			mt.majortype = MEDIATYPE_Interleaved;
			mt.subtype = FOURCCMap(s->strh.fccHandler);
			mt.formattype = FORMAT_DvInfo;
			mt.SetFormat(s->strf.GetData(), max((ULONG)s->strf.GetCount(), sizeof(DVINFO)));
			mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0
							 ? s->strh.dwSuggestedBufferSize*3/2
							 : (1024*1024));
			mts.Add(mt);
		}

		if (mts.IsEmpty()) {
			TRACE(_T("CAviSourceFilter: Unsupported stream (%d)\n"), i);
			continue;
		}

		//Put filename at front sometime(eg. ~temp.avi) will cause filter graph
		//stop check this pin. Not sure the reason exactly. but it happens.
		//If you know why, please emailto: tomasen@gmail.com
		if (s->strn.IsEmpty()) {
			name.Format(L"%s %u", label, i);
		} else {
			name.Format(L"%s (%s %u)", CStringW(s->strn), label, i);
		}

		HRESULT hr;

		CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CAviSplitterOutputPin(mts, name, this, this, &hr));
		AddOutputPin(i, pPinOut);
	}

	POSITION pos = m_pFile->m_info.GetStartPosition();
	while (pos) {
		DWORD fcc;
		CStringA value;
		m_pFile->m_info.GetNextAssoc(pos, fcc, value);

		switch (fcc) {
			case FCC('INAM'):
				SetProperty(L"TITL", CStringW(value));
				break;
			case FCC('IART'):
				SetProperty(L"AUTH", CStringW(value));
				break;
			case FCC('ICOP'):
				SetProperty(L"CPYR", CStringW(value));
				break;
			case FCC('ISBJ'):
				SetProperty(L"DESC", CStringW(value));
				break;
		}
	}

	m_tFrame.Attach(DNew DWORD[m_pFile->m_avih.dwStreams]);

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CAviSplitterFilter::DemuxInit()
{
	SetThreadName((DWORD)-1, "CAviSplitterFilter");

	if (!m_pFile) {
		return false;
	}

	return true;
}

HRESULT CAviSplitterFilter::ReIndex(__int64 end, UINT64& Size, DWORD TrackNumber)
{
	HRESULT hr = S_OK;

	if (TrackNumber >= m_pFile->m_avih.dwStreams) {
		return E_FAIL;
	}

	while (S_OK == hr && m_pFile->GetPos() < end && SUCCEEDED(hr) && !m_fAbort) {
		__int64 pos = m_pFile->GetPos();

		DWORD id = 0, size;
		if (S_OK != m_pFile->ReadAvi(id) || id == 0) {
			return E_FAIL;
		}

		if (id == FCC('RIFF') || id == FCC('LIST')) {
			if (S_OK != m_pFile->ReadAvi(size) || S_OK != m_pFile->ReadAvi(id)) {
				return E_FAIL;
			}

			size += (size&1) + 8;

			if (id == FCC('AVI ') || id == FCC('AVIX') || id == FCC('movi') || id == FCC('rec ')) {
				hr = ReIndex(pos + size, Size, TrackNumber);
			}
		} else {
			if (S_OK != m_pFile->ReadAvi(size)) {
				return E_FAIL;
			}

			DWORD nTrackNumber = TRACKNUM(id);

			if (nTrackNumber == TrackNumber) {
				CAviFile::strm_t* s = m_pFile->m_strms[nTrackNumber];

				WORD type = TRACKTYPE(id);

				if (type == 'db' || type == 'dc' || /*type == 'pc' ||*/ type == 'wb'
						|| type == 'iv' || type == '__' || type == 'xx') {
					CAviFile::strm_t::chunk c;
					c.filepos	= pos;
					c.size		= Size;
					c.orgsize	= size;
					c.fKeyFrame	= size > 0; // TODO: find a better way...
					c.fChunkHdr	= true;
					s->cs.Add(c);

					Size += s->GetChunkSize(size);

					REFERENCE_TIME rt	= s->GetRefTime((DWORD)s->cs.GetCount()-1, Size);
					m_rtDuration		= max(rt, m_rtDuration);
				}
			}

			size += (size&1) + 8;
		}

		m_pFile->Seek(pos + size);

		m_nOpenProgress = m_pFile->GetPos()*100/m_pFile->GetLength();

		DWORD cmd;
		if (CheckRequest(&cmd)) {
			if (cmd == CMD_EXIT) {
				m_fAbort = true;
			} else {
				Reply(S_OK);
			}
		}
	}

	return hr;
}

void CAviSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	memset((DWORD*)m_tFrame, 0, m_pFile->m_avih.dwStreams * sizeof(DWORD));
	m_pFile->Seek(0);

	DbgLog((LOG_TRACE, 0, _T("Seek: %I64d"), rt / 10000));

	if (rt > 0) {
		for (DWORD track = 0; track < m_pFile->m_avih.dwStreams; track++) {
			CAviFile::strm_t* s = m_pFile->m_strms[track];

			if (s->IsRawSubtitleStream() || s->cs.IsEmpty()) {
				continue;
			}

			//ASSERT(s->GetFrame(rt) == s->GetKeyFrame(rt)); // fast seek test
			m_tFrame[track] = s->GetKeyFrame(rt);
		}
	}
}

bool CAviSplitterFilter::DemuxLoop()
{
	HRESULT hr = S_OK;

	CAtlArray<BOOL> fDiscontinuity;
	fDiscontinuity.SetCount(m_pFile->m_avih.dwStreams);
	memset(fDiscontinuity.GetData(), 0, m_pFile->m_avih.dwStreams * sizeof(BOOL));

	while (SUCCEEDED(hr) && !CheckRequest(nullptr)) {
		DWORD curTrack = DWORD_MAX;

		REFERENCE_TIME minTime = INT64_MAX;
		UINT64 minpos = INT64_MAX;
		for (DWORD track = 0; track < m_pFile->m_avih.dwStreams; track++) { 
			CAviFile::strm_t* s = m_pFile->m_strms[track];
			DWORD f = m_tFrame[track];

			if (f >= (DWORD)s->cs.GetCount()) {
				continue;
			}

			if (s->IsRawSubtitleStream()) {
				// TODO: get subtitle time from index
				minTime  = 0;
				curTrack = track;
				break; // read all subtitles at once
			}

			REFERENCE_TIME start = s->GetRefTime(f, s->cs[f].size);
			if (start < minTime || (start == minTime && s->cs[f].filepos < minpos)) {
				minTime  = start;
				curTrack = track;
				minpos   = s->cs[f].filepos;
			}
		}

		if (minTime == INT64_MAX) {
			return true;
		}

		do {
			CAviFile::strm_t* s = m_pFile->m_strms[curTrack];
			DWORD f = m_tFrame[curTrack];
			//TRACE(_T("CAviFile::DemuxLoop(): track %d, time %I64d, pos %I64d\n"), curTrack, minTime, s->cs[f].filepos);

			m_pFile->Seek(s->cs[f].filepos);
			DWORD size = 0;

			if (s->cs[f].fChunkHdr) {
				DWORD id = 0;
				if (S_OK != m_pFile->ReadAvi(id) || id == 0 || curTrack != TRACKNUM(id) || S_OK != m_pFile->ReadAvi(size)) {
					fDiscontinuity[curTrack] = true;
					break;
				}

				if (size != s->cs[f].orgsize) {
					TRACE(_T("WARNING: CAviFile::DemuxLoop() incorrect chunk size. By index: %d, by header: %d\n"), s->cs[f].orgsize, size);
					fDiscontinuity[curTrack] = true;
					//ASSERT(0);
					//break; // Why so, why break ??? If anyone knows - please describe ...
				}
			} else {
				size = s->cs[f].orgsize;
			}

			CAutoPtr<Packet> p(DNew Packet());

			p->TrackNumber		= (DWORD)curTrack;
			p->bSyncPoint		= (BOOL)s->cs[f].fKeyFrame;
			p->bDiscontinuity	= fDiscontinuity[curTrack];
			p->rtStart			= s->GetRefTime(f, s->cs[f].size);
			p->rtStop			= s->GetRefTime(f + 1, f + 1 < (DWORD)s->cs.GetCount() ? s->cs[f + 1].size : s->totalsize);
			p->SetCount(size);
			if (S_OK != (hr = m_pFile->ByteRead(p->GetData(), p->GetCount()))) {
				return true;    // break;
			}
#if defined(_DEBUG) && 0
			DbgLog((LOG_TRACE, 0,
					_T("%d (%d): %I64d - %I64d, %I64d - %I64d (size = %d)"),
					minTrack, (int)p->bSyncPoint,
					(p->rtStart) / 10000, (p->rtStop) / 10000,
					(p->rtStart - m_rtStart) / 10000, (p->rtStop - m_rtStart) / 10000,
					size));
#endif
			m_maxTimeStamp = max(m_maxTimeStamp, p->rtStart);

			hr = DeliverPacket(p);

			fDiscontinuity[curTrack] = false;
		} while (0);

		m_tFrame[curTrack]++;
	}

	if (m_maxTimeStamp != INVALID_TIME) {
		m_rtCurrent = m_maxTimeStamp;
	}
	return true;
}

// IMediaSeeking

STDMETHODIMP CAviSplitterFilter::GetDuration(LONGLONG* pDuration)
{
	CheckPointer(pDuration, E_POINTER);
	CheckPointer(m_pFile, VFW_E_NOT_CONNECTED);

	if (m_timeformat == TIME_FORMAT_FRAME) {
		for (size_t i = 0; i < m_pFile->m_strms.GetCount(); i++) {
			CAviFile::strm_t* s = m_pFile->m_strms[i];
			if (s->strh.fccType == FCC('vids')) {
				*pDuration = s->cs.GetCount();
				return S_OK;
			}
		}

		return E_UNEXPECTED;
	}

	return __super::GetDuration(pDuration);
}

//

STDMETHODIMP CAviSplitterFilter::IsFormatSupported(const GUID* pFormat)
{
	CheckPointer(pFormat, E_POINTER);
	HRESULT hr = __super::IsFormatSupported(pFormat);
	if (S_OK == hr) {
		return hr;
	}
	return *pFormat == TIME_FORMAT_FRAME ? S_OK : S_FALSE;
}

STDMETHODIMP CAviSplitterFilter::GetTimeFormat(GUID* pFormat)
{
	CheckPointer(pFormat, E_POINTER);
	*pFormat = m_timeformat;
	return S_OK;
}

STDMETHODIMP CAviSplitterFilter::IsUsingTimeFormat(const GUID* pFormat)
{
	CheckPointer(pFormat, E_POINTER);
	return *pFormat == m_timeformat ? S_OK : S_FALSE;
}

STDMETHODIMP CAviSplitterFilter::SetTimeFormat(const GUID* pFormat)
{
	CheckPointer(pFormat, E_POINTER);
	if (S_OK != IsFormatSupported(pFormat)) {
		return E_FAIL;
	}
	m_timeformat = *pFormat;
	return S_OK;
}

STDMETHODIMP CAviSplitterFilter::GetStopPosition(LONGLONG* pStop)
{
	CheckPointer(pStop, E_POINTER);
	if (FAILED(__super::GetStopPosition(pStop))) {
		return E_FAIL;
	}
	if (m_timeformat == TIME_FORMAT_MEDIA_TIME) {
		return S_OK;
	}
	LONGLONG rt = *pStop;
	if (FAILED(ConvertTimeFormat(pStop, &TIME_FORMAT_FRAME, rt, &TIME_FORMAT_MEDIA_TIME))) {
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP CAviSplitterFilter::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat)
{
	CheckPointer(pTarget, E_POINTER);

	const GUID& SourceFormat = pSourceFormat ? *pSourceFormat : m_timeformat;
	const GUID& TargetFormat = pTargetFormat ? *pTargetFormat : m_timeformat;

	if (TargetFormat == SourceFormat) {
		*pTarget = Source;
		return S_OK;
	} else if (TargetFormat == TIME_FORMAT_FRAME && SourceFormat == TIME_FORMAT_MEDIA_TIME) {
		for (size_t i = 0; i < m_pFile->m_strms.GetCount(); i++) {
			CAviFile::strm_t* s = m_pFile->m_strms[i];
			if (s->strh.fccType == FCC('vids')) {
				*pTarget = s->GetFrame(Source);
				return S_OK;
			}
		}
	} else if (TargetFormat == TIME_FORMAT_MEDIA_TIME && SourceFormat == TIME_FORMAT_FRAME) {
		for (size_t i = 0; i < m_pFile->m_strms.GetCount(); i++) {
			CAviFile::strm_t* s = m_pFile->m_strms[i];
			if (s->strh.fccType == FCC('vids')) {
				if (Source < 0 || Source >= (LONGLONG)s->cs.GetCount()) {
					return E_FAIL;
				}
				CAviFile::strm_t::chunk& c = s->cs[(size_t)Source];
				*pTarget = s->GetRefTime((DWORD)Source, c.size);
				return S_OK;
			}
		}
	}

	return E_FAIL;
}

STDMETHODIMP CAviSplitterFilter::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop)
{
	HRESULT hr;
	if (FAILED(hr = __super::GetPositions(pCurrent, pStop)) || m_timeformat != TIME_FORMAT_FRAME) {
		return hr;
	}

	if (pCurrent)
		if (FAILED(ConvertTimeFormat(pCurrent, &TIME_FORMAT_FRAME, *pCurrent, &TIME_FORMAT_MEDIA_TIME))) {
			return E_FAIL;
		}
	if (pStop)
		if (FAILED(ConvertTimeFormat(pStop, &TIME_FORMAT_FRAME, *pStop, &TIME_FORMAT_MEDIA_TIME))) {
			return E_FAIL;
		}

	return S_OK;
}

HRESULT CAviSplitterFilter::SetPositionsInternal(void* id, LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
	if (m_timeformat != TIME_FORMAT_FRAME) {
		return __super::SetPositionsInternal(id, pCurrent, dwCurrentFlags, pStop, dwStopFlags);
	}

	if (!pCurrent && !pStop
			|| (dwCurrentFlags&AM_SEEKING_PositioningBitsMask) == AM_SEEKING_NoPositioning
			&& (dwStopFlags&AM_SEEKING_PositioningBitsMask) == AM_SEEKING_NoPositioning) {
		return S_OK;
	}

	REFERENCE_TIME
	rtCurrent = m_rtCurrent,
	rtStop = m_rtStop;

	if ((dwCurrentFlags&AM_SEEKING_PositioningBitsMask)
			&& FAILED(ConvertTimeFormat(&rtCurrent, &TIME_FORMAT_FRAME, rtCurrent, &TIME_FORMAT_MEDIA_TIME))) {
		return E_FAIL;
	}
	if ((dwStopFlags&AM_SEEKING_PositioningBitsMask)
			&& FAILED(ConvertTimeFormat(&rtStop, &TIME_FORMAT_FRAME, rtStop, &TIME_FORMAT_MEDIA_TIME))) {
		return E_FAIL;
	}

	if (pCurrent)
		switch (dwCurrentFlags&AM_SEEKING_PositioningBitsMask) {
			case AM_SEEKING_NoPositioning:
				break;
			case AM_SEEKING_AbsolutePositioning:
				rtCurrent = *pCurrent;
				break;
			case AM_SEEKING_RelativePositioning:
				rtCurrent = rtCurrent + *pCurrent;
				break;
			case AM_SEEKING_IncrementalPositioning:
				rtCurrent = rtCurrent + *pCurrent;
				break;
		}

	if (pStop)
		switch (dwStopFlags&AM_SEEKING_PositioningBitsMask) {
			case AM_SEEKING_NoPositioning:
				break;
			case AM_SEEKING_AbsolutePositioning:
				rtStop = *pStop;
				break;
			case AM_SEEKING_RelativePositioning:
				rtStop += *pStop;
				break;
			case AM_SEEKING_IncrementalPositioning:
				rtStop = rtCurrent + *pStop;
				break;
		}

	if ((dwCurrentFlags&AM_SEEKING_PositioningBitsMask)
			&& pCurrent)
		if (FAILED(ConvertTimeFormat(pCurrent, &TIME_FORMAT_MEDIA_TIME, rtCurrent, &TIME_FORMAT_FRAME))) {
			return E_FAIL;
		}
	if ((dwStopFlags&AM_SEEKING_PositioningBitsMask)
			&& pStop)
		if (FAILED(ConvertTimeFormat(pStop, &TIME_FORMAT_MEDIA_TIME, rtStop, &TIME_FORMAT_FRAME))) {
			return E_FAIL;
		}

	return __super::SetPositionsInternal(id, pCurrent, dwCurrentFlags, pStop, dwStopFlags);
}

// IKeyFrameInfo

STDMETHODIMP CAviSplitterFilter::GetKeyFrameCount(UINT& nKFs)
{
	if (!m_pFile) {
		return E_UNEXPECTED;
	}

	HRESULT hr = S_OK;

	nKFs = 0;

	for (size_t i = 0; i < m_pFile->m_strms.GetCount(); i++) {
		CAviFile::strm_t* s = m_pFile->m_strms[i];
		if (s->strh.fccType != FCC('vids')) {
			continue;
		}

		for (size_t j = 0; j < s->cs.GetCount(); j++) {
			CAviFile::strm_t::chunk& c = s->cs[j];
			if (c.fKeyFrame) {
				++nKFs;
			}
		}

		if (nKFs == s->cs.GetCount()) {
			hr = S_FALSE;
		}

		break;
	}

	return hr;
}

STDMETHODIMP CAviSplitterFilter::GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs)
{
	CheckPointer(pFormat, E_POINTER);
	CheckPointer(pKFs, E_POINTER);

	if (!m_pFile) {
		return E_UNEXPECTED;
	}
	if (*pFormat != TIME_FORMAT_MEDIA_TIME && *pFormat != TIME_FORMAT_FRAME) {
		return E_INVALIDARG;
	}

	for (size_t i = 0; i < m_pFile->m_strms.GetCount(); i++) {
		CAviFile::strm_t* s = m_pFile->m_strms[i];
		if (s->strh.fccType != FCC('vids')) {
			continue;
		}
		bool fConvertToRefTime = !!(*pFormat == TIME_FORMAT_MEDIA_TIME);

		UINT nKFsTmp = 0;

		for (size_t j = 0; j < s->cs.GetCount() && nKFsTmp < nKFs; j++) {
			if (s->cs[j].fKeyFrame) {
				pKFs[nKFsTmp++] = fConvertToRefTime ? s->GetRefTime(j, s->cs[j].size) : j;
			}
		}
		nKFs = nKFsTmp;

		return S_OK;
	}

	return E_FAIL;
}

// ISpecifyPropertyPages2

STDMETHODIMP CAviSplitterFilter::GetPages(CAUUID* pPages)
{
	CheckPointer(pPages, E_POINTER);

	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
	pPages->pElems[0] = __uuidof(CAviSplitterSettingsWnd);

	return S_OK;
}

STDMETHODIMP CAviSplitterFilter::CreatePage(const GUID& guid, IPropertyPage** ppPage)
{
	CheckPointer(ppPage, E_POINTER);

	if (*ppPage != NULL) {
		return E_INVALIDARG;
	}

	HRESULT hr;

	if (guid == __uuidof(CAviSplitterSettingsWnd)) {
		(*ppPage = DNew CInternalPropertyPageTempl<CAviSplitterSettingsWnd>(NULL, &hr))->AddRef();
	}

	return *ppPage ? S_OK : E_FAIL;
}

// IAviSplitterFilter
STDMETHODIMP CAviSplitterFilter::Apply()
{
#ifdef REGISTER_FILTER
	CRegKey key;
	if (ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, OPT_REGKEY_AVISplit)) {
		key.SetDWORDValue(OPT_BadInterleaved, m_bBadInterleavedSuport);
		key.SetDWORDValue(OPT_NeededReindex, m_bSetReindex);
	}
#else
	AfxGetApp()->WriteProfileInt(OPT_SECTION_AVISplit, OPT_BadInterleaved, m_bBadInterleavedSuport);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_AVISplit, OPT_NeededReindex, m_bSetReindex);
#endif
	return S_OK;
}

STDMETHODIMP CAviSplitterFilter::SetBadInterleavedSuport(BOOL nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_bBadInterleavedSuport = !!nValue;
	return S_OK;
}

STDMETHODIMP_(BOOL) CAviSplitterFilter::GetBadInterleavedSuport()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_bBadInterleavedSuport;
}

STDMETHODIMP CAviSplitterFilter::SetReindex(BOOL nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_bSetReindex = !!nValue;
	return S_OK;
}

STDMETHODIMP_(BOOL) CAviSplitterFilter::GetReindex()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_bSetReindex;
}

//
// CAviSourceFilter
//

CAviSourceFilter::CAviSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CAviSplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}

//
// CAviSplitterOutputPin
//

CAviSplitterOutputPin::CAviSplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
	: CBaseSplitterOutputPin(mts, pName, pFilter, pLock, phr, 0, 1)
{
}

HRESULT CAviSplitterOutputPin::CheckConnect(IPin* pPin)
{
	int iPosition = 0;
	CMediaType mt;
	while (S_OK == GetMediaType(iPosition++, &mt)) {
		if (mt.majortype == MEDIATYPE_Video
				&& (mt.subtype == FOURCCMap(FCC('IV32'))
					|| mt.subtype == FOURCCMap(FCC('IV31'))
					|| mt.subtype == FOURCCMap(FCC('IF09')))) {
			CLSID clsid = GetCLSID(GetFilterFromPin(pPin));
			if (clsid == CLSID_VideoMixingRenderer || clsid == CLSID_OverlayMixer) {
				return E_FAIL;
			}
		}

		mt.InitMediaType();
	}

	return __super::CheckConnect(pPin);
}
