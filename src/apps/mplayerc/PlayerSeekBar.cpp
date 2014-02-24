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
#include "PlayerSeekBar.h"

// CPlayerSeekBar

IMPLEMENT_DYNAMIC(CPlayerSeekBar, CDialogBar)

CPlayerSeekBar::CPlayerSeekBar() :
	m_start(0), m_stop(100), m_pos(0), m_posreal(0),
	m_fEnabled(false),
	m_tooltipState(TOOLTIP_HIDDEN), m_tooltipLastPos(-1), m_tooltipTimer(1),
	r_Lock(0,0,0,0)
{
}

CPlayerSeekBar::~CPlayerSeekBar()
{
}

BOOL CPlayerSeekBar::Create(CWnd* pParentWnd)
{
	VERIFY(CDialogBar::Create(pParentWnd, IDD_PLAYERSEEKBAR, WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM, IDD_PLAYERSEEKBAR));

	ModifyStyleEx(WS_EX_LAYOUTRTL, WS_EX_NOINHERITLAYOUT);

	m_tooltip.Create(this, TTS_NOPREFIX | TTS_ALWAYSTIP);

	m_tooltip.SetMaxTipWidth(SHRT_MAX);

	m_tooltip.SetDelayTime(TTDT_AUTOPOP, SHRT_MAX);
	m_tooltip.SetDelayTime(TTDT_INITIAL, 0);
	m_tooltip.SetDelayTime(TTDT_RESHOW, 0);

	memset(&m_ti, 0, sizeof(TOOLINFO));
	m_ti.cbSize		= sizeof(TOOLINFO);
	m_ti.uFlags		= TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	m_ti.hwnd		= m_hWnd;
	m_ti.hinst		= AfxGetInstanceHandle();
	m_ti.lpszText	= NULL;
	m_ti.uId		= (UINT)m_hWnd;

	m_tooltip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&m_ti);

	if (m_BackGroundbm.FileExists(CString(L"background"))) {
		m_BackGroundbm.LoadExternalGradient(L"background");
	}

	return TRUE;
}

BOOL CPlayerSeekBar::PreCreateWindow(CREATESTRUCT& cs)
{
	VERIFY(CDialogBar::PreCreateWindow(cs));

	m_dwStyle &= ~CBRS_BORDER_TOP;
	m_dwStyle &= ~CBRS_BORDER_BOTTOM;
	m_dwStyle |= CBRS_SIZE_FIXED;

	return TRUE;
}

void CPlayerSeekBar::Enable(bool fEnable)
{
	m_fEnabled = fEnable;

	Invalidate();
}

void CPlayerSeekBar::GetRange(__int64& start, __int64& stop)
{
	start = m_start;
	stop = m_stop;
}

void CPlayerSeekBar::SetRange(__int64 start, __int64 stop)
{
	if (start > stop) {
		start ^= stop, stop ^= start, start ^= stop;
	}

	m_start = start;
	m_stop = stop;

	if (m_pos < m_start || m_pos >= m_stop) {
		SetPos(m_start);
	}
}

__int64 CPlayerSeekBar::GetPos()
{
	return(m_pos);
}

__int64 CPlayerSeekBar::GetPosReal()
{
	return(m_posreal);
}

void CPlayerSeekBar::SetPos(__int64 pos)
{
	CWnd* w = GetCapture();

	if (w && w->m_hWnd == m_hWnd) {
		return;
	}

	SetPosInternal(pos);
}

void CPlayerSeekBar::SetPosInternal(__int64 pos)
{
	AppSettings& s = AfxGetAppSettings();

	if (s.fDisableXPToolbars) {
		if (m_pos == pos || m_stop <= pos) return;
	} else {
		if (m_pos == pos) {
			return;
		}
	}

	CRect before = GetThumbRect();
	m_pos = min(max(pos, m_start), m_stop);
	m_posreal = pos;
	CRect after = GetThumbRect();

	if (before != after) {
		if (!s.fDisableXPToolbars) {
			InvalidateRect (before | after);
		}
	}
}

