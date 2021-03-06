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

#include <afxstr.h>
#include "NullRenderers.h"
#include "H264Nalu.h"
#include "MediaTypeEx.h"
#include "vd.h"
#include "text.h"
#include "..\..\include\basestruct.h"

#define LCID_NOSUBTITLES	-1
#define INVALID_TIME		_I64_MIN

#ifndef FCC
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))
#endif

#define SCALE64(a, b, c) (__int64)((double)(a) * (b) / (c))

extern CString			ResStr(UINT nID);

extern int				CountPins(IBaseFilter* pBF, int& nIn, int& nOut, int& nInC, int& nOutC);
extern bool				IsSplitter(IBaseFilter* pBF, bool fCountConnectedOnly = false);
extern bool				IsMultiplexer(IBaseFilter* pBF, bool fCountConnectedOnly = false);
extern bool				IsStreamStart(IBaseFilter* pBF);
extern bool				IsStreamEnd(IBaseFilter* pBF);
extern bool				IsVideoDecoder(IBaseFilter* pBF, bool fCountConnectedOnly = false);
extern bool				IsVideoRenderer(IBaseFilter* pBF);
extern bool				IsVideoRenderer(const CLSID clsid);
extern bool				IsAudioWaveRenderer(IBaseFilter* pBF);

extern IBaseFilter*		GetUpStreamFilter(IBaseFilter* pBF, IPin* pInputPin = NULL);
extern IPin*			GetUpStreamPin(IBaseFilter* pBF, IPin* pInputPin = NULL);
extern IBaseFilter*		GetDownStreamFilter(IBaseFilter* pBF, IPin* pInputPin = NULL);
extern IPin*			GetDownStreamPin(IBaseFilter* pBF, IPin* pInputPin = NULL);
extern IPin*			GetFirstPin(IBaseFilter* pBF, PIN_DIRECTION dir = PINDIR_INPUT);
extern IPin*			GetFirstDisconnectedPin(IBaseFilter* pBF, PIN_DIRECTION dir);
extern void				NukeDownstream(IBaseFilter* pBF, IFilterGraph* pFG);
extern void				NukeDownstream(IPin* pPin, IFilterGraph* pFG);
extern IBaseFilter*		FindFilter(LPCWSTR clsid, IFilterGraph* pFG);
extern IBaseFilter*		FindFilter(const CLSID& clsid, IFilterGraph* pFG);
extern IPin*			FindPin(IBaseFilter* pBF, PIN_DIRECTION direction, const AM_MEDIA_TYPE* pRequestedMT);
extern IPin*			FindPin(IBaseFilter* pBF, PIN_DIRECTION direction, const GUID majortype);
extern CString			GetFilterName(IBaseFilter* pBF);
extern CString			GetPinName(IPin* pPin);
extern IFilterGraph*	GetGraphFromFilter(IBaseFilter* pBF);
extern IBaseFilter*		GetFilterFromPin(IPin* pPin);
extern IPin*			AppendFilter(IPin* pPin, CString DisplayName, IGraphBuilder* pGB);
extern IBaseFilter*		AppendFilter(IPin* pPin, IMoniker* pMoniker, IGraphBuilder* pGB);
extern IPin*			InsertFilter(IPin* pPin, CString DisplayName, IGraphBuilder* pGB);
extern bool				CreateFilter(CString DisplayName, IBaseFilter** ppBF, CString& FriendlyName);

extern void				ExtractMediaTypes(IPin* pPin, CAtlArray<GUID>& types);
extern void				ExtractMediaTypes(IPin* pPin, CAtlList<CMediaType>& mts);
extern bool				ExtractBIH(const AM_MEDIA_TYPE* pmt, BITMAPINFOHEADER* bih);
extern bool				ExtractBIH(IMediaSample* pMS, BITMAPINFOHEADER* bih);
extern bool				ExtractAvgTimePerFrame(const AM_MEDIA_TYPE* pmt, REFERENCE_TIME& rtAvgTimePerFrame);
extern bool				ExtractDim(const AM_MEDIA_TYPE* pmt, int& w, int& h, int& arx, int& ary);

extern CLSID			GetCLSID(IBaseFilter* pBF);
extern CLSID			GetCLSID(IPin* pPin);

extern void				ShowPPage(CString DisplayName, HWND hParentWnd);
extern void				ShowPPage(IUnknown* pUnknown, HWND hParentWnd);

extern bool				IsCLSIDRegistered(LPCTSTR clsid);
extern bool				IsCLSIDRegistered(const CLSID& clsid);

extern void				CStringToBin(CString str, CAtlArray<BYTE>& data);
extern CString			BinToCString(const BYTE* ptr, size_t len);

