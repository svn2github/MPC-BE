/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "mplayerc.h"
#include <math.h>
#include <atlbase.h>
#include <afxpriv.h>
#include "PlayerToolBar.h"
#include "MainFrm.h"


// CPlayerToolBar

typedef HRESULT (__stdcall * SetWindowThemeFunct)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);

IMPLEMENT_DYNAMIC(CPlayerToolBar, CToolBar)
CPlayerToolBar::CPlayerToolBar()
	: fDisableImgListRemap(false)
	, m_pButtonsImages(NULL)
{
}

CPlayerToolBar::~CPlayerToolBar()
{
	if (m_pButtonsImages) {
		delete m_pButtonsImages;
	}
}

void CPlayerToolBar::SwitchTheme()
{
	AppSettings& s = AfxGetAppSettings();

	m_nButtonHeight = 16;

	if (iDisableXPToolbars != (__int64)s.fDisableXPToolbars) {
		VERIFY(LoadToolBar(IDB_PLAYERTOOLBAR));

		ModifyStyleEx(WS_EX_LAYOUTRTL, WS_EX_NOINHERITLAYOUT);

		GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

		CToolBarCtrl& tb = GetToolBarCtrl();
		tb.DeleteButton(1);
		tb.DeleteButton(tb.GetButtonCount()-1);
		tb.DeleteButton(tb.GetButtonCount()-1);

		SetMute(s.fMute);

		UINT styles[] = {
			TBBS_CHECKGROUP, TBBS_CHECKGROUP,
			TBBS_SEPARATOR,
			TBBS_BUTTON, TBBS_BUTTON, TBBS_BUTTON, TBBS_BUTTON,
			TBBS_SEPARATOR,
			TBBS_BUTTON,
			TBBS_BUTTON,
			TBBS_SEPARATOR,
			TBBS_SEPARATOR,
			TBBS_CHECKBOX,
		};

		for (int i = 0; i < _countof(styles); i++) {
			SetButtonStyle(i, styles[i] | TBBS_DISABLED);
		}

		iDisableXPToolbars = s.fDisableXPToolbars;
	}

	if (s.fDisableXPToolbars) {
		if (HMODULE h = LoadLibrary(_T("uxtheme.dll"))) {
			SetWindowThemeFunct f = (SetWindowThemeFunct)GetProcAddress(h, "SetWindowTheme");

			if (f) {
				f(m_hWnd, L" ", L" ");
			}

			FreeLibrary(h);
		}

		SwitchRemmapedImgList(IDB_PLAYERTOOLBAR, 0);// 0 Remap Active

		COLORSCHEME cs;
		cs.dwSize		= sizeof(COLORSCHEME);
		cs.clrBtnHighlight	= 0x0046413c;
		cs.clrBtnShadow		= 0x0037322d;

		GetToolBarCtrl().SetColorScheme(&cs);
		GetToolBarCtrl().SetIndent(5);
	} else {
		if (HMODULE h = LoadLibrary(_T("uxtheme.dll"))) {
			SetWindowThemeFunct f = (SetWindowThemeFunct)GetProcAddress(h, "SetWindowTheme");

			if (f) {
				f(m_hWnd, L"Explorer", NULL);
			}

			FreeLibrary(h);
		}

		SwitchRemmapedImgList(IDB_PLAYERTOOLBAR, 2);// 2 Undo  Active

		COLORSCHEME cs;
		cs.dwSize		= sizeof(COLORSCHEME);
		cs.clrBtnHighlight	= GetSysColor(COLOR_BTNFACE);
		cs.clrBtnShadow		= GetSysColor(COLOR_BTNSHADOW);

		GetToolBarCtrl().SetColorScheme(&cs);
		GetToolBarCtrl().SetIndent(0);
	}

	int fp = m_logobm.FileExists("toolbar");

	HBITMAP hBmp;
	if (s.fDisableXPToolbars && NULL == fp) {
		//int col = s.clrFaceABGR;
		//int r, g, b, R, G, B;
		//r = col & 0xFF;
		//g = (col >> 8) & 0xFF;
		//b = col >> 16;
		hBmp = m_logobm.LoadExternalImage("toolbar", s.nThemeBrightness, s.nThemeRed, s.nThemeGreen, s.nThemeBlue);
	} else if (fp) {
		hBmp = m_logobm.LoadExternalImage("toolbar", -1, -1, -1, -1);
	}

	if (NULL != hBmp) {
		CBitmap *bmp = DNew CBitmap();
		bmp->Attach(hBmp);
		BITMAP bitmapBmp;
		bmp->GetBitmap(&bitmapBmp);

		if (bitmapBmp.bmWidth == bitmapBmp.bmHeight * 15) {

			SetSizes(CSize(bitmapBmp.bmHeight + 7, bitmapBmp.bmHeight + 6), CSize(bitmapBmp.bmHeight, bitmapBmp.bmHeight));

			CDC dc;
			dc.CreateCompatibleDC(NULL);

			DIBSECTION dib;
			::GetObject(hBmp, sizeof(dib), &dib);
			int fileDepth = dib.dsBmih.biBitCount;

			if (m_pButtonsImages) {
				delete m_pButtonsImages;
				m_pButtonsImages = NULL;
			}

			m_pButtonsImages = DNew CImageList();

			if (32 == fileDepth) {
				m_pButtonsImages->Create(bitmapBmp.bmHeight, bitmapBmp.bmHeight, ILC_COLOR32 | ILC_MASK, 1, 0);
				m_pButtonsImages->Add(bmp, static_cast<CBitmap*>(0));
			} else {
				m_pButtonsImages->Create(bitmapBmp.bmHeight, bitmapBmp.bmHeight, ILC_COLOR24 | ILC_MASK, 1, 0);
				m_pButtonsImages->Add(bmp, RGB(255, 0, 255));
			}

			m_nButtonHeight = bitmapBmp.bmHeight;
			GetToolBarCtrl().SetImageList(m_pButtonsImages);
			fDisableImgListRemap = true;
		}

		delete bmp;
		DeleteObject(hBmp);
	}

	if (!s.fDisableXPToolbars) {
		fDisableImgListRemap = true;
	}

	if (s.fDisableXPToolbars) {
		if (!fDisableImgListRemap) {
			SwitchRemmapedImgList(IDB_PLAYERTOOLBAR, 0);// 0 Remap Active
			SwitchRemmapedImgList(IDB_PLAYERTOOLBAR, 1);// 1 Remap Disabled
		}
	} else {
		if (NULL == fp) {
			SwitchRemmapedImgList(IDB_PLAYERTOOLBAR, 2);// 2 Undo  Active

			if (!fDisableImgListRemap) {
				SwitchRemmapedImgList(IDB_PLAYERTOOLBAR, 3);// 3 Undo  Disabled
			}
		}
	}

	if (::IsWindow(m_volctrl.GetSafeHwnd())) {
		m_volctrl.EnableWindow(FALSE);
		m_volctrl.EnableWindow(TRUE);
	}
}

