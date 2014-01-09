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

#include "PPageBase.h"
#include "FloatEdit.h"
#include "../../filters/switcher/AudioSwitcher/AudioSwitcher.h"


// CPPageAudioSwitcher dialog

class CPPageAudioSwitcher : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageAudioSwitcher)

private:
	CComQIPtr<IAudioSwitcherFilter> m_pASF;
	DWORD m_pSpeakerToChannelMap[AS_MAX_CHANNELS][AS_MAX_CHANNELS];
	DWORD m_dwChannelMask;

public:
	CPPageAudioSwitcher(IFilterGraph* pFG);
	virtual ~CPPageAudioSwitcher();

	enum { IDD = IDD_PPAGEAUDIOSWITCHER };

	BOOL m_fEnableAudioSwitcher;
	BOOL m_fAudioNormalize;
	BOOL m_fAudioNormalizeRecover;
	int m_AudioBoostPos;
	CSliderCtrl m_AudioBoostCtrl;
	BOOL m_fAudioTimeShift;
	CButton m_fAudioTimeShiftCtrl;
	int m_tAudioTimeShift;
	CIntEdit m_tAudioTimeShiftCtrl;
	CSpinButtonCtrl m_tAudioTimeShiftSpin;
	BOOL m_fCustomChannelMapping;
	CButton m_fCustomChannelMappingCtrl;
	CEdit m_nChannelsCtrl;
	int m_nChannels;
	CSpinButtonCtrl m_nChannelsSpinCtrl;
	CListCtrl m_list;

	CToolTipCtrl m_tooltip;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnUpdateAudioSwitcher(CCmdUI* pCmdUI);
	afx_msg void OnUpdateChannelMapping(CCmdUI* pCmdUI);
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR * pNMHDR, LRESULT * pResult);
	virtual void OnCancel();
};
