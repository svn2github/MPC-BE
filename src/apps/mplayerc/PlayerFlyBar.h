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

#pragma once

#include "PngImage.h"

class CFlyBar : public CWnd
{
public:
	CFlyBar();
	~CFlyBar();

	HRESULT Create(CWnd* pWnd);

	int iw;

	void CalcButtonsRect();

	DECLARE_DYNAMIC(CFlyBar)

private:
	CRect r_ExitIcon;
	CRect r_MinIcon;
	CRect r_RestoreIcon;
	CRect r_SettingsIcon;
	CRect r_InfoIcon;
	CRect r_FSIcon;
	CRect r_LockIcon;

	CToolTipCtrl m_tooltip;
	CImageList *m_pButtonsImages;

	int bt_idx;

	void DrawButton(CDC *pDC, int x, int y, int z);
	void UpdateWnd(CPoint point);
	void DrawWnd();

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};
