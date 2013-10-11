/*
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
#include "VMROSD.h"

#define DEFFLAGS				SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_ASYNCWINDOWPOS

#define SEEKBAR_HEIGHT			60
#define SLIDER_BAR_HEIGHT		10
#define SLIDER_CURSOR_HEIGHT	30
#define SLIDER_CURSOR_WIDTH		15
#define SLIDER_CHAP_WIDTH		4
#define SLIDER_CHAP_HEIGHT		10


CVMROSD::CVMROSD()
	: m_nMessagePos(OSD_NOMESSAGE)
	, m_bSeekBarVisible(false)
	, m_bFlyBarVisible(false)
	, m_bCursorMoving(false)
	, m_pMFVMB(NULL)
	, m_pVMB(NULL)
	, m_pMVTO(NULL)
	, m_pWnd(NULL)
	, m_FontSize(0)
	, m_OSD_Transparent(0)
	, bMouseOverExitButton(false)
	, bMouseOverCloseButton(false)
	, m_bShowMessage(true)
	, m_bVisibleMessage(false)
	, m_OSDType(OSD_TYPE_NONE)
	, m_pChapterBag(NULL)
{
	m_Color[OSD_TRANSPARENT]	= RGB(  0,   0,   0);
	m_Color[OSD_BACKGROUND]		= RGB( 32,  40,  48);
	m_Color[OSD_BORDER]			= RGB( 48,  56,  62);
	m_Color[OSD_TEXT]			= RGB(224, 224, 224);
	m_Color[OSD_BAR]			= RGB( 64,  72,  80);
	m_Color[OSD_CURSOR]			= RGB(192, 200, 208);
	m_Color[OSD_DEBUGCLR]		= RGB(128, 136, 144);

	m_penBorder.CreatePen(PS_SOLID, 1, m_Color[OSD_BORDER]);
	m_penCursor.CreatePen(PS_SOLID, 4, m_Color[OSD_CURSOR]);
	m_brushBack.CreateSolidBrush(m_Color[OSD_BACKGROUND]);
	m_brushBar.CreateSolidBrush (m_Color[OSD_BAR]);
	m_brushChapter.CreateSolidBrush(m_Color[OSD_CURSOR]);
	m_debugBrushBack.CreateSolidBrush(m_Color[OSD_DEBUGCLR]);
	m_debugPenBorder.CreatePen(PS_SOLID, 1, m_Color[OSD_BORDER]);

	memset(&m_BitmapInfo, 0, sizeof(m_BitmapInfo));
	
	m_MainWndRect = m_MainWndRectCashed = CRect(0,0,0,0);

	int fp = m_bm.FileExists(CString(_T("flybar")));

	HBITMAP hBmp = m_bm.LoadExternalImage("flybar", IDB_PLAYERFLYBAR_PNG, -1, -1, -1, -1, -1);
	BITMAP bm;
	::GetObject(hBmp, sizeof(bm), &bm);

	if (fp && bm.bmWidth != bm.bmHeight * 25) {
		hBmp = m_bm.LoadExternalImage("", IDB_PLAYERFLYBAR_PNG, -1, -1, -1, -1, -1);
		::GetObject(hBmp, sizeof(bm), &bm);
	}

	if (NULL != hBmp) {
		CBitmap *bmp = DNew CBitmap();

		if (bmp) {
			bmp->Attach(hBmp);

			if (bm.bmWidth == bm.bmHeight * 25) {
				m_pButtonsImages = DNew CImageList();
				m_pButtonsImages->Create(bm.bmHeight, bm.bmHeight, ILC_COLOR32 | ILC_MASK, 1, 0);
				m_pButtonsImages->Add(bmp, static_cast<CBitmap*>(0));

				m_nButtonHeight = bm.bmHeight;
			}

			delete bmp;
		}

		DeleteObject(hBmp);
	}

	//Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
}

CVMROSD::~CVMROSD()
{
	Stop();

	if (m_MemDC) {
		m_MemDC.DeleteDC();
	}

	if (m_pButtonsImages) {
		delete m_pButtonsImages;
	}

	//Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

IMPLEMENT_DYNAMIC(CVMROSD, CWnd)

BEGIN_MESSAGE_MAP(CVMROSD, CWnd)
	ON_MESSAGE_VOID(WM_HIDE, OnHide)
	ON_MESSAGE_VOID(WM_OSD_DRAW, OnDrawWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()

void CVMROSD::OnHide()
{
	ShowWindow(SW_HIDE);
}

void CVMROSD::OnDrawWnd()
{
	DrawWnd();
}

void CVMROSD::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	__super::OnWindowPosChanged(lpwndpos);

	if ((lpwndpos->flags & SWP_HIDEWINDOW) && m_pWnd) {
		SetWindowPos(NULL, 0, 0, 0, 0, DEFFLAGS | SWP_NOMOVE);

		if (!m_bVisibleMessage) {
			m_strMessage.Empty();
		}
	}
}

void CVMROSD::OnSize(UINT nType, int cx, int cy)
{
	if (m_pWnd && (m_pVMB || m_pMFVMB)) {
		if (m_bSeekBarVisible || m_bFlyBarVisible) {
			m_bCursorMoving		= false;
			m_bSeekBarVisible	= false;
			m_bFlyBarVisible	= false;
		}
		InvalidateVMROSD();
		UpdateBitmap();
	} else if (m_pWnd) {
		PostMessage(WM_OSD_DRAW);
	}
}

void CVMROSD::UpdateBitmap()
{
	CAutoLock Lock(&m_Lock);

	CRect	rc;
	CDC*	pDC = CDC::FromHandle(::GetWindowDC(m_pWnd->m_hWnd));

	CalcRect();

	if (m_MemDC) {
		m_MemDC.DeleteDC();
	}

	memset(&m_BitmapInfo, 0, sizeof(m_BitmapInfo));

	if (m_MemDC.CreateCompatibleDC (pDC)) {
		BITMAPINFO	bmi = {0};
		HBITMAP		hbmpRender;

		ZeroMemory( &bmi.bmiHeader, sizeof(BITMAPINFOHEADER) );
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = m_rectWnd.Width();
		bmi.bmiHeader.biHeight = - (int) m_rectWnd.Height(); // top-down
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		hbmpRender = CreateDIBSection( m_MemDC, &bmi, DIB_RGB_COLORS, NULL, NULL, NULL );
		m_MemDC.SelectObject (hbmpRender);

		if (::GetObject(hbmpRender, sizeof(BITMAP), &m_BitmapInfo) != 0) {
			// Configure the VMR's bitmap structure
			if (m_pVMB) {
				ZeroMemory(&m_VMR9AlphaBitmap, sizeof(m_VMR9AlphaBitmap) );
				m_VMR9AlphaBitmap.dwFlags		= VMRBITMAP_HDC | VMRBITMAP_SRCCOLORKEY;
				m_VMR9AlphaBitmap.hdc			= m_MemDC;
				m_VMR9AlphaBitmap.rSrc			= m_rectWnd;
				m_VMR9AlphaBitmap.rDest.left	= 0;
				m_VMR9AlphaBitmap.rDest.top		= 0;
				m_VMR9AlphaBitmap.rDest.right	= 1.0;
				m_VMR9AlphaBitmap.rDest.bottom	= 1.0;
				m_VMR9AlphaBitmap.fAlpha		= 1.0;
				m_VMR9AlphaBitmap.clrSrcKey		= m_Color[OSD_TRANSPARENT];
			} else if (m_pMFVMB) {
				ZeroMemory(&m_MFVideoAlphaBitmap, sizeof(m_MFVideoAlphaBitmap) );
				m_MFVideoAlphaBitmap.params.dwFlags			= MFVideoAlphaBitmap_SrcColorKey;
				m_MFVideoAlphaBitmap.params.clrSrcKey		= m_Color[OSD_TRANSPARENT];
				m_MFVideoAlphaBitmap.params.rcSrc			= m_rectWnd;
				m_MFVideoAlphaBitmap.params.nrcDest.right	= 1;
				m_MFVideoAlphaBitmap.params.nrcDest.bottom	= 1;
				m_MFVideoAlphaBitmap.GetBitmapFromDC		= TRUE;
				m_MFVideoAlphaBitmap.bitmap.hdc				= m_MemDC;
			}
			m_MemDC.SetTextColor(m_Color[OSD_TEXT]);
			m_MemDC.SetBkMode(TRANSPARENT);
		}

		if (m_MainFont.GetSafeHandle()) {
			m_MemDC.SelectObject(m_MainFont);
		}

		DeleteObject(hbmpRender);
	}

	::ReleaseDC(m_pWnd->m_hWnd, pDC->m_hDC);
}

void CVMROSD::Reset()
{
	m_bShowMessage	= true;

	m_MainWndRect.SetRectEmpty();
	m_strMessage.Empty();
	m_MainWndRectCashed.SetRectEmpty();
	m_strMessageCashed.Empty();
}

void CVMROSD::Start(CWnd* pWnd, IVMRMixerBitmap9* pVMB)
{
	m_pVMB			= pVMB;
	m_pMFVMB		= NULL;
	m_pMVTO			= NULL;
	m_pWnd			= pWnd;
	m_OSDType		= OSD_TYPE_BITMAP;

	Reset();

	UpdateBitmap();
}

void CVMROSD::Start(CWnd* pWnd, IMFVideoMixerBitmap* pMFVMB)
{
	m_pMFVMB		= pMFVMB;
	m_pVMB			= NULL;
	m_pMVTO			= NULL;
	m_pWnd			= pWnd;
	m_OSDType		= OSD_TYPE_BITMAP;

	Reset();

	UpdateBitmap();
}

void CVMROSD::Start(CWnd* pWnd, IMadVRTextOsd* pMVTO)
{
	m_pMFVMB		= NULL;
	m_pVMB			= NULL;
	m_pMVTO			= pMVTO;
	m_pWnd			= pWnd;
	m_OSDType		= OSD_TYPE_MADVR;

	Reset();
}

void CVMROSD::Start(CWnd* pWnd)
{
	m_pMFVMB	= NULL;
	m_pVMB		= NULL;
	m_pMVTO		= NULL;
	m_pWnd		= pWnd;
	m_OSDType	= OSD_TYPE_GDI;

	Reset();
}

void CVMROSD::Stop()
{
	if (m_pWnd) {
		::KillTimer(m_pWnd->m_hWnd, (UINT_PTR)this);
	}

	ClearMessage();

	if (m_pVMB) {
		m_pVMB.Release();
	}

	if (m_pMFVMB) {
		m_pMFVMB.Release();
	}

	if (m_pMVTO) {
		m_pMVTO.Release();
	}

	m_pWnd = NULL;

	Reset();
}

void CVMROSD::CalcRect()
{
	if (m_pWnd) {
		m_pWnd->GetClientRect(&m_rectWnd);

		m_rectSeekBar.left			= m_rectWnd.left	+ 10;
		m_rectSeekBar.right			= m_rectWnd.right	- 10;
		m_rectSeekBar.top			= m_rectWnd.bottom	- SEEKBAR_HEIGHT;
		m_rectSeekBar.bottom		= m_rectSeekBar.top	+ SEEKBAR_HEIGHT;

		m_rectFlyBar.left			= m_rectWnd.left;
		m_rectFlyBar.right			= m_rectWnd.right;
		m_rectFlyBar.top			= m_rectWnd.top;
		m_rectFlyBar.bottom			= m_rectWnd.top		+ 100;

		m_rectExitButton.left		= m_rectWnd.right	- 34;
		m_rectExitButton.right		= m_rectWnd.right	- 10;
		m_rectExitButton.top		= m_rectWnd.top		- 10;
		m_rectExitButton.bottom		= m_rectWnd.top		+ 34;

		m_rectCloseButton.left		= m_rectExitButton.left	- 28;
		m_rectCloseButton.right		= m_rectExitButton.left	- 4;
		m_rectCloseButton.top		= m_rectWnd.top			- 10;
		m_rectCloseButton.bottom	= m_rectWnd.top			+ 34;
	}
}

void CVMROSD::DrawRect(CRect* rect, CBrush* pBrush, CPen* pPen)
{
	if (pPen) {
		m_MemDC.SelectObject(pPen);
	} else {
		m_MemDC.SelectStockObject(NULL_PEN);
	}

	if (pBrush) {
		m_MemDC.SelectObject(pBrush);
	} else {
		m_MemDC.SelectStockObject(HOLLOW_BRUSH);
	}

	m_MemDC.Rectangle(rect);
}

void CVMROSD::DrawSlider(CRect* rect, __int64 llMin, __int64 llMax, __int64 llPos)
{
	m_rectBar.left		= rect->left  + 10;
	m_rectBar.right		= rect->right - 10;
	m_rectBar.top		= rect->top   + (rect->Height() - SLIDER_BAR_HEIGHT) / 2;
	m_rectBar.bottom	= m_rectBar.top + SLIDER_BAR_HEIGHT;

	if (llMax == llMin) {
		m_rectCursor.left	= m_rectBar.left;
	} else {
		m_rectCursor.left	= m_rectBar.left + (long)((m_rectBar.Width() - SLIDER_CURSOR_WIDTH) * llPos / (llMax-llMin));
	}

	m_rectCursor.right		= m_rectCursor.left + SLIDER_CURSOR_WIDTH;
	m_rectCursor.top		= rect->top   + (rect->Height() - SLIDER_CURSOR_HEIGHT) / 2;
	m_rectCursor.bottom		= m_rectCursor.top + SLIDER_CURSOR_HEIGHT;

	DrawRect (rect, &m_brushBack, &m_penBorder);
	DrawRect (&m_rectBar, &m_brushBar);

	if (AfxGetAppSettings().fChapterMarker) {
		CAutoLock lock(&m_CBLock);

		if (m_pChapterBag && m_pChapterBag->ChapGetCount() > 1 && llMax != llMin) {
			REFERENCE_TIME rt;
			for (DWORD i = 0; i < m_pChapterBag->ChapGetCount(); ++i) {
				if (SUCCEEDED(m_pChapterBag->ChapGet(i, &rt, nullptr))) {
					__int64 pos = m_rectBar.Width() * rt / (llMax - llMin);
					if (pos < 0) {
						continue;
					}

					CRect r;
					r.left		= m_rectBar.left + (LONG)pos - SLIDER_CHAP_WIDTH / 2;
					r.top		= rect->top + (rect->Height() - SLIDER_CHAP_HEIGHT) / 2;
					r.right		= r.left + SLIDER_CHAP_WIDTH;
					r.bottom	= r.top + SLIDER_CHAP_HEIGHT;

					DrawRect(&r, &m_brushChapter);
				}
			}
		}
	}

	DrawRect (&m_rectCursor, NULL, &m_penCursor);
}

void CVMROSD::DrawFlyBar(CRect* rect)
{
	icoExit = m_pButtonsImages->ExtractIcon(0);
	DrawIconEx(m_MemDC, m_rectWnd.right-34, 10, icoExit, 0, 0, 0, NULL, DI_NORMAL);
	DestroyIcon(icoExit);

	icoClose = m_pButtonsImages->ExtractIcon(23);
	DrawIconEx(m_MemDC, m_rectWnd.right-62, 10, icoClose, 0, 0, 0, NULL, DI_NORMAL);
	DestroyIcon(icoClose);

	if (bMouseOverExitButton) {
		icoExit_a = m_pButtonsImages->ExtractIcon(1);
		DrawIconEx(m_MemDC, m_rectWnd.right-34, 10, icoExit_a, 0, 0, 0, NULL, DI_NORMAL);
		DestroyIcon(icoExit_a);
	}
	if (bMouseOverCloseButton) {
		icoClose_a = m_pButtonsImages->ExtractIcon(24);
		DrawIconEx(m_MemDC, m_rectWnd.right-62, 10, icoClose_a, 0, 0, 0, NULL, DI_NORMAL);
		DestroyIcon(icoClose_a);
	}
}

void CVMROSD::DrawMessage()
{
	if (m_BitmapInfo.bmWidth*m_BitmapInfo.bmHeight*(m_BitmapInfo.bmBitsPixel/8) == 0) {
		return;
	}

	if (m_nMessagePos != OSD_NOMESSAGE) {
		CRect		rectText (0,0,0,0);
		CRect		rectMessages;

		m_MemDC.DrawText (m_strMessage, &rectText, DT_CALCRECT);
		rectText.InflateRect(20, 10);
		switch (m_nMessagePos) {
			case OSD_TOPLEFT :
				rectMessages = CRect  (10, 10, min((rectText.right + 10),(m_rectWnd.right - 10)), (rectText.bottom + 12));
				break;
			case OSD_TOPRIGHT :
			default :
				rectMessages = CRect  (max(10,m_rectWnd.right-10-rectText.Width()), 10, m_rectWnd.right-10, rectText.bottom + 10);
				break;
		}

		//m_MemDC.BeginPath();
		//m_MemDC.RoundRect(rectMessages.left, rectMessages.top, rectMessages.right, rectMessages.bottom, 10, 10);
		//m_MemDC.EndPath();
		//m_MemDC.SelectClipPath(RGN_COPY);

		int R, G, B, R1, G1, B1, R_, G_, B_, R1_, G1_, B1_;
		R = GetRValue((AfxGetAppSettings().clrGrad1ABGR));
		G = GetGValue((AfxGetAppSettings().clrGrad1ABGR));
		B = GetBValue((AfxGetAppSettings().clrGrad1ABGR));
		R1 = GetRValue((AfxGetAppSettings().clrGrad2ABGR));
		G1 = GetGValue((AfxGetAppSettings().clrGrad2ABGR));
		B1 = GetBValue((AfxGetAppSettings().clrGrad2ABGR));
		R_ = (R+32>=255?255:R+32);
		R1_ = (R1+32>=255?255:R1+32);
		G_ = (G+32>=255?255:G+32);
		G1_ = (G1+32>=255?255:G1+32);
		B_ = (B+32>=255?255:B+32);
		B1_ = (B1+32>=255?255:B1+32);
		m_OSD_Transparent	=	AfxGetAppSettings().nOSDTransparent;
		int iBorder = AfxGetAppSettings().nOSDBorder;

		GRADIENT_RECT gr[1] = {{0, 1}};
		TRIVERTEX tv[2] = {
					{rectMessages.left, rectMessages.top, R*256, G*256, B*256, m_OSD_Transparent*256},
					{rectMessages.right, rectMessages.bottom, R1*256, G1*256, B1*256, m_OSD_Transparent*256},
				};
		m_MemDC.GradientFill(tv, 2, gr, 1, GRADIENT_FILL_RECT_V);

		if (iBorder>0) {
			GRADIENT_RECT gr2[1] = {{0, 1}};
			TRIVERTEX tv2[2] = {
					{rectMessages.left, rectMessages.top, R_*256, G_*256, B_*256, m_OSD_Transparent*256},
					{rectMessages.left+iBorder, rectMessages.bottom, R1_*256, G1_*256, B1_*256, m_OSD_Transparent*256},
				};
			m_MemDC.GradientFill(tv2, 2, gr2, 1, GRADIENT_FILL_RECT_V);

			GRADIENT_RECT gr3[1] = {{0, 1}};
			TRIVERTEX tv3[2] = {
			{rectMessages.right, rectMessages.top, R_*256, G_*256, B_*256, m_OSD_Transparent*256},
			{rectMessages.right-iBorder, rectMessages.bottom, R1_*256, G1_*256, B1_*256, m_OSD_Transparent*256},
				};
			m_MemDC.GradientFill(tv3, 2, gr3, 1, GRADIENT_FILL_RECT_V);

			GRADIENT_RECT gr4[1] = {{0, 1}};
			TRIVERTEX tv4[2] = {
			{rectMessages.left, rectMessages.top, R_*256, G_*256, B_*256, m_OSD_Transparent*256},
			{rectMessages.right, rectMessages.top+iBorder, R_*256, G_*256, B_*256, m_OSD_Transparent*256},
				};
			m_MemDC.GradientFill(tv4, 2, gr4, 1, GRADIENT_FILL_RECT_V);

			GRADIENT_RECT gr5[1] = {{0, 1}};
			TRIVERTEX tv5[2] = {
			{rectMessages.left, rectMessages.bottom, R1_*256, G1_*256, B1_*256, m_OSD_Transparent*256},
			{rectMessages.right, rectMessages.bottom-iBorder, R1_*256, G1_*256, B1_*256, m_OSD_Transparent*256},
				};
			m_MemDC.GradientFill(tv5, 2, gr5, 1, GRADIENT_FILL_RECT_V);
		}

		DWORD uFormat = DT_LEFT|DT_VCENTER|DT_NOPREFIX;

		if (rectText.right +10 >= (rectMessages.right)) {
			uFormat = uFormat|DT_END_ELLIPSIS;
		}

		CRect r;
		if (AfxGetAppSettings().fFontShadow) {
			r = rectMessages;
			r.left += 12;
			r.top += 7;
			m_MemDC.SetTextColor(RGB(16,24,32));
			m_MemDC.DrawText (m_strMessage, &r, uFormat);
		}

		r = rectMessages;
		r.left += 10;
		r.top += 5;
		m_MemDC.SetTextColor(AfxGetAppSettings().clrFontABGR);
		m_MemDC.DrawText (m_strMessage, &r, uFormat);
	}
}

void CVMROSD::DrawDebug()
{
	if ( !m_debugMessages.IsEmpty() ) {
		CString msg, tmp;
		POSITION pos;
		pos = m_debugMessages.GetHeadPosition();
		msg.Format(_T("%s"), m_debugMessages.GetNext(pos));

		while (pos) {
			tmp = m_debugMessages.GetNext(pos);
			if ( !tmp.IsEmpty() ) {
				msg.AppendFormat(_T("\r\n%s"), tmp);
			}
		}

		CRect rectText(0,0,0,0);
		CRect rectMessages;
		m_MemDC.DrawText(msg, &rectText, DT_CALCRECT);
		rectText.InflateRect(20, 10);

		int l, r, t, b;
		l = (m_rectWnd.Width() >> 1) - (rectText.Width() >> 1) - 10;
		r = (m_rectWnd.Width() >> 1) + (rectText.Width() >> 1) + 10;
		t = (m_rectWnd.Height() >> 1) - (rectText.Height() >> 1) - 10;
		b = (m_rectWnd.Height() >> 1) + (rectText.Height() >> 1) + 10;
		rectMessages = CRect(l, t, r, b);
		DrawRect(&rectMessages, &m_debugBrushBack, &m_debugPenBorder);
		m_MemDC.DrawText(msg, &rectMessages, DT_CENTER | DT_VCENTER);
	}
}

void CVMROSD::InvalidateVMROSD()
{
	CAutoLock Lock(&m_Lock);
	CalcRect();
	if (m_BitmapInfo.bmWidth*m_BitmapInfo.bmHeight*(m_BitmapInfo.bmBitsPixel/8) == 0) {
		return;
	}

	memsetd(m_BitmapInfo.bmBits, 0xff000000, m_BitmapInfo.bmWidth*m_BitmapInfo.bmHeight*(m_BitmapInfo.bmBitsPixel/8));

	if (m_bSeekBarVisible) {
		DrawSlider(&m_rectSeekBar, m_llSeekMin, m_llSeekMax, m_llSeekPos);
	}
	if (m_bFlyBarVisible) {
		DrawFlyBar(&m_rectFlyBar);
	}
	DrawMessage();
	DrawDebug();

	if (m_pVMB) {
		m_VMR9AlphaBitmap.dwFlags &= ~VMRBITMAP_DISABLE;
		m_pVMB->SetAlphaBitmap(&m_VMR9AlphaBitmap);
	} else if (m_pMFVMB) {
		m_pMFVMB->SetAlphaBitmap (&m_MFVideoAlphaBitmap);
	}
}

void CVMROSD::UpdateSeekBarPos(CPoint point)
{
	m_llSeekPos = (point.x - m_rectBar.left) * (m_llSeekMax-m_llSeekMin) / (m_rectBar.Width() - SLIDER_CURSOR_WIDTH);
	m_llSeekPos = max (m_llSeekPos, m_llSeekMin);
	m_llSeekPos = min (m_llSeekPos, m_llSeekMax);

	if (m_pWnd) {
		AfxGetApp()->GetMainWnd()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_llSeekPos, SB_THUMBTRACK), (LPARAM)m_pWnd->m_hWnd);
	}
}

bool CVMROSD::OnMouseMove(UINT nFlags, CPoint point)
{
	bool		bRet = false;

	if (m_pVMB || m_pMFVMB) {
		if (m_bCursorMoving) {
			UpdateSeekBarPos(point);
			InvalidateVMROSD();
		} else if (!m_bSeekBarVisible && AfxGetAppSettings().fIsFSWindow && m_rectSeekBar.PtInRect(point)) {
			m_bSeekBarVisible = true;
			InvalidateVMROSD();
		} else if (!m_bFlyBarVisible && AfxGetAppSettings().fIsFSWindow && m_rectFlyBar.PtInRect(point)) {
			m_bFlyBarVisible = true;
			InvalidateVMROSD();
		} else if (m_bFlyBarVisible && AfxGetAppSettings().fIsFSWindow && m_rectFlyBar.PtInRect(point)) {
			if (!bMouseOverExitButton && m_rectExitButton.PtInRect(point)) {
				bMouseOverCloseButton	= false;
				bMouseOverExitButton	= true;
				SetCursor(LoadCursor(NULL, IDC_HAND));
				InvalidateVMROSD();
			} else if (!bMouseOverCloseButton && m_rectCloseButton.PtInRect(point)) {
				bMouseOverExitButton	= false;
				bMouseOverCloseButton	= true;
				SetCursor(LoadCursor(NULL, IDC_HAND));
				InvalidateVMROSD();
			} else if ((bMouseOverCloseButton && !m_rectCloseButton.PtInRect(point)) || (bMouseOverExitButton && !m_rectExitButton.PtInRect(point))) {
				bMouseOverExitButton	= false;
				bMouseOverCloseButton	= false;
				InvalidateVMROSD();
			} else if (m_rectCloseButton.PtInRect(point) || m_rectExitButton.PtInRect(point)) {
				SetCursor(LoadCursor(NULL, IDC_HAND));
			}
		} else if (m_bFlyBarVisible && !m_rectFlyBar.PtInRect(point)) {
			m_bFlyBarVisible = false;
			bMouseOverCloseButton = false;
			bMouseOverExitButton = false;
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			if (m_pWnd) {
				::KillTimer(m_pWnd->m_hWnd, (UINT_PTR)this);
				::SetTimer(m_pWnd->m_hWnd, (UINT_PTR)this, 1000, (TIMERPROC)TimerFunc);
			}
			InvalidateVMROSD();
		} else if (m_bSeekBarVisible && !m_rectSeekBar.PtInRect(point)) {
			m_bSeekBarVisible = false;
			// Add new timer for removing any messages
			if (m_pWnd) {
				::KillTimer(m_pWnd->m_hWnd, (UINT_PTR)this);
				::SetTimer(m_pWnd->m_hWnd, (UINT_PTR)this, 1000, (TIMERPROC)TimerFunc);
			}
			InvalidateVMROSD();
		} else {
			bRet = false;
		}
	}

	return bRet;
}

bool CVMROSD::OnLButtonDown(UINT nFlags, CPoint point)
{
	bool		bRet = false;

	if (m_pVMB || m_pMFVMB) {
		if (m_rectCursor.PtInRect (point)) {
			m_bCursorMoving	= true;
			bRet			= true;
		} else if (m_rectExitButton.PtInRect(point) || m_rectCloseButton.PtInRect(point)) {
			bRet			= true;
		} else if (m_rectSeekBar.PtInRect(point)) {
			bRet			= true;
			UpdateSeekBarPos(point);
			InvalidateVMROSD();
		}
	}

	return bRet;
}

bool CVMROSD::OnLButtonUp(UINT nFlags, CPoint point)
{
	bool		bRet = false;

	if (m_pVMB || m_pMFVMB) {
		m_bCursorMoving = false;

		if (m_rectFlyBar.PtInRect(point)) {

			if (m_rectExitButton.PtInRect(point)) {
				AfxGetApp()->GetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_EXIT); // Alt+X
			}

			if (m_rectCloseButton.PtInRect(point)) {
				AfxGetApp()->GetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_CLOSEPLAYLIST); //Ctrl+C
			}

		}

		bRet = (m_rectCursor.PtInRect (point) || m_rectSeekBar.PtInRect(point));
	}

	return bRet;
}

__int64 CVMROSD::GetPos() const
{
	return m_llSeekPos;
}

void CVMROSD::SetPos(__int64 pos)
{
	m_llSeekPos = pos;
}

void CVMROSD::SetRange(__int64 start,  __int64 stop)
{
	m_llSeekMin	= start;
	m_llSeekMax = stop;
}

void CVMROSD::GetRange(__int64& start, __int64& stop)
{
	start	= m_llSeekMin;
	stop	= m_llSeekMax;
}

void CVMROSD::TimerFunc(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime)
{
	CVMROSD* pVMROSD = (CVMROSD*)nIDEvent;

	if (pVMROSD) {
		pVMROSD->SetVisible(false);
		pVMROSD->ClearMessage();
	}
	::KillTimer(hWnd, nIDEvent);
}

void CVMROSD::ClearMessage(bool hide)
{
	CAutoLock Lock(&m_Lock);

	if (m_bSeekBarVisible || m_bFlyBarVisible) {
		return;
	}

	m_strMessageCashed.Empty();

	if (!hide) {
		m_nMessagePos = OSD_NOMESSAGE;
	}

	if (m_pVMB) {
		DWORD dwBackup				= (m_VMR9AlphaBitmap.dwFlags | VMRBITMAP_DISABLE);
		m_VMR9AlphaBitmap.dwFlags	= VMRBITMAP_DISABLE;
		m_pVMB->SetAlphaBitmap(&m_VMR9AlphaBitmap);
		m_VMR9AlphaBitmap.dwFlags	= dwBackup;
	} else if (m_pMFVMB) {
		m_pMFVMB->ClearAlphaBitmap();
	} else if (m_pMVTO) {
		m_pMVTO->OsdClearMessage();
	} else {
		if (::IsWindow(m_hWnd) && IsWindowVisible()) {
			PostMessage(WM_HIDE);
		}
	}
}

void CVMROSD::DisplayMessage(OSD_MESSAGEPOS nPos, LPCTSTR strMsg, int nDuration, int FontSize, CString OSD_Font)
{
	if (!m_bShowMessage) {
		return;
	}

	if (m_pVMB || m_pMFVMB) {
		if (nPos != OSD_DEBUG) {
			m_nMessagePos	= nPos;
			m_strMessage	= strMsg;
		} else {
			m_debugMessages.AddTail(strMsg);
			if (m_debugMessages.GetCount() > 20) {
				m_debugMessages.RemoveHead();
			}
			nDuration = -1;
		}

		int temp_m_FontSize		= m_FontSize;
		CString temp_m_OSD_Font	= m_OSD_Font;

		m_FontSize = FontSize ? FontSize : AfxGetAppSettings().nOSDSize;

		if (m_FontSize < 10 || m_FontSize > 26) {
			m_FontSize = 20;
		}

		m_OSD_Font = OSD_Font.IsEmpty() ? AfxGetAppSettings().strOSDFont : OSD_Font;

		if (/*(temp_m_FontSize != m_FontSize) || (temp_m_OSD_Font != m_OSD_Font)*/TRUE) {
			if (m_MainFont.GetSafeHandle()) {
				m_MainFont.DeleteObject();
			}

			LOGFONT lf;
			memset(&lf, 0, sizeof(lf));
			lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
			LPCTSTR fonts[] = {m_OSD_Font};
			int fonts_size[] = {m_FontSize*10};
			_tcscpy_s(lf.lfFaceName, fonts[0]);
			lf.lfHeight = fonts_size[0];
			lf.lfQuality = AfxGetAppSettings().fFontAA ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY;

			m_MainFont.CreatePointFontIndirect(&lf,&m_MemDC);
			m_MemDC.SelectObject(m_MainFont);
		}

		if (m_pWnd) {
			::KillTimer(m_pWnd->m_hWnd, (UINT_PTR)this);
			if (nDuration != -1) {
				::SetTimer(m_pWnd->m_hWnd, (UINT_PTR)this, nDuration, (TIMERPROC)TimerFunc);
			}
		}

		InvalidateVMROSD();
	} else if (m_pMVTO) {
		m_pMVTO->OsdDisplayMessage(strMsg, nDuration);
	} else if (m_pWnd) {
		if (nPos != OSD_DEBUG) {
			m_nMessagePos	= nPos;
			m_strMessage	= strMsg;
		} else {
			m_debugMessages.AddTail(strMsg);
			if ( m_debugMessages.GetCount() > 20 ) {
				m_debugMessages.RemoveHead();
			}
			nDuration = -1;
		}

		int temp_m_FontSize		= m_FontSize;
		CString temp_m_OSD_Font	= m_OSD_Font;

		m_FontSize = FontSize ? FontSize : AfxGetAppSettings().nOSDSize;

		if (m_FontSize < 10 || m_FontSize > 26) {
			m_FontSize = 20;
		}

		m_OSD_Font = OSD_Font.IsEmpty() ? AfxGetAppSettings().strOSDFont : OSD_Font;

		if (m_pWnd) {
			::KillTimer(m_pWnd->m_hWnd, (UINT_PTR)this);
			if (nDuration != -1) {
				m_bVisibleMessage = true;
				::SetTimer(m_pWnd->m_hWnd, (UINT_PTR)this, nDuration, (TIMERPROC)TimerFunc);
			}

			SetWindowPos(NULL, 0, 0, 0, 0, DEFFLAGS | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			PostMessage(WM_OSD_DRAW);
		}
	}
}

