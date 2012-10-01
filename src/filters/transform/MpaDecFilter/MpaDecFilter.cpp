/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include <math.h>
#include <atlbase.h>
#include <MMReg.h>
#include <sys/timeb.h>
#include "MpaDecFilter.h"

#include "../../../DSUtil/DSUtil.h"
#include "../../../DSUtil/AudioParser.h"

#ifdef REGISTER_FILTER
void* __imp_toupper   = toupper;
void* __imp_time64    = _time64;
void* __imp_vscprintf = _vscprintf;

#include <InitGuid.h>

extern "C" {
	void __mingw_raise_matherr(int typ, const char* name, double a1, double a2, double rslt) {}
}
#endif // REGISTER_FILTER

#include "moreuuids.h"

#include <vector>

#include "ffmpeg/libavcodec/avcodec.h"

// options names
#define OPT_REGKEY_MpaDec   _T("Software\\MPC-BE Filters\\MPEG Audio Decoder")
#define OPT_SECTION_MpaDec  _T("Filters\\MPEG Audio Decoder")
#define OPTION_SFormat_i16  _T("SampleFormat_int16")
#define OPTION_SFormat_i24  _T("SampleFormat_int24")
#define OPTION_SFormat_i32  _T("SampleFormat_int32")
#define OPTION_SFormat_flt  _T("SampleFormat_float")
#define OPTION_Mixer        _T("Mixer")
#define OPTION_MixerLayout  _T("MixerLayout")
#define OPTION_DRC          _T("DRC")
#define OPTION_SPDIF_ac3    _T("SPDIF_ac3")
#define OPTION_SPDIF_dts    _T("SPDIF_dts")

#define INT24_MAX       8388607i32
#define INT24_MIN     (-8388607i32 - 1)

#define INT8_PEAK       128
#define INT16_PEAK      32768
#define INT24_PEAK      8388608
#define INT32_PEAK      2147483648

#define F16MAX ( float(INT16_MAX) / INT16_PEAK)
#define F24MAX ( float(INT24_MAX) / INT24_PEAK)
#define D32MAX (double(INT32_MAX) / INT32_PEAK)

#define round_f(x) ((x) > 0 ? (x) + 0.5f : (x) - 0.5f)
#define round_d(x) ((x) > 0 ? (x) + 0.5  : (x) - 0.5)

#define AC3_HEADER_SIZE 7
#define MAX_JITTER      1000000i64 // +-100ms jitter is allowed for now 

#define PADDING_SIZE    FF_INPUT_BUFFER_PADDING_SIZE

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
	// MPEG Audio
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MP3},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG1AudioPayload},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG1Payload},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG1Packet},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MPEG2_AUDIO},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_MPEG2_AUDIO},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_MPEG2_AUDIO},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG2_AUDIO},
	// AC3, E-AC3, TrueHD
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DOLBY_AC3},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DOLBY_AC3},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DOLBY_AC3},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DOLBY_AC3},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DOLBY_DDPLUS},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DOLBY_DDPLUS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DOLBY_DDPLUS},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DOLBY_TRUEHD},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DOLBY_TRUEHD},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DOLBY_TRUEHD},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_WAVE_DOLBY_AC3},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MLP},
	// DTS
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DTS},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DTS},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DTS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DTS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_WAVE_DTS},
	// LPCM	
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_HDMV_LPCM_AUDIO},
	// AAC
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_AAC},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_AAC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_AAC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_LATM_AAC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_AAC_ADTS},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_mp4a},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_mp4a},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_mp4a},
	// AMR
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_AMR},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_SAMR},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_SAWB},
	// PS2Audio
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_PS2_PCM},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_PS2_PCM},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PS2_PCM},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_PS2_ADPCM},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_PS2_ADPCM},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PS2_ADPCM},
	// Vorbis
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_Vorbis2},
	// Flac
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_FLAC_FRAMED},
	// NellyMoser
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_NELLYMOSER},
	// PCM
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_NONE},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_RAW},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_TWOS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_SOWT},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_IN24},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_IN32},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_FL32},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_FL64},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_IEEE_FLOAT},  // only for 64-bit float PCM
	// ADPCM
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_IMA4},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_ADPCM_SWF},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_ADPCM_AMV},
	// RealAudio
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_14_4},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_28_8},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_ATRC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_COOK},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DNET},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_SIPR},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_RAAC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_RACP},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_RALF},
	// Alac
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_ALAC},
	// ALS
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_ALS},
	// QDM2
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_QDM2},
	// WavPack
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_WAVPACK4},
	// MusePack
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPC7},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPC8},
	// APE
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_APE},
	// TTA
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_TTA1},
	// TRUESPEECH
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_TRUESPEECH},
	// Windows Media Audio 9 Professional
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_WMAUDIO3},
	// Windows Media Audio Lossless
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_WMAUDIO_LOSSLESS},
	// Windows Media Audio 1, 2
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MSAUDIO1},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_WMAUDIO2},
	// Windows Media Audio Voice
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_WMSP1},
	// Bink Audio
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_BINKA_DCT},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_BINKA_RDFT},
	// Indeo Audio
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_IAC},
	// Opus Audio
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_OPUS},
};

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM},
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CMpaDecFilter), MPCAudioDecName, /*MERIT_DO_NOT_USE*/0x40000001, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, &__uuidof(CMpaDecFilter), CreateInstance<CMpaDecFilter>, NULL, &sudFilter[0]},
	{L"CMpaDecPropertyPage", &__uuidof(CMpaDecSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CMpaDecSettingsWnd> >},
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

//

#include "../../core/FilterApp.h"

CFilterApp theApp;

#endif

#pragma warning(disable : 4245)
static struct scmap_t {
	WORD nChannels;
	BYTE ch[8];
	DWORD dwChannelMask;
}
// dshow: left, right, center, LFE, left surround, right surround
// lets see how we can map these things to dshow (oh the joy!)

s_scmap_hdmv[] = {
	//   FL FR FC LFe BL BR FLC FRC
	{0, {-1,-1,-1,-1,-1,-1,-1,-1 }, 0}, // INVALID
	{1, { 0,-1,-1,-1,-1,-1,-1,-1 }, 0}, // Mono    M1, 0
	{0, {-1,-1,-1,-1,-1,-1,-1,-1 }, 0}, // INVALID
	{2, { 0, 1,-1,-1,-1,-1,-1,-1 }, 0}, // Stereo  FL, FR
	{4, { 0, 1, 2,-1,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER},															// 3/0      FL, FR, FC
	{4, { 0, 1, 2,-1,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY},															// 2/1      FL, FR, Surround
	{4, { 0, 1, 2, 3,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY},										// 3/1      FL, FR, FC, Surround
	{4, { 0, 1, 2, 3,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},											// 2/2      FL, FR, BL, BR
	{6, { 0, 1, 2, 3, 4,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},						// 3/2      FL, FR, FC, BL, BR
	{6, { 0, 1, 2, 5, 3, 4,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// 3/2+LFe  FL, FR, FC, BL, BR, LFe
	{8, { 0, 1, 2, 3, 6, 4, 5,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT},// 3/4  FL, FR, FC, BL, Bls, Brs, BR
	{8, { 0, 1, 2, 7, 4, 5, 3, 6 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT},// 3/4+LFe  FL, FR, FC, BL, Bls, Brs, BR, LFe
};
#pragma warning(default : 4245)

static struct channel_mode_t {
	WORD channels;
	DWORD ch_layout;
	LPCTSTR op_value;
}
channel_mode[] = {
	//n  libavcodec                           ID          Name
	{1, AV_CH_LAYOUT_MONO   , _T("1.0") }, // SPK_MONO   "Mono"
	{2, AV_CH_LAYOUT_STEREO , _T("2.0") }, // SPK_STEREO "Stereo"
	{4, AV_CH_LAYOUT_QUAD   , _T("4.0") }, // SPK_4_0    "4.0"
	{6, AV_CH_LAYOUT_5POINT1, _T("5.1") }, // SPK_5_1    "5.1"
	{8, AV_CH_LAYOUT_7POINT1, _T("7.1") }, // SPK_7_1    "7.1"
};

enum MPCSampleFormat conv_sample_fmt(enum AVSampleFormat av_samplefmt)
{
	switch (av_samplefmt) {
		default:
		case AV_SAMPLE_FMT_S16:
			return SF_PCM16;
		case AV_SAMPLE_FMT_S32:
			return SF_PCM32;
		case AV_SAMPLE_FMT_FLT:
			return SF_FLOAT;
	}
}

CMpaDecFilter::CMpaDecFilter(LPUNKNOWN lpunk, HRESULT* phr)
	: CTransformFilter(NAME("CMpaDecFilter"), lpunk, __uuidof(this))
	, m_bResync(false)
	, m_buff(PADDING_SIZE)
{
	if (phr) {
		*phr = S_OK;
	}

	m_pInput = DNew CMpaDecInputPin(this, phr, L"In");
	if (!m_pInput) {
		*phr = E_OUTOFMEMORY;
	}
	if (FAILED(*phr)) {
		return;
	}

	m_pOutput = DNew CTransformOutputPin(NAME("CTransformOutputPin"), this, phr, L"Out");
	if (!m_pOutput) {
		*phr = E_OUTOFMEMORY;
	}
	if (FAILED(*phr)) {
		delete m_pInput, m_pInput = NULL;
		return;
	}

	m_InputParams.Reset();

	// default settings
	m_fSampleFmt[SF_PCM16] = true;
	m_fSampleFmt[SF_PCM24] = false;
	m_fSampleFmt[SF_PCM32] = false;
	m_fSampleFmt[SF_FLOAT] = false;
	m_fMixer        = false;
	m_iMixerLayout  = SPK_STEREO;
	m_fDRC          = false;
	m_fSPDIF[ac3]   = false;
	m_fSPDIF[dts]   = false;

	// read settings
	CString layout_str;
#ifdef REGISTER_FILTER
	CRegKey key;
	ULONG len = 8;
	if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, OPT_REGKEY_MpaDec, KEY_READ)) {
		DWORD dw;
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPTION_SFormat_i16, dw)) {
			m_fSampleFmt[SF_PCM16] = !!dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPTION_SFormat_i24, dw)) {
			m_fSampleFmt[SF_PCM24] = !!dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPTION_SFormat_i32, dw)) {
			m_fSampleFmt[SF_PCM32] = !!dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPTION_SFormat_flt, dw)) {
			m_fSampleFmt[SF_FLOAT] = !!dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPTION_Mixer, dw)) {
			m_fMixer = !!dw;
		}
		if (ERROR_SUCCESS == key.QueryStringValue(OPTION_MixerLayout, layout_str.GetBuffer(8), &len)) {
			layout_str.ReleaseBufferSetLength(len);
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPTION_DRC, dw)) {
			m_fDRC = !!dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPTION_SPDIF_ac3, dw)) {
			m_fSPDIF[ac3] = !!dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPTION_SPDIF_dts, dw)) {
			m_fSPDIF[dts] = !!dw;
		}
	}