void CPlayerSeekBar::SetPosInternal2(__int64 pos2)
{
	AppSettings& s = AfxGetAppSettings();

	if (s.fDisableXPToolbars) {
		if (m_pos2 == pos2 || m_stop <= pos2) return;
	} else {
		if (m_pos2 == pos2) {
			return;
		}
	}

	CRect before = GetThumbRect();
	m_pos2 = min(max(pos2, m_start), m_stop);
	m_posreal2 = pos2;
	CRect after = GetThumbRect();

	if (before != after) {
		if (!s.fDisableXPToolbars) {
			InvalidateRect (before | after);
		}
	}
}

CRect CPlayerSeekBar::GetChannelRect()
{
	CRect r;
	GetClientRect(&r);

	if (AfxGetAppSettings().fDisableXPToolbars) {
		//r.DeflateRect(1,1,1,1);
	} else {
		r.DeflateRect(8, 9, 9, 0);
		r.bottom = r.top + 5;
	}

	return(r);
}

CRect CPlayerSeekBar::GetThumbRect()
{
	CRect r = GetChannelRect();

	int x = r.left + (int)((m_start < m_stop) ? (__int64)r.Width() * (m_pos - m_start) / (m_stop - m_start) : 0);
	int y = r.CenterPoint().y;

	if (AfxGetAppSettings().fDisableXPToolbars) {
		 r.SetRect(x, y-2, x+3, y+3);
	} else {
		r.SetRect(x, y, x, y);
		r.InflateRect(6, 7, 7, 8);
	}

	return(r);
}

CRect CPlayerSeekBar::GetInnerThumbRect()
{
	CRect r = GetThumbRect();

	bool fEnabled = m_fEnabled && m_start < m_stop;
	r.DeflateRect(3, fEnabled ? 5 : 4, 3, fEnabled ? 5 : 4);

	return(r);
}

__int64 CPlayerSeekBar::CalculatePosition(CPoint point)
{
	__int64 pos = -1;
	CRect r = GetChannelRect();

	if (r.left >= r.right) {
		pos = -1;
	} else if (point.x < r.left) {
		pos = m_start;
	} else if (point.x >= r.right) {
		pos = m_stop;
	} else {
		__int64 w = r.right - r.left;

		if (m_start < m_stop) {
			pos = m_start + ((m_stop - m_start) * (point.x - r.left) + (w/2)) / w;
		}
	}

	return pos;
}

__int64 CPlayerSeekBar::CalculatePosition2(CPoint point)
{
	__int64 pos2 = -1;
	CRect r = GetChannelRect();

	if (r.left >= r.right) {
		pos2 = -1;
	} else if (point.x < r.left) {
		pos2 = m_start;
	} else if (point.x >= r.right) {
		pos2 = m_stop;
	} else {
		__int64 w = r.right - r.left;

		if (m_start < m_stop) {
			pos2 = m_start + ((m_stop - m_start) * (point.x - r.left) + (w/2)) / w;
		}
	}

	return pos2;
}


void CPlayerSeekBar::MoveThumb(CPoint point)
{
	__int64 pos = CalculatePosition(point);

	if (pos >= 0) {
		SetPosInternal(pos);
	}
}

void CPlayerSeekBar::MoveThumb2(CPoint point)
{
	__int64 pos2 = CalculatePosition2(point);

	if (pos2 >= 0) {
		SetPosInternal2(pos2);
	}
}

BEGIN_MESSAGE_MAP(CPlayerSeekBar, CDialogBar)
	//{{AFX_MSG_MAP(CPlayerSeekBar)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_COMMAND_EX(ID_PLAY_STOP, OnPlayStop)
END_MESSAGE_MAP()

// CPlayerSeekBar message handlers

