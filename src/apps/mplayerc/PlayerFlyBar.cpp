/*
 * Copyright (C) 2012 Sergey "judelaw" and Sergey "Exodus8"
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
#include "PlayerFlyBar.h"

// CPrevView

CFlyBar::CFlyBar() :
	bt_idx(-1),
	r_ExitIcon(0,0,0,0),
	r_MinIcon(0,0,0,0),
	r_RestoreIcon(0,0,0,0),
	r_SettingsIcon(0,0,0,0),
	r_InfoIcon(0,0,0,0),
	r_FSIcon(0,0,0,0),
	r_LockIcon(0,0,0,0),
	m_pButtonsImages(NULL)
{
	int fp = m_logobm.FileExists(CString(_T("flybar")));

	HBITMAP hBmp = m_logobm.LoadExternalImage("flybar", IDB_PLAYERFLYBAR_PNG, -1);
	BITMAP bm;
	::GetObject(hBmp, sizeof(bm), &bm);

	if (fp && bm.bmWidth != bm.bmHeight * 25) {
		hBmp = m_logobm.LoadExternalImage("", IDB_PLAYERFLYBAR_PNG, -1);
		::GetObject(hBmp, sizeof(bm), &bm);
	}

	if (NULL != hBmp) {
		CBitmap *bmp = DNew CBitmap();
		bmp->Attach(hBmp);

		if (bm.bmWidth == bm.bmHeight * 25) {
			m_pButtonsImages = DNew CImageList();
			m_pButtonsImages->Create(bm.bmHeight, bm.bmHeight, ILC_COLOR32 | ILC_MASK, 1, 0);
			m_pButtonsImages->Add(bmp, static_cast<CBitmap*>(0));

			iw = bm.bmHeight;
		}

		delete bmp;
		DeleteObject(hBmp);
	}
}

CFlyBar::~CFlyBar()
{
	Destroy();
}

void CFlyBar::Destroy()
{
	if (m_pButtonsImages) {
		delete m_pButtonsImages;
	}
}

IMPLEMENT_DYNAMIC(CFlyBar, CWnd)

BEGIN_MESSAGE_MAP(CFlyBar, CWnd)
	ON_WM_CREATE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CFlyBar message handlers

BOOL CFlyBar::PreTranslateMessage(MSG* pMsg)
{
	m_tooltip.RelayEvent(pMsg);

	switch (pMsg->message) {
		case WM_LBUTTONDOWN :
		case WM_LBUTTONDBLCLK :
		case WM_MBUTTONDOWN :
		case WM_MBUTTONUP :
		case WM_MBUTTONDBLCLK :
		case WM_RBUTTONDOWN :
		case WM_RBUTTONUP :
		case WM_RBUTTONDBLCLK :

		(CMainFrame*)GetParentFrame()->SetFocus();

		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

LRESULT CFlyBar::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_MOUSELEAVE) {
		CPoint point;
		GetCursorPos(&point);
		UpdateWnd(point);
	}

	return CWnd::WindowProc(message, wParam, lParam);
}

BOOL CFlyBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs)) {
		return FALSE;
	}

	cs.style &= ~WS_BORDER;

	return TRUE;
}

int CFlyBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	m_tooltip.Create(this, TTS_ALWAYSTIP);
	EnableToolTips(true);

	m_tooltip.AddTool(this, _T(""));
	m_tooltip.Activate(TRUE);

	m_tooltip.SetDelayTime(TTDT_AUTOPOP, -1);
	m_tooltip.SetDelayTime(TTDT_INITIAL, 500);
	m_tooltip.SetDelayTime(TTDT_RESHOW, 0);

	return 0;
}

void CFlyBar::CalcButtonsRect()
{
	CRect rcBar;
	GetWindowRect(&rcBar);

	r_ExitIcon		= CRect(rcBar.right-5-(iw),	rcBar.top+5, rcBar.right-5, rcBar.bottom-5);
	r_MinIcon		= CRect(rcBar.right-5-(iw*3), rcBar.top+5, rcBar.right-5-(iw*2), rcBar.bottom-5);
	r_RestoreIcon	= CRect(rcBar.right-5-(iw*2), rcBar.top+5, rcBar.right-5-(iw), rcBar.bottom-5);
	r_SettingsIcon	= CRect(rcBar.right-5-(iw*7), rcBar.top+5, rcBar.right-5-(iw*6), rcBar.bottom-5);
	r_InfoIcon		= CRect(rcBar.right-5-(iw*6), rcBar.top+5, rcBar.right-5-(iw*5), rcBar.bottom-5);
	r_FSIcon		= CRect(rcBar.right-5-(iw*4), rcBar.top+5, rcBar.right-5-(iw*3), rcBar.bottom-5);
	r_LockIcon		= CRect(rcBar.right-5-(iw*9), rcBar.top+5, rcBar.right-5-(iw*8), rcBar.bottom-5);
}

void CFlyBar::DrawButton(CDC *pDC, int x, int y, int z)
{
	HICON hIcon = m_pButtonsImages->ExtractIcon(y);
	DrawIconEx(pDC->m_hDC, x - 5 - (iw * z), 5, hIcon, 0, 0, 0, NULL, DI_NORMAL);
	DestroyIcon(hIcon);
}

void CFlyBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();
	pFrame->SetFocus();

	WINDOWPLACEMENT wp;
	pFrame->GetWindowPlacement(&wp);
	CPoint p;
	GetCursorPos(&p);
	CalcButtonsRect();

	bt_idx = -1;

	if (r_ExitIcon.PtInRect(p)) {
		ShowWindow(SW_HIDE);
		pFrame->OnClose();
	} else if (r_MinIcon.PtInRect(p)) {
		pFrame->m_fTrayIcon ? pFrame->SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, -1) : pFrame->ShowWindow(SW_SHOWMINIMIZED);
	} else if (r_RestoreIcon.PtInRect(p)) {
		if (wp.showCmd != SW_SHOWMAXIMIZED && pFrame->m_fFullScreen) {
			pFrame->ToggleFullscreen(true, true);
			pFrame->ShowWindow(SW_SHOWMAXIMIZED);
		} else if (wp.showCmd == SW_SHOWMAXIMIZED) {
			pFrame->ShowWindow(SW_SHOWNORMAL);
		} else if (wp.showCmd != SW_SHOWMAXIMIZED) {
			pFrame->ShowWindow(SW_SHOWMAXIMIZED);
		}
		Invalidate();
	} else if (r_SettingsIcon.PtInRect(p)) {
		pFrame->OnViewOptions();
		Invalidate();
	} else if (r_InfoIcon.PtInRect(p)) {
		OAFilterState fs = pFrame->GetMediaState();
		if (fs != -1) {
			pFrame->OnFileProperties();
		}
		Invalidate();
	} else if (r_FSIcon.PtInRect(p)) {
		pFrame->ToggleFullscreen(true, true);
		Invalidate();
	} else if (r_LockIcon.PtInRect(p)) {
		AppSettings& s = AfxGetAppSettings();
		s.fFlybarOnTop = !s.fFlybarOnTop;
		UpdateWnd(point);
	}
}

void CFlyBar::OnMouseMove(UINT nFlags, CPoint point)
{
	SetCursor(LoadCursor(NULL, IDC_HAND));
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.hwndTrack = GetSafeHwnd();
	tme.dwFlags = TME_LEAVE;
	TrackMouseEvent(&tme);

	UpdateWnd(point);
	//CWnd::OnMouseMove(nFlags, point);
}

void CFlyBar::UpdateWnd(CPoint point)
{
	ClientToScreen(&point);

	CString str, str2;
	m_tooltip.GetText(str,this);

	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();

	if (r_ExitIcon.PtInRect(point)) {
		if (str != ResStr(IDS_AG_EXIT)) {
			m_tooltip.UpdateTipText(ResStr(IDS_AG_EXIT), this);
		}
		bt_idx = 0;
	} else if (r_MinIcon.PtInRect(point)) {
		if (str != ResStr(IDS_TOOLTIP_MINIMIZE)) {
			m_tooltip.UpdateTipText(ResStr(IDS_TOOLTIP_MINIMIZE), this);
		}
		bt_idx = 1;
	} else if (r_RestoreIcon.PtInRect(point)) {
		WINDOWPLACEMENT wp;
		pFrame->GetWindowPlacement(&wp);
		wp.showCmd == SW_SHOWMAXIMIZED ? str2 = ResStr(IDS_TOOLTIP_RESTORE) : str2 = ResStr(IDS_TOOLTIP_MAXIMIZE);
		if (str != str2) {
			m_tooltip.UpdateTipText(str2, this);
		}
		bt_idx = 2;
	} else if (r_SettingsIcon.PtInRect(point)) {
		if (str != ResStr(IDS_AG_OPTIONS)) {
			m_tooltip.UpdateTipText(ResStr(IDS_AG_OPTIONS), this);
		}
		bt_idx = 3;
	} else if (r_InfoIcon.PtInRect(point)) {
		if (str != ResStr(IDS_AG_PROPERTIES)) {
			m_tooltip.UpdateTipText(ResStr(IDS_AG_PROPERTIES), this);
		}
		bt_idx = 4;
	} else if (r_FSIcon.PtInRect(point)) {
		pFrame->m_fFullScreen ? str2 = ResStr(IDS_TOOLTIP_WINDOW) : str2 = ResStr(IDS_TOOLTIP_FULLSCREEN);
		if (str != str2) {
			m_tooltip.UpdateTipText(str2, this);
		}
		bt_idx = 5;
	} else if (r_LockIcon.PtInRect(point)) {
		AfxGetAppSettings().fFlybarOnTop ? str2 = ResStr(IDS_TOOLTIP_UNLOCK) : str2 = ResStr(IDS_TOOLTIP_LOCK);
		if (str != str2) {
			m_tooltip.UpdateTipText(str2, this);
		}
		bt_idx = 6;
	} else {
		if (str.GetLength() > 0) {
			m_tooltip.UpdateTipText(_T(""), this);
		}
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		bt_idx = -1;
	}

	// set tooltip position
	CRect r_tooltip;
	m_tooltip.GetWindowRect(&r_tooltip);
	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(MonitorFromWindow(this->m_hWnd, MONITOR_DEFAULTTONEAREST), &mi);
	CPoint p;
	p.x = max(0, min(point.x, mi.rcMonitor.right - r_tooltip.Width()));
	int iCursorHeight = 24;
	m_tooltip.SetWindowPos(NULL, p.x, point.y + iCursorHeight, r_tooltip.Width(), iCursorHeight, SWP_NOACTIVATE|SWP_NOZORDER);

	Invalidate();
}

void CFlyBar::OnPaint()
{
	CPaintDC dc(this);
	DrawWnd();
}

void CFlyBar::DrawWnd()
{
	CClientDC dc (this);

	if (IsWindowVisible()) {

		AppSettings& s = AfxGetAppSettings();

		CRect rcBar;
		GetClientRect(&rcBar);
		int x = rcBar.Width();

		CMainFrame* pFrame	= (CMainFrame*)GetParentFrame();
		WINDOWPLACEMENT wp;
		pFrame->GetWindowPlacement(&wp);

		OAFilterState fs	= pFrame->GetMediaState();
		CDC mdc;
		mdc.CreateCompatibleDC(&dc);
		CBitmap bm;
		bm.CreateCompatibleBitmap(&dc, x, rcBar.Height());
		CBitmap* pOldBm = mdc.SelectObject(&bm);
		mdc.SetBkMode(TRANSPARENT);
		mdc.FillSolidRect(rcBar, RGB(0,0,0));

		int sep[][2] = {{0,1},{13,14},{15,16},{12,12},{10,11},{17,18},{5,6},{7,7},{21,22},{4,4},{2,3},{8,9},{19,20}};

		for (int i = 0; i < 2; i++) {

			if (!i || bt_idx == 0) { // exit
				DrawButton(&mdc, x, sep[0][i], 1);
			}

			if (!i || bt_idx == 1) { // min
				DrawButton(&mdc, x, sep[1][i], 3);
			}

			if (!i || bt_idx == 2) { // restore
				if (wp.showCmd == SW_SHOWMAXIMIZED) {
					DrawButton(&mdc, x, sep[2][i], 2);
				} else {
					DrawButton(&mdc, x, sep[4][i], 2);
				}
			}

			if (!i || bt_idx == 3) { // settings
				DrawButton(&mdc, x, sep[5][i], 7);
			}

			if (!i || bt_idx == 4) { // info
				if (fs != -1) {
					DrawButton(&mdc, x, sep[6][i], 6);
				} else {
					DrawButton(&mdc, x, sep[7][i], 6);
				}
			}

			if (!i || bt_idx == 5) { // fs
				if (pFrame->m_fFullScreen) {
					DrawButton(&mdc, x, sep[8][i], 4);
				} else {
					DrawButton(&mdc, x, sep[10][i], 4);
				}
			}

			if (!i || bt_idx == 6) { // lock
				if (s.fFlybarOnTop) {
					DrawButton(&mdc, x, sep[11][i], 9);
				} else {
					DrawButton(&mdc, x, sep[12][i], 9);
				}
			}
		}

		dc.BitBlt(0, 0, x, rcBar.Height(), &mdc, 0, 0, SRCCOPY);

		mdc.SelectObject(pOldBm);
		bm.DeleteObject();
		mdc.DeleteDC();
	}

	bt_idx = -1;
}

BOOL CFlyBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