#else
	m_fSampleFmt[SF_PCM16] = !!AfxGetApp()->GetProfileInt(OPT_SECTION_MpaDec, OPTION_SFormat_i16, m_fSampleFmt[SF_PCM16]);
	m_fSampleFmt[SF_PCM24] = !!AfxGetApp()->GetProfileInt(OPT_SECTION_MpaDec, OPTION_SFormat_i24, m_fSampleFmt[SF_PCM24]);
	m_fSampleFmt[SF_PCM32] = !!AfxGetApp()->GetProfileInt(OPT_SECTION_MpaDec, OPTION_SFormat_i32, m_fSampleFmt[SF_PCM32]);
	m_fSampleFmt[SF_FLOAT] = !!AfxGetApp()->GetProfileInt(OPT_SECTION_MpaDec, OPTION_SFormat_flt, m_fSampleFmt[SF_FLOAT]);
	m_fMixer               = !!AfxGetApp()->GetProfileInt(OPT_SECTION_MpaDec, OPTION_Mixer, m_fMixer);
	layout_str             = AfxGetApp()->GetProfileString(OPT_SECTION_MpaDec, OPTION_MixerLayout, channel_mode[m_iMixerLayout].op_value);
	m_fDRC                 = !!AfxGetApp()->GetProfileInt(OPT_SECTION_MpaDec, OPTION_DRC, m_fDRC);
	m_fSPDIF[ac3]          = !!AfxGetApp()->GetProfileInt(OPT_SECTION_MpaDec, OPTION_SPDIF_ac3, m_fSPDIF[ac3]);
	m_fSPDIF[dts]          = !!AfxGetApp()->GetProfileInt(OPT_SECTION_MpaDec, OPTION_SPDIF_dts, m_fSPDIF[dts]);
#endif
	if (!(m_fSampleFmt[SF_PCM16] || m_fSampleFmt[SF_PCM24] || m_fSampleFmt[SF_PCM32] || m_fSampleFmt[SF_FLOAT])) {
		m_fSampleFmt[SF_PCM16] = true;
	}

	for (int i = SPK_MONO; i <= SPK_7_1; i++) {
		if (layout_str == channel_mode[i].op_value) {
			m_iMixerLayout = i;
			break;
		}
	}
}

CMpaDecFilter::~CMpaDecFilter()
{
	StopStreaming();
}

STDMETHODIMP CMpaDecFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(IMpaDecFilter)
		QI(ISpecifyPropertyPages)
		QI(ISpecifyPropertyPages2)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CMpaDecFilter::EndOfStream()
{
	CAutoLock cAutoLock(&m_csReceive);

	enum AVCodecID nCodecId = m_FFAudioDec.GetCodecId();
	if (nCodecId != AV_CODEC_ID_NONE) {
		ProcessFFmpeg(nCodecId); // process the remaining data in the buffer.
		// LOOKATTHIS // necessary or not?
	}

	return __super::EndOfStream();
}

HRESULT CMpaDecFilter::BeginFlush()
{
	return __super::BeginFlush();
}

HRESULT CMpaDecFilter::EndFlush()
{
	CAutoLock cAutoLock(&m_csReceive);
	m_buff.RemoveAll();
	return __super::EndFlush();
}

HRESULT CMpaDecFilter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CAutoLock cAutoLock(&m_csReceive);
	m_buff.RemoveAll();
	m_ps2_state.sync = false;
	m_FFAudioDec.FlushBuffers();

	m_bResync = true;
	m_rtStart = 0; // LOOKATTHIS // reset internal timer?

	return __super::NewSegment(tStart, tStop, dRate);
}