BOOL CPlayerToolBar::Create(CWnd* pParentWnd)
{
	VERIFY(__super::CreateEx(pParentWnd,
			TBSTYLE_FLAT|TBSTYLE_TRANSPARENT|TBSTYLE_AUTOSIZE|TBSTYLE_CUSTOMERASE,
			WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM|CBRS_TOOLTIPS));

	m_volctrl.Create(this);
	m_volctrl.SetRange(0, 100);

	iDisableXPToolbars = 2;

	SwitchTheme();

	return TRUE;
}

BOOL CPlayerToolBar::PreCreateWindow(CREATESTRUCT& cs)
{
	VERIFY(__super::PreCreateWindow(cs));

	m_dwStyle &= ~CBRS_BORDER_TOP;
	m_dwStyle &= ~CBRS_BORDER_BOTTOM;

	return TRUE;
}

void CPlayerToolBar::CreateRemappedImgList(UINT bmID, int nRemapState, CImageList& reImgList)
{
	// 0 Remap Active
	// 1 Remap Disabled
	// 2 Undo  Active
	// 3 Undo  Disabled

	AppSettings& s = AfxGetAppSettings();

	COLORMAP cmActive[] =
	{
		0x00000000, s.clrFaceABGR,
		0x00808080, s.clrOutlineABGR,
		0x00c0c0c0, 0x00ff00ff//background = transparency mask
	};

	COLORMAP cmDisabled[] =
	{
		0x00000000, 0x00ff00ff,//button_face -> transparency mask
		0x00808080, s.clrOutlineABGR,
		0x00c0c0c0, 0x00ff00ff//background = transparency mask
	};

	COLORMAP cmUndoActive[] =
	{
		0x00c0c0c0, 0x00ff00ff//background = transparency mask
	};

	COLORMAP cmUndoDisabled[] =
	{
		0x00000000, 0x00A0A0A0,//button_face -> black to gray
		0x00c0c0c0, 0x00ff00ff//background = transparency mask
	};

	CBitmap bm;

	switch (nRemapState)
	{
		default:
		case 0:
			bm.LoadMappedBitmap(bmID, CMB_MASKED, cmActive, 3);
			break;
		case 1:
			bm.LoadMappedBitmap(bmID, CMB_MASKED, cmDisabled, 3);
			break;
		case 2:
			bm.LoadMappedBitmap(bmID, CMB_MASKED, cmUndoActive, 1);
			break;
		case 3:
			bm.LoadMappedBitmap(bmID, CMB_MASKED, cmUndoDisabled, 2);
			break;
	}

	BITMAP bmInfo;
	VERIFY(bm.GetBitmap(&bmInfo));
	VERIFY(reImgList.Create(bmInfo.bmHeight, bmInfo.bmHeight, bmInfo.bmBitsPixel | ILC_MASK, 1, 0));
	VERIFY(reImgList.Add(&bm, 0x00ff00ff) != -1);
}

