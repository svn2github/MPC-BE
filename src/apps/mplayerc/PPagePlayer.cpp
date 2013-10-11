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

#include "stdafx.h"
#include "MainFrm.h"
#include "PPagePlayer.h"


// CPPagePlayer dialog

IMPLEMENT_DYNAMIC(CPPagePlayer, CPPageBase)
CPPagePlayer::CPPagePlayer()
	: CPPageBase(CPPagePlayer::IDD, CPPagePlayer::IDD)
	, m_iMultipleInst(1)
	, m_iAlwaysOnTop(FALSE)
	, m_fTrayIcon(FALSE)
	, m_iTitleBarTextStyle(0)
	, m_bTitleBarTextTitle(0)
	, m_fRememberWindowPos(FALSE)
	, m_fRememberWindowSize(FALSE)
	, m_fSavePnSZoom(FALSE)
	, m_fSnapToDesktopEdges(FALSE)
	, m_fUseIni(FALSE)
	, m_fKeepHistory(FALSE)
	, m_fHideCDROMsSubMenu(FALSE)
	, m_priority(FALSE)
	, m_fShowOSD(FALSE)
	, m_fLimitWindowProportions(TRUE)
	, m_fRememberDVDPos(FALSE)
	, m_fRememberFilePos(FALSE)
{
}

CPPagePlayer::~CPPagePlayer()
{
}

void CPPagePlayer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_RADIO1, m_iMultipleInst);
	DDX_Radio(pDX, IDC_RADIO4, m_iTitleBarTextStyle);
	DDX_Check(pDX, IDC_CHECK13, m_bTitleBarTextTitle);
	//DDX_Check(pDX, IDC_CHECK2, m_iAlwaysOnTop);
	DDX_Check(pDX, IDC_CHECK3, m_fTrayIcon);
	DDX_Check(pDX, IDC_CHECK6, m_fRememberWindowPos);
	DDX_Check(pDX, IDC_CHECK7, m_fRememberWindowSize);
	DDX_Check(pDX, IDC_CHECK11, m_fSavePnSZoom);
	DDX_Check(pDX, IDC_CHECK12, m_fSnapToDesktopEdges);
	DDX_Check(pDX, IDC_CHECK8, m_fUseIni);
	DDX_Check(pDX, IDC_CHECK1, m_fKeepHistory);
	DDX_Check(pDX, IDC_CHECK10, m_fHideCDROMsSubMenu);
	DDX_Check(pDX, IDC_CHECK9, m_priority);
	DDX_Check(pDX, IDC_SHOW_OSD, m_fShowOSD);
	DDX_Check(pDX, IDC_CHECK4, m_fLimitWindowProportions);
	DDX_Check(pDX, IDC_DVD_POS, m_fRememberDVDPos);
	DDX_Check(pDX, IDC_FILE_POS, m_fRememberFilePos);
	DDX_Check(pDX, IDC_CHECK2, m_bRememberPlaylistItems);
}

BEGIN_MESSAGE_MAP(CPPagePlayer, CPPageBase)
	ON_UPDATE_COMMAND_UI(IDC_CHECK13, OnUpdateCheck13)
	ON_UPDATE_COMMAND_UI(IDC_DVD_POS, OnUpdatePos)
	ON_UPDATE_COMMAND_UI(IDC_FILE_POS, OnUpdatePos)
END_MESSAGE_MAP()

// CPPagePlayer message handlers

BOOL CPPagePlayer::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	m_iMultipleInst = s.iMultipleInst;
	m_iTitleBarTextStyle = s.iTitleBarTextStyle;
	m_bTitleBarTextTitle = s.fTitleBarTextTitle;
	m_iAlwaysOnTop = s.iOnTop;
	m_fTrayIcon = s.fTrayIcon;
	m_fRememberWindowPos = s.fRememberWindowPos;
	m_fRememberWindowSize = s.fRememberWindowSize;
	m_fSavePnSZoom = s.fSavePnSZoom;
	m_fSnapToDesktopEdges = s.fSnapToDesktopEdges;
	m_fUseIni = AfxGetMyApp()->IsIniValid();
	m_fKeepHistory = s.fKeepHistory;
	m_fHideCDROMsSubMenu = s.fHideCDROMsSubMenu;
	m_priority = s.dwPriority != NORMAL_PRIORITY_CLASS;
	m_fShowOSD = s.fShowOSD;
	m_fRememberDVDPos = s.fRememberDVDPos;
	m_fRememberFilePos = s.fRememberFilePos;
	m_fLimitWindowProportions = s.fLimitWindowProportions;
	m_bRememberPlaylistItems = s.bRememberPlaylistItems;

	UpdateData(FALSE);

	GetDlgItem(IDC_FILE_POS)->EnableWindow(s.fKeepHistory);
	GetDlgItem(IDC_DVD_POS)->EnableWindow(s.fKeepHistory);

	return TRUE;
}