HRESULT CMpaDecFilter::Receive(IMediaSample* pIn)
{
	CAutoLock cAutoLock(&m_csReceive);

	HRESULT hr;

	AM_SAMPLE2_PROPERTIES* const pProps = m_pInput->SampleProps();
	if (pProps->dwStreamId != AM_STREAM_MEDIA) {
		return m_pOutput->Deliver(pIn);
	}

	AM_MEDIA_TYPE* pmt;
	if (SUCCEEDED(pIn->GetMediaType(&pmt)) && pmt) {
		CMediaType mt(*pmt);
		m_pInput->SetMediaType(&mt);
		DeleteMediaType(pmt);
		pmt = NULL;
	}

	BYTE* pDataIn = NULL;
	if (FAILED(hr = pIn->GetPointer(&pDataIn))) {
		return hr;
	}

	long len = pIn->GetActualDataLength();
	// skip empty packet (StreamBufferSource can produce empty data)
	if (len == 0) {
		return S_OK;
	}

	(static_cast<CDeCSSInputPin*>(m_pInput))->StripPacket(pDataIn, len);

	REFERENCE_TIME rtStart = _I64_MIN, rtStop = _I64_MIN;
	hr = pIn->GetTime(&rtStart, &rtStop);

#if 0
	if (SUCCEEDED(hr)) {
		TRACE(_T("CMpaDecFilter::Receive(): rtStart = %10I64d, rtStop = %10I64d\n"), rtStart, rtStop);
	} else {
		TRACE(_T("CMpaDecFilter::Receive(): frame without timestamp\n"));
	}
#endif

	if (pIn->IsDiscontinuity() == S_OK) {
		m_fDiscontinuity = true;
		m_buff.RemoveAll();
		// LOOKATTHIS // m_rtStart = rtStart;
		m_bResync = true;
		if (FAILED(hr)) {
			TRACE(_T("CMpaDecFilter::Receive() : Discontinuity without timestamp\n"));
			// LOOKATTHIS // return S_OK;
		}
	}

	const GUID& subtype = m_pInput->CurrentMediaType().subtype;

	if (subtype == MEDIASUBTYPE_COOK && S_OK == pIn->IsSyncPoint() ||
			_abs64(m_rtStart - rtStart) > MAX_JITTER && subtype != MEDIASUBTYPE_COOK && subtype != MEDIASUBTYPE_ATRC && subtype != MEDIASUBTYPE_SIPR) {
		m_bResync = true;
	}

	if (SUCCEEDED(hr) && m_bResync) {
		m_buff.RemoveAll();
		if (rtStart != _I64_MIN) { // LOOKATTHIS
			m_rtStart = rtStart;
		}
		m_bResync = false;
	}

	size_t bufflen = m_buff.GetCount();
	m_buff.SetCount(bufflen + len, 4096);
	memcpy(m_buff.GetData() + bufflen, pDataIn, len);
	len += (long)bufflen;

	if (GetSPDIF(ac3) && (subtype == MEDIASUBTYPE_DOLBY_AC3 || subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3 || subtype == MEDIASUBTYPE_DNET)) {
		return ProcessAC3_SPDIF();
	}
	else if (GetSPDIF(dts) && (subtype == MEDIASUBTYPE_DTS || subtype == MEDIASUBTYPE_WAVE_DTS)) {
		return ProcessDTS_SPDIF();
	}
	enum AVCodecID nCodecId = FindCodec(subtype);
	if (nCodecId != AV_CODEC_ID_NONE) {
		return ProcessFFmpeg(nCodecId);
	}

	if (subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) {
		hr = ProcessLPCM();
	} else if (subtype == MEDIASUBTYPE_HDMV_LPCM_AUDIO) {
		// TODO: check if the test is really correct
		hr = ProcessHdmvLPCM(pIn->IsSyncPoint() == S_FALSE);
	} else if (subtype == MEDIASUBTYPE_PS2_PCM) {
		hr = ProcessPS2PCM();
	} else if (subtype == MEDIASUBTYPE_PS2_ADPCM) {
		hr = ProcessPS2ADPCM();
	} else if (subtype == MEDIASUBTYPE_PCM_NONE ||
			   subtype == MEDIASUBTYPE_PCM_RAW) {
		hr = ProcessPCMraw();
	} else if (subtype == MEDIASUBTYPE_PCM_TWOS ||
			   subtype == MEDIASUBTYPE_PCM_IN24 ||
			   subtype == MEDIASUBTYPE_PCM_IN32) {
		hr = ProcessPCMintBE();
	} else if (subtype == MEDIASUBTYPE_PCM_SOWT) {
		hr = ProcessPCMintLE();
	} else if (subtype == MEDIASUBTYPE_PCM_FL32 ||
			   subtype == MEDIASUBTYPE_PCM_FL64) {
		hr = ProcessPCMfloatBE();
	} else if (subtype == MEDIASUBTYPE_IEEE_FLOAT) {
		hr = ProcessPCMfloatLE();
	}

	return hr;
}

HRESULT CMpaDecFilter::ProcessLPCM()
{
	WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();

	WORD nChannels = wfein->nChannels;
	if (nChannels < 1 || nChannels > 8) {
		return ERROR_NOT_SUPPORTED;
	}

	BYTE* const base = m_buff.GetData();
	BYTE* const end = base + m_buff.GetCount();
	BYTE* p = base;

	unsigned int blocksize = nChannels * 2 * wfein->wBitsPerSample / 8;
	size_t nSamples = (m_buff.GetCount() / blocksize) * 2 * nChannels;

	MPCSampleFormat out_sf;
	int outSize = nSamples * (wfein->wBitsPerSample <= 16 ? 2 : 4); // convert to 16 and 32-bit
	CAtlArray<BYTE> outBuff;
	outBuff.SetCount(outSize);

	switch (wfein->wBitsPerSample) {
		case 16: {
			out_sf = SF_PCM16;
			int16_t* pDataOut = (int16_t*)outBuff.GetData();

			for (size_t i = 0; i < nSamples; i++) {
				uint16_t u16 = (uint16_t)(*p) << 8 | (uint16_t)(*(p+1));
				pDataOut[i] = (int16_t)u16;
				p += 2;
			}
		}
		break;
		case 24 : {
			out_sf = SF_PCM32;
			int32_t* pDataOut = (int32_t*)outBuff.GetData();

			size_t m = nChannels * 2;
			for (size_t k = 0, n = nSamples / m; k < n; k++) {
				BYTE* q = p + m * 2;
				for (size_t i = 0; i < m; i++) {
					uint32_t u32 = (uint32_t)(*p) << 24 | (uint32_t)(*(p+1)) << 16 | (uint32_t)(*q) << 8;
					pDataOut[i] = (int32_t)u32;
					p += 2;
					q++;
				}
				p += m;
				pDataOut += m;
			}
		}
		break;
		case 20 : {
			out_sf = SF_PCM32;
			int32_t* pDataOut = (int32_t*)outBuff.GetData();

			size_t m = nChannels * 2;
			for (size_t k = 0, n = nSamples / m; k < n; k++) {
				BYTE* q = p + m * 2;
				for (size_t i = 0; i < m; i++) {
					uint32_t u32 = (uint32_t)(*p) << 24 | (uint32_t)(*(p+1)) << 16;
					if (i & 1) {
						u32 |= (*(uint8_t*)q & 0x0F) << 12;
						q++;
					} else {
						u32 |= (*(uint8_t*)q & 0xF0) << 8;
					}
					pDataOut[i] = (int32_t)u32;
					p += 2;
				}
				p += nChannels;
				pDataOut += m;
			}
		}
		break;
	}

	memmove(base, p, end - p);
	m_buff.SetCount(end - p);

	return Deliver(outBuff.GetData(), outSize, out_sf, wfein->nSamplesPerSec, wfein->nChannels, GetDefChannelMask(wfein->nChannels));
}


HRESULT CMpaDecFilter::ProcessHdmvLPCM(bool bAlignOldBuffer) // Blu ray LPCM
{
	WAVEFORMATEX_HDMV_LPCM* wfein = (WAVEFORMATEX_HDMV_LPCM*)m_pInput->CurrentMediaType().Format();

	scmap_t* remap     = &s_scmap_hdmv [wfein->channel_conf];
	int nChannels      = wfein->nChannels;
	int xChannels      = nChannels + (nChannels % 2);
	int BytesPerSample = (wfein->wBitsPerSample + 7) / 8;
	int BytesPerFrame  = BytesPerSample * xChannels;

	BYTE* pDataIn      = m_buff.GetData();
	int len            = (int)m_buff.GetCount();
	len -= len % BytesPerFrame;
	if (bAlignOldBuffer) {
		m_buff.SetCount(len);
	}
	int nFrames = len/xChannels/BytesPerSample;

	MPCSampleFormat out_sf;
	int outSize = nFrames * nChannels * (wfein->wBitsPerSample <= 16 ? 2 : 4); // convert to 16 and 32-bit
	CAtlArray<BYTE> outBuff;
	outBuff.SetCount(outSize);

	switch (wfein->wBitsPerSample) {
		case 16: {
			out_sf = SF_PCM16;
			int16_t* pDataOut = (int16_t*)outBuff.GetData();

			for (int i=0; i<nFrames; i++) {
				for (int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = (int16_t)(pDataIn[nRemap * 2] << 8 | pDataIn[nRemap * 2 + 1]);
					pDataOut++;
				}
				pDataIn += xChannels*2;
			}
		}
			break;
		case 24 :
		case 20: {
			out_sf = SF_PCM32;
			int32_t* pDataOut = (int32_t*)outBuff.GetData();

			for (int i=0; i<nFrames; i++) {
				for (int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = (int32_t)(pDataIn[nRemap * 3] << 24 | pDataIn[nRemap * 3 + 1] << 16 | pDataIn[nRemap * 3 + 2] << 8);
					pDataOut++;
				}
				pDataIn += xChannels*3;
			}
		}
			break;
	}
	memmove(m_buff.GetData(), pDataIn, m_buff.GetCount() - len );
	m_buff.SetCount(m_buff.GetCount() - len);

	return Deliver(outBuff.GetData(), outSize, out_sf, wfein->nSamplesPerSec, wfein->nChannels, remap->dwChannelMask);
}