typedef enum {
	CDROM_NotFound,
	CDROM_Audio,
	CDROM_VideoCD,
	CDROM_DVDVideo,
	CDROM_BDVideo,
	CDROM_Unknown
} cdrom_t;
extern cdrom_t			GetCDROMType(TCHAR drive, CAtlList<CString>& files);
extern CString			GetDriveLabel(TCHAR drive);

extern bool				GetKeyFrames(CString fn, CUIntArray& kfs);

extern DVD_HMSF_TIMECODE	RT2HMSF(REFERENCE_TIME rt, double fps = 0); // use to remember the current position
extern DVD_HMSF_TIMECODE	RT2HMS_r(REFERENCE_TIME rt);                // use only for information (for display on the screen)
extern REFERENCE_TIME		HMSF2RT(DVD_HMSF_TIMECODE hmsf, double fps = 0);
extern CString				ReftimeToString(const REFERENCE_TIME& rtVal);
extern CString				ReftimeToString2(const REFERENCE_TIME& rtVal);
extern CString				DVDtimeToString(const DVD_HMSF_TIMECODE& rtVal, bool bAlwaysShowHours=false);
extern REFERENCE_TIME		StringToReftime(LPCTSTR strVal);
extern REFERENCE_TIME		StringToReftime2(LPCTSTR strVal);

extern void				memsetd(void* dst, unsigned int c, size_t nbytes);
extern void				memsetw(void* dst, unsigned short c, size_t nbytes);

extern CString			GetFriendlyName(CString DisplayName);
extern HRESULT			LoadExternalObject(LPCTSTR path, REFCLSID clsid, REFIID iid, void** ppv);
extern HRESULT			LoadExternalFilter(LPCTSTR path, REFCLSID clsid, IBaseFilter** ppBF);
extern HRESULT			LoadExternalPropertyPage(IPersist* pP, REFCLSID clsid, IPropertyPage** ppPP);
extern void				UnloadExternalObjects();

extern CString			MakeFullPath(LPCTSTR path);

extern CString			GetMediaTypeName(const GUID& guid);
extern GUID				GUIDFromCString(CString str);
extern HRESULT			GUIDFromCString(CString str, GUID& guid);
extern CString			CStringFromGUID(const GUID& guid);

extern CString			ConvertToUTF16(LPCSTR lpMultiByteStr, UINT CodePage);
extern CString			UTF8To16(LPCSTR lpMultiByteStr);
extern CStringA			UTF16To8(LPCWSTR lpWideCharStr);
extern CString			AltUTF8To16(LPCSTR lpMultiByteStr);
extern CString			ISO6391ToLanguage(LPCSTR code);
extern CString			ISO6392ToLanguage(LPCSTR code);

extern bool				IsISO639Language(LPCSTR code);
extern CString			ISO639XToLanguage(LPCSTR code, bool bCheckForFullLangName = false);
extern LCID				ISO6391ToLcid(LPCSTR code);
extern LCID				ISO6392ToLcid(LPCSTR code);
extern CString			ISO6391To6392(LPCSTR code);
extern CString			ISO6392To6391(LPCSTR code);
extern CString			LanguageToISO6392(LPCTSTR lang);

extern bool				DeleteRegKey(LPCTSTR pszKey, LPCTSTR pszSubkey);
extern bool				SetRegKeyValue(LPCTSTR pszKey, LPCTSTR pszSubkey, LPCTSTR pszValueName, LPCTSTR pszValue);
extern bool				SetRegKeyValue(LPCTSTR pszKey, LPCTSTR pszSubkey, LPCTSTR pszValue);

extern void				RegisterSourceFilter(const CLSID& clsid, const GUID& subtype2, LPCTSTR chkbytes, LPCTSTR ext = NULL, ...);
extern void				RegisterSourceFilter(const CLSID& clsid, const GUID& subtype2, const CAtlList<CString>& chkbytes, LPCTSTR ext = NULL, ...);
extern void				UnRegisterSourceFilter(const GUID& subtype);

extern CString			GetDXVAMode(const GUID* guidDecoder);

extern COLORREF			YCrCbToRGB_Rec601(BYTE Y, BYTE Cr, BYTE Cb);
extern COLORREF			YCrCbToRGB_Rec709(BYTE Y, BYTE Cr, BYTE Cb);
extern DWORD			YCrCbToRGB_Rec601(BYTE A, BYTE Y, BYTE Cr, BYTE Cb);
extern DWORD			YCrCbToRGB_Rec709(BYTE A, BYTE Y, BYTE Cr, BYTE Cb);

extern void				TraceFilterInfo(IBaseFilter* pBF);
extern void				TracePinInfo(IPin* pPin);

