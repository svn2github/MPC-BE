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

#include "SettingsDefines.h"
#include "FilterEnum.h"
#include "../../filters/renderer/VideoRenderers/RenderersSettings.h"
#include "../../Subtitles/STS.h"
#include "MediaFormats.h"
#include "DVBChannel.h"
#include <afxsock.h>

// flags for AppSettings::nCS
#define CS_NONE			0
#define CS_SEEKBAR		(1 << 0)
#define CS_TOOLBAR		(1 << 1)
#define CS_INFOBAR		(1 << 2)
#define CS_STATSBAR		(1 << 3)
#define CS_STATUSBAR	(1 << 4)

#define CLSW_NONE				0
#define CLSW_OPEN				(1 << 0)
#define CLSW_PLAY				(1 << 1)
#define CLSW_CLOSE				(1 << 2)
#define CLSW_STANDBY			(1 << 3)
#define CLSW_HIBERNATE			(1 << 4)
#define CLSW_SHUTDOWN			(1 << 5)
#define CLSW_LOGOFF				(1 << 6)
#define CLSW_LOCK				(1 << 7)
#define CLSW_AFTERPLAYBACK_MASK	(CLSW_CLOSE|CLSW_STANDBY|CLSW_SHUTDOWN|CLSW_HIBERNATE|CLSW_LOGOFF|CLSW_LOCK)
#define CLSW_FULLSCREEN			(1 << 8)
#define CLSW_NEW				(1 << 9)
#define CLSW_HELP				(1 << 10)
#define CLSW_DVD				(1 << 11)
#define CLSW_CD					(1 << 12)
#define CLSW_ADD				(1 << 13)
#define CLSW_MINIMIZED			(1 << 14)

#define CLSW_REGEXTVID			(1 << 16)
#define CLSW_REGEXTAUD			(1 << 17)
#define CLSW_REGEXTPL			(1 << 18)
#define CLSW_UNREGEXT			(1 << 19)

#define CLSW_STARTVALID			(1 << 21)
#define CLSW_NOFOCUS			(1 << 22)
#define CLSW_FIXEDSIZE			(1 << 23)
#define CLSW_MONITOR			(1 << 24)
#define CLSW_D3DFULLSCREEN		(1 << 25)
#define CLSW_ADMINOPTION		(1 << 26)
#define CLSW_SLAVE				(1 << 27)
#define CLSW_AUDIORENDERER		(1 << 28)
#define CLSW_RESET				(1 << 29)
#define CLSW_UNRECOGNIZEDSWITCH	(1 << 30)

enum {
	MODE_SHOWCAPTIONMENU,
	MODE_HIDEMENU,
	MODE_FRAMEONLY,
	MODE_BORDERLESS,
	MODE_COUNT
}; // flags for Caption & Menu Mode

enum {
	VIDRNDT_DS_DEFAULT,
	VIDRNDT_DS_OLDRENDERER, // obsolete
	VIDRNDT_DS_OVERLAYMIXER,
	VIDRNDT_DS_VMR7WINDOWED,
	VIDRNDT_DS_VMR9WINDOWED,
	VIDRNDT_DS_VMR7RENDERLESS,
	VIDRNDT_DS_VMR9RENDERLESS,
	VIDRNDT_DS_DXR,
	VIDRNDT_DS_NULL_COMP,
	VIDRNDT_DS_NULL_UNCOMP,
	VIDRNDT_DS_EVR,
	VIDRNDT_DS_EVR_CUSTOM,
	VIDRNDT_DS_MADVR,
	VIDRNDT_DS_SYNC
};

// Enumeration for MCE remote control (careful : add 0x010000 for all keys!)
enum MCE_RAW_INPUT {
	MCE_DETAILS				= 0x010209,
	MCE_GUIDE				= 0x01008D,
	MCE_TVJUMP				= 0x010025,
	MCE_STANDBY				= 0x010082,
	MCE_OEM1				= 0x010080,
	MCE_OEM2				= 0x010081,
	MCE_MYTV				= 0x010046,
	MCE_MYVIDEOS			= 0x01004A,
	MCE_MYPICTURES			= 0x010049,
	MCE_MYMUSIC				= 0x010047,
	MCE_RECORDEDTV			= 0x010048,
	MCE_DVDANGLE			= 0x01004B,
	MCE_DVDAUDIO			= 0x01004C,
	MCE_DVDMENU				= 0x010024,
	MCE_DVDSUBTITLE			= 0x01004D,
	MCE_RED					= 0x01005B,
	MCE_GREEN				= 0x01005C,
	MCE_YELLOW				= 0x01005D,
	MCE_BLUE				= 0x01005E,
	MCE_MEDIA_NEXTTRACK		= 0x0100B5,
	MCE_MEDIA_PREVIOUSTRACK	= 0x0100B6,
};

