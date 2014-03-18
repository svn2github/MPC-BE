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

enum {
	WM_REARRANGERENDERLESS = WM_APP+1,
	WM_RESET_DEVICE,
};

#define WM_MYMOUSELAST WM_XBUTTONDBLCLK

enum {
	VIDRNDT_RM_DEFAULT,
	VIDRNDT_RM_DX7,
	VIDRNDT_RM_DX9,
};

enum {
	VIDRNDT_QT_DEFAULT,
	VIDRNDT_QT_DX7,
	VIDRNDT_QT_DX9,
};

enum {
	VIDRNDT_AP_SURFACE,
	VIDRNDT_AP_TEXTURE2D,
	VIDRNDT_AP_TEXTURE3D,
};

enum VideoSystem {
	VIDEO_SYSTEM_UNKNOWN,
	VIDEO_SYSTEM_HDTV,
	VIDEO_SYSTEM_SDTV_NTSC,
	VIDEO_SYSTEM_SDTV_PAL,
};

enum AmbientLight {
	AMBIENT_LIGHT_BRIGHT,
	AMBIENT_LIGHT_DIM,
	AMBIENT_LIGHT_DARK,
};

enum ColorRenderingIntent {
	COLOR_RENDERING_INTENT_PERCEPTUAL,
	COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC,
	COLOR_RENDERING_INTENT_SATURATION,
	COLOR_RENDERING_INTENT_ABSOLUTE_COLORIMETRIC,
};

class CRenderersSettings
{

public:
	bool fResetDevice;

	// Subtitle position settings
	int bPositionRelative;

	// Stereoscopic Subtitles
	BOOL bStereoDisabled;
	BOOL bSideBySide;
	BOOL bTopAndBottom;

	// Subtitle position
	CPoint nShiftPos;

	class CAdvRendererSettings
	{
	public:
		CAdvRendererSettings() {SetDefault();}

		bool   fVMR9AlterativeVSync;
		int    iVMR9VSyncOffset;
		bool   iVMR9VSyncAccurate;
		bool   iVMR9FullscreenGUISupport;
		bool   iVMR9VSync;
		bool   iVMR9FullFloatingPointProcessing;
		bool   iVMR9HalfFloatingPointProcessing;
		bool   iVMR9ColorManagementEnable;
		int    iVMR9ColorManagementInput;
		int    iVMR9ColorManagementAmbientLight;
		int    iVMR9ColorManagementIntent;
		bool   iVMRDisableDesktopComposition;
		int    iVMRFlushGPUBeforeVSync;
		int    iVMRFlushGPUAfterPresent;
		int    iVMRFlushGPUWait;

		// EVR
		bool   iEVRHighColorResolution;
		bool   iEVRForceInputHighColorResolution;
		bool   iEVREnableFrameTimeCorrection;
		int    iEVROutputRange;

		// SyncRenderer settings
		int    bSynchronizeVideo;
		int    bSynchronizeDisplay;
		int    bSynchronizeNearest;
		int    iLineDelta;
		int    iColumnDelta;
		double fCycleDelta;
		double fTargetSyncOffset;
		double fControlLimit;

		void SetDefault();
		void SetOptimal();
	};

	CAdvRendererSettings m_AdvRendSets;

	int			iAPSurfaceUsage;
	int			iDX9Resizer;
	bool		fVMRMixerMode;
	bool		fVMRMixerYUV;

	int			iEvrBuffers;

	int			nSPCSize;
	int			nSPMaxTexRes;
	bool		fSPCPow2Tex;
	bool		fSPCAllowAnimationWhenBuffering;

	CString		D3D9RenderDevice;
	void		UpdateData(bool fSave);
};

class CRenderersData
{
	HINSTANCE	m_hD3DX9Dll;
	UINT		m_nDXSdkRelease;

public:
	CRenderersData();

	// Casimir666
	bool		m_fTearingTest;
	int			m_fDisplayStats;
	bool		m_bResetStats; // Set to reset the presentation statistics
	CString		m_strD3DX9Version;

	// Hardware feature support
	bool		m_bFP16Support;
	bool		m_b10bitSupport;

	LONGLONG	GetPerfCounter();
	HINSTANCE	GetD3X9Dll();
};

extern CRenderersData*		GetRenderersData();
extern CRenderersSettings&	GetRenderersSettings();

extern bool LoadResource(UINT resid, CStringA& str, LPCTSTR restype);