void CPlayerSeekBar::OnPaint()
{
	CPaintDC dc(this);

	AppSettings& s = AfxGetAppSettings();

	int R, G, B, R2, G2, B2;

	bool fEnabled = m_fEnabled && m_start < m_stop;

	if (s.fDisableXPToolbars) {
		CRect rt;
		CString str = ((CMainFrame*)AfxGetMyApp()->GetMainWnd())->GetStrForTitle();
		CDC memdc;
		CBitmap m_bmPaint;
		CRect r,rf,rc;
		GetClientRect(&r);
		memdc.CreateCompatibleDC(&dc);
		m_bmPaint.CreateCompatibleBitmap(&dc, r.Width(), r.Height());
		CBitmap *bmOld = memdc.SelectObject(&m_bmPaint);

		GRADIENT_RECT gr[1] = {{0, 1}};
		int pa = 255 * 256;

		if (m_BackGroundbm.IsExtGradiendLoading()) {
			ThemeRGB(s.nThemeRed, s.nThemeGreen, s.nThemeBlue, R, G, B);
			m_BackGroundbm.PaintExternalGradient(&memdc, r, 0, s.nThemeBrightness, R, G, B);
		} else {
			ThemeRGB(0, 5, 10, R, G, B);
			ThemeRGB(15, 20, 25, R2, G2, B2);
			TRIVERTEX tv[2] = {
				{r.left, r.top, R*256, G*256, B*256, pa},
				{r.right, r.bottom, R2*256, G2*256, B2*256, pa},
			};
			memdc.GradientFill(tv, 2, gr, 1, GRADIENT_FILL_RECT_V);
		}

		memdc.SetBkMode(TRANSPARENT);

		CPen penPlayed(s.clrFaceABGR == 0x00ff00ff ? PS_NULL : PS_SOLID, 0, s.clrFaceABGR);
		CPen penPlayedOutline(s.clrOutlineABGR == 0x00ff00ff ? PS_NULL : PS_SOLID, 0, s.clrOutlineABGR);

		rc = GetChannelRect();
		int nposx = GetThumbRect().right-2;
		int nposy = r.top;

		ThemeRGB(30, 35, 40, R, G, B);
		CPen penPlayed1(PS_SOLID,0,RGB(R,G,B));
		memdc.SelectObject(&penPlayed1);
		memdc.MoveTo(rc.left, rc.top);
		memdc.LineTo(rc.right, rc.top);

		ThemeRGB(80, 85, 90, R, G, B);
		CPen penPlayed2(PS_SOLID,0,RGB(R,G,B));
		memdc.SelectObject(&penPlayed2);
		memdc.MoveTo(rc.left -1, rc.bottom-1);
		memdc.LineTo(rc.right+2, rc.bottom-1);

		// buffer
		r_Lock.SetRect(-1,-1,-1,-1);
		int Progress;
		if (((CMainFrame*)AfxGetMyApp()->GetMainWnd())->GetBufferingProgress(&Progress)) {
			r_Lock = r;
			int r_right = ((__int64)r.Width()/100) * Progress;
			ThemeRGB(45, 55, 60, R, G, B);
				ThemeRGB(65, 70, 75, R2, G2, B2);
				TRIVERTEX tvb[2] = {
					{r.left, r.top, R*256, G*256, B*256, pa},
					{r_right, r.bottom-3, R2*256, G2*256, B2*256, pa},
				};
				memdc.GradientFill(tvb, 2, gr, 1, GRADIENT_FILL_RECT_V);
				r_Lock.left = r_right;
		}

		if (fEnabled) {
			if (m_BackGroundbm.IsExtGradiendLoading()) {
				rc.right = nposx;
				rc.left = rc.left + 1;
				rc.top = rc.top + 1;
				rc.bottom = rc.bottom - 2;

				ThemeRGB(s.nThemeRed, s.nThemeGreen, s.nThemeBlue, R, G, B);
				m_BackGroundbm.PaintExternalGradient(&memdc, r, 0, s.nThemeBrightness, R, G, B);

				rc = GetChannelRect();
			} else {
				ThemeRGB(0, 5, 10, R, G, B);
				ThemeRGB(105, 110, 115, R2, G2, B2);
				TRIVERTEX tv[2] = {
					{rc.left, rc.top, R*256, G*256, B*256, pa},
					{nposx, rc.bottom-3, R2*256, G2*256, B2*256, pa},
				};
				memdc.GradientFill(tv, 2, gr, 1, GRADIENT_FILL_RECT_V);

				CRect rc2;
				rc2.left = nposx - 5;
				rc2.right = nposx;
				rc2.top = rc.top;
				rc2.bottom = rc.bottom;

				ThemeRGB(0, 5, 10, R, G, B);
				ThemeRGB(205, 210, 215, R2, G2, B2);
				TRIVERTEX tv2[2] = {
					{rc2.left, rc2.top, R*256, G*256, B*256, pa},
					{rc2.right, rc.bottom-3, R2*256, G2*256, B2*256, pa},
				};
				memdc.GradientFill(tv2, 2, gr, 1, GRADIENT_FILL_RECT_V);
			}

			ThemeRGB(80, 85, 90, R, G, B);
			CPen penPlayed3(PS_SOLID,0,RGB(R,G,B));
			memdc.SelectObject(&penPlayed3);
			memdc.MoveTo(rc.left, rc.top);//active_top
			memdc.LineTo(nposx, rc.top);

			// ������ ������� ����
			if (s.fChapterMarker) {
				CAutoLock lock(&m_CBLock);

				CMainFrame* pFrame = (CMainFrame*)GetParentFrame();
				if (m_pChapterBag && m_pChapterBag->ChapGetCount()) {
					CRect rc2 = rc;
					for (DWORD idx = 0; idx < m_pChapterBag->ChapGetCount(); idx++) {
						CRect r = GetChannelRect();
						REFERENCE_TIME rt;

						if (FAILED(m_pChapterBag->ChapGet(idx, &rt, NULL))) {
							continue;
						}

						if (rt <= 0 || (rt >= m_stop)) {
							continue;
						}

						int x = r.left + (int)((m_start < m_stop) ? (__int64)r.Width() * (rt - m_start) / (m_stop - m_start) : 0);

						// ����� ������ ��������� ������ ������ ��� ������ ���������
						// HICON appIcon = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MARKERS), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
						// ::DrawIconEx(memdc, x, rc2.top + 10, appIcon, 0,0, 0, NULL, DI_NORMAL);
						// ::DestroyIcon(appIcon);

						ThemeRGB(255, 255, 255, R, G, B);
						CPen penPlayed2(PS_SOLID, 0, RGB(R,G,B));
						memdc.SelectObject(&penPlayed2);

						memdc.MoveTo(x, rc2.top + 14);
						memdc.LineTo(x, rc2.bottom - 2);
						memdc.MoveTo(x - 1, rc2.bottom - 2);
						memdc.LineTo(x + 2, rc2.bottom - 2);
 					}
				}
			}
		}

		if (s.fFileNameOnSeekBar || !s.bStatusBarIsVisible || !strChap.IsEmpty()) {
			CFont font2;
			ThemeRGB(135, 140, 145, R, G, B);
			memdc.SetTextColor(RGB(R,G,B));

			font2.CreateFont(int(13 * s.scalefont), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
 					  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma"));

			CFont* oldfont2 = memdc.SelectObject(&font2);
			SetBkMode(memdc, TRANSPARENT);

			LONG xt = s.bStatusBarIsVisible ? 0 : s.strTimeOnSeekBar.GetLength() <= 21 ? 150 : 160;

			if (s.fFileNameOnSeekBar || !strChap.IsEmpty()) {
				if (!strChap.IsEmpty() && fEnabled) {
					str = strChap;
				}

				// draw filename || chapter name.
				rt = rc;
				rt.left  += 6;
				rt.top   -= 2;
				rt.right -= xt;
				memdc.DrawText(str, str.GetLength(), &rt, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);

				// highlights string
				ThemeRGB(205, 210, 215, R, G, B);
				memdc.SetTextColor(RGB(R,G,B));
				if (nposx > rt.right - 15) {
					memdc.DrawText(str, str.GetLength(), &rt, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
				} else {
					rt.right = nposx;
					memdc.DrawText(str, str.GetLength(), &rt, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
				}
			}

			if (!s.bStatusBarIsVisible) {
				CString strT = s.strTimeOnSeekBar;
				rt = rc;
				rt.left  -= xt - 10;
				rt.top   -= 2;
				rt.right -= 6;
				ThemeRGB(200, 205, 210, R, G, B);
				memdc.SetTextColor(RGB(R,G,B));
				memdc.DrawText(strT, strT.GetLength(), &rt, DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
			}
		}

		dc.BitBlt(r.left, r.top, r.Width(), r.Height(), &memdc, 0, 0, SRCCOPY);
		DeleteObject(memdc.SelectObject(bmOld));
		memdc.DeleteDC();
		m_bmPaint.DeleteObject();
	} else {
		COLORREF
		white  = GetSysColor(COLOR_WINDOW),
		shadow = GetSysColor(COLOR_3DSHADOW),
		light  = GetSysColor(COLOR_3DHILIGHT),
		bkg    = GetSysColor(COLOR_BTNFACE);

		// thumb
		{
			CRect r = GetThumbRect(), r2 = GetInnerThumbRect();
			CRect rt = r, rit = r2;

			dc.Draw3dRect(&r, light, 0);
			r.DeflateRect(0, 0, 1, 1);
			dc.Draw3dRect(&r, light, shadow);
			r.DeflateRect(1, 1, 1, 1);

			CBrush b(bkg);

			dc.FrameRect(&r, &b);
			r.DeflateRect(0, 1, 0, 1);
			dc.FrameRect(&r, &b);

			r.DeflateRect(1, 1, 0, 0);
			dc.Draw3dRect(&r, shadow, bkg);

			if (fEnabled) {
				r.DeflateRect(1, 1, 1, 2);
				CPen white(PS_INSIDEFRAME, 1, white);
				CPen* old = dc.SelectObject(&white);
				dc.MoveTo(r.left, r.top);
				dc.LineTo(r.right, r.top);
				dc.MoveTo(r.left, r.bottom);
				dc.LineTo(r.right, r.bottom);
				dc.SelectObject(old);
				dc.SetPixel(r.CenterPoint().x, r.top, 0);
				dc.SetPixel(r.CenterPoint().x, r.bottom, 0);
			}

			dc.SetPixel(r.CenterPoint().x+5, r.top-4, bkg);

			{
				CRgn rgn1, rgn2;
				rgn1.CreateRectRgnIndirect(&rt);
				rgn2.CreateRectRgnIndirect(&rit);
				ExtSelectClipRgn(dc, rgn1, RGN_DIFF);
				ExtSelectClipRgn(dc, rgn2, RGN_OR);
			}
		}

		// channel
		{
			CRect r = GetChannelRect();

			dc.FillSolidRect(&r, fEnabled ? white : bkg);
			r.InflateRect(1, 1);
			dc.Draw3dRect(&r, shadow, light);
			dc.ExcludeClipRect(&r);
		}

		// background
		{
			CRect r;
			GetClientRect(&r);
			CBrush b(bkg);
			dc.FillRect(&r, &b);
		}
	}
}

void CPlayerSeekBar::OnSize(UINT nType, int cx, int cy)
{
	HideToolTip();

	CDialogBar::OnSize(nType, cx, cy);

	Invalidate();
}

void CPlayerSeekBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	__int64 pos = CalculatePosition(point);
	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();

	if (pFrame->ValidateSeek(pos, m_stop)) {

		if (AfxGetAppSettings().fDisableXPToolbars && m_fEnabled) {
			SetCapture();
			MoveThumb(point);
		} else {
			if (m_fEnabled && (GetChannelRect() | GetThumbRect()).PtInRect(point)) {
				SetCapture();
				MoveThumb(point);
				GetParent()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_pos, SB_THUMBPOSITION), (LPARAM)m_hWnd);
			} else {
				if (!pFrame->m_fFullScreen) {
					MapWindowPoints(pFrame, &point, 1);
					pFrame->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
				}
			}
		}
	}

	CDialogBar::OnLButtonDown(nFlags, point);
}

void CPlayerSeekBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();

	__int64 pos = CalculatePosition(point);

	if (((CMainFrame*)GetParentFrame())->ValidateSeek(pos, m_stop)) {

		if (AfxGetAppSettings().fDisableXPToolbars && m_fEnabled) {
			GetParent()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_pos, SB_THUMBPOSITION), (LPARAM)m_hWnd);
		}
	}

	CDialogBar::OnLButtonUp(nFlags, point);
}

