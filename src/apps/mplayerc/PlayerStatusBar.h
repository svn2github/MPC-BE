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

#include "PngImage.h"
#include "StatusLabel.h"

// CPlayerStatusBar

class CPlayerStatusBar : public CDialogBar
{
	DECLARE_DYNAMIC(CPlayerStatusBar)

	CStatic m_type;
	CStatusLabel m_status, m_time;
	CBitmap m_bm;
	UINT m_bmid;

	CMPCPngImage m_BackGroundbm;

	CRect m_time_rect;
	CRect m_time_rect2;

	void Relayout();

public:
	CPlayerStatusBar();
	virtual ~CPlayerStatusBar();

	void Clear();

	void SetStatusBitmap(UINT id);
	void SetStatusMessage(CString str);
	void SetStatusTimer(CString str);
	void SetStatusTimer(REFERENCE_TIME rtNow, REFERENCE_TIME rtDur, bool fHighPrecision, const GUID& timeFormat = TIME_FORMAT_MEDIA_TIME);

	CString GetStatusTimer();
	CString GetStatusMessage();

	void ShowTimer(bool fShow);

	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	DECLARE_MESSAGE_MAP()

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
