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

#include "stdafx.h"
#include "MainFrm.h"
#include "PlayerStatusBar.h"
#include "OpenImage.h"

// CPlayerStatusBar

IMPLEMENT_DYNAMIC(CPlayerStatusBar, CDialogBar)

CPlayerStatusBar::CPlayerStatusBar()
	: m_status(false, false)
	, m_time(true, false)
	, m_bmid(0)
	, m_time_rect(-1, -1, -1, -1)
	, m_time_rect2(-1, -1, -1, -1)
{
}

CPlayerStatusBar::~CPlayerStatusBar()
{
}

BOOL CPlayerStatusBar::Create(CWnd* pParentWnd)
{
	return CDialogBar::Create(pParentWnd, IDD_PLAYERSTATUSBAR, WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM, IDD_PLAYERSTATUSBAR);
}

BOOL CPlayerStatusBar::PreCreateWindow(CREATESTRUCT& cs)
{
	VERIFY(CDialogBar::PreCreateWindow(cs));

	m_dwStyle &= ~CBRS_BORDER_TOP;
	m_dwStyle &= ~CBRS_BORDER_BOTTOM;

	return TRUE;
}

int CPlayerStatusBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogBar::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	if (m_BackGroundbm.FileExists(CString(L"background"))) {
		m_BackGroundbm.LoadExternalGradient(L"background");
	}

	CRect r;
	r.SetRectEmpty();

	m_type.Create(_T(""), WS_CHILD|WS_VISIBLE|SS_ICON, r, this, IDC_STATIC1);
	m_status.Create(_T(""), WS_CHILD|WS_VISIBLE|SS_OWNERDRAW, r, this, IDC_PLAYERSTATUS);
	m_status.SetWindowPos(&m_time, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	m_time.Create(_T(""), WS_CHILD|WS_VISIBLE|SS_OWNERDRAW, r, this, IDC_PLAYERTIME);

	Relayout();

	return 0;
}

void CPlayerStatusBar::Relayout()
{
	AppSettings& s = AfxGetAppSettings();

	if (!s.fDisableXPToolbars) {
		m_type.ShowWindow(/*SW_SHOW*/SW_HIDE);
		m_status.ShowWindow(SW_SHOW);
		m_time.ShowWindow(SW_SHOW);

		BITMAP bm;
		memset(&bm, 0, sizeof(bm));
		if (m_bm.m_hObject) {
			m_bm.GetBitmap(&bm);
		}

		CRect r, r2;
		GetClientRect(r);

		r.DeflateRect(/*2*/7, 5, bm.bmWidth + 8, 4);
		int div = r.right - (m_time.IsWindowVisible() ? 140 : 0);

		CString str;
		m_time.GetWindowText(str);
		if (CDC* pDC = m_time.GetDC()) {
			CFont* pOld = pDC->SelectObject(&m_time.GetFont());
			div = r.right - pDC->GetTextExtent(str).cx;
			pDC->SelectObject(pOld);
			m_time.ReleaseDC(pDC);
		}

		r2 = r;
		r2.right = div - 2;
		m_status.MoveWindow(&r2);

		r2 = r;
		r2.left = div;
		m_time.MoveWindow(&r2);
		m_time_rect = r2;

		/*
		GetClientRect(r);
		r.SetRect(6, r.top+4, 22, r.bottom-4);
		m_type.MoveWindow(r);
		*/
	} else {
		m_type.ShowWindow(SW_HIDE);
		m_status.ShowWindow(SW_HIDE);
		m_time.ShowWindow(SW_HIDE);
		s.strTimeOnSeekBar = GetStatusTimer();
	}
}

void CPlayerStatusBar::Clear()
{
	m_status.SetWindowText(_T(""));
	m_time.SetWindowText(_T(""));

	SetStatusBitmap(0);

	Relayout();

	Invalidate();
}