void CPlayerSeekBar::OnRButtonDown(UINT nFlags, CPoint point)
{
	AppSettings& s = AfxGetAppSettings();

	if (!s.bStatusBarIsVisible) {
		CRect rc = GetChannelRect();
		CRect rT = rc;
		rT.left  = rc.right - 140;
		rT.right = rc.right - 6;
		CPoint p;
		GetCursorPos(&p);
		ScreenToClient(&p);

		if (rT.PtInRect(p)) {
			s.fRemainingTime = !s.fRemainingTime;
		}
	}

	if (CMainFrame* pFrame = (CMainFrame*)GetParentFrame()) {
		pFrame->PostMessage(WM_COMMAND, ID_PLAY_GOTO);
	}

	CDialogBar::OnRButtonDown(nFlags, point);
}


void CPlayerSeekBar::UpdateTooltip(CPoint point)
{
	m_tooltipPos = CalculatePosition(point);
	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();
	if (m_fEnabled && m_start < m_stop && (GetChannelRect() | GetThumbRect()).PtInRect(point)) {
		if (m_tooltipState == TOOLTIP_HIDDEN && m_tooltipPos != m_tooltipLastPos) {

			TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
			tme.hwndTrack = m_hWnd;
			tme.dwFlags = TME_LEAVE;
			TrackMouseEvent(&tme);

			m_tooltipState = TOOLTIP_TRIGGERED;
			m_tooltipTimer = SetTimer(m_tooltipTimer, pFrame->m_wndPreView.IsWindowVisible() ? 10 : SHOW_DELAY, NULL);
		}
	} else {
		HideToolTip();
	}

	if (m_tooltipState == TOOLTIP_VISIBLE && m_tooltipPos != m_tooltipLastPos) {
		UpdateToolTipText();

		if (!pFrame->CanPreviewUse()) {
			UpdateToolTipPosition(point);
		}
		m_tooltipTimer = SetTimer(m_tooltipTimer, pFrame->CanPreviewUse() ? 10 : AUTOPOP_DELAY, NULL);
	}
}