#define AUDRNDT_NULL_COMP _T("Null Audio Renderer (Any)")
#define AUDRNDT_NULL_UNCOMP _T("Null Audio Renderer (Uncompressed)")
#define AUDRNDT_MPC _T("MPC Audio Renderer")

#define AS_MAX_CHANNELS 18 // from AudioSwitcher.h

#define DEFAULT_AUDIO_PATHS    _T(".;.\\audio;.\\fandub")
#define DEFAULT_SUBTITLE_PATHS _T(".;.\\subtitles;.\\subs")
#define DEFAULT_JUMPDISTANCE_1  1000
#define DEFAULT_JUMPDISTANCE_2  5000
#define DEFAULT_JUMPDISTANCE_3 20000

enum dvstype {
	DVS_HALF,
	DVS_NORMAL,
	DVS_DOUBLE,
	DVS_STRETCH,
	DVS_FROMINSIDE,
	DVS_FROMOUTSIDE,
	DVS_ZOOM1,
	DVS_ZOOM2
};

enum favtype {
	FAV_FILE,
	FAV_DVD,
	FAV_DEVICE
};

#define MAX_DVD_POSITION 20
struct DVD_POSITION {
	ULONGLONG			llDVDGuid;
	ULONG				lTitle;
	DVD_HMSF_TIMECODE	Timecode;
};

#define MAX_FILE_POSITION 20
struct FILE_POSITION {
	CString				strFile;
	LONGLONG			llPosition;
};

enum {
	TIME_TOOLTIP_ABOVE_SEEKBAR,
	TIME_TOOLTIP_BELOW_SEEKBAR
};

#pragma pack(push, 1)
struct dispmode {
	bool fValid;
	CSize size;
	int bpp, freq;
	DWORD dmDisplayFlags;
};

struct fpsmode {
	double vfr_from;
	double vfr_to;
	bool fChecked;
	dispmode dmFSRes;
	bool fIsData;
};

#define MaxFpsCount 30
struct AChFR { //AutoChangeFullscrRes
	int bEnabled;
	fpsmode dmFullscreenRes[MaxFpsCount];
	bool bApplyDefault;
	bool bSetGlobal;
};

struct AccelTbl {
	bool bEnable;
	int cmd;
	int key;
	int id;
	int mwnd;
	int mfs;
	int appcmd;
	int remcmd;
	int repcnt;
};

#pragma pack(pop)

class wmcmd : public ACCEL
{
	ACCEL backup;
	UINT appcmdorg;
	UINT mouseorg;
	UINT mouseFSorg;

public:
	DWORD dwname;
	UINT appcmd;
	enum {
		NONE,
		LDOWN,
		LUP,
		LDBLCLK,
		MDOWN,
		MUP,
		MDBLCLK,
		RDOWN,
		RUP,
		RDBLCLK,
		X1DOWN,
		X1UP,
		X1DBLCLK,
		X2DOWN,
		X2UP,
		X2DBLCLK,
		WUP,
		WDOWN,
		LAST
	};
	UINT mouse;
	UINT mouseFS;
	CStringA rmcmd;
	int rmrepcnt;

	wmcmd(WORD cmd = 0) {
		this->cmd = cmd;
	}

	wmcmd(WORD cmd, WORD key, BYTE fVirt, DWORD dwname, UINT appcmd = 0, UINT mouse = NONE, UINT mouseFS = NONE, LPCSTR rmcmd = "", int rmrepcnt = 5) {
		this->cmd = cmd;
		this->key = key;
		this->fVirt = fVirt;
		this->appcmd = appcmdorg = appcmd;
		this->dwname = dwname;
		this->mouse = mouseorg = mouse;
		this->mouseFS = mouseFSorg = mouseFS;
		this->rmcmd = rmcmd;
		this->rmrepcnt = rmrepcnt;
		backup = *this;
	}