void CPlayerToolBar::SwitchRemmapedImgList(UINT bmID, int nRemapState)
{
	// 0 Remap Active
	// 1 Remap Disabled
	// 2 Undo  Active
	// 3 Undo  Disabled

	CToolBarCtrl& ctrl = GetToolBarCtrl();

	if (nRemapState == 0 || nRemapState == 2) {
		if (m_reImgListActive.GetSafeHandle()) {
			m_reImgListActive.DeleteImageList();
		}

		CreateRemappedImgList(bmID, nRemapState, m_reImgListActive);
		ASSERT(m_reImgListActive.GetSafeHandle());
		ctrl.SetImageList(&m_reImgListActive);
	} else {
		if (m_reImgListDisabled.GetSafeHandle()) {
			m_reImgListDisabled.DeleteImageList();
		}

		CreateRemappedImgList(bmID, nRemapState, m_reImgListDisabled);
		ASSERT(m_reImgListDisabled.GetSafeHandle());
		ctrl.SetDisabledImageList(&m_reImgListDisabled);
	}
}

void CPlayerToolBar::ArrangeControls()
{
	if (!::IsWindow(m_volctrl.m_hWnd)) {
		return;
	}

	SwitchTheme();

	CRect r;
	GetClientRect(&r);

	CRect br = GetBorders();

	CRect r10;
	GetItemRect(10, &r10);

	CRect vr2;

	if (AfxGetAppSettings().fDisableXPToolbars) {
		int m_nBMedian = r.bottom - 3 - 0.5 * m_nButtonHeight;
		vr2.SetRect(r.right + br.right - 60, m_nBMedian - 14, r.right +br.right + 6, m_nBMedian + 10);
	} else {
		vr2.SetRect(r.right + br.right - 60, r.bottom - 25, r.right +br.right + 6, r.bottom);
	}

	m_volctrl.MoveWindow(vr2);

	SetButtonInfo(7, GetItemID(7), TBBS_SEPARATOR | TBBS_DISABLED, 48);
	SetButtonInfo(11, GetItemID(11), TBBS_SEPARATOR | TBBS_DISABLED, vr2.left - r10.right - r10.bottom - r10.top);
}

void CPlayerToolBar::SetMute(bool fMute)
{
	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	bi.iImage = fMute ? 13 : 12;
	tb.SetButtonInfo(ID_VOLUME_MUTE, &bi);

	AfxGetAppSettings().fMute = fMute;
}