void CPlayerSeekBar::OnMouseMove(UINT nFlags, CPoint point)
{
	AppSettings& s = AfxGetAppSettings();

	CWnd* w = GetCapture();
	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();

	if (w && w->m_hWnd == m_hWnd && (nFlags & MK_LBUTTON)) {
		__int64 pos = CalculatePosition(point);
		if (pFrame->ValidateSeek(pos, m_stop)) {
			MoveThumb(point);
		}

		if (!s.fDisableXPToolbars) {
			GetParent()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_pos, SB_THUMBTRACK), (LPARAM)m_hWnd);
		}
	}

	if (s.fUseTimeTooltip || pFrame->CanPreviewUse()) {
		UpdateTooltip(point);
	}

	OAFilterState fs = pFrame->GetMediaState();

	if (fs != -1) {
		MoveThumb2(point);
		if (pFrame->CanPreviewUse()) UpdateToolTipPosition(point);
	} else {
		pFrame->PreviewWindowHide();
	}
	CDialogBar::OnMouseMove(nFlags, point);
}

LRESULT CPlayerSeekBar::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_MOUSELEAVE) {
		HideToolTip();
		((CMainFrame*)GetParentFrame())->PreviewWindowHide();
		strChap.Empty();
		Invalidate();
	}

	return CWnd::WindowProc(message, wParam, lParam);
}