HRESULT CMpaDecFilter::ProcessFFmpeg(enum AVCodecID nCodecId)
{
	HRESULT hr;
	BYTE* const base = m_buff.GetData();
	BYTE* end = base + m_buff.GetCount();
	BYTE* p = base;

	if (m_FFAudioDec.GetCodecId() != nCodecId) {
		m_FFAudioDec.Init(nCodecId, m_pInput);
		m_FFAudioDec.SetDRC(GetDynamicRangeControl());
	}

	// RealAudio
	CPaddedArray buffRA(FF_INPUT_BUFFER_PADDING_SIZE);
	bool isRA = false;
	if (nCodecId == AV_CODEC_ID_ATRAC3 || nCodecId == AV_CODEC_ID_COOK || nCodecId == AV_CODEC_ID_SIPR) {
		if (m_FFAudioDec.RealPrepare(p, end - p, buffRA)) {
			p = buffRA.GetData();
			end = p + buffRA.GetCount();
			isRA = true;
		} else {
			return S_OK;
		}
	}

	while (p < end) {
		int size = 0;
		CAtlArray<BYTE> output;
		AVSampleFormat avsamplefmt = AV_SAMPLE_FMT_NONE;

		hr = m_FFAudioDec.Decode(nCodecId, p, int(end - p), size, m_bResync, output, avsamplefmt);
		if (FAILED(hr)) {
			m_buff.RemoveAll();
			m_bResync = true;
			return S_OK;
		} else if (output.GetCount() > 0) { // && SUCCEEDED(hr)
			hr = Deliver(output.GetData(), output.GetCount(), conv_sample_fmt(avsamplefmt), m_FFAudioDec.GetSampleRate(), m_FFAudioDec.GetChannels(), m_FFAudioDec.GetChannelMask());
		} else if (size == 0) { // && pBuffOut.GetCount() == 0
			break;
		}

		p += size;
	}

	if (isRA) { // RealAudio
		p = base + buffRA.GetCount();
		end = base + m_buff.GetCount();
	}

	memmove(base, p, end - p);
	m_buff.SetCount(end - p);

	return S_OK;
}

HRESULT CMpaDecFilter::ProcessAC3_SPDIF()
{
	HRESULT hr;
	BYTE* const base = m_buff.GetData();
	BYTE* const end = base + m_buff.GetCount();
	BYTE* p = base;

	while (p + AC3_HEADER_SIZE <= end) {
		int size = 0;
		int samplerate, channels, framelength, bitrate;

		size = ParseAC3Header(p, &samplerate, &channels, &framelength, &bitrate);

		if (size == 0) {
			p++;
			continue;
		}
		if (p + size > end) {
			break;
		}

		if (FAILED(hr = DeliverBitstream(p, size, samplerate, 1536, 0x0001))) {
			return hr;
		}

		p += size;
	}

	memmove(base, p, end - p);
	m_buff.SetCount(end - p);

	return S_OK;
}