bool CPlayerToolBar::IsMuted()
{
	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	tb.GetButtonInfo(ID_VOLUME_MUTE, &bi);
	return(bi.iImage == 13);
}

int CPlayerToolBar::GetVolume()
{
	AppSettings& s = AfxGetAppSettings();

	int volume = m_volctrl.GetPos(), size = m_volctrl.GetPageSize();

	if (!s.fMute || s.nVolume >= volume + size || s.nVolume <= volume - size) {
		if ((!IsMuted() && !volume) || (IsMuted() && volume)) {
			OnVolumeMute(0);
			SendMessage(WM_COMMAND, ID_VOLUME_MUTE);
		}
	}

	if (IsMuted() || volume <= 0) {
		volume = -10000;
	} else {
		volume = min((int)(4000 * log10(volume / 100.0f)), 0);// 4000=2.0*100*20, where 2.0 is a special factor
	}

	return volume;
}

int CPlayerToolBar::GetMinWidth()
{
	return m_nButtonHeight * 12 + 155;
}

void CPlayerToolBar::SetVolume(int volume)
{
	m_volctrl.SetPosInternal(volume);
}

BEGIN_MESSAGE_MAP(CPlayerToolBar, CToolBar)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)
	ON_COMMAND_EX(ID_VOLUME_MUTE, OnVolumeMute)
	ON_UPDATE_COMMAND_UI(ID_VOLUME_MUTE, OnUpdateVolumeMute)

	ON_COMMAND_EX(ID_PLAY_PAUSE, OnPause)
	ON_COMMAND_EX(ID_PLAY_PLAY, OnPlay)
	ON_COMMAND_EX(ID_PLAY_STOP, OnStop)
	ON_COMMAND_EX(ID_FILE_CLOSEMEDIA, OnStop)

	ON_COMMAND_EX(ID_VOLUME_UP, OnVolumeUp)
	ON_COMMAND_EX(ID_VOLUME_DOWN, OnVolumeDown)

	ON_WM_NCPAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFFFFFF, OnToolTipNotify)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// CPlayerToolBar message handlers