	bool operator == (const wmcmd& wc) const {
		return(cmd > 0 && cmd == wc.cmd);
	}

	CString GetName() const {
		return ResStr (dwname);
	}

	void Restore() {
		*(ACCEL*)this = backup;
		appcmd = appcmdorg;
		mouse = mouseorg;
		mouseFS = mouseFSorg;
		rmcmd.Empty();
		rmrepcnt = 5;
	}

	bool IsModified() const {
		return (memcmp((const ACCEL*)this, &backup, sizeof(ACCEL)) || appcmd != appcmdorg || mouse != mouseorg || mouseFS != mouseFSorg || !rmcmd.IsEmpty() || rmrepcnt != 5);
	}
};

class CRemoteCtrlClient : public CAsyncSocket
{
protected:
	CCritSec m_csLock;
	CWnd* m_pWnd;
	enum {DISCONNECTED, CONNECTED, CONNECTING} m_nStatus;
	CString m_addr;

	virtual void OnConnect(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);

	virtual void OnCommand(CStringA str) = 0;

	void ExecuteCommand(CStringA cmd, int repcnt);

public:
	CRemoteCtrlClient();
	void SetHWND(HWND hWnd);
	void Connect(CString addr);
	void DisConnect();
	int GetStatus() const { return(m_nStatus); }
};

class CWinLircClient : public CRemoteCtrlClient
{
protected:
	virtual void OnCommand(CStringA str);

public:
	CWinLircClient();
};

class CUIceClient : public CRemoteCtrlClient
{
protected:
	virtual void OnCommand(CStringA str);

public:
	CUIceClient();
};


class CFiltersPrioritySettings {
	template <class T = CString, class S = CString>
	class CAtlStringMap : public CAtlMap<S, T, CStringElementTraits<S> > {};

public:
	CAtlStringMap<CLSID> values;

	CFiltersPrioritySettings() {
		SetDefault();
	}
	void SetDefault() {
		static CString formats[] = {L"avi", L"mkv", L"mpegts", L"mpeg", L"mp4", L"flv", L"wmv"};

		values.RemoveAll();
		for (size_t i = 0; i < _countof(formats); i++) {
			values[formats[i]] = CLSID_NULL;
		}
	};

	void LoadSettings() {
		SetDefault();

		POSITION pos = values.GetStartPosition();
		while (pos) {
			CAtlStringMap<CLSID>::CPair* pPair = values.GetNext(pos);
			pPair->m_value = GUIDFromCString(AfxGetApp()->GetProfileString(IDS_R_SETTINGS IDS_R_FILTERS_PRIORITY, pPair->m_key, CStringFromGUID(pPair->m_value)));
		}
	};

	void SaveSettings() {
		POSITION pos = values.GetStartPosition();
		while (pos) {
			CAtlStringMap<CLSID>::CPair* pPair = values.GetNext(pos);
			AfxGetApp()->WriteProfileString(IDS_R_SETTINGS IDS_R_FILTERS_PRIORITY, pPair->m_key, CStringFromGUID(pPair->m_value));
		}
	};
};

class CPerfomanceSettings {
public:
	void SetDefault() {
		iCacheLen = DEFAULT_CACHE_LENGTH;

		iMinQueueSize = MINQUEUESIZE;
		iMaxQueueSize = MAXQUEUESIZE;

		iMinQueuePackets = MINQUEUEPACKETS;
		iMaxQueuePackets = MAXQUEUEPACKETS;

		fDefault = TRUE;
	};