BOOL CPPagePlayer::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.iMultipleInst = m_iMultipleInst;
	s.iTitleBarTextStyle = m_iTitleBarTextStyle;
	s.fTitleBarTextTitle = !!m_bTitleBarTextTitle;
	s.iOnTop = m_iAlwaysOnTop;
	s.fTrayIcon = !!m_fTrayIcon;
	s.fRememberWindowPos = !!m_fRememberWindowPos;
	s.fRememberWindowSize = !!m_fRememberWindowSize;
	s.fSavePnSZoom = !!m_fSavePnSZoom;
	s.fSnapToDesktopEdges = !!m_fSnapToDesktopEdges;
	s.fKeepHistory = !!m_fKeepHistory;
	s.fHideCDROMsSubMenu = !!m_fHideCDROMsSubMenu;
	s.dwPriority = !m_priority ? NORMAL_PRIORITY_CLASS : ABOVE_NORMAL_PRIORITY_CLASS;
	s.fShowOSD = !!m_fShowOSD;
	if (m_fShowOSD) {
		((CMainFrame*)AfxGetMainWnd())->m_OSD.Start(((CMainFrame*)AfxGetMainWnd())->m_pOSDWnd);
		((CMainFrame*)AfxGetMainWnd())->m_OSD.ClearMessage(false);
	} else {
		((CMainFrame*)AfxGetMainWnd())->m_OSD.Stop();
		((CMainFrame*)AfxGetMainWnd())->m_OSD.ShowWindow(SW_HIDE);
	}
	s.fLimitWindowProportions = !!m_fLimitWindowProportions;
	s.fRememberDVDPos = m_fRememberDVDPos ? true : false;
	s.fRememberFilePos = m_fRememberFilePos ? true : false;
	s.bRememberPlaylistItems = !!m_bRememberPlaylistItems;

	if (!m_fKeepHistory) {
		for (int i = s.MRU.GetSize() - 1; i >= 0; i--) {
			s.MRU.Remove(i);
		}

		for (int i = s.MRUDub.GetSize() - 1; i >= 0; i--) {
			s.MRUDub.Remove(i);
		}

		s.MRU.WriteList();
		s.MRUDub.WriteList();

		// Empty the "Recent" jump list
		CComPtr<IApplicationDestinations> pDests;
		HRESULT hr = pDests.CoCreateInstance(CLSID_ApplicationDestinations, NULL, CLSCTX_INPROC_SERVER);
		if (SUCCEEDED(hr)) {
			hr = pDests->RemoveAllDestinations();
		}

		s.ClearFilePositions();
		s.ClearDVDPositions();
	}

	// Check if the settings location needs to be changed
	if (AfxGetMyApp()->IsIniValid() != !!m_fUseIni) {
		AfxGetMyApp()->ChangeSettingsLocation(!!m_fUseIni);
	}

	((CMainFrame*)AfxGetMainWnd())->ShowTrayIcon(s.fTrayIcon);

	::SetPriorityClass(::GetCurrentProcess(), s.dwPriority);

	GetDlgItem(IDC_FILE_POS)->EnableWindow(s.fKeepHistory);
	GetDlgItem(IDC_DVD_POS)->EnableWindow(s.fKeepHistory);

	return __super::OnApply();
}

void CPPagePlayer::OnUpdateCheck13(CCmdUI* pCmdUI)
{
	UpdateData();

	pCmdUI->Enable(m_iTitleBarTextStyle == 1);
}

void CPPagePlayer::OnUpdatePos(CCmdUI* pCmdUI)
{
	UpdateData();

	pCmdUI->Enable(!!m_fKeepHistory);
}
