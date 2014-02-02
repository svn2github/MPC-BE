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

#include <atlbase.h>
#include <IKeyFrameInfo.h>
#include <IBufferInfo.h>
#include <IBitRateInfo.h>
#include "Packet.h"
#include "../../../DSUtil/DSMPropertyBag.h"

class CBaseSplitterFilter;

class CBaseSplitterOutputPin
	: public CBaseOutputPin
	, public IDSMPropertyBagImpl
	, protected CAMThread
	, public IMediaSeeking
	, public IBitRateInfo
{
protected:
	CAtlArray<CMediaType> m_mts;
	int m_nBuffers;

private:
	CPacketQueue m_queue;

	HRESULT m_hrDeliver;

	bool m_fFlushing, m_fFlushed;
	CAMEvent m_eEndFlush;

	enum {
		CMD_EXIT
	};
	DWORD ThreadProc();

	void MakeISCRHappy();

	// please only use DeliverPacket from the derived class
	HRESULT GetDeliveryBuffer(IMediaSample** ppSample, REFERENCE_TIME* pStartTime, REFERENCE_TIME* pEndTime, DWORD dwFlags);
	HRESULT Deliver(IMediaSample* pSample);

	// bitrate stats

	struct {
		UINT64 nTotalBytesDelivered;
		REFERENCE_TIME rtTotalTimeDelivered;
		UINT64 nBytesSinceLastDeliverTime;
		REFERENCE_TIME rtLastDeliverTime;
		DWORD nCurrentBitRate;
		DWORD nAverageBitRate;
	} m_brs;

	DWORD m_MinQueuePackets, m_MaxQueuePackets;
	DWORD m_MinQueueSize, m_MaxQueueSize;

protected:
	REFERENCE_TIME m_rtPrev, m_rtOffset;
	REFERENCE_TIME m_rtStart;

	// override this if you need some second level stream specific demuxing (optional)
	// the default implementation will send the sample as is
	virtual HRESULT DeliverPacket(CAutoPtr<Packet> p);

	// IMediaSeeking

	STDMETHODIMP GetCapabilities(DWORD* pCapabilities);
	STDMETHODIMP CheckCapabilities(DWORD* pCapabilities);
	STDMETHODIMP IsFormatSupported(const GUID* pFormat);
	STDMETHODIMP QueryPreferredFormat(GUID* pFormat);
	STDMETHODIMP GetTimeFormat(GUID* pFormat);
	STDMETHODIMP IsUsingTimeFormat(const GUID* pFormat);
	STDMETHODIMP SetTimeFormat(const GUID* pFormat);
	STDMETHODIMP GetDuration(LONGLONG* pDuration);
	STDMETHODIMP GetStopPosition(LONGLONG* pStop);
	STDMETHODIMP GetCurrentPosition(LONGLONG* pCurrent);
	STDMETHODIMP ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat);
	STDMETHODIMP SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags);
	STDMETHODIMP GetPositions(LONGLONG* pCurrent, LONGLONG* pStop);
	STDMETHODIMP GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest);
	STDMETHODIMP SetRate(double dRate);
	STDMETHODIMP GetRate(double* pdRate);
	STDMETHODIMP GetPreroll(LONGLONG* pllPreroll);

public:
	CBaseSplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr, int nBuffers = 0, int factor = 1);
	CBaseSplitterOutputPin(LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr, int nBuffers = 0, int factor = 1);
	virtual ~CBaseSplitterOutputPin();

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	HRESULT SetName(LPCWSTR pName);

	HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT CheckMediaType(const CMediaType* pmt);
	HRESULT GetMediaType(int iPosition, CMediaType* pmt);
	CMediaType& CurrentMediaType() {return m_mt;}

	STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);

	// Queueing

	HANDLE GetThreadHandle() {
		ASSERT(m_hThread != NULL);
		return m_hThread;
	}
	void SetThreadPriority(int nPriority) {
		if (m_hThread) {
			::SetThreadPriority(m_hThread, nPriority);
		}
	}

	HRESULT Active();
	HRESULT Inactive();

	HRESULT DeliverBeginFlush();
	HRESULT DeliverEndFlush();
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	size_t QueueCount();
	size_t QueueSize();
	HRESULT QueueEndOfStream();
	HRESULT QueuePacket(CAutoPtr<Packet> p);

	// returns true for everything which (the lack of) would not block other streams (subtitle streams, basically)
	virtual bool IsDiscontinuous();

	// returns IStreamsSwitcherInputPin::IsActive(), when it can find one downstream
	bool IsActive();

	// IBitRateInfo

	STDMETHODIMP_(DWORD) GetCurrentBitRate() { return m_brs.nCurrentBitRate; }
	STDMETHODIMP_(DWORD) GetAverageBitRate() { return m_brs.nAverageBitRate; }

	REFERENCE_TIME GetOffset() { return m_rtOffset; }
};