	void LoadSettings() {
		MEMORYSTATUSEX msEx;
		msEx.dwLength = sizeof(msEx);
		::GlobalMemoryStatusEx(&msEx);
		DWORDLONG halfMemMB = msEx.ullTotalPhys/0x200000;

		iCacheLen = max(16, min(KILOBYTE, AfxGetApp()->GetProfileInt(IDS_R_SETTINGS IDS_R_PERFOMANCE, IDS_RS_PERFOMANCE_CACHE_LENGTH, DEFAULT_CACHE_LENGTH)));

		iMinQueueSize = max(64, min(KILOBYTE, AfxGetApp()->GetProfileInt(IDS_R_SETTINGS IDS_R_PERFOMANCE, IDS_RS_PERFOMANCE_MINQUEUESIZE, MINQUEUESIZE)));
		iMaxQueueSize = max(10, min(min(512, halfMemMB), AfxGetApp()->GetProfileInt(IDS_R_SETTINGS IDS_R_PERFOMANCE, IDS_RS_PERFOMANCE_MAXQUEUESIZE, MAXQUEUESIZE)));

		iMinQueuePackets = max(10, min(MAXQUEUEPACKETS, AfxGetApp()->GetProfileInt(IDS_R_SETTINGS IDS_R_PERFOMANCE, IDS_RS_PERFOMANCE_MINQUEUEPACKETS, MINQUEUEPACKETS)));
		iMaxQueuePackets = max(iMinQueuePackets*2, min(MAXQUEUEPACKETS*10, AfxGetApp()->GetProfileInt(IDS_R_SETTINGS IDS_R_PERFOMANCE, IDS_RS_PERFOMANCE_MAXQUEUEPACKETS, MAXQUEUEPACKETS)));

		fDefault = (iCacheLen == DEFAULT_CACHE_LENGTH
					&& iMinQueueSize == MINQUEUESIZE
					&& iMaxQueueSize == MAXQUEUESIZE
					&& iMinQueuePackets == MINQUEUEPACKETS
					&& iMaxQueuePackets == MAXQUEUEPACKETS);
	};

	void SaveSettings() {
		AfxGetApp()->WriteProfileInt(IDS_R_SETTINGS IDS_R_PERFOMANCE, IDS_RS_PERFOMANCE_CACHE_LENGTH, iCacheLen);
		AfxGetApp()->WriteProfileInt(IDS_R_SETTINGS IDS_R_PERFOMANCE, IDS_RS_PERFOMANCE_MINQUEUESIZE, iMinQueueSize);
		AfxGetApp()->WriteProfileInt(IDS_R_SETTINGS IDS_R_PERFOMANCE, IDS_RS_PERFOMANCE_MAXQUEUESIZE, iMaxQueueSize);
		AfxGetApp()->WriteProfileInt(IDS_R_SETTINGS IDS_R_PERFOMANCE, IDS_RS_PERFOMANCE_MINQUEUEPACKETS, iMinQueuePackets);
		AfxGetApp()->WriteProfileInt(IDS_R_SETTINGS IDS_R_PERFOMANCE, IDS_RS_PERFOMANCE_MAXQUEUEPACKETS, iMaxQueuePackets);
	};

	void UpdateStatus() {
		fDefault = (iCacheLen == DEFAULT_CACHE_LENGTH
					&& iMinQueueSize == MINQUEUESIZE
					&& iMaxQueueSize == MAXQUEUESIZE
					&& iMinQueuePackets == MINQUEUEPACKETS
					&& iMaxQueuePackets == MAXQUEUEPACKETS);
	}

	DWORD iCacheLen;
	DWORD iMinQueueSize, iMaxQueueSize;
	DWORD iMinQueuePackets, iMaxQueuePackets;

	BOOL fDefault;
};

class CAppSettings
{
	bool	fInitialized;

	CWnd*	p_MainWnd;

	class CRecentFileAndURLList : public CRecentFileList
	{
	public:
		CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
							  LPCTSTR lpszEntryFormat, int nSize,
							  int nMaxDispLen = AFX_ABBREV_FILENAME_LEN);

		virtual void Add(LPCTSTR lpszPathName); // we have to override CRecentFileList::Add because the original version can't handle URLs
	};