BOOL CPlayerSeekBar::PreTranslateMessage(MSG* pMsg)
{
	POINT ptWnd(pMsg->pt);
	this->ScreenToClient(&ptWnd);

	if (m_fEnabled && AfxGetAppSettings().fUseTimeTooltip && m_start < m_stop && (GetChannelRect() | GetThumbRect()).PtInRect(ptWnd)) {
		m_tooltip.RelayEvent(pMsg);
	}

	return CDialogBar::PreTranslateMessage(pMsg);
}

BOOL CPlayerSeekBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

BOOL CPlayerSeekBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint p;
	GetCursorPos(&p);
	ScreenToClient(&p);
	if (r_Lock.PtInRect(p)) {
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

		return TRUE;
	}


	if (m_fEnabled && m_start < m_stop && m_stop != 100) {
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));

		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CPlayerSeekBar::OnPlayStop(UINT nID)
{
	SetPos(0);

	Invalidate();

	return FALSE;
}

void CPlayerSeekBar::OnTimer(UINT_PTR nIDEvent)
{
	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();

	if (nIDEvent == m_tooltipTimer) {
		switch (m_tooltipState) {
			case TOOLTIP_TRIGGERED:
			{
				CPoint point;

				GetCursorPos(&point);
				ScreenToClient(&point);

				if (m_fEnabled && m_start < m_stop && (GetChannelRect() | GetThumbRect()).PtInRect(point)) {
					m_tooltipTimer = SetTimer(m_tooltipTimer, pFrame->CanPreviewUse() ? 10 : AUTOPOP_DELAY, NULL);
					m_tooltipPos = CalculatePosition(point);
					UpdateToolTipText();

					if (!pFrame->CanPreviewUse()) {
						m_tooltip.SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ti);
					}

					m_tooltipState = TOOLTIP_VISIBLE;
				}
			}
			break;
			case TOOLTIP_VISIBLE:
			{
				HideToolTip();
				pFrame->PreviewWindowShow(m_pos2);
			}
			break;
		}
	}

	CWnd::OnTimer(nIDEvent);
}