void CVMROSD::DebugMessage(LPCTSTR format, ...)
{
	CString tmp;
	va_list argList;
	va_start(argList, format);
	tmp.FormatV(format, argList);
	va_end(argList);

	DisplayMessage(OSD_DEBUG, tmp);
}

void CVMROSD::HideMessage(bool hide)
{
	if (m_pVMB || m_pMFVMB) {
		if (hide) {
			ClearMessage(true);
		} else {
			InvalidateVMROSD();
		}
	} else {
		if (hide) {
			ClearMessage(true);
		} else {
			if (m_bVisibleMessage) {
				SetWindowPos(NULL, 0, 0, 0, 0, DEFFLAGS | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			}
			PostMessage(WM_OSD_DRAW);
		}
	}
}

void CVMROSD::HideExclusiveBars()
{
	if (m_pVMB || m_pMFVMB) {

		if (m_bFlyBarVisible || m_bSeekBarVisible) {
			m_bFlyBarVisible	= false;
			m_bSeekBarVisible	= false;
			if (m_pWnd) {
				::KillTimer(m_pWnd->m_hWnd, (UINT_PTR)this);
				::SetTimer(m_pWnd->m_hWnd, (UINT_PTR)this, 1000, (TIMERPROC)TimerFunc);
			}
			InvalidateVMROSD();
		}
	}
}

BOOL CVMROSD::PreTranslateMessage(MSG* pMsg)
{

	if (m_pWnd) {
		switch (pMsg->message) {
			case WM_LBUTTONDOWN :
			case WM_LBUTTONDBLCLK :
			case WM_MBUTTONDOWN :
			case WM_MBUTTONUP :
			case WM_MBUTTONDBLCLK :
			case WM_RBUTTONDOWN :
			case WM_RBUTTONUP :
			case WM_RBUTTONDBLCLK :
				m_pWnd->SetFocus();
			break;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

BOOL CVMROSD::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs)) {
		return FALSE;
	}

	cs.style &= ~WS_BORDER;

	return TRUE;
}

int CVMROSD::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	return 0;
}

