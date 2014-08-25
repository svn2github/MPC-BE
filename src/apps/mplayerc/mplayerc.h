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

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"
#include <afxadv.h>
#include <atlsync.h>
#include "FakeFilterMapper2.h"
#include "AppSettings.h"
#include "../../../Include/Version.h"
#include "WinDebugMonitor.h"

#define DEF_LOGO IDF_LOGO1

enum {
	WM_GRAPHNOTIFY = WM_RESET_DEVICE+1,
	WM_TUNER_SCAN_PROGRESS,
	WM_TUNER_SCAN_END,
	WM_TUNER_STATS,
	WM_TUNER_NEW_CHANNEL,
	WM_POSTOPEN
};

#define WM_MYMOUSELAST WM_XBUTTONDBLCLK

extern HICON LoadIcon(CString fn, bool fSmall);
extern bool LoadType(CString fn, CString& type);
extern bool LoadResource(UINT resid, CStringA& str, LPCTSTR restype);
extern CStringA GetContentType(CString fn, CAtlList<CString>* redir = NULL);
extern WORD AssignedToCmd(UINT keyOrMouseValue, bool bIsFullScreen = false, bool bCheckMouse = true);

extern bool GetCurDispMode(dispmode& dm, CString& DisplayName);
extern bool GetDispMode(int i, dispmode& dm, CString& DisplayName);
extern void SetDispMode(dispmode& dm, CString& DisplayName);
extern void SetAudioRenderer(int AudioDevNo);
extern void ThemeRGB(int iR, int iG, int iB, int& iRed, int& iGreen, int& iBlue);

extern void SetHandCursor(HWND m_hWnd, UINT nID);

struct LanguageResource {
	const UINT resourceID;
	const LANGID localeID;
	const LPCTSTR name;
	const LPCTSTR strcode;
};

class CMPlayerCApp : public CWinApp
{
	ATL::CMutex m_mutexOneInstance;

	CAtlList<CString> m_cmdln;
	void PreProcessCommandLine();
	BOOL SendCommandLine(HWND hWnd);
	UINT GetVKFromAppCommand(UINT nAppCommand);

	static UINT	GetRemoteControlCodeMicrosoft(UINT nInputcode, HRAWINPUT hRawInput);
	static UINT	GetRemoteControlCodeSRM7500(UINT nInputcode, HRAWINPUT hRawInput);

	static UINT RunTHREADCopyData(LPVOID pParam);
public:
	CMPlayerCApp();

	void ShowCmdlnSwitches() const;

	bool StoreSettingsToIni();
	bool StoreSettingsToRegistry();
	CString GetIniPath() const;
	bool IsIniValid() const;
	bool IsIniUTF16LE() const;
	bool ChangeSettingsLocation(bool useIni);
	void ExportSettings();

	bool GetAppSavePath(CString& path);

	CRenderersData m_Renderers;
	CString		m_AudioRendererDisplayName_CL;

	CAppSettings m_s;

	typedef UINT (*PTR_GetRemoteControlCode)(UINT nInputcode, HRAWINPUT hRawInput);

	PTR_GetRemoteControlCode	GetRemoteControlCode;

	static const LanguageResource languageResources[];
	static const size_t languageResourcesCount;

	static void					SetLanguage(int nLanguage);
	static CString				GetSatelliteDll(int nLanguage);
	static int					GetLanguageIndex(UINT resID);
	static int					GetLanguageIndex(CString langcode);
	static int					GetDefLanguage();

	static bool					IsVSFilterInstalled();
	static bool					HasEVR();
	static HRESULT				GetElevationType(TOKEN_ELEVATION_TYPE* ptet);
	static void					RunAsAdministrator(LPCTSTR strCommand, LPCTSTR strArgs, bool bWaitProcess);

	void						RegisterHotkeys();
	void						UnregisterHotkeys();
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMPlayerCApp)
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	// Implementation

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnAppAbout();
	afx_msg void OnFileExit();
	afx_msg void OnHelpShowcommandlineswitches();
};

#define AfxGetMyApp() static_cast<CMPlayerCApp*>(AfxGetApp())
#define AfxGetAppSettings() static_cast<CMPlayerCApp*>(AfxGetApp())->m_s
#define AppSettings CAppSettings

class CDebugMonitor : public CWinDebugMonitor
{
	FILE* m_File;

private:
	CString GetLocalTime() {
		SYSTEMTIME st;
		::GetLocalTime(&st);

		CString lt;
		lt.Format(L"%04d.%02d.%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		return lt;
	}

public:
	CDebugMonitor(DWORD dwProcessId) : CWinDebugMonitor(dwProcessId) {
		static CString sDesktop;
		if (sDesktop.IsEmpty()) {
			TCHAR szPath[MAX_PATH];
			if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, szPath))) {
				sDesktop = CString(szPath) + L"\\";
			}
		}

		m_File = NULL;
		if (bIsInitialize) {
			CString fname = sDesktop + L"mpc-be_debug.log";
			m_File = _tfopen(fname, L"at, ccs=UTF-8");
			if (m_File) {
				fseek(m_File, 0, 2);

				_ftprintf_s(m_File, _T("=== Start MPC-BE Debug log [%s] ===\n"), GetLocalTime());
			}
		}
	}

	~CDebugMonitor() {
		if (m_File) {
			_ftprintf_s(m_File, _T("=== End MPC-BE Debug log [%s] ===\n"), GetLocalTime());

			fclose(m_File);
		}
	}

	virtual void OutputWinDebugString(const char *str) {
		if (m_File) {
			_ftprintf_s(m_File, _T("%s : %s"), GetLocalTime(), CString(str));
		}
	};
};

