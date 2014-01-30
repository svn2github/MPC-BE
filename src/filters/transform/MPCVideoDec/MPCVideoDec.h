/*
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

#include <d3dx9.h>
#include <videoacc.h>	// DXVA1
#include <dxva.h>
#include <dxva2api.h>	// DXVA2
#include "../BaseVideoFilter/BaseVideoFilter.h"
#include "IMPCVideoDec.h"
#include "MPCVideoDecSettingsWnd.h"
#include "DXVADecoder.h"
#include "FormatConverter.h"
#include "../../../apps/mplayerc/FilterEnum.h"

#define MPCVideoDecName L"MPC Video Decoder"

#define CHECK_HR(x)			hr = ##x; if (FAILED(hr)) { DbgLog((LOG_TRACE, 3, L"Error : 0x%08x, %s : %i", hr, CString(__FILE__), __LINE__)); return hr; }
#define CHECK_HR_FALSE(x)	hr = ##x; if (FAILED(hr)) { DbgLog((LOG_TRACE, 3, L"Error : 0x%08x, %s : %i", hr, CString(__FILE__), __LINE__)); return S_FALSE; }

struct AVCodec;
struct AVCodecContext;
struct AVCodecParserContext;
struct AVFrame;

class CCpuId;

struct RMDemuxContext {
	bool	video_after_seek;
	__int32	kf_pts;		///< timestamp of next video keyframe
	__int64	kf_base;	///< timestamp of the prev. video keyframe
};

class __declspec(uuid("008BAC12-FBAF-497b-9670-BC6F6FBAE2C4"))
	CMPCVideoDecFilter
	: public CBaseVideoFilter
	, public IMPCVideoDecFilter
	, public ISpecifyPropertyPages2
{
protected:
	// === Persistants parameters (registry)
	int										m_nThreadNumber;
	int										m_nDiscardMode;
	MPC_DEINTERLACING_FLAGS					m_nDeinterlacing;
	int										m_nARMode;
	int										m_nDXVACheckCompatibility;
	int										m_nDXVA_SD;
	bool									m_fPixFmts[PixFmt_count];
	int										m_nSwPreset;
	int										m_nSwStandard;
	int										m_nSwRGBLevels;
	//

	CCpuId*									m_pCpuId;
	CCritSec								m_csProps;

	bool									m_FFmpegFilters[FFM_LAST + !FFM_LAST];
	bool									m_DXVAFilters[TRA_DXVA_LAST + !TRA_DXVA_LAST];

	bool									m_bDXVACompatible;
	unsigned __int64						m_nActiveCodecs;
	AVPixelFormat							m_PixelFormat;

	FF_FIELD_TYPE							m_nFrameType;

	// === FFMpeg variables
	AVCodec*								m_pAVCodec;
	AVCodecContext*							m_pAVCtx;
	AVCodecParserContext*					m_pParser;
	AVFrame*								m_pFrame;
	int										m_nCodecNb;
	enum AVCodecID							m_nCodecId;
	int										m_nWorkaroundBug;
	int										m_nErrorConcealment;
	REFERENCE_TIME							m_rtAvrTimePerFrame;
	bool									m_bReorderBFrame;

	struct B_FRAME {
		REFERENCE_TIME	rtStart;
		REFERENCE_TIME	rtStop;
	};
	B_FRAME									m_BFrames[2];
	int										m_nPosB;

	bool									m_bWaitKeyFrame;

	int										m_nOutputWidth;
	int										m_nOutputHeight;
	int										m_nARX, m_nARY;

	BOOL									m_bIsEVO;

	// Buffer management for truncated stream (store stream chunks & reference time sent by splitter)
	BYTE*									m_pFFBuffer;
	int										m_nFFBufferSize;
	BYTE*									m_pFFBuffer2;
	int										m_nFFBufferSize2;

	REFERENCE_TIME							m_rtLastStop;			// rtStop for last delivered frame
	double									m_dRate;
	REFERENCE_TIME							m_rtPrevStop;

	bool									m_bUseDXVA;
	bool									m_bUseFFmpeg;
	CFormatConverter						m_FormatConverter;
	CSize									m_pOutSize;				// Picture size on output pin

	// === DXVA common variables
	VIDEO_OUTPUT_FORMATS*					m_pVideoOutputFormat;
	int										m_nVideoOutputCount;
	CDXVADecoder*							m_pDXVADecoder;
	GUID									m_DXVADecoderGUID;

	DWORD									m_nPCIVendor;
	DWORD									m_nPCIDevice;
	LARGE_INTEGER							m_VideoDriverVersion;
	CString									m_strDeviceDescription;

	// === DXVA1 variables
	DDPIXELFORMAT							m_DDPixelFormat;

	// === DXVA2 variables
	CComPtr<IDirect3DDeviceManager9>		m_pDeviceManager;
	CComPtr<IDirectXVideoDecoderService>	m_pDecoderService;
	CComPtr<IDirect3DSurface9>				m_pDecoderRenderTarget;
	DXVA2_ConfigPictureDecode				m_DXVA2Config;
	HANDLE									m_hDevice;
	DXVA2_VideoDesc							m_VideoDesc;

	BOOL									m_bWaitingForKeyFrame;

	RMDemuxContext							rm;
	REFERENCE_TIME							m_rtStart;

	REFERENCE_TIME							m_rtStartCache;

	// === Private functions
	void				Cleanup();
	void				ffmpegCleanup();
	int					FindCodec(const CMediaType* mtIn, bool bForced = false);
	void				AllocExtradata(AVCodecContext* pAVCtx, const CMediaType* mt);
	void				GetOutputFormats (int& nNumber, VIDEO_OUTPUT_FORMATS** ppFormats);
	void				DetectVideoCard(HWND hWnd);
	void				BuildOutputFormat();

	HRESULT				SoftwareDecode(IMediaSample* pIn, BYTE* pDataIn, int nSize, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);
	HRESULT				ChangeOutputMediaFormat(int nType);

	HRESULT				ReopenVideo();
	HRESULT				FindDecoderConfiguration();

	HRESULT				InitDecoder(const CMediaType *pmt);

public:
	CMPCVideoDecFilter(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CMPCVideoDecFilter();

	DECLARE_IUNKNOWN
	STDMETHODIMP			NonDelegatingQueryInterface(REFIID riid, void** ppv);
	virtual bool			IsVideoInterlaced();
	virtual void			GetOutputSize(int& w, int& h, int& arx, int& ary, int& RealWidth, int& RealHeight);
	CTransformOutputPin*	GetOutputPin() {
		return m_pOutput;
	}

	REFERENCE_TIME	GetDuration();
	void			UpdateFrameTime(REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop, bool pulldown_flag = false);
	bool			IsAVI();

	// === Overriden DirectShow functions
	HRESULT			SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);
	HRESULT			CheckInputType(const CMediaType* mtIn);
	HRESULT			CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT			Transform(IMediaSample* pIn);
	HRESULT			CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin);
	HRESULT			DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT			BeginFlush();
	HRESULT			EndFlush();
	HRESULT			NewSegment(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, double dRate);
	HRESULT			EndOfStream();

	HRESULT			BreakConnect(PIN_DIRECTION dir);

	// === ISpecifyPropertyPages2

	STDMETHODIMP	GetPages(CAUUID* pPages);
	STDMETHODIMP	CreatePage(const GUID& guid, IPropertyPage** ppPage);

	// === IMPCVideoDecFilter
	STDMETHODIMP SetThreadNumber(int nValue);
	STDMETHODIMP_(int) GetThreadNumber();
	STDMETHODIMP SetDiscardMode(int nValue);
	STDMETHODIMP_(int) GetDiscardMode();
	STDMETHODIMP SetDeinterlacing(MPC_DEINTERLACING_FLAGS nValue);
	STDMETHODIMP_(MPC_DEINTERLACING_FLAGS) GetDeinterlacing();
	STDMETHODIMP SetARMode(int nValue);
	STDMETHODIMP_(int) GetARMode();

	STDMETHODIMP SetDXVACheckCompatibility(int nValue);
	STDMETHODIMP_(int) GetDXVACheckCompatibility();
	STDMETHODIMP SetDXVA_SD(int nValue);
	STDMETHODIMP_(int) GetDXVA_SD();

	STDMETHODIMP SetSwRefresh(int nValue);
	STDMETHODIMP SetSwPixelFormat(MPCPixelFormat pf, bool enable);
	STDMETHODIMP_(bool) GetSwPixelFormat(MPCPixelFormat pf);
	STDMETHODIMP SetSwPreset(int nValue);
	STDMETHODIMP_(int) GetSwPreset();
	STDMETHODIMP SetSwStandard(int nValue);
	STDMETHODIMP_(int) GetSwStandard();
	STDMETHODIMP SetSwRGBLevels(int nValue);
	STDMETHODIMP_(int) GetSwRGBLevels();

	STDMETHODIMP SetActiveCodecs(ULONGLONG nValue);
	STDMETHODIMP_(ULONGLONG) GetActiveCodecs();

	STDMETHODIMP SaveSettings();

	STDMETHODIMP_(CString) GetInformation(MPCInfo index);

	STDMETHODIMP_(GUID*) GetDXVADecoderGuid();
	STDMETHODIMP_(int) GetColorSpaceConversion();
	STDMETHODIMP GetOutputMediaType(CMediaType* pmt);
	STDMETHODIMP_(int) GetFrameType();

	// === DXVA common functions
	BOOL						IsSupportedDecoderConfig(const D3DFORMAT nD3DFormat, const DXVA2_ConfigPictureDecode& config, bool& bIsPrefered);
	BOOL						IsSupportedDecoderMode(const GUID* mode);
	int							GetPicEntryNumber();
	int							PictWidth();
	int							PictHeight();
	int							PictWidthRounded();
	int							PictHeightRounded();

	inline bool					UseDXVA2()				{ return (m_nDecoderMode == MODE_DXVA2); };
	inline AVCodecContext*		GetAVCtx()				{ return m_pAVCtx; };
	inline AVFrame*				GetFrame()				{ return m_pFrame; };
	inline enum AVCodecID		GetCodec()				{ return m_nCodecId; };
	inline bool					IsReorderBFrame()		{ return m_bReorderBFrame; };
	inline BOOL					IsEvo()					{ return m_bIsEVO; };
	inline DWORD				GetPCIVendor()			{ return m_nPCIVendor; };
	inline DWORD				GetPCIDevice()			{ return m_nPCIDevice; };
	inline double				GetRate()				{ return m_dRate; };
	bool						IsDXVASupported();
	void						UpdateAspectRatio();
	void						ReorderBFrames(REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);
	void						FlushDXVADecoder()	{
		if (m_pDXVADecoder) {
			m_pDXVADecoder->Flush();
		}
	}

	void						SetTypeSpecificFlags(IMediaSample* pMS);

	// === DXVA1 functions
	DDPIXELFORMAT*				GetPixelFormat() { return &m_DDPixelFormat; }
	HRESULT						FindDXVA1DecoderConfiguration(IAMVideoAccelerator* pAMVideoAccelerator, const GUID* guidDecoder, DDPIXELFORMAT* pPixelFormat);
	HRESULT						CheckDXVA1Decoder(const GUID *pGuid);
	void						SetDXVA1Params(const GUID* pGuid, DDPIXELFORMAT* pPixelFormat);
	WORD						GetDXVA1RestrictedMode();
	HRESULT						CreateDXVA1Decoder(IAMVideoAccelerator* pAMVideoAccelerator, const GUID* pDecoderGuid, DWORD dwSurfaceCount);


	// === DXVA2 functions
	void						FillInVideoDescription(DXVA2_VideoDesc *pDesc);
	HRESULT						ConfigureDXVA2(IPin *pPin);
	HRESULT						SetEVRForDXVA2(IPin *pPin);
	HRESULT						FindDXVA2DecoderConfiguration(IDirectXVideoDecoderService *pDecoderService,
															  const GUID& guidDecoder,
															  DXVA2_ConfigPictureDecode *pSelectedConfig,
															  BOOL *pbFoundDXVA2Configuration);
	HRESULT						CreateDXVA2Decoder(UINT nNumRenderTargets, IDirect3DSurface9** pDecoderRenderTargets);
	HRESULT						InitAllocator(IMemAllocator **ppAlloc);

	// === EVR functions
	HRESULT						DetectVideoCard_EVR(IPin *pPin);

	// === Codec functions
	HRESULT						SetFFMpegCodec(int nCodec, bool bEnabled);
	HRESULT						SetDXVACodec(int nCodec, bool bEnabled);

private:
	friend class CVideoDecDXVAAllocator;
	CVideoDecDXVAAllocator*		m_pDXVA2Allocator;

	// *** from LAV
	// *** Re-Commit the allocator (creates surfaces and new decoder)
	HRESULT						RecommitAllocator();
};

class CMPCVideoDecFilter;
class CVideoDecDXVAAllocator;

class CVideoDecOutputPin : public CBaseVideoOutputPin
	, public IAMVideoAcceleratorNotify
{
public:
	CVideoDecOutputPin(TCHAR* pObjectName, CBaseVideoFilter* pFilter, HRESULT* phr, LPCWSTR pName);
	~CVideoDecOutputPin();

	HRESULT				InitAllocator(IMemAllocator **ppAlloc);

	DECLARE_IUNKNOWN
	STDMETHODIMP		NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// IAMVideoAcceleratorNotify
	STDMETHODIMP		GetUncompSurfacesInfo(const GUID *pGuid, LPAMVAUncompBufferInfo pUncompBufferInfo);
	STDMETHODIMP		SetUncompSurfacesInfo(DWORD dwActualUncompSurfacesAllocated);
	STDMETHODIMP		GetCreateVideoAcceleratorData(const GUID *pGuid, LPDWORD pdwSizeMiscData, LPVOID *ppMiscData);

private :
	CMPCVideoDecFilter*	m_pVideoDecFilter;
	DWORD				m_dwDXVA1SurfaceCount;
	GUID				m_GuidDecoderDXVA1;
	DDPIXELFORMAT		m_ddUncompPixelFormat;
};

//
struct SUPPORTED_FORMATS {
	const CLSID*	clsMajorType;
	const CLSID*	clsMinorType;

	const int		FFMPEGCode;
	const int		DXVACode;
};

void GetFormatList(CAtlList<SUPPORTED_FORMATS>& fmts);