public:

	void		SetMainWnd(CWnd* p_CWnd)	{ p_MainWnd = p_CWnd; };
	const CWnd*	GetMainWnd()				{ return p_MainWnd; };

	bool fReset;

	bool fShadersNeedSave;

	// cmdline params
	UINT nCLSwitches;
	CAtlList<CString>	slFiles, slDubs, slSubs, slFilters;

	// Initial position (used by command line flags)
	__int64				rtShift;
	__int64				rtStart;
	ULONG				lDVDTitle;
	ULONG				lDVDChapter;
	DVD_HMSF_TIMECODE	DVDPosition;
	bool				fStartMainTitle;
	bool				fNormalStartDVD;

	CSize sizeFixedWindow;
	bool HasFixedWindowSize() const { return sizeFixedWindow.cx > 0 || sizeFixedWindow.cy > 0; }
	//int			iFixedWidth, iFixedHeight;
	int				iMonitor;

	CString			ParseFileName(CString const& param);
	void			ParseCommandLine(CAtlList<CString>& cmdln);

	// Added a Debug display to the screen (/debug option)
	bool			fShowDebugInfo;
	int				iAdminOption;
	int				iDXVer;


	// Player
	int				iMultipleInst;
	bool			fTrayIcon;
	bool			fShowOSD;
	bool			fLimitWindowProportions;
	bool			fSnapToDesktopEdges;
	bool			fHideCDROMsSubMenu;
	DWORD			dwPriority;
	int				iTitleBarTextStyle;
	bool			fTitleBarTextTitle;
	bool			fKeepHistory;
	CRecentFileAndURLList MRU;
	CRecentFileAndURLList MRUDub;
	bool			fRememberDVDPos;
	bool			fRememberFilePos;
	bool			bRememberPlaylistItems;
	bool			fRememberWindowPos;
	bool			fRememberWindowSize;
	bool			fSavePnSZoom;
	float			dZoomX;
	float			dZoomY;

	// Formats
	CMediaFormats	m_Formats;
	bool			fAssociatedWithIcons;
	engine_t		iRtspHandler;
	bool			fRtspFileExtFirst;

	// Keys
	CList<wmcmd>	wmcmds;
	HACCEL			hAccel;
	bool			fWinLirc;
	CString			strWinLircAddr;
	CWinLircClient	WinLircClient;
	bool			fUIce;
	CString			strUIceAddr;
	CUIceClient		UIceClient;
	bool			fGlobalMedia;

	// Logo
	UINT			nLogoId;
	bool			fLogoExternal;
	CString			strLogoFileName;

	// Web Inteface
	BOOL			fEnableWebServer;
	int				nWebServerPort;
	int				nWebServerQuality;
	int				nCmdlnWebServerPort;
	bool			fWebServerUseCompression;
	bool			fWebServerLocalhostOnly;
	bool			fWebServerPrintDebugInfo;
	CString			strWebRoot, strWebDefIndex;
	CString			strWebServerCGI;

	// Playback
	int				nVolume;
	int				nVolumeStep;
	bool			fMute;
	int				nBalance;
	int				nLoops;
	bool			fLoopForever;
	bool			fRewind;
	int				nSpeedStep;
	bool			fRememberZoomLevel;
	int				iZoomLevel;
	CStringW		strSubtitlesLanguageOrder;
	CStringW		strAudiosLanguageOrder;
	bool			fUseInternalSelectTrackLogic;
	bool			fEnableWorkerThreadForOpening;
	bool			fReportFailedPins;

	// Audio
	bool			fAutoloadAudio;
	bool			fPrioritizeExternalAudio;
	CString			strAudioPaths;

	// DVD/OGM
	bool			fUseDVDPath;
	CString			strDVDPath;
	LCID			idMenuLang, idAudioLang, idSubtitlesLang;
	bool			fClosedCaptions;

	// Output
	CRenderersSettings m_RenderersSettings;
	int				iDSVideoRendererType;
	int				iRMVideoRendererType;
	int				iQTVideoRendererType;

	bool			fDualAudioOutput;
	CString			strAudioRendererDisplayName;
	CString			strSecondAudioRendererDisplayName;

	bool			fD3DFullscreen;
	bool			fIsFSWindow;

	// Fullscreen
	bool			fLaunchfullscreen;
	bool			fShowBarsWhenFullScreen;
	int				nShowBarsWhenFullScreenTimeOut;
	bool			fExitFullScreenAtTheEnd;
	bool			fExitFullScreenAtFocusLost;
	CStringW		strFullScreenMonitor;
	CStringW		strFullScreenMonitorID;
	AChFR			AutoChangeFullscrRes;
	AccelTbl		AccelTblColWidth;
	bool			fRestoreResAfterExit;
	dispmode		dm_def;
	double			dFPS;

	// Sync Renderer Settings

	// Capture (BDA configuration)
	// BDA configuration
	int				iDefaultCaptureDevice;		// Default capture device (analog=0, 1=digital)
	CString			strAnalogVideo;
	CString			strAnalogAudio;
	int				iAnalogCountry;
	CString			strBDANetworkProvider;
	CString			strBDATuner;
	CString			strBDAReceiver;
	//CString			strBDAStandard;
	int				iBDAScanFreqStart;
	int				iBDAScanFreqEnd;
	int				iBDABandwidth;
	bool			fBDAUseOffset;
	int				iBDAOffset;
	bool			fBDAIgnoreEncryptedChannels;
	UINT			nDVBLastChannel;
	CAtlList<CDVBChannel> m_DVBChannels;

	// Internal Filters
	bool			SrcFilters[SRC_LAST];
	bool			DXVAFilters[VDEC_DXVA_LAST];
	bool			VideoFilters[VDEC_LAST];
	bool			AudioFilters[ADEC_LAST];

	// Audio Switcher
	bool			fEnableAudioSwitcher;
	bool			fAudioNormalize;
	bool			fAudioNormalizeRecover;
	float			dAudioBoost_dB;
	bool			fAudioTimeShift;
	int				iAudioTimeShift;
	bool			fCustomChannelMapping;
	int				nSpeakerChannels;
	DWORD			pSpeakerToChannelMap[AS_MAX_CHANNELS][AS_MAX_CHANNELS];

	// External Filters
	CAutoPtrList<FilterOverride> m_filters;

	// Subtitles
	bool			fAutoloadSubtitles;
	bool			fBlockVSFilter;
	CString			strSubtitlePaths;
	bool			fPrioritizeExternalSubtitles;
	bool			fDisableInternalSubtitles;
	bool			fAutoReloadExtSubtitles;
	bool			fUseSybresync;
	CString			strISDb;

	// Subtitles - Rendering
	bool			fOverridePlacement;
	int				nHorPos, nVerPos;
	int				nSubDelayInterval;

	// Subtitles - Default Style
	STSStyle		subdefstyle;

	// Interface
	bool			fDisableXPToolbars;
	int				nThemeBrightness;
	int				nThemeRed;
	int				nThemeGreen;
	int				nThemeBlue;
	int				nOSDTransparent;
	int				nOSDBorder;

	COLORREF		clrFaceABGR;
	COLORREF		clrOutlineABGR;
	COLORREF		clrFontABGR;
	COLORREF		clrGrad1ABGR;
	COLORREF		clrGrad2ABGR;

	bool			fUseTimeTooltip;
	int				nTimeTooltipPosition;
	bool			fFileNameOnSeekBar;
	bool			fSmartSeek;
	bool			fChapterMarker;
	bool			fFlybar;
	bool			fFontShadow;
	bool			fFontAA;
	bool			fFlybarOnTop;
	bool			fUseWin7TaskBar;
	int				nOSDSize;
	CString			strOSDFont;
	double			scalefont;

	int				iDlgPropX;
	int				iDlgPropY;

	// Tweaks
	int				nJumpDistS;
	int				nJumpDistM;
	int				nJumpDistL;
	bool			fDontUseSearchInFolder;
	bool			fPreventMinimize;
	bool			fLCDSupport;
	bool			fFastSeek;
	bool			fMiniDump;

	// Miscellaneous
	int				iBrightness;
	int				iContrast;
	int				iHue;
	int				iSaturation;

	// MENUS
	// View
	int				iCaptionMenuMode; // normal -> hidemenu -> frameonly -> borderless
	bool			fHideNavigation;
	UINT			nCS; // Control state for toolbars
	// Language
	int				iLanguage;
	// Subtitles menu
	bool			fEnableSubtitles;
	bool			fUseDefaultSubtitlesStyle;
	bool			fForcedSubtitles;
	// Video Frame
	int				iDefaultVideoSize;
	bool			fKeepAspectRatio;
	CSize			sizeAspectRatio;
	bool			fCompMonDeskARDiff;
	// Pan&Scan
	CString			strPnSPreset;
	CStringArray	m_pnspresets;
	// On top menu
	int				iOnTop;
	// After Playback
	bool			fExitAfterPlayback;
	bool			fNextInDirAfterPlayback;

	// WINDOWS
	// Add Favorite
	bool			bFavRememberPos;
	bool			bFavRelativeDrive;
	// Save Image...
	CString			strSnapShotPath, strSnapShotExt;
	// Save Thumbnails...
	int				iThumbRows, iThumbCols, iThumbWidth, iThumbQuality, iThumbLevelPNG;
	// Shader Editor
	struct Shader {
		CString		label;
		CString		target;
		CString		srcdata;
	};
	CAtlList<Shader> m_shaders;
	// Shader Combiner
	bool			fToggleShader;
	bool			fToggleShaderScreenSpace;
	CString			strShaderList;
	CString			strShaderListScreenSpace;
	// Playlist (contex menu)
	bool			bShufflePlaylistItems;
	bool			bHidePlaylistFullScreen;

	// OTHER STATES
	CStringW		strLastOpenDir;
	UINT			nLastWindowType;
	CRect			rcLastWindowPos;
	UINT			nLastUsedPage;
	bool			fRemainingTime;
	bool			fLastFullScreen;

	CString			strTimeOnSeekBar;
	bool			bStatusBarIsVisible;

	bool			fIntRealMedia;
	//bool			fRealMediaRenderless;
	//float			dRealMediaQuickTimeFPS;
	//int			iVideoRendererType;
	//int			iQuickTimeRenderer;
	//bool			fMonitorAutoRefreshRate;
	bool			fEnableEDLEditor;

	HWND			hMasterWnd;

	bool			IsD3DFullscreen() const;
	CString			SelectedAudioRenderer() const;
	void			ResetPositions();
	DVD_POSITION*	CurrentDVDPosition();
	bool			NewDvd(ULONGLONG llDVDGuid);
	FILE_POSITION*	CurrentFilePosition();
	bool			NewFile(LPCTSTR strFileName);

	void			SaveCurrentDVDPosition();
	void			ClearDVDPositions();
	void			SaveCurrentFilePosition();
	void			ClearFilePositions();

	void			DeserializeHex (LPCTSTR strVal, BYTE* pBuffer, int nBufSize) const;
	CString			SerializeHex (BYTE* pBuffer, int nBufSize) const;

	// list of temporary files
	CAtlList<CString> slTMPFilesList;

	CString			strLastOpenFilterDir;

	int				iYoutubeTag;
	int				iYoutubeSource;

	DWORD			nLastFileInfoPage;