void CPlayerSeekBar::HideToolTip()
{
	if (m_tooltipState > TOOLTIP_HIDDEN) {
		KillTimer(m_tooltipTimer);
		m_tooltip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
		m_tooltipState = TOOLTIP_HIDDEN;
	}
}

void CPlayerSeekBar::UpdateToolTipPosition(CPoint& point)
{
	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();

	if (pFrame->CanPreviewUse()) {
		CRect Rect;
		pFrame->m_wndPreView.GetWindowRect(Rect);
		int r_width  = Rect.Width();
		int r_height = Rect.Height();

		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi);

		CPoint p1(point);
		point.x -= r_width / 2 - 2;
		point.y = GetChannelRect().TopLeft().y - (r_height + 13);
		ClientToScreen(&point);
		point.x = max(mi.rcWork.left + 5, min(point.x, mi.rcWork.right - r_width - 5));

		if (point.y <= 5) {
			CRect r = mi.rcWork;
			if (!r.PtInRect(point)) {
				p1.y = GetChannelRect().TopLeft().y + 30;
				ClientToScreen(&p1);

				point.y = p1.y;
			}
		}

		pFrame->m_wndPreView.MoveWindow(point.x, point.y, r_width, r_height);
	} else {
		CSize size = m_tooltip.GetBubbleSize(&m_ti);
		CRect r;
		GetWindowRect(r);

		if (AfxGetAppSettings().nTimeTooltipPosition == TIME_TOOLTIP_ABOVE_SEEKBAR) {
			point.x -= size.cx / 2 - 2;
			point.y = GetChannelRect().TopLeft().y - (size.cy + 13);
		} else {
			point.x += 10;
			point.y += 20;
		}

		point.x = max(0, min(point.x, r.Width() - size.cx));
		ClientToScreen(&point);

		m_tooltip.SendMessage(TTM_TRACKPOSITION, 0, MAKELPARAM(point.x, point.y));
	}

	m_tooltipLastPos = m_tooltipPos;
}

void CPlayerSeekBar::UpdateToolTipText()
{
	DVD_HMSF_TIMECODE tcNow = RT2HMS_r(m_tooltipPos);

	/*
	if (tcNow.bHours > 0) {
		m_tooltipText.Format(_T("%02d:%02d:%02d"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
	} else {
		m_tooltipText.Format(_T("%02d:%02d"), tcNow.bMinutes, tcNow.bSeconds);
	}
	*/

	CString tooltipText;
	tooltipText.Format(_T("%02d:%02d:%02d"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();

	if (!pFrame->CanPreviewUse()) {
		m_ti.lpszText = (LPTSTR)(LPCTSTR)tooltipText;
		m_tooltip.SendMessage(TTM_SETTOOLINFO, 0, (LPARAM)&m_ti);
	} else {
		pFrame->m_wndPreView.SetWindowText(tooltipText);
	}

	{
		CAutoLock lock(&m_CBLock);

		strChap.Empty();
		if (AfxGetAppSettings().fDisableXPToolbars
			&& AfxGetAppSettings().fChapterMarker
			/*&& AfxGetAppSettings().fFileNameOnSeekBar*/
			&& m_pChapterBag && m_pChapterBag->ChapGetCount()) {

			CComBSTR chapterName;
			REFERENCE_TIME rt = m_tooltipPos;
			m_pChapterBag->ChapLookup(&rt, &chapterName);

			if (chapterName.Length() > 0) {
				strChap.Format(_T("%s%s"), ResStr(IDS_AG_CHAPTER2), chapterName);
				Invalidate();
			}
		}
	}
}

void CPlayerSeekBar::OnMButtonDown(UINT nFlags, CPoint point)
{
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

	if (pFrame->m_wndPreView) {
		pFrame->PreviewWindowHide();
		AfxGetAppSettings().fSmartSeek = !AfxGetAppSettings().fSmartSeek;
		OnMouseMove(nFlags,point);
	}
}

void CPlayerSeekBar::SetChapterBag(CComPtr<IDSMChapterBag>& pCB)
{
	CAutoLock lock(&m_CBLock);

	if (pCB) {
		m_pChapterBag.Release();
		pCB.CopyTo(&m_pChapterBag);
	}
}