void CPlayerStatusBar::SetStatusBitmap(UINT id)
{
	if (m_bmid == id) {
		return;
	}

	if (m_bm.m_hObject) {
		m_bm.DeleteObject();
	}

	if (id) {
		CImage img;
		img.LoadFromResource(AfxGetInstanceHandle(), id);
		m_bm.Attach(img.Detach());
	}

	m_bmid = id;

	Relayout();

	Invalidate();
}

void CPlayerStatusBar::SetStatusMessage(CString str)
{
	str.Trim();
	str.Replace(L"&", L"&&");
	m_status.SetWindowText(str);

	Relayout();

	Invalidate();
}

CString CPlayerStatusBar::GetStatusTimer()
{
	CString strResult;
	m_time.GetWindowText(strResult);

	return strResult;
}

CString CPlayerStatusBar::GetStatusMessage()
{
	CString strResult;
	m_status.GetWindowText(strResult);

	return strResult;
}

void CPlayerStatusBar::SetStatusTimer(CString str)
{
	CString tmp;
	m_time.GetWindowText(tmp);

	if (tmp == str) {
		return;
	}

	if (OpenImageCheck(((CMainFrame*)AfxGetMyApp()->GetMainWnd())->m_strFn)) {
		m_time.SetWindowText(_T(""));
	} else {
		str.Trim();
		m_time.SetWindowText(str);
	}

	Relayout();

	Invalidate();
}

