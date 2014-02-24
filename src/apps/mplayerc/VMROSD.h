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

#include <atlbase.h>
#include <d3d9.h>
#include <evr9.h>
#include <vmr9.h>
#include <madVRAllocatorPresenter.h>
#include "PngImage.h"
#include "..\..\DSUtil\DSMPropertyBag.h"
//#include <Gdiplus.h>

#define WM_HIDE			(WM_USER + 1001)
#define WM_OSD_DRAW		(WM_USER + 1002)

typedef enum {
	OSD_TRANSPARENT,
	OSD_BACKGROUND,
	OSD_BORDER,
	OSD_TEXT,
	OSD_BAR,
	OSD_CURSOR,
	OSD_DEBUGCLR,
	OSD_LAST
} OSD_COLORS;

typedef enum {
	OSD_NOMESSAGE,
	OSD_TOPLEFT,
	OSD_TOPRIGHT,
	OSD_DEBUG,
} OSD_MESSAGEPOS;

typedef enum {
	OSD_TYPE_NONE,
	OSD_TYPE_BITMAP,
	OSD_TYPE_MADVR,
	OSD_TYPE_GDI
} OSD_TYPE;

class CVMROSD : public CWnd
{
public:
	CVMROSD();
	~CVMROSD();
	void Start(CWnd* pWnd, IVMRMixerBitmap9* pVMB);
	void Start(CWnd* pWnd, IMFVideoMixerBitmap* pVMB);
	void Start(CWnd* pWnd, IMadVRTextOsd* pMVTO);
	void Start(CWnd* pWnd);
	void Stop();

	void DisplayMessage(OSD_MESSAGEPOS nPos, LPCTSTR strMsg, int nDuration = 5000, int FontSize = 0, CString OSD_Font = _T(""));
	void DebugMessage(LPCTSTR format, ...);
	void ClearMessage(bool hide = false);
	void HideMessage(bool hide);
	void HideExclusiveBars();

	void EnableShowMessage(bool bValue = true)	{ m_bShowMessage = bValue; };
	void SetVisible(bool bValue)				{ m_bVisibleMessage = bValue; };

	__int64 GetPos() const;
	void SetPos(__int64 pos);
	void SetRange(__int64 start,  __int64 stop);
	void GetRange(__int64& start, __int64& stop);

	void OnSize(UINT nType, int cx, int cy);
	bool OnMouseMove(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool bMouseOverCloseButton;
	bool bMouseOverExitButton;

	void SetWndRect(CRect rc) { m_MainWndRect = rc; };

	OSD_TYPE GetOSDType() { return m_OSDType; };

	void SetChapterBag(CComPtr<IDSMChapterBag>& pCB);

	DECLARE_DYNAMIC(CVMROSD)

private :
	CComPtr<IVMRMixerBitmap9>		m_pVMB;
	CComPtr<IMFVideoMixerBitmap>	m_pMFVMB;
	CComPtr<IMadVRTextOsd>			m_pMVTO;

	CWnd*							m_pWnd;

	CCritSec			m_Lock;
	CDC					m_MemDC;
	VMR9AlphaBitmap		m_VMR9AlphaBitmap;
	MFVideoAlphaBitmap	m_MFVideoAlphaBitmap;
	BITMAP				m_BitmapInfo;

	CFont	m_MainFont;
	CPen	m_penBorder;
	CPen	m_penCursor;
	CBrush	m_brushBack;
	CBrush	m_brushBar;
	CBrush	m_brushChapter;
	CPen	m_debugPenBorder;
	CBrush	m_debugBrushBack;
	int		m_FontSize;
	CString	m_OSD_Font;
	int		m_OSD_Transparent;

	CRect		m_rectWnd;
	COLORREF	m_Color[OSD_LAST];

	CRect	m_rectSeekBar;
	CRect	m_rectFlyBar;
	CRect	m_rectCloseButton;
	CRect	m_rectExitButton;

	int			m_nButtonHeight;
	CImageList	*m_pButtonsImages;

	CRect	m_rectCursor;
	CRect	m_rectBar;
	bool	m_bCursorMoving;
	bool	m_bSeekBarVisible;
	bool	m_bFlyBarVisible;
	__int64	m_llSeekMin;
	__int64	m_llSeekMax;
	__int64	m_llSeekPos;
	HICON	icoExit;
	HICON	icoExit_a;
	HICON	icoClose;
	HICON	icoClose_a;

	bool	m_bShowMessage;
	bool	m_bVisibleMessage;

	CRect	m_MainWndRect;
	CRect	m_MainWndRectCashed;

	OSD_TYPE		m_OSDType;

	CString			m_strMessage;
	CString			m_strMessageCashed;
	OSD_MESSAGEPOS	m_nMessagePos;
	CList<CString>	m_debugMessages;

	CCritSec				m_CBLock;
	CComPtr<IDSMChapterBag>	m_pChapterBag;

	void UpdateBitmap();
	void CalcRect();
	void UpdateSeekBarPos(CPoint point);
	void DrawSlider(CRect* rect, __int64 llMin, __int64 llMax, __int64 llPos);
	void DrawFlyBar(CRect* rect);
	void DrawRect(CRect* rect, CBrush* pBrush = NULL, CPen* pPen = NULL);
	void InvalidateVMROSD();
	void DrawMessage();
	void DrawDebug();
	static void CALLBACK TimerFunc(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime);

	void Reset();

	void DrawWnd();

	// Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
	// ULONG_PTR m_gdiplusToken;

protected:
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL PreTranslateMessage(MSG* pMsg);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnPaint();
	BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnHide();
	afx_msg void OnDrawWnd();
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
};
