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

#include "StreamSwitcher.h"

#define AudioSwitcherName L"MPC AudioSwitcher"

#define AS_MAX_CHANNELS 18

interface __declspec(uuid("CEDB2890-53AE-4231-91A3-B0AAFCD1DBDE"))
IAudioSwitcherFilter :
public IUnknown {
	STDMETHOD(GetInputSpeakerConfig) (DWORD* pdwChannelMask) = 0;
	STDMETHOD(GetSpeakerConfig) (bool* pfCustomChannelMapping, DWORD pSpeakerToChannelMap[AS_MAX_CHANNELS][AS_MAX_CHANNELS]) = 0;
	STDMETHOD(SetSpeakerConfig) (bool fCustomChannelMapping, DWORD pSpeakerToChannelMap[AS_MAX_CHANNELS][AS_MAX_CHANNELS]) = 0;
	STDMETHOD_(int, GetNumberOfInputChannels) () = 0;
	STDMETHOD_(REFERENCE_TIME, GetAudioTimeShift) () = 0;
	STDMETHOD(SetAudioTimeShift) (REFERENCE_TIME rtAudioTimeShift) = 0;
	STDMETHOD(GetNormalizeBoost) (bool& fNormalize, bool& fNormalizeRecover, float& boost) = 0;
	STDMETHOD(SetNormalizeBoost) (bool fNormalize, bool fNormalizeRecover, float boost) = 0;
};

class __declspec(uuid("18C16B08-6497-420e-AD14-22D21C2CEAB7"))
	CAudioSwitcherFilter : public CStreamSwitcherFilter, public IAudioSwitcherFilter
{
	typedef struct {
		DWORD Speaker, Channel;
	} ChMap;
	CAtlArray<ChMap> m_chs[AS_MAX_CHANNELS];

	bool m_fCustomChannelMapping;
	DWORD m_pSpeakerToChannelMap[AS_MAX_CHANNELS][AS_MAX_CHANNELS];
	REFERENCE_TIME m_rtAudioTimeShift;
	bool m_fNormalize, m_fNormalizeRecover;
	double m_normalizeFactor;
	float m_boost_mul;

	REFERENCE_TIME m_rtNextStart, m_rtNextStop;

public:
	CAudioSwitcherFilter(LPUNKNOWN lpunk, HRESULT* phr);

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	HRESULT CheckMediaType(const CMediaType* pmt);
	HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
	CMediaType CreateNewOutputMediaType(CMediaType mt, long& cbBuffer);
	void OnNewOutputMediaType(const CMediaType& mtIn, const CMediaType& mtOut);

	HRESULT DeliverEndFlush();
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	// IAudioSwitcherFilter
	STDMETHODIMP GetInputSpeakerConfig(DWORD* pdwChannelMask);
	STDMETHODIMP GetSpeakerConfig(bool* pfCustomChannelMapping, DWORD pSpeakerToChannelMap[AS_MAX_CHANNELS][AS_MAX_CHANNELS]);
	STDMETHODIMP SetSpeakerConfig(bool fCustomChannelMapping, DWORD pSpeakerToChannelMap[AS_MAX_CHANNELS][AS_MAX_CHANNELS]);
	STDMETHODIMP_(int) GetNumberOfInputChannels();
	STDMETHODIMP_(REFERENCE_TIME) GetAudioTimeShift();
	STDMETHODIMP SetAudioTimeShift(REFERENCE_TIME rtAudioTimeShift);
	STDMETHODIMP GetNormalizeBoost(bool& fNormalize, bool& fNormalizeRecover, float& boost);
	STDMETHODIMP SetNormalizeBoost(bool fNormalize, bool fNormalizeRecover, float boost);

	// IAMStreamSelect
	STDMETHODIMP Enable(long lIndex, DWORD dwFlags);
};