void CPlayerStatusBar::SetStatusTimer(REFERENCE_TIME rtNow, REFERENCE_TIME rtDur, bool fHighPrecision, const GUID* pTimeFormat)
{
	//ASSERT(pTimeFormat);
	//ASSERT(rtNow <= rtDur);

	CString str;
	CString posstr, durstr, rstr;

	if (*pTimeFormat == TIME_FORMAT_MEDIA_TIME) {
		DVD_HMSF_TIMECODE tcNow, tcDur, tcRt;

		if (fHighPrecision) {
			tcNow = RT2HMSF(rtNow);
			tcDur = RT2HMSF(rtDur);
			tcRt  = RT2HMSF(rtDur-rtNow);
		} else {
			tcNow = RT2HMS_r(rtNow);
			tcDur = RT2HMS_r(rtDur);
			tcRt  = RT2HMS_r(rtDur-rtNow);
		}

#if 0
		if (tcDur.bHours > 0 || (rtNow >= rtDur && tcNow.bHours > 0)) {
			posstr.Format(_T("%02d:%02d:%02d"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
			rstr.Format(_T("%02d:%02d:%02d"), tcRt.bHours, tcRt.bMinutes, tcRt.bSeconds);
		} else {
			posstr.Format(_T("%02d:%02d"), tcNow.bMinutes, tcNow.bSeconds);
			rstr.Format(_T("%02d:%02d"), tcRt.bMinutes, tcRt.bSeconds);
		}

		if (tcDur.bHours > 0) {
			durstr.Format(_T("%02d:%02d:%02d"), tcDur.bHours, tcDur.bMinutes, tcDur.bSeconds);
		} else {
			durstr.Format(_T("%02d:%02d"), tcDur.bMinutes, tcDur.bSeconds);
		}
#else
		posstr.Format(_T("%02d:%02d:%02d"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
		rstr.Format(_T("%02d:%02d:%02d"), tcRt.bHours, tcRt.bMinutes, tcRt.bSeconds);
		durstr.Format(_T("%02d:%02d:%02d"), tcDur.bHours, tcDur.bMinutes, tcDur.bSeconds);
#endif

		if (fHighPrecision) {
			posstr.AppendFormat(_T(".%03I64d"), (rtNow/10000)%1000);
			durstr.AppendFormat(_T(".%03I64d"), (rtDur/10000)%1000);
			rstr.AppendFormat(_T(".%03I64d"), ((rtDur - rtNow)/10000)%1000);
		}
	} else if (*pTimeFormat == TIME_FORMAT_FRAME) {
		posstr.Format(_T("%I64d"), rtNow);
		durstr.Format(_T("%I64d"), rtDur);
		rstr.Format(_T("%I64d"), rtDur - rtNow);
	}

	if (!AfxGetAppSettings().fRemainingTime) {
		str = ((rtDur <= 0) || (rtDur < rtNow)) ? posstr : posstr + _T(" / ") + durstr;
	} else {
		str = ((rtDur <= 0) || (rtDur < rtNow)) ? posstr : _T("- ") + rstr + _T(" / ") + durstr;
	}

	SetStatusTimer(str);
}

void CPlayerStatusBar::ShowTimer(bool fShow)
{
	m_time.ShowWindow(fShow ? SW_SHOW : SW_HIDE);

	Relayout();

	Invalidate();
}

BEGIN_MESSAGE_MAP(CPlayerStatusBar, CDialogBar)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CPlayerStatusBar message handlers

BOOL CPlayerStatusBar::OnEraseBkgnd(CDC* pDC)
{
	AppSettings& s = AfxGetAppSettings();
	CRect r;

	if (!s.fDisableXPToolbars) {
		for (CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetNextWindow()) {
			if (!pChild->IsWindowVisible()) {
				continue;
			}

			pChild->GetClientRect(&r);
			pChild->MapWindowPoints(this, &r);
			pDC->ExcludeClipRect(&r);
		}

		GetClientRect(&r);

		CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

		if (pFrame->m_pLastBar != this || pFrame->m_fFullScreen) {
			r.InflateRect(0, 0, 0, 1);
		}

		if (pFrame->m_fFullScreen) {
			r.InflateRect(1, 0, 1, 0);
		}

		pDC->Draw3dRect(&r, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));
		r.DeflateRect(1, 1);
		pDC->FillSolidRect(&r, 0);
	}

	return TRUE;
}

void CPlayerStatusBar::OnPaint()
{
	CPaintDC dc(this);
	int R, G, B, R2, G2, B2;

	if (GetSafeHwnd()) {
		Relayout();
	}

	AppSettings& s = AfxGetAppSettings();
	CRect r;

	if (!s.fDisableXPToolbars) {
		if (m_bm.m_hObject) {
			BITMAP bm;
			m_bm.GetBitmap(&bm);
			CDC memdc;
			memdc.CreateCompatibleDC(&dc);
			memdc.SelectObject(&m_bm);
			GetClientRect(&r);
			dc.BitBlt(r.right-bm.bmWidth-1, (r.Height() - bm.bmHeight)/2, bm.bmWidth, bm.bmHeight, &memdc, 0, 0, SRCCOPY);
		}
	} else {
 		CDC memdc;
 		GetClientRect(&r);
		CBitmap m_bmPaint;
		memdc.CreateCompatibleDC(&dc);
		m_bmPaint.CreateCompatibleBitmap(&dc, r.Width(), r.Height());
		memdc.SelectObject(&m_bmPaint);

		//background

		GRADIENT_RECT gr[1] = {{0, 1}};

		if (m_BackGroundbm.IsExtGradiendLoading()) {
			ThemeRGB(s.nThemeRed, s.nThemeGreen, s.nThemeBlue, R, G, B);
			m_BackGroundbm.PaintExternalGradient(&dc, r, 55, s.nThemeBrightness, R, G, B);
		} else {
			ThemeRGB(30, 35, 40, R, G, B);
			ThemeRGB(0, 5, 10, R2, G2, B2);
			TRIVERTEX tv[2] = {
				{r.left, r.top, R*256, G*256, B*256, 255*256},
				{r.right, r.bottom, R2*256, G2*256, B2*256, 255*256},
			};
			memdc.GradientFill(tv, 2, gr, 1, GRADIENT_FILL_RECT_V);
		}

		memdc.SetBkMode(TRANSPARENT);

		CPen penPlayed1(PS_SOLID,0,RGB(0,0,0));
		memdc.SelectObject(&penPlayed1);
		memdc.MoveTo(r.left, r.top +1);
		memdc.LineTo(r.right, r.top +1);
		memdc.MoveTo(r.left, r.top +2);
		memdc.LineTo(r.right, r.top +2);

		ThemeRGB(50, 55, 60, R, G, B);
		CPen penPlayed2(PS_SOLID,0,RGB(R,G,B));
		memdc.SelectObject(&penPlayed2);
		memdc.MoveTo(r.left, r.top +3);
		memdc.LineTo(r.right, r.top +3);

		CFont font2;
		ThemeRGB(165, 170, 175, R, G, B);
		memdc.SetTextColor(RGB(R,G,B));

		font2.CreateFont(int(13 * s.scalefont), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
 					  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma"));

		memdc.SelectObject(&font2);
		CString str;
		str = GetStatusTimer();
		s.strTimeOnSeekBar = str;

		CRect rt = r;
		m_time_rect2		= rt;
		m_time_rect2.right	= rt.right;
		m_time_rect2.top	= rt.top;
		m_time_rect2.left	= rt.right - 120;
		m_time_rect2.bottom	= rt.bottom;

		rt.right	= r.right - 6;
		rt.top		= r.top + 4;
		memdc.DrawText(str, str.GetLength(), &rt, DT_RIGHT|DT_VCENTER|DT_SINGLELINE);

		CString str2;
		str2 = GetStatusMessage();

		CRect rs	= r;
		rs.left		= r.left + 7;
		rs.top		= r.top + 3;

		memdc.DrawText(str2, str2.GetLength(), &rs, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

		dc.BitBlt(r.left, r.top, r.Width(), r.Height(), &memdc, 0, 0, SRCCOPY);
	}
}

void CPlayerStatusBar::OnSize(UINT nType, int cx, int cy)
{
	CDialogBar::OnSize(nType, cx, cy);

	Relayout();

	Invalidate();
}

void CPlayerStatusBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	AppSettings& s = AfxGetAppSettings();

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	pFrame->GetWindowPlacement(&wp);

	if (m_time_rect.PtInRect(point) || m_time_rect2.PtInRect(point)) {
		s.fRemainingTime = !s.fRemainingTime;
		pFrame->OnTimer(2);
		return;
	}

	if (!pFrame->m_fFullScreen && wp.showCmd != SW_SHOWMAXIMIZED) {
		CRect r;
		GetClientRect(r);
		CPoint p = point;

		MapWindowPoints(pFrame, &point, 1);

		pFrame->PostMessage(WM_NCLBUTTONDOWN,
							(p.x >= r.Width()-r.Height() && !pFrame->IsCaptionHidden()) ? HTBOTTOMRIGHT :
							HTCAPTION,
							MAKELPARAM(point.x, point.y));
	}
}

BOOL CPlayerStatusBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	pFrame->GetWindowPlacement(&wp);

	CPoint p;
	GetCursorPos(&p);
	ScreenToClient(&p);

	if (m_time_rect.PtInRect(p) || m_time_rect2.PtInRect(p)) {
		SetCursor(LoadCursor(NULL, IDC_HAND));
		return TRUE;
	}

	if (!pFrame->m_fFullScreen && wp.showCmd != SW_SHOWMAXIMIZED) {
		CRect r;
		GetClientRect(r);
		if (p.x >= r.Width()-r.Height() && !pFrame->IsCaptionHidden()) {
			SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			return TRUE;
		}
	}

	return CDialogBar::OnSetCursor(pWnd, nHitTest, message);
}

HBRUSH CPlayerStatusBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogBar::OnCtlColor(pDC, pWnd, nCtlColor);

	if (!AfxGetAppSettings().fDisableXPToolbars) {
		if (*pWnd == m_type) {
			hbr = GetStockBrush(BLACK_BRUSH);
		}
	}

	return hbr;
}