BOOL CVMROSD::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CVMROSD::OnPaint()
{
	CPaintDC dc(this);
	PostMessage(WM_OSD_DRAW);
}

void CVMROSD::DrawWnd()
{
	CAutoLock Lock(&m_Lock);

	if (!IsWindowVisible() || !m_pWnd || m_OSDType != OSD_TYPE_GDI || m_strMessage.IsEmpty()) {
		return;
	}

	if (m_strMessageCashed == m_strMessage && m_MainWndRectCashed == m_MainWndRect) {
		return;
	}

	if (IsWindowVisible() && IsWindowEnabled()) {
		m_strMessageCashed	= m_strMessage;
		m_MainWndRectCashed	= m_MainWndRect;
	}

	CClientDC dc(this);

	CDC temp_DC;
	temp_DC.CreateCompatibleDC(&dc);
	CBitmap temp_BM;
	temp_BM.CreateCompatibleBitmap(&temp_DC, m_MainWndRect.Width(), m_MainWndRect.Height());
	CBitmap* temp_pOldBmt = temp_DC.SelectObject(&temp_BM);

	if (m_MainFont.GetSafeHandle()) {
		m_MainFont.DeleteObject();
	}

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
	LPCTSTR fonts[]		= {m_OSD_Font};
	int fonts_size[]	= {m_FontSize*10};
	lf.lfHeight			= fonts_size[0];
	lf.lfQuality		= AfxGetAppSettings().fFontAA ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY;
	_tcscpy_s(lf.lfFaceName, fonts[0]);

	m_MainFont.CreatePointFontIndirect(&lf,&temp_DC);
	temp_DC.SelectObject(m_MainFont);

	CRect rectText;
	CRect rectMessages;
	temp_DC.DrawText(m_strMessage, &rectText, DT_CALCRECT);
	rectText.InflateRect(0, 0, 10, 10);

	switch (m_nMessagePos) {
		case OSD_TOPLEFT :
			rectMessages = CRect(0, 0, min((rectText.right + 10), m_MainWndRect.Width() - 20), min((rectText.bottom + 2), m_MainWndRect.Height() - 20));
			break;
		case OSD_TOPRIGHT :
		default :
			int imax = max(0, m_MainWndRect.Width() - rectText.Width() - 30);
			rectMessages = CRect(imax, 0, (m_MainWndRect.Width() - 20) + imax, min((rectText.bottom + 2), m_MainWndRect.Height() - 20));
			break;
	}

	temp_DC.SelectObject(temp_pOldBmt);
	temp_BM.DeleteObject();
	temp_DC.DeleteDC();

	CRect wr(m_MainWndRect.left + 10 + rectMessages.left, m_MainWndRect.top + 10, rectMessages.Width() - rectMessages.left, rectMessages.Height());
	SetWindowPos(NULL, wr.left, wr.top, wr.right, wr.bottom, DEFFLAGS);

	CRect rcBar;
	GetClientRect(&rcBar);
	//rcBar = rectMessages;

	CDC mdc;
	mdc.CreateCompatibleDC(&dc);
	CBitmap bm;
	bm.CreateCompatibleBitmap(&dc, rcBar.Width(), rcBar.Height());
	CBitmap* pOldBm = mdc.SelectObject(&bm);
	mdc.SetBkMode(TRANSPARENT);
	//mdc.FillSolidRect(rcBar, RGB(0,0,0)); // transparent color (LWA_COLORKEY)

	if (m_nMessagePos != OSD_NOMESSAGE) {

		if (m_MainFont.GetSafeHandle()) {
			m_MainFont.DeleteObject();
		}

		m_MainFont.CreatePointFontIndirect(&lf,&mdc);
		mdc.SelectObject(m_MainFont);


		int R, G, B, R1, G1, B1, R_, G_, B_, R1_, G1_, B1_;
		R	= GetRValue((AfxGetAppSettings().clrGrad1ABGR));
		G	= GetGValue((AfxGetAppSettings().clrGrad1ABGR));
		B	= GetBValue((AfxGetAppSettings().clrGrad1ABGR));
		R1	= GetRValue((AfxGetAppSettings().clrGrad2ABGR));
		G1	= GetGValue((AfxGetAppSettings().clrGrad2ABGR));
		B1	= GetBValue((AfxGetAppSettings().clrGrad2ABGR));
		R_	= (R+32  >= 255 ? 255 : R+32);
		R1_	= (R1+32 >= 255 ? 255 : R1+32);
		G_	= (G+32  >= 255 ? 255 : G+32);
		G1_	= (G1+32 >= 255 ? 255 : G1+32);
		B_	= (B+32  >= 255 ? 255 : B+32);
		B1_	= (B1+32 >= 255 ? 255 : B1+32);

		m_OSD_Transparent	= 255;//AfxGetAppSettings().nOSDTransparent;
		int iBorder			= AfxGetAppSettings().nOSDBorder;

		GRADIENT_RECT gr[1] = {{0, 1}};
		TRIVERTEX tv[2] = {
			{rcBar.left, rcBar.top, R*256, G*256, B*256, m_OSD_Transparent*256},
			{rcBar.right, rcBar.bottom, R1*256, G1*256, B1*256, m_OSD_Transparent*256},
		};
		mdc.GradientFill(tv, 2, gr, 1, GRADIENT_FILL_RECT_V);

		if (iBorder > 0) {
			GRADIENT_RECT gr2[1] = {{0, 1}};
			TRIVERTEX tv2[2] = {
				{rcBar.left, rcBar.top, R_*256, G_*256, B_*256, m_OSD_Transparent*256},
				{rcBar.left+iBorder, rcBar.bottom, R1_*256, G1_*256, B1_*256, m_OSD_Transparent*256},
			};
			mdc.GradientFill(tv2, 2, gr2, 1, GRADIENT_FILL_RECT_V);

			GRADIENT_RECT gr3[1] = {{0, 1}};
			TRIVERTEX tv3[2] = {
				{rcBar.right, rcBar.top, R_*256, G_*256, B_*256, m_OSD_Transparent*256},
				{rcBar.right-iBorder, rcBar.bottom, R1_*256, G1_*256, B1_*256, m_OSD_Transparent*256},
			};
			mdc.GradientFill(tv3, 2, gr3, 1, GRADIENT_FILL_RECT_V);

			GRADIENT_RECT gr4[1] = {{0, 1}};
			TRIVERTEX tv4[2] = {
				{rcBar.left, rcBar.top, R_*256, G_*256, B_*256, m_OSD_Transparent*256},
				{rcBar.right, rcBar.top+iBorder, R_*256, G_*256, B_*256, m_OSD_Transparent*256},
			};
			mdc.GradientFill(tv4, 2, gr4, 1, GRADIENT_FILL_RECT_V);

			GRADIENT_RECT gr5[1] = {{0, 1}};
			TRIVERTEX tv5[2] = {
				{rcBar.left, rcBar.bottom, R1_*256, G1_*256, B1_*256, m_OSD_Transparent*256},
				{rcBar.right, rcBar.bottom-iBorder, R1_*256, G1_*256, B1_*256, m_OSD_Transparent*256},
			};
			mdc.GradientFill(tv5, 2, gr5, 1, GRADIENT_FILL_RECT_V);
		}

		DWORD uFormat = DT_LEFT|DT_TOP|DT_END_ELLIPSIS|DT_NOPREFIX;

		CRect r;

		if (AfxGetAppSettings().fFontShadow) {
			r			= rcBar;
			r.left		= 12;
			r.top		= 7;
			r.bottom	+= rectText.Height();

			mdc.SetTextColor(RGB(16,24,32));
			mdc.DrawText(m_strMessage, &r, uFormat);
		}

		r			= rcBar;
		r.left		= 10;
		r.top		= 5;
		r.bottom	+= rectText.Height();

		mdc.SetTextColor(AfxGetAppSettings().clrFontABGR);
		mdc.DrawText(m_strMessage, m_strMessage.GetLength(), &r, uFormat);

		/*
		// GDI+ handling

		using namespace Gdiplus;
		Graphics graphics(mdc.GetSafeHdc());
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		CString ss(message);
		CString f(m_OSD_Font);
		Font font(mdc);

		FontFamily fontFamily;
		font.GetFamily(&fontFamily);

		StringFormat strformat;
		//strformat.SetAlignment((StringAlignment) 0);
		//strformat.SetLineAlignment(StringAlignmentCenter);
		strformat.SetFormatFlags(StringFormatFlagsNoWrap);
		strformat.SetTrimming(StringTrimmingEllipsisCharacter);//SetFormatFlags(StringFormatFlagsLineLimit );
		RectF p(r.left+10, r.top, r.Width(), r.Height());

		REAL enSize = font.GetSize();

		GraphicsPath path;
		path.AddString(ss.AllocSysString(), ss.GetLength(), &fontFamily,
		FontStyleRegular, enSize, p, &strformat );

		Pen pen(Color(76,80,86), 5);
		pen.SetLineJoin(LineJoinRound);
		graphics.DrawPath(&pen, &path);

		//for(int i=1; i<8; ++i)
		//{
		//	Pen pen(Color(32, 32, 28, 30), i);
		//	pen.SetLineJoin(LineJoinRound);
		//	graphics.DrawPath(&pen, &path);
		//}


		LinearGradientBrush brush(Gdiplus::Rect(r.left, r.top, r.Width(), r.Height()),
			Color(255,255,255), Color(217,229,247), LinearGradientModeVertical);
		graphics.FillPath(&brush, &path);
		*/
	}

	dc.BitBlt(0, 0, rcBar.Width(), rcBar.Height(), &mdc, 0, 0, SRCCOPY);

	mdc.SelectObject(pOldBm);
	bm.DeleteObject();
	mdc.DeleteDC();
}

void CVMROSD::SetChapterBag(CComPtr<IDSMChapterBag>& pCB)
{
	CAutoLock lock(&m_CBLock);

	if (pCB) {
		m_pChapterBag.Release();
		pCB.CopyTo(&m_pChapterBag);
	}
}