private :
	DVD_POSITION	DvdPosition[MAX_DVD_POSITION];
	int				nCurrentDvdPosition;
	FILE_POSITION	FilePosition[MAX_FILE_POSITION];
	int				nCurrentFilePosition;

	CString			SrcFiltersKeys[SRC_LAST];
	CString			DXVAFiltersKeys[VDEC_DXVA_LAST];
	CString			VideoFiltersKeys[VDEC_LAST];
	CString			AudioFiltersKeys[ADEC_LAST];

	__int64			ConvertTimeToMSec(CString& time) const;
	void			ExtractDVDStartPos(CString& strParam);

	void			CreateCommands();

public:
	CAppSettings();
	virtual ~CAppSettings();
	void			SaveSettings();
	void			SaveExternalFilters();
	void			LoadSettings();
	void			SaveShaders();
	void			LoadShaders();

	void			GetFav(favtype ft, CAtlList<CString>& sl);
	void			SetFav(favtype ft, CAtlList<CString>& sl);
	void			AddFav(favtype ft, CString s);
	CDVBChannel*	FindChannelByPref(int nPrefNumber);

	int				GetMultiInst();

	engine_t		GetRtspHandler(bool& lookext);
	void			SetRtspHandler(engine_t e, bool lookext);
	bool			IsUsingRtspEngine(CString path, engine_t e);
	engine_t		GetRtspEngine(CString path);

private:
	void			UpdateRenderersData(bool fSave);
	friend	void	CRenderersSettings::UpdateData(bool bSave);

public:
	CPerfomanceSettings			PerfomanceSettings;
	CFiltersPrioritySettings	FiltersPrioritySettings;

	CAtlList<CString>			slSubtitlePathsAddons;
	CAtlList<CString>			slAudioPathsAddons;
};