void CPlayerToolBar::OnCustomDraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTBCUSTOMDRAW pTBCD = reinterpret_cast<LPNMTBCUSTOMDRAW>(pNMHDR);
	LRESULT lr = CDRF_DODEFAULT;
	AppSettings& s = AfxGetAppSettings();

	int R, G, B, R2, G2, B2;

	GRADIENT_RECT gr[1] = {{0, 1}};

	int sep[] = {2, 7, 10, 11};

	if (s.fDisableXPToolbars) {

		int fp = m_logobm.FileExists("background");

		switch(pTBCD->nmcd.dwDrawStage)
		{
		case CDDS_PREERASE:
			m_volctrl.Invalidate();
			lr = CDRF_SKIPDEFAULT;
			break;
		case CDDS_PREPAINT:
			{
			CDC dc;
			dc.Attach(pTBCD->nmcd.hdc);
			CRect r;

			GetClientRect(&r);

			if (NULL != fp) {
				ThemeRGB(s.nThemeRed, s.nThemeGreen, s.nThemeBlue, R, G, B);
				m_logobm.LoadExternalGradient("background", &dc, r, 21, s.nThemeBrightness, R, G, B);
			} else {
				ThemeRGB(50, 55, 60, R, G, B);
				ThemeRGB(20, 25, 30, R2, G2, B2);
				TRIVERTEX tv[2] = {
					{r.left, r.top, R*256, G*256, B*256, 255*256},
					{r.right, r.bottom, R2*256, G2*256, B2*256, 255*256},
				};
				dc.GradientFill(tv, 2, gr, 1, GRADIENT_FILL_RECT_V);
			}

			dc.Detach();
			}
			lr |= CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			lr = CDRF_DODEFAULT;

			lr |= TBCDRF_NOETCHEDEFFECT;
			lr |= TBCDRF_NOBACKGROUND;
			lr |= TBCDRF_NOEDGES;
			lr |= TBCDRF_NOOFFSET;

			lr |= CDRF_NOTIFYPOSTPAINT;
			lr |= CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPOSTPAINT:
			lr = CDRF_DODEFAULT;

			CDC dc;
			dc.Attach(pTBCD->nmcd.hdc);
			CRect r;
			CopyRect(&r,&pTBCD->nmcd.rc);

			CRect rGlassLike(0,0,8,8);
			int nW = rGlassLike.Width(), nH = rGlassLike.Height();
			CDC memdc;
			memdc.CreateCompatibleDC(&dc);
			CBitmap *bmOld, bmGlassLike;
			bmGlassLike.CreateCompatibleBitmap(&dc, nW, nH);
			bmOld = memdc.SelectObject(&bmGlassLike);

			TRIVERTEX tv[2] = {
				{0, 0, 255*256, 255*256, 255*256, 255*256},
				{nW, nH, 0, 0, 0, 0},
			};
			memdc.GradientFill(tv, 2, gr, 1, GRADIENT_FILL_RECT_V);

			BLENDFUNCTION bf;
			bf.AlphaFormat	= AC_SRC_ALPHA;
			bf.BlendFlags	= 0;
			bf.BlendOp	= AC_SRC_OVER;
			bf.SourceConstantAlpha = 90;

			CPen penFrHot(PS_SOLID,0,0x00e9e9e9);//clr_resFace
			CPen *penSaved = dc.SelectObject(&penFrHot);
			CBrush *brushSaved = (CBrush*)dc.SelectStockObject(NULL_BRUSH);

			//CDIS_SELECTED,CDIS_GRAYED,CDIS_DISABLED,CDIS_CHECKED,CDIS_FOCUS,CDIS_DEFAULT,CDIS_HOT,CDIS_MARKED,CDIS_INDETERMINATE

			if (CDIS_HOT == pTBCD->nmcd.uItemState || CDIS_CHECKED + CDIS_HOT == pTBCD->nmcd.uItemState) {
				dc.SelectObject(&penFrHot);
				dc.RoundRect(r.left + 1, r.top + 1, r.right - 2, r.bottom - 1, 6, 4);
				AlphaBlend(dc.m_hDC, r.left + 2, r.top + 2, r.Width() - 4, 0.7 * r.Height() - 2, memdc, 0, 0, nW, nH, bf);
			}
/*
			if (CDIS_CHECKED == pTBCD->nmcd.uItemState) {
				CPen penFrChecked(PS_SOLID,0,0x00808080);//clr_resDark
				dc.SelectObject(&penFrChecked);
				dc.RoundRect(r.left + 1, r.top + 1, r.right - 2, r.bottom - 1, 6, 4);
			}
*/
			for (int j = 0; j < _countof(sep); j++) {
				GetItemRect(sep[j], &r);

				if (NULL != fp) {
					ThemeRGB(s.nThemeRed, s.nThemeGreen, s.nThemeBlue, R, G, B);
					m_logobm.LoadExternalGradient("background", &dc, r, 21, s.nThemeBrightness, R, G, B);
				} else {
					ThemeRGB(50, 55, 60, R, G, B);
					ThemeRGB(20, 25, 30, R2, G2, B2);
					TRIVERTEX tv[2] = {
						{r.left, r.top, R*256, G*256, B*256, 255*256},
						{r.right, r.bottom, R2*256, G2*256, B2*256, 255*256},
					};
					dc.GradientFill(tv, 2, gr, 1, GRADIENT_FILL_RECT_V);
				}
			}

			dc.SelectObject(&penSaved);
			dc.SelectObject(&brushSaved);
			dc.Detach();
			DeleteObject(memdc.SelectObject(bmOld));
			memdc.DeleteDC();
			lr |= CDRF_SKIPDEFAULT;
			break;
		}
	} else {
		switch(pTBCD->nmcd.dwDrawStage)
		{
		case CDDS_PREERASE:
			m_volctrl.Invalidate();
			lr = CDRF_SKIPDEFAULT;
			break;
		case CDDS_PREPAINT:
			{
			CDC dc;
			dc.Attach(pTBCD->nmcd.hdc);
			CRect r;
			GetClientRect(&r);
			dc.FillSolidRect(r, GetSysColor(COLOR_BTNFACE));
			dc.Detach();
			}
			lr |= CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			lr = CDRF_DODEFAULT;

			lr |= CDRF_NOTIFYPOSTPAINT;
			lr |= CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPOSTPAINT:
			lr = CDRF_DODEFAULT;

			CDC dc;
			dc.Attach(pTBCD->nmcd.hdc);
			CRect r;

			for (int j = 0; j < _countof(sep); j++) {
				GetItemRect(sep[j], &r);

				dc.FillSolidRect(r, GetSysColor(COLOR_BTNFACE));
			}

			dc.Detach();
			lr |= CDRF_SKIPDEFAULT;
			break;
		}
	}

	*pResult = lr;
}

void CPlayerToolBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	ArrangeControls();

	if (!((CMainFrame*)AfxGetMainWnd())->m_fFullScreen) {
		::PostMessage(((CMainFrame*)AfxGetMainWnd())->m_hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(0, 0));
	}
}

void CPlayerToolBar::OnInitialUpdate()
{
	ArrangeControls();
}

BOOL CPlayerToolBar::OnVolumeMute(UINT nID)
{
	SetMute(!IsMuted());

	if (::IsWindow(m_volctrl.GetSafeHwnd())) {
		m_volctrl.EnableWindow(FALSE);
		m_volctrl.EnableWindow(TRUE);
	}

	return FALSE;
}

void CPlayerToolBar::OnUpdateVolumeMute(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(IsMuted());
}

BOOL CPlayerToolBar::OnVolumeUp(UINT nID)
{
	m_volctrl.IncreaseVolume();

	return FALSE;
}

BOOL CPlayerToolBar::OnVolumeDown(UINT nID)
{
	m_volctrl.DecreaseVolume();

	return FALSE;
}

void CPlayerToolBar::OnNcPaint()
{
	CRect wr, cr;
	CWindowDC dc(this);
	GetClientRect(&cr);
	ClientToScreen(&cr);
	GetWindowRect(&wr);
	cr.OffsetRect(-wr.left, -wr.top);
	wr.OffsetRect(-wr.left, -wr.top);
	dc.ExcludeClipRect(&cr);

	if (!AfxGetAppSettings().fDisableXPToolbars) {
		dc.FillSolidRect(wr, GetSysColor(COLOR_BTNFACE));
	}
}

void CPlayerToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CMainFrame* pFrame	= (CMainFrame*)GetParentFrame();

	int i = getHitButtonIdx(point);

	if (i == -1 || (GetButtonStyle(i) & (TBBS_SEPARATOR | TBBS_DISABLED))) {
	} else {
		if (i > 10 || i < 2 || i == 6 || (i < 10 && (pFrame->IsSomethingLoaded() || pFrame->IsD3DFullScreenMode()))) {
			::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
		}
	}

	__super::OnMouseMove(nFlags, point);
}

BOOL CPlayerToolBar::OnPlay(UINT nID)
{
	CMainFrame* pFrame	= (CMainFrame*)GetParentFrame();
	OAFilterState fs	= pFrame->GetMediaState();

	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	tb.GetButtonInfo(ID_PLAY_PLAY, &bi);

	int pos = (fs == State_Paused || fs == State_Stopped) ? 1 : 0;

	if (!bi.iImage) {
		bi.iImage = 1;
	} else {
		bi.iImage = pos;
	}

	tb.SetButtonInfo(ID_PLAY_PLAY, &bi);

	return FALSE;
}

BOOL CPlayerToolBar::OnStop(UINT nID)
{
	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	tb.GetButtonInfo(ID_PLAY_PLAY, &bi);
	bi.iImage = 0;
	tb.SetButtonInfo(ID_PLAY_PLAY, &bi);

	return FALSE;
}

BOOL CPlayerToolBar::OnPause(UINT nID)
{
	CMainFrame* pFrame	= (CMainFrame*)GetParentFrame();
	OAFilterState fs	= pFrame->GetMediaState();

	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	tb.GetButtonInfo(ID_PLAY_PLAY, &bi);
	bi.iImage = (fs == State_Paused) ? 1 : 0;
	tb.SetButtonInfo(ID_PLAY_PLAY, &bi);

	return FALSE;
}

void CPlayerToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CMainFrame* pFrame	= (CMainFrame*)GetParentFrame();
	OAFilterState fs	= pFrame->GetMediaState();

	int i = getHitButtonIdx(point);

	if (i == 0 && fs != -1) {
		pFrame->PostMessage(WM_COMMAND, ID_PLAY_PLAYPAUSE);
	} else if (i == -1 || (GetButtonStyle(i) & (TBBS_SEPARATOR | TBBS_DISABLED))) {
		if (!pFrame->m_fFullScreen) {
			MapWindowPoints(pFrame, &point, 1);
			pFrame->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
		}
	} else {
		if (i > 10 || (i < 10 && (pFrame->IsSomethingLoaded() || pFrame->IsD3DFullScreenMode()))) {
			::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
		}

		__super::OnLButtonDown(nFlags, point);
	}
}

int CPlayerToolBar::getHitButtonIdx(CPoint point)
{
	int hit = -1;
	CRect r;

	for (int i = 0, j = GetToolBarCtrl().GetButtonCount(); i < j; i++) {
		GetItemRect(i, r);

		if (r.PtInRect(point)) {
			hit = i;
			break;
		}
	}

	return hit;
}

BOOL CPlayerToolBar::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	TOOLTIPTEXT* pTTT	= (TOOLTIPTEXT*)pNMHDR;
	CMainFrame* pFrame	= (CMainFrame*)GetParentFrame();
	OAFilterState fs	= pFrame->GetMediaState();

	::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(INT)1000);

	static CString m_strTipText;

	if (pNMHDR->idFrom == ID_PLAY_PLAY) {
		(fs == State_Running) ? m_strTipText = ResStr(IDS_AG_PAUSE) : m_strTipText = ResStr(IDS_AG_PLAY);

	} else if (pNMHDR->idFrom == ID_PLAY_STOP) {
		m_strTipText = ResStr(IDS_AG_STOP) + _T(" | ") + ResStr(IDS_AG_CLOSE);

	} else if (pNMHDR->idFrom == ID_PLAY_FRAMESTEP) {
		m_strTipText = ResStr(IDS_AG_STEP) + _T(" | ") + ResStr(IDS_AG_JUMP_TO);

	} else if (pNMHDR->idFrom == ID_VOLUME_MUTE) {
		m_strTipText = ResStr(IDS_AG_VOLUME_MUTE);

	} else if (pNMHDR->idFrom == ID_FILE_OPENQUICK) {
		m_strTipText = ResStr(IDS_MPLAYERC_0);

	} else if (pNMHDR->idFrom == ID_NAVIGATE_SKIPFORWARD) {
		m_strTipText = ResStr(IDS_AG_NEXT);

	} else if (pNMHDR->idFrom == ID_NAVIGATE_SKIPBACK) {
		m_strTipText = ResStr(IDS_AG_PREVIOUS);

	} else if (pNMHDR->idFrom == ID_NAVIGATE_SUBTITLES) {
		m_strTipText = ResStr(IDS_AG_SUBTITLELANG) + _T(" | ") + ResStr(IDS_AG_OPTIONS);

	} else if (pNMHDR->idFrom == ID_NAVIGATE_AUDIO) {
		m_strTipText = ResStr(IDS_AG_AUDIOLANG) + _T(" | ") + ResStr(IDS_AG_OPTIONS);

	}

	pTTT->lpszText = m_strTipText.GetBuffer();

	*pResult = 0;

	return TRUE;
}

void CPlayerToolBar::OnRButtonDown(UINT nFlags, CPoint point)
{
	CMainFrame* pFrame	= (CMainFrame*)GetParentFrame();
	int Idx				= getHitButtonIdx(point);
	OAFilterState fs	= pFrame->GetMediaState();

	if (Idx == 1) {
		pFrame->PostMessage(WM_COMMAND, ID_FILE_CLOSEMEDIA);
	} else if (Idx == 5) {
		pFrame->OnMenuNavJumpTo();
	} else if (Idx == 8) {
		pFrame->OnMenuNavAudioOptions();
	} else if (Idx == 9) {
		pFrame->OnMenuNavSubtitleOptions();
	}

	__super::OnRButtonDown(nFlags, point);
}