extern void				SetThreadName(DWORD dwThreadID, LPCSTR szThreadName);
extern void				CorrectComboListWidth(CComboBox& pComboBox);
extern void				CorrectCWndWidth(CWnd* pWnd);

extern void				getExtraData(const BYTE *format, const GUID *formattype, const size_t formatlen, BYTE *extra, unsigned int *extralen);
extern void				audioFormatTypeHandler(const BYTE *format, const GUID *formattype, DWORD *pnSamples, WORD *pnChannels, WORD *pnBitsPerSample, WORD *pnBlockAlign, DWORD *pnBytesPerSec);

extern int				MakeAACInitData(BYTE* pData, int profile, int freq, int channels);
extern bool				MakeMPEG2MediaType(CMediaType& mt, BYTE* seqhdr, DWORD len, int w, int h);
extern HRESULT			CreateMPEG2VIfromAVC(CMediaType* mt, BITMAPINFOHEADER* pbmi, REFERENCE_TIME AvgTimePerFrame, CSize aspect, BYTE* extra, size_t extralen);
extern HRESULT			CreateMPEG2VISimple(CMediaType* mt, BITMAPINFOHEADER* pbmi, REFERENCE_TIME AvgTimePerFrame, CSize aspect, BYTE* extra, size_t extralen);
extern HRESULT			CreateAVCfromH264(CMediaType* mt);

extern void				CreateVorbisMediaType(CMediaType& mt, CAtlArray<CMediaType>& mts, DWORD Channels, DWORD SamplesPerSec, DWORD BitsPerSample, const BYTE* pData, size_t Count);

extern CStringA			VobSubDefHeader(int w, int h, CStringA palette = "");
extern void				CorrectWaveFormatEx(CMediaType& mt);

extern void				ReduceDim(LONG& num, LONG& den);
extern void				ReduceDim(SIZE &dim);
extern SIZE				ReduceDim(double value);

typedef enum FF_FIELD_TYPE{
	PICT_NONE,
	PICT_TOP_FIELD,
	PICT_BOTTOM_FIELD,
	PICT_FRAME
};

typedef enum SUBTITLE_TYPE{
	ST_TEXT,
	ST_VOBSUB,
	ST_DVB,
	ST_HDMV,
	ST_XSUB
};

class CPinInfo : public PIN_INFO
{
public:
	CPinInfo() { pFilter = NULL; }
	~CPinInfo() { if (pFilter) { pFilter->Release(); } }
};

class CFilterInfo : public FILTER_INFO
{
public:
	CFilterInfo() { pGraph = NULL; }
	~CFilterInfo() { if (pGraph) { pGraph->Release(); } }
};