HRESULT CMpaDecFilter::ProcessDTS_SPDIF()
{
	HRESULT hr;
	BYTE* const base = m_buff.GetData();
	BYTE* const end = base + m_buff.GetCount();
	BYTE* p = base;

	while (p + 16 <= end) {
		int size = 0;
		int samplerate, channels, framelength, bitrate;

		size = ParseDTSHeader(p, &samplerate, &channels, &framelength, &bitrate);

		if (size == 0) {
			p++;
			continue;
		}
		if (p + size > end) {
			break;
		}

		if (FAILED(hr = DeliverBitstream(p, size, samplerate, framelength, 0x000b))) {
			return hr;
		}

		p += size;
	}

	memmove(base, p, end - p);
	m_buff.SetCount(end - p);

	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMraw() //'raw '
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	size_t nSamples   = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	MPCSampleFormat out_sf = SF_PCM16;
	int outSize = nSamples * sizeof(int16_t); // convert to 16-bit
	CAtlArray<BYTE> outBuff;
	outBuff.SetCount(outSize);
	int16_t* pDataOut = (int16_t*)outBuff.GetData();

	switch (wfe->wBitsPerSample) {
		case 8: { // unsigned 8-bit
			uint8_t* b = (uint8_t*)m_buff.GetData();
			for (size_t i = 0; i < nSamples; i++) {
				pDataOut[i] = (int16_t)(int8_t)(b[i] + 128) * 256;
			}
		}
		break;
		case 16: { // signed big-endian 16 bit
			uint16_t* d = (uint16_t*)m_buff.GetData(); // signed take as an unsigned to shift operations.
			for (size_t i = 0; i < nSamples; i++) {
				pDataOut[i] = (int16_t)(d[i] << 8 | d[i] >> 8);
			}
		}
		break;
	}

	HRESULT hr;
	if (S_OK != (hr = Deliver(outBuff.GetData(), outSize, out_sf, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMintBE() // 'twos', big-endian 'in24' and 'in32'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	size_t nSamples   = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	MPCSampleFormat out_sf;
	int outSize = nSamples * (wfe->wBitsPerSample <= 16 ? 2 : 4); // convert to 16 and 32-bit
	CAtlArray<BYTE> outBuff;
	outBuff.SetCount(outSize);

	switch (wfe->wBitsPerSample) {
		case 8: { //signed 8-bit
			out_sf = SF_PCM16;
			int16_t* pDataOut = (int16_t*)outBuff.GetData();

			int8_t* b = (int8_t*)m_buff.GetData();
			for (size_t i = 0; i < nSamples; i++) {
				pDataOut[i] = (int16_t)b[i] * 256;
			}
		}
		break;
		case 16: { // signed big-endian 16-bit
			out_sf = SF_PCM16;
			int16_t* pDataOut = (int16_t*)outBuff.GetData();

			uint16_t* d = (uint16_t*)m_buff.GetData(); // signed take as an unsigned to shift operations.
			for (size_t i = 0; i < nSamples; i++) {
				pDataOut[i] = (int16_t)(d[i] << 8 | d[i] >> 8);
			}
		}
		break;
		case 24: { // signed big-endian 24-bit
			out_sf = SF_PCM32;
			int32_t* pDataOut = (int32_t*)outBuff.GetData();

			uint8_t* b = (uint8_t*)m_buff.GetData();
			for (size_t i = 0; i < nSamples; i++) {
				pDataOut[i] = (int32_t)((uint32_t)b[3 * i] << 24 |
										(uint32_t)b[3*i+1] << 16 |
										(uint32_t)b[3 * i + 2] << 8);
			}
		}
		break;
		case 32: { // signed big-endian 32-bit
			out_sf = SF_PCM32;
			int32_t* pDataOut = (int32_t*)outBuff.GetData();

			uint32_t* q = (uint32_t*)m_buff.GetData(); // signed take as an unsigned to shift operations.
			for (size_t i = 0; i < nSamples; i++) {
				pDataOut[i] = (int32_t)(q[i] >> 24 |
									   (q[i] & 0x00ff0000) >> 8 |
									   (q[i] & 0x0000ff00) << 8 |
									    q[i] << 24);
			}
		}
		break;
	}

	HRESULT hr;
	if (S_OK != (hr = Deliver(outBuff.GetData(), outSize, out_sf, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMintLE() // 'sowt', little-endian 'in24' and 'in32'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	size_t nSamples   = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	MPCSampleFormat out_sf;
	int outSize = nSamples * (wfe->wBitsPerSample <= 16 ? 2 : 4); // convert to 16 and 32-bit
	CAtlArray<BYTE> outBuff;
	outBuff.SetCount(outSize);

	switch (wfe->wBitsPerSample) {
		case 8: { //signed 8-bit
			out_sf = SF_PCM16;
			int16_t* pDataOut = (int16_t*)outBuff.GetData();

			int8_t* b = (int8_t*)m_buff.GetData();
			for (size_t i = 0; i < nSamples; i++) {
				pDataOut[i] = (int16_t)b[i] * 256;
			}
		}
		break;
		case 16: { // signed little-endian 16-bit
			memcpy(outBuff.GetData(), m_buff.GetData(), outSize);
			}
		break;
		case 24: { // signed little-endian 32-bit
			out_sf = SF_PCM32;
			int32_t* pDataOut = (int32_t*)outBuff.GetData();

			uint8_t* b = (uint8_t*)m_buff.GetData();
			for (size_t i = 0; i < nSamples; i++) {
				pDataOut[i] = (int32_t)((uint32_t)b[3 * i]   << 8  |
										(uint32_t)b[3*i+1] << 16 |
										(uint32_t)b[3 * i + 2] << 24);
			}
		}
		break;
		case 32: { // signed little-endian 32-bit
			memcpy(outBuff.GetData(), m_buff.GetData(), outSize);
		}
		break;
	}

	HRESULT hr;
	if (S_OK != (hr = Deliver(outBuff.GetData(), outSize, out_sf, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMfloatBE() // big-endian 'fl32' and 'fl64'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	size_t nSamples   = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	MPCSampleFormat out_sf = SF_FLOAT;
	int outSize = nSamples * sizeof(float); // convert to float
	CAtlArray<BYTE> outBuff;
	outBuff.SetCount(outSize);

	switch (wfe->wBitsPerSample) {
		case 32: {
			uint32_t* q  = (uint32_t*)m_buff.GetData();
			uint32_t* vf = (uint32_t*)outBuff.GetData();
			for (size_t i = 0; i < nSamples; i++) {
				vf[i] = q[i] >> 24 |
						(q[i] & 0x00ff0000) >> 8 |
						(q[i] & 0x0000ff00) << 8 |
						q[i] << 24;
			}
		}
		break;
		case 64: {
			float* pDataOut = (float*)outBuff.GetData();

			uint64_t* q = (uint64_t*)m_buff.GetData();
			uint64_t x;
			for (size_t i = 0; i < nSamples; i++) {
				x = q[i] >>56 |
					(q[i] & 0x00FF000000000000) >> 40 |
					(q[i] & 0x0000FF0000000000) >> 24 |
					(q[i] & 0x000000FF00000000) >>  8 |
					(q[i] & 0x00000000FF000000) <<  8 |
					(q[i] & 0x0000000000FF0000) << 24 |
					(q[i] & 0x000000000000FF00) << 40 |
					q[i] << 56;
				pDataOut[i] = (float)(*(double*)&x);
			}
		}
		break;
	}

	HRESULT hr;
	if (S_OK != (hr = Deliver(outBuff.GetData(), outSize, out_sf, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMfloatLE() // little-endian 'fl32' and 'fl64'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	size_t nSamples   = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	MPCSampleFormat out_sf = SF_FLOAT;
	int outSize = nSamples * sizeof(float); // convert to float
	CAtlArray<BYTE> outBuff;
	outBuff.SetCount(outSize);

	switch (wfe->wBitsPerSample) {
		case 32: {
			memcpy(outBuff.GetData(), m_buff.GetData(), outSize);
		}
		break;
		case 64: {
			float* pDataOut = (float*)outBuff.GetData();

			double* q = (double*)m_buff.GetData();
			for (size_t i = 0; i < nSamples; i++) {
				pDataOut[i] = (float)q[i];
			}
		}
		break;
	}

	HRESULT hr;
	if (S_OK != (hr = Deliver(outBuff.GetData(), outSize, out_sf, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPS2PCM()
{
	BYTE* const base = m_buff.GetData();
	BYTE* const end = base + m_buff.GetCount();
	BYTE* p = base;

	WAVEFORMATEXPS2* wfe = (WAVEFORMATEXPS2*)m_pInput->CurrentMediaType().Format();
	int size     = wfe->dwInterleave*wfe->nChannels;
	int samples  = wfe->dwInterleave/(wfe->wBitsPerSample>>3);
	int channels = wfe->nChannels;

	MPCSampleFormat out_sf = SF_PCM16;
	int outSize = samples * channels * sizeof(int16_t); // convert to 16-bit
	CAtlArray<BYTE> outBuff;
	outBuff.SetCount(outSize);
	int16_t* pDataOut = (int16_t*)outBuff.GetData();

	while (end - p >= size) {
		DWORD* dw = (DWORD*)p;

		if (dw[0] == 'dhSS') {
			p += dw[1] + 8;
		} else if (dw[0] == 'dbSS') {
			p += 8;
			m_ps2_state.sync = true;
		} else {
			if (m_ps2_state.sync) {
				short* s = (short*)p;

				for (int i = 0; i < samples; i++)
					for (int j = 0; j < channels; j++) {
						pDataOut[i * channels + j] = s[j * samples + i];
					}
			} else {
				for (int i = 0, n = samples*channels; i < n; i++) {
					pDataOut[i] = 0;
				}
			}

			HRESULT hr;
			if (S_OK != (hr = Deliver(outBuff.GetData(), outSize, out_sf, wfe->nSamplesPerSec, wfe->nChannels))) {
				return hr;
			}

			p += size;
		}
	}

	memmove(base, p, end - p);
	m_buff.SetCount(end - p);

	return S_OK;
}

static void decodeps2adpcm(ps2_state_t& s, int channel, BYTE* pin, double* pout)
{
	int tbl_index = pin[0]>>4;
	int shift     = pin[0]&0xf;
	int unk       = pin[1]; // ?
	UNREFERENCED_PARAMETER(unk);

	if (tbl_index >= 10) {
		ASSERT(0);
		return;
	}
	// if (unk == 7) {ASSERT(0); return;} // ???

	static double s_tbl[] = {
		0.0, 0.0,  0.9375, 0.0,  1.796875, -0.8125,  1.53125, -0.859375,  1.90625, -0.9375,
		0.0, 0.0, -0.9375, 0.0, -1.796875,  0.8125, -1.53125,  0.859375, -1.90625,  0.9375
	};

	double* tbl = &s_tbl[tbl_index*2];
	double& a = s.a[channel];
	double& b = s.b[channel];

	for (int i = 0; i < 28; i++) {
		short input = (short)(((pin[2+i/2] >> ((i&1) << 2)) & 0xf) << 12) >> shift;
		double output = a * tbl[1] + b * tbl[0] + input;

		a = b;
		b = output;

		*pout++ = output / INT16_PEAK;
	}
}

HRESULT CMpaDecFilter::ProcessPS2ADPCM()
{
	BYTE* const base = m_buff.GetData();
	BYTE* const end = base + m_buff.GetCount();
	BYTE* p = base;

	WAVEFORMATEXPS2* wfe = (WAVEFORMATEXPS2*)m_pInput->CurrentMediaType().Format();
	int size = wfe->dwInterleave*wfe->nChannels;
	int samples = wfe->dwInterleave * 14 / 16 * 2;
	int channels = wfe->nChannels;

	MPCSampleFormat out_sf = SF_FLOAT;
	int outSize = samples * channels * sizeof(float); // convert to float
	CAtlArray<BYTE> outBuff;
	outBuff.SetCount(outSize);
	float* pDataOut = (float*)outBuff.GetData();

	while (end - p >= size) {
		DWORD* dw = (DWORD*)p;

		if (dw[0] == 'dhSS') {
			p += dw[1] + 8;
		} else if (dw[0] == 'dbSS') {
			p += 8;
			m_ps2_state.sync = true;
		} else {
			if (m_ps2_state.sync) {
				double* tmp = DNew double[samples*channels];

				for (int channel = 0, j = 0, k = 0; channel < channels; channel++, j += wfe->dwInterleave)
					for (DWORD i = 0; i < wfe->dwInterleave; i += 16, k += 28) {
						decodeps2adpcm(m_ps2_state, channel, p + i + j, tmp + k);
					}

				for (int i = 0, k = 0; i < samples; i++)
					for (int j = 0; j < channels; j++, k++) {
						pDataOut[k] = (float)tmp[j * samples + i];
					}

				delete [] tmp;
			} else {
				for (int i = 0, n = samples*channels; i < n; i++) {
					pDataOut[i] = 0.0;
				}
			}

			HRESULT hr;
			if (S_OK != (hr = Deliver(outBuff.GetData(), outSize, out_sf, wfe->nSamplesPerSec, wfe->nChannels))) {
				return hr;
			}

			p += size;
		}
	}

	memmove(base, p, end - p);
	m_buff.SetCount(end - p);

	return S_OK;
}

HRESULT CMpaDecFilter::GetDeliveryBuffer(IMediaSample** pSample, BYTE** pData)
{
	HRESULT hr;
	*pData = NULL;

	if (FAILED(hr = m_pOutput->GetDeliveryBuffer(pSample, NULL, NULL, 0))
			|| FAILED(hr = (*pSample)->GetPointer(pData))) {
		return hr;
	}

	AM_MEDIA_TYPE* pmt = NULL;
	if (SUCCEEDED((*pSample)->GetMediaType(&pmt)) && pmt) {
		CMediaType mt = *pmt;
		m_pOutput->SetMediaType(&mt);
		DeleteMediaType(pmt);
		pmt = NULL;
	}

	return S_OK;
}

HRESULT CMpaDecFilter::Deliver(BYTE* pBuff, int size, MPCSampleFormat sfmt, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask)
{
	HRESULT hr;

	int nSamples;
	AVSampleFormat in_avsf;
	switch (sfmt) {
		case SF_PCM16:
			nSamples = size / (nChannels * sizeof(int16_t));
			in_avsf = AV_SAMPLE_FMT_S16;
			break;
		case SF_PCM32:
			nSamples = size / (nChannels * sizeof(int32_t));
			in_avsf = AV_SAMPLE_FMT_S32;
			break;
		case SF_FLOAT:
			nSamples = size / (nChannels * sizeof(float));
			in_avsf = AV_SAMPLE_FMT_FLT;
			break;
		//case SF_PCM24: // used for output only and not supported by avresample
		default:
			return E_FAIL;
	}

	MPCSampleFormat out_sf;
	if (GetSampleFormat(sfmt)) {
		out_sf = sfmt;
	} else {
		out_sf = GetSampleFormat2();
	}

	REFERENCE_TIME rtDur = 10000000i64 * nSamples / nSamplesPerSec;
	REFERENCE_TIME rtStart = m_rtStart, rtStop = m_rtStart + rtDur;
	m_rtStart += rtDur;
	//TRACE(_T("CMpaDecFilter: %I64d - %I64d\n"), rtStart/10000, rtStop/10000);
	if (rtStart < 0 /*200000*/ /* < 0, FIXME: 0 makes strange noises */) {
		return S_OK;
	}

	if (dwChannelMask == 0) {
		dwChannelMask = GetDefChannelMask(nChannels);
	}
	//ASSERT(nChannels == av_get_channel_layout_nb_channels(dwChannelMask));

	BYTE*  pDataIn  = pBuff;
	float* pDataMix = NULL;

	if (GetMixer()) {
		int sc = GetMixerLayout();
		WORD  mixed_channels = channel_mode[sc].channels;
		DWORD mixed_mask     = channel_mode[sc].ch_layout;

		pDataMix = new float[nSamples * mixed_channels];

		if (m_InputParams.LayoutUpdate(nChannels, dwChannelMask)) {
			m_Mixer.Reset();
		}

		hr = m_Mixer.Mixing(pDataMix, mixed_channels, mixed_mask, pDataIn, nSamples, nChannels, dwChannelMask, in_avsf);
		if (hr == S_OK) {
			pDataIn       = (BYTE*)pDataMix;
			sfmt          = SF_FLOAT; // float after mixing
			size          = nSamples * mixed_channels * sizeof(float);
			nChannels     = mixed_channels;
			dwChannelMask = mixed_mask;
		}
	}

	CMediaType mt = CreateMediaType(out_sf, nSamplesPerSec, nChannels, dwChannelMask);
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();

	if (FAILED(hr = ReconnectOutput(nSamples, mt))) {
		return hr;
	}

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if (FAILED(GetDeliveryBuffer(&pOut, &pDataOut))) {
		goto fail;
	}

	if (hr == S_OK) {
		m_pOutput->SetMediaType(&mt);
		pOut->SetMediaType(&mt);
	}

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	pOut->SetPreroll(FALSE);
	pOut->SetDiscontinuity(m_fDiscontinuity);
	m_fDiscontinuity = false;
	pOut->SetSyncPoint(TRUE);

	pOut->SetActualDataLength(nSamples * nChannels * wfe->wBitsPerSample / 8);

	WAVEFORMATEX* wfeout = (WAVEFORMATEX*)m_pOutput->CurrentMediaType().Format();
	ASSERT(wfeout->nChannels == wfe->nChannels);
	ASSERT(wfeout->nSamplesPerSec == wfe->nSamplesPerSec);
	UNREFERENCED_PARAMETER(wfeout);

	if (sfmt == out_sf) {
		memcpy(pDataOut, pDataIn, size);
	} else {
		BYTE* p = pDataIn;
		switch (sfmt) {
			case SF_PCM16:
				for (unsigned int i = 0, len = nSamples * nChannels; i < len; i++) {
					switch (out_sf) {
						case SF_PCM24:
							*pDataOut++ = 0;
							*pDataOut++ = *(p);
							*pDataOut++ = *(p + 1);
							break;
						case SF_PCM32:
							*(int32_t*)pDataOut = (int32_t)(*(int16_t*)p) << 16;
							pDataOut += sizeof(int32_t);
							break;
						case SF_FLOAT:
							*(float*)pDataOut = (float)(*(int16_t*)p) / INT16_PEAK;;
							pDataOut += sizeof(float);
							break;
					}
					p += sizeof(int16_t);
				}
				break;
			case SF_PCM32:
				for (unsigned int i = 0, len = nSamples * nChannels; i < len; i++) {
					switch (out_sf) {
						case SF_PCM16:
							*(int16_t*)pDataOut = *(int32_t*)(p + 2);
							pDataOut += sizeof(int16_t);
							break;
						case SF_PCM24:
							*pDataOut++ = *(p + 1);
							*pDataOut++ = *(p + 2);
							*pDataOut++ = *(p + 3);
							break;
						case SF_FLOAT:
							*(float*)pDataOut = (float)(double(*(int32_t*)p) / INT32_PEAK);
							pDataOut += sizeof(float);
							break;
					}
					p += sizeof(int32_t);
				}
				break;
			case SF_FLOAT:
				for (unsigned int i = 0, len = nSamples * nChannels; i < len; i++) {
					float f = *(float*)p;
					if (f < -1) { f = -1; }
					switch (out_sf) {
						case SF_PCM16: {
							if (f > F16MAX) { f = F16MAX; }
							*(int16_t*)pDataOut = (int16_t)round_f(f * INT16_PEAK);
							pDataOut += sizeof(int16_t);
						}
						break;
						case SF_PCM24: {
							if (f > F24MAX) { f = F24MAX; }
							DWORD i24 = (DWORD)(int32_t)round_f(f * INT24_PEAK);
							*pDataOut++ = (BYTE)(i24);
							*pDataOut++ = (BYTE)(i24 >> 8);
							*pDataOut++ = (BYTE)(i24 >> 16);
						}
						break;
						case SF_PCM32: {
							double d = (double)f;
							if (d > D32MAX) { d = D32MAX; }
							*(int32_t*)pDataOut = (int32_t)round_d(d * INT32_PEAK);
							pDataOut += sizeof(int32_t);
						}
						break;
					}
					p += sizeof(float);
				}
				break;
			default:
				goto fail;
		}
	}

	if (pDataMix) {
		pDataIn = NULL;
		delete [] pDataMix;
	}

	return m_pOutput->Deliver(pOut);

fail:
	if (pDataMix) {
		pDataIn = NULL;
		delete [] pDataMix;
	}

	return E_FAIL;
}

HRESULT CMpaDecFilter::DeliverBitstream(BYTE* pBuff, int size, int sample_rate, int samples, BYTE type)
{
	HRESULT hr;
	bool isDTSWAV = false;

	int length = 0;

	if (type == 0x0b) { // DTS
		if (size == 4096 && sample_rate == 44100 && samples == 1024) { // DTSWAV
			length = size;
			isDTSWAV = true;
		} else while (length < size + 16) {
				length += 2048;
			}
	} else { //if (type == 0x01) { // AC3
		length = samples*4;
	}

	CMediaType mt;
	if (isDTSWAV) {
		mt = CreateMediaTypeSPDIF(sample_rate);
	} else {
		mt = CreateMediaTypeSPDIF();
	}

	if (FAILED(hr = ReconnectOutput(length, mt))) {
		return hr;
	}

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if (FAILED(GetDeliveryBuffer(&pOut, &pDataOut))) {
		return E_FAIL;
	}

	if (isDTSWAV) {
		memcpy(pDataOut, pBuff, size);
	} else {
		WORD* pDataOutW = (WORD*)pDataOut;
		pDataOutW[0] = 0xf872;
		pDataOutW[1] = 0x4e1f;
		pDataOutW[2] = type;
		pDataOutW[3] = size*8;
		_swab((char*)pBuff, (char*)&pDataOutW[4], size+1); // if the size is odd, the function "_swab" lose the last byte. need add one.
	}

	REFERENCE_TIME rtDur;
	rtDur = 10000000i64 * samples / sample_rate;
	REFERENCE_TIME rtStart = m_rtStart, rtStop = m_rtStart + rtDur;
	m_rtStart += rtDur;

	if (rtStart < 0) {
		return S_OK;
	}

	if (hr == S_OK) {
		m_pOutput->SetMediaType(&mt);
		pOut->SetMediaType(&mt);
	}

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	pOut->SetPreroll(FALSE);
	pOut->SetDiscontinuity(m_fDiscontinuity);
	m_fDiscontinuity = false;
	pOut->SetSyncPoint(TRUE);

	pOut->SetActualDataLength(length);

	return m_pOutput->Deliver(pOut);
}

HRESULT CMpaDecFilter::ReconnectOutput(int nSamples, CMediaType& mt)
{
	HRESULT hr;

	CComQIPtr<IMemInputPin> pPin = m_pOutput->GetConnected();
	if (!pPin) {
		return E_NOINTERFACE;
	}

	CComPtr<IMemAllocator> pAllocator;
	if (FAILED(hr = pPin->GetAllocator(&pAllocator)) || !pAllocator) {
		return hr;
	}

	ALLOCATOR_PROPERTIES props, actual;
	if (FAILED(hr = pAllocator->GetProperties(&props))) {
		return hr;
	}

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();
	long cbBuffer = nSamples * wfe->nBlockAlign;

	if (mt != m_pOutput->CurrentMediaType() || cbBuffer > props.cbBuffer) {
		if (cbBuffer > props.cbBuffer) {
			props.cBuffers = 4;
			props.cbBuffer = cbBuffer*3/2;

			if (FAILED(hr = m_pOutput->DeliverBeginFlush())
					|| FAILED(hr = m_pOutput->DeliverEndFlush())
					|| FAILED(hr = pAllocator->Decommit())
					|| FAILED(hr = pAllocator->SetProperties(&props, &actual))
					|| FAILED(hr = pAllocator->Commit())) {
				return hr;
			}

			if (props.cBuffers > actual.cBuffers || props.cbBuffer > actual.cbBuffer) {
				NotifyEvent(EC_ERRORABORT, hr, 0);
				return E_FAIL;
			}
		}

		return S_OK;
	}

	return S_FALSE;
}

CMediaType CMpaDecFilter::CreateMediaType(MPCSampleFormat sf, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask)
{
	CMediaType mt;

	mt.majortype  = MEDIATYPE_Audio;
	mt.subtype    = sf == SF_FLOAT ? MEDIASUBTYPE_IEEE_FLOAT : MEDIASUBTYPE_PCM;
	mt.formattype = FORMAT_WaveFormatEx;

	WAVEFORMATEXTENSIBLE wfex;
	//memset(&wfex, 0, sizeof(wfex));

	WAVEFORMATEX& wfe  = wfex.Format;
	wfe.nChannels      = nChannels;
	wfe.nSamplesPerSec = nSamplesPerSec;
	switch (sf) {
		default:
		case SF_PCM16:
			wfe.wBitsPerSample = 16;
			break;
		case SF_PCM24:
			wfe.wBitsPerSample = 24;
			break;
		case SF_PCM32:
		case SF_FLOAT:
			wfe.wBitsPerSample = 32;
			break;
	}
	wfe.nBlockAlign     = nChannels * wfe.wBitsPerSample / 8;
	wfe.nAvgBytesPerSec = nSamplesPerSec * wfe.nBlockAlign;

	if (nChannels <= 2 && dwChannelMask <= 0x4 && (sf == SF_PCM16 || sf == SF_FLOAT)) {
		// WAVEFORMATEX
		wfe.wFormatTag = (sf == SF_FLOAT) ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
		wfe.cbSize = 0;

		mt.SetFormat((BYTE*)&wfe, sizeof(wfe));
	} else {
		// WAVEFORMATEXTENSIBLE
		wfex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wfex.Format.cbSize = sizeof(wfex) - sizeof(wfex.Format); // 22
		wfex.Samples.wValidBitsPerSample = wfex.Format.wBitsPerSample;
		if (dwChannelMask == 0) {
			dwChannelMask = GetDefChannelMask(nChannels);
		}
		wfex.dwChannelMask = dwChannelMask;
		wfex.SubFormat = mt.subtype;

		mt.SetFormat((BYTE*)&wfex, sizeof(wfex));
	}
	mt.SetSampleSize(wfex.Format.nBlockAlign);

	return mt;
}

CMediaType CMpaDecFilter::CreateMediaTypeSPDIF(DWORD nSamplesPerSec)
{
	CMediaType mt = CreateMediaType(SF_PCM16, nSamplesPerSec, 2);
	((WAVEFORMATEX*)mt.pbFormat)->wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;
	return mt;
}

HRESULT CMpaDecFilter::CheckInputType(const CMediaType* mtIn)
{
	if (mtIn->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) {
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)mtIn->Format();
		if (wfe->nChannels < 1 || wfe->nChannels > 8 || (wfe->wBitsPerSample != 16 && wfe->wBitsPerSample != 20 && wfe->wBitsPerSample != 24)) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	} else if (mtIn->subtype == MEDIASUBTYPE_PS2_ADPCM) {
		WAVEFORMATEXPS2* wfe = (WAVEFORMATEXPS2*)mtIn->Format();
		if (wfe->dwInterleave & 0xf) { // has to be a multiple of the block size (16 bytes)
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	} else if (mtIn->subtype == MEDIASUBTYPE_IEEE_FLOAT) {
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)mtIn->Format();
		if (wfe->wBitsPerSample != 64) {    // only for 64-bit float PCM
			return VFW_E_TYPE_NOT_ACCEPTED; // not needed any decoders for 32-bit float
		}
	}

	for (int i = 0; i < _countof(sudPinTypesIn); i++) {
		if (*sudPinTypesIn[i].clsMajorType == mtIn->majortype
				&& *sudPinTypesIn[i].clsMinorType == mtIn->subtype) {
			return S_OK;
		}
	}

	return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CMpaDecFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	return SUCCEEDED(CheckInputType(mtIn))
		   && mtOut->majortype == MEDIATYPE_Audio && mtOut->subtype == MEDIASUBTYPE_PCM
		   || mtOut->majortype == MEDIATYPE_Audio && mtOut->subtype == MEDIASUBTYPE_IEEE_FLOAT
		   ? S_OK
		   : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CMpaDecFilter::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	if (m_pInput->IsConnected() == FALSE) {
		return E_UNEXPECTED;
	}

	CMediaType& mt		= m_pInput->CurrentMediaType();
	WAVEFORMATEX* wfe	= (WAVEFORMATEX*)mt.Format();
	UNREFERENCED_PARAMETER(wfe);

	pProperties->cBuffers = 4;
	// pProperties->cbBuffer = 1;
	pProperties->cbBuffer = 48000*6*(32/8)/10; // 48KHz 6ch 32bps 100ms
	pProperties->cbAlign = 1;
	pProperties->cbPrefix = 0;

	HRESULT hr;
	ALLOCATOR_PROPERTIES Actual;
	if (FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) {
		return hr;
	}

	return pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
		   ? E_FAIL
		   : NOERROR;
}

HRESULT CMpaDecFilter::GetMediaType(int iPosition, CMediaType* pmt)
{
	if (m_pInput->IsConnected() == FALSE) {
		return E_UNEXPECTED;
	}

	if (iPosition < 0) {
		return E_INVALIDARG;
	}
	if (iPosition > 0) {
		return VFW_S_NO_MORE_ITEMS;
	}

	CMediaType mt = m_pInput->CurrentMediaType();
	const GUID& subtype = mt.subtype;
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();
	if (wfe == NULL) {
		return E_INVALIDARG;
	}

	if (GetSPDIF(ac3) && (subtype == MEDIASUBTYPE_DOLBY_AC3 || subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3) ||
			GetSPDIF(dts) && (subtype == MEDIASUBTYPE_DTS || subtype == MEDIASUBTYPE_WAVE_DTS)) {
		if (wfe->nSamplesPerSec == 44100) { // DTS-WAVE
			*pmt = CreateMediaTypeSPDIF(44100);
		} else {
			*pmt = CreateMediaTypeSPDIF();
		}
		return S_OK;
	}

	if (GetMixer()) {
		int sc = GetMixerLayout();
		*pmt = CreateMediaType(GetSampleFormat2(), wfe->nSamplesPerSec, channel_mode[sc].channels, channel_mode[sc].ch_layout);
	}
	else if (m_FFAudioDec.GetCodecId() != AV_CODEC_ID_NONE) {
		MPCSampleFormat sf = conv_sample_fmt(m_FFAudioDec.GetSampleFmt());
		if (!GetSampleFormat(sf)) {
			sf = GetSampleFormat2();
		}
		*pmt = CreateMediaType(sf, m_FFAudioDec.GetSampleRate(), m_FFAudioDec.GetChannels(), m_FFAudioDec.GetChannelMask());
	}
	else {
		*pmt = CreateMediaType(GetSampleFormat2(), wfe->nSamplesPerSec, wfe->nChannels);
	}

	return S_OK;
}

HRESULT CMpaDecFilter::StartStreaming()
{
	HRESULT hr = __super::StartStreaming();
	if (FAILED(hr)) {
		return hr;
	}

	m_ps2_state.reset();

	m_fDiscontinuity = false;

	return S_OK;
}

HRESULT CMpaDecFilter::StopStreaming()
{
	m_FFAudioDec.StreamFinish();

	return __super::StopStreaming();
}

HRESULT	CMpaDecFilter::SetMediaType(PIN_DIRECTION dir, const CMediaType *pmt)
{
	if (dir == PINDIR_INPUT) {
		enum AVCodecID nCodecId = FindCodec(pmt->subtype);
		if (nCodecId != AV_CODEC_ID_NONE && !m_FFAudioDec.Init(nCodecId, m_pInput)) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}

	return __super::SetMediaType(dir, pmt);
}

// IMpaDecFilter

STDMETHODIMP CMpaDecFilter::SetSampleFormat(MPCSampleFormat sf, bool enable)
{
	CAutoLock cAutoLock(&m_csProps);
	if (sf >= 0 && sf < sfcount) {
		m_fSampleFmt[sf] = enable;
	} else {
		return E_INVALIDARG;
	}

	return S_OK;
}

STDMETHODIMP_(bool) CMpaDecFilter::GetSampleFormat(MPCSampleFormat sf)
{
	CAutoLock cAutoLock(&m_csProps);
	if (sf >= 0 && sf < sfcount) {
		return m_fSampleFmt[sf];
	}
	return false;
}

STDMETHODIMP_(MPCSampleFormat) CMpaDecFilter::GetSampleFormat2()
{
	CAutoLock cAutoLock(&m_csProps);
	if (m_fSampleFmt[SF_FLOAT]) {
		return SF_FLOAT;
	}
	if (m_fSampleFmt[SF_PCM24]) {
		return SF_PCM24;
	}
	if (m_fSampleFmt[SF_PCM16]) {
		return SF_PCM16;
	}
	if (m_fSampleFmt[SF_PCM32]) {
		return SF_PCM32;
	}
	return SF_PCM16;
}

STDMETHODIMP CMpaDecFilter::SetMixer(bool fMixer)
{
	CAutoLock cAutoLock(&m_csProps);
	m_fMixer = fMixer;

	return S_OK;
}

STDMETHODIMP_(bool) CMpaDecFilter::GetMixer()
{
	CAutoLock cAutoLock(&m_csProps);

	return m_fMixer;
}

STDMETHODIMP CMpaDecFilter::SetMixerLayout(int sc)
{
	CAutoLock cAutoLock(&m_csProps);
	if (sc < SPK_MONO || sc > SPK_7_1) {
		return E_INVALIDARG;
	}

	if (m_iMixerLayout != sc) {
		m_Mixer.Reset();
		m_iMixerLayout = sc;
	}

	return S_OK;
}

STDMETHODIMP_(int) CMpaDecFilter::GetMixerLayout()
{
	CAutoLock cAutoLock(&m_csProps);

	return m_iMixerLayout;
}

STDMETHODIMP CMpaDecFilter::SetDynamicRangeControl(bool fDRC)
{
	CAutoLock cAutoLock(&m_csProps);
	m_fDRC = fDRC;

	m_FFAudioDec.SetDRC(fDRC);

	return S_OK;
}

STDMETHODIMP_(bool) CMpaDecFilter::GetDynamicRangeControl()
{
	CAutoLock cAutoLock(&m_csProps);

	return m_fDRC;
}

STDMETHODIMP CMpaDecFilter::SetSPDIF(enctype et, bool fSPDIF)
{
	CAutoLock cAutoLock(&m_csProps);
	if (et >= 0 && et < etcount) {
		m_fSPDIF[et] = fSPDIF;
	} else {
		return E_INVALIDARG;
	}

	return S_OK;
}

STDMETHODIMP_(bool) CMpaDecFilter::GetSPDIF(enctype et)
{
	CAutoLock cAutoLock(&m_csProps);
	if (et >= 0 && et < etcount) {
		return m_fSPDIF[et];
	}
	return false;
}

STDMETHODIMP CMpaDecFilter::SaveSettings()
{
	CAutoLock cAutoLock(&m_csProps);

#ifdef REGISTER_FILTER
	CRegKey key;
	if (ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, OPT_REGKEY_MpaDec)) {
		key.SetDWORDValue(OPTION_SFormat_i16, m_fSampleFmt[SF_PCM16]);
		key.SetDWORDValue(OPTION_SFormat_i24, m_fSampleFmt[SF_PCM24]);
		key.SetDWORDValue(OPTION_SFormat_i32, m_fSampleFmt[SF_PCM32]);
		key.SetDWORDValue(OPTION_SFormat_flt, m_fSampleFmt[SF_FLOAT]);
		key.SetDWORDValue(OPTION_Mixer, m_fMixer);
		key.SetStringValue(OPTION_MixerLayout, channel_mode[m_iMixerLayout].op_value);
		key.SetDWORDValue(OPTION_DRC, m_fDRC);
		key.SetDWORDValue(OPTION_SPDIF_ac3, m_fSPDIF[ac3]);
		key.SetDWORDValue(OPTION_SPDIF_dts, m_fSPDIF[dts]);
	}
#else
	AfxGetApp()->WriteProfileInt(OPT_SECTION_MpaDec, OPTION_SFormat_i16, m_fSampleFmt[SF_PCM16]);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_MpaDec, OPTION_SFormat_i24, m_fSampleFmt[SF_PCM24]);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_MpaDec, OPTION_SFormat_i32, m_fSampleFmt[SF_PCM32]);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_MpaDec, OPTION_SFormat_flt, m_fSampleFmt[SF_FLOAT]);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_MpaDec, OPTION_Mixer, m_fMixer);
	AfxGetApp()->WriteProfileString(OPT_SECTION_MpaDec, OPTION_MixerLayout, channel_mode[m_iMixerLayout].op_value);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_MpaDec, OPTION_DRC, m_fDRC);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_MpaDec, OPTION_SPDIF_ac3, m_fSPDIF[ac3]);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_MpaDec, OPTION_SPDIF_dts, m_fSPDIF[dts]);
#endif

	return S_OK;
}

// ISpecifyPropertyPages2

STDMETHODIMP CMpaDecFilter::GetPages(CAUUID* pPages)
{
	CheckPointer(pPages, E_POINTER);

	HRESULT hr = S_OK;

	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
	if (pPages->pElems != NULL) {
	pPages->pElems[0] = __uuidof(CMpaDecSettingsWnd);
	} else {
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

STDMETHODIMP CMpaDecFilter::CreatePage(const GUID& guid, IPropertyPage** ppPage)
{
	CheckPointer(ppPage, E_POINTER);

	if (*ppPage != NULL) {
		return E_INVALIDARG;
	}

	HRESULT hr;

	if (guid == __uuidof(CMpaDecSettingsWnd)) {
		(*ppPage = DNew CInternalPropertyPageTempl<CMpaDecSettingsWnd>(NULL, &hr))->AddRef();
	}

	return *ppPage ? S_OK : E_FAIL;
}

//
// CMpaDecInputPin
//

CMpaDecInputPin::CMpaDecInputPin(CTransformFilter* pFilter, HRESULT* phr, LPWSTR pName)
	: CDeCSSInputPin(NAME("CMpaDecInputPin"), pFilter, phr, pName)
{
}