#define BeginEnumFilters(pFilterGraph, pEnumFilters, pBaseFilter) \
	{CComPtr<IEnumFilters> pEnumFilters; \
	if (pFilterGraph && SUCCEEDED(pFilterGraph->EnumFilters(&pEnumFilters))) \
	{ \
		for (CComPtr<IBaseFilter> pBaseFilter; S_OK == pEnumFilters->Next(1, &pBaseFilter, 0); pBaseFilter = NULL) \
		{ \
 
#define EndEnumFilters }}}

#define BeginEnumCachedFilters(pGraphConfig, pEnumFilters, pBaseFilter) \
	{CComPtr<IEnumFilters> pEnumFilters; \
	if (pGraphConfig && SUCCEEDED(pGraphConfig->EnumCacheFilter(&pEnumFilters))) \
	{ \
		for (CComPtr<IBaseFilter> pBaseFilter; S_OK == pEnumFilters->Next(1, &pBaseFilter, 0); pBaseFilter = NULL) \
		{ \
 
#define EndEnumCachedFilters }}}

#define BeginEnumPins(pBaseFilter, pEnumPins, pPin) \
	{CComPtr<IEnumPins> pEnumPins; \
	if (pBaseFilter && SUCCEEDED(pBaseFilter->EnumPins(&pEnumPins))) \
	{ \
		for (CComPtr<IPin> pPin; S_OK == pEnumPins->Next(1, &pPin, 0); pPin = NULL) \
		{ \
 
#define EndEnumPins }}}

#define BeginEnumMediaTypes(pPin, pEnumMediaTypes, pMediaType) \
	{CComPtr<IEnumMediaTypes> pEnumMediaTypes; \
	if (pPin && SUCCEEDED(pPin->EnumMediaTypes(&pEnumMediaTypes))) \
	{ \
		AM_MEDIA_TYPE* pMediaType = NULL; \
		for (; S_OK == pEnumMediaTypes->Next(1, &pMediaType, NULL); DeleteMediaType(pMediaType), pMediaType = NULL) \
		{ \
 
#define EndEnumMediaTypes(pMediaType) } if (pMediaType) DeleteMediaType(pMediaType); }}

#define BeginEnumSysDev(clsid, pMoniker) \
	{CComPtr<ICreateDevEnum> pDevEnum4$##clsid; \
	pDevEnum4$##clsid.CoCreateInstance(CLSID_SystemDeviceEnum); \
	CComPtr<IEnumMoniker> pClassEnum4$##clsid; \
	if (SUCCEEDED(pDevEnum4$##clsid->CreateClassEnumerator(clsid, &pClassEnum4$##clsid, 0)) \
	&& pClassEnum4$##clsid) \
	{ \
		for (CComPtr<IMoniker> pMoniker; pClassEnum4$##clsid->Next(1, &pMoniker, 0) == S_OK; pMoniker = NULL) \
		{ \
 
#define EndEnumSysDev }}}

#define QI(i)  (riid == __uuidof(i)) ? GetInterface((i*)this, ppv) :
#define QI2(i) (riid == IID_##i) ? GetInterface((i*)this, ppv) :

template <typename T> __inline void INITDDSTRUCT(T& dd)
{
	ZeroMemory(&dd, sizeof(dd));
	dd.dwSize = sizeof(dd);
}

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

template <class T>
static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT* phr)
{
	*phr = S_OK;
	CUnknown* punk = DNew T(lpunk, phr);
	if (punk == NULL) {
		*phr = E_OUTOFMEMORY;
	}
	return punk;
}

#define SAFE_DELETE(p)       { if (p) { delete (p);     (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p) { if (p) { delete [] (p);  (p) = NULL; } }
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = NULL; } }
#define SAFE_CLOSE_HANDLE(p) { if (p) { if ((p) != INVALID_HANDLE_VALUE) VERIFY(CloseHandle(p)); (p) = NULL; } }

#define EXIT_ON_ERROR(hres)  { if (FAILED(hres)) return hres; }

#ifndef uint8
	typedef unsigned char		uint8;
	typedef unsigned short		uint16;
	typedef unsigned long		uint32;
	typedef unsigned __int64	uint64;
	typedef signed char			int8;
	typedef signed short		int16;
	typedef signed long			int32;
	typedef signed __int64		int64;
#endif

static const TCHAR* subext[] = {
	L"srt", L"sub", L"smi", L"psb",
	L"ssa", L"ass", L"idx", L"usf",
	L"xss", L"txt", L"rt",  L"sup",
	L"mks"
};

enum {
	IEC61937_AC3                = 0x01,          ///< AC-3 data
	IEC61937_MPEG1_LAYER1       = 0x04,          ///< MPEG-1 layer 1
	IEC61937_MPEG1_LAYER23      = 0x05,          ///< MPEG-1 layer 2 or 3 data or MPEG-2 without extension
	IEC61937_MPEG2_EXT          = 0x06,          ///< MPEG-2 data with extension
	IEC61937_MPEG2_AAC          = 0x07,          ///< MPEG-2 AAC ADTS
	IEC61937_MPEG2_LAYER1_LSF   = 0x08,          ///< MPEG-2, layer-1 low sampling frequency
	IEC61937_MPEG2_LAYER2_LSF   = 0x09,          ///< MPEG-2, layer-2 low sampling frequency
	IEC61937_MPEG2_LAYER3_LSF   = 0x0A,          ///< MPEG-2, layer-3 low sampling frequency
	IEC61937_DTS1               = 0x0B,          ///< DTS type I   (512 samples)
	IEC61937_DTS2               = 0x0C,          ///< DTS type II  (1024 samples)
	IEC61937_DTS3               = 0x0D,          ///< DTS type III (2048 samples)
	IEC61937_ATRAC              = 0x0E,          ///< Atrac data
	IEC61937_ATRAC3             = 0x0F,          ///< Atrac 3 data
	IEC61937_ATRACX             = 0x10,          ///< Atrac 3 plus data
	IEC61937_DTSHD              = 0x11,          ///< DTS HD data
	IEC61937_WMAPRO             = 0x12,          ///< WMA 9 Professional data
	IEC61937_MPEG2_AAC_LSF_2048 = 0x13,          ///< MPEG-2 AAC ADTS half-rate low sampling frequency
	IEC61937_MPEG2_AAC_LSF_4096 = 0x13 | 0x20,   ///< MPEG-2 AAC ADTS quarter-rate low sampling frequency
	IEC61937_EAC3               = 0x15,          ///< E-AC-3 data
	IEC61937_TRUEHD             = 0x16,          ///< TrueHD data
};

#define IsWaveFormatExtensible(wfe) (wfe->wFormatTag == WAVE_FORMAT_EXTENSIBLE && wfe->cbSize == (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)))