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

#include "stdafx.h"
#include "MainFrm.h"
#include "PPageVideo.h"
#include <moreuuids.h>
#include "PPageMisc.h"
#include <psapi.h>


// CPPageMisc dialog

IMPLEMENT_DYNAMIC(CPPageMisc, CPPageBase)
CPPageMisc::CPPageMisc()
	: CPPageBase(CPPageMisc::IDD, CPPageMisc::IDD)
	, m_iBrightness(0)
	, m_iContrast(0)
	, m_iHue(0)
	, m_iSaturation(0)
	, m_nUpdaterDelay(7)
{
}

CPPageMisc::~CPPageMisc()
{
}

void CPPageMisc::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SLI_BRIGHTNESS, m_SliBrightness);
	DDX_Control(pDX, IDC_SLI_CONTRAST, m_SliContrast);
	DDX_Control(pDX, IDC_SLI_HUE, m_SliHue);
	DDX_Control(pDX, IDC_SLI_SATURATION, m_SliSaturation);
	DDX_Text(pDX, IDC_STATIC1, m_sBrightness);
	DDX_Text(pDX, IDC_STATIC2, m_sContrast);
	DDX_Text(pDX, IDC_STATIC3, m_sHue);
	DDX_Text(pDX, IDC_STATIC4, m_sSaturation);
	DDX_Control(pDX, IDC_CHECK1, m_updaterAutoCheckCtrl);
	DDX_Control(pDX, IDC_EDIT1, m_updaterDelayCtrl);
	DDX_Control(pDX, IDC_SPIN1, m_updaterDelaySpin);
	DDX_Text(pDX, IDC_EDIT1, m_nUpdaterDelay);
}

BEGIN_MESSAGE_MAP(CPPageMisc, CPPageBase)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RESET, OnBnClickedReset)
	ON_BN_CLICKED(IDC_RESET_SETTINGS, OnResetSettings)
	ON_BN_CLICKED(IDC_EXPORT_SETTINGS, OnExportSettings)
	ON_UPDATE_COMMAND_UI(IDC_EDIT1, OnUpdateDelayEditBox)
	ON_UPDATE_COMMAND_UI(IDC_SPIN1, OnUpdateDelayEditBox)
	ON_UPDATE_COMMAND_UI(IDC_STATIC5, OnUpdateDelayEditBox)
	ON_UPDATE_COMMAND_UI(IDC_STATIC6, OnUpdateDelayEditBox)
END_MESSAGE_MAP()

// CPPageMisc message handlers

BOOL CPPageMisc::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	CreateToolTip();

	m_iBrightness = s.iBrightness;
	m_iContrast   = s.iContrast;
	m_iHue        = s.iHue;
	m_iSaturation = s.iSaturation;

	m_SliBrightness.EnableWindow	(TRUE);
	m_SliBrightness.SetRange		(-100, 100, TRUE);
	m_SliBrightness.SetTic			(0);
	m_SliBrightness.SetPos			(m_iBrightness);
	m_SliBrightness.SetPageSize		(10);

	m_SliContrast.EnableWindow		(TRUE);
	m_SliContrast.SetRange			(-100, 100, TRUE);
	m_SliContrast.SetTic			(0);
	m_SliContrast.SetPos			(m_iContrast);
	m_SliContrast.SetPageSize		(10);

	m_SliHue.EnableWindow			(TRUE);
	m_SliHue.SetRange				(-180, 180, TRUE);
	m_SliHue.SetTic					(0);
	m_SliHue.SetPos					(m_iHue);
	m_SliHue.SetPageSize			(10);

	m_SliSaturation.EnableWindow	(TRUE);
	m_SliSaturation.SetRange		(-100, 100, TRUE);
	m_SliSaturation.SetTic			(0);
	m_SliSaturation.SetPos			(m_iSaturation);
	m_SliSaturation.SetPageSize		(10);

	m_iBrightness ? m_sBrightness.Format(_T("%+d"), m_iBrightness) : m_sBrightness = _T("0");
	m_iContrast   ? m_sContrast.Format  (_T("%+d"), m_iContrast)   : m_sContrast   = _T("0");
	m_iHue        ? m_sHue.Format       (_T("%+d"), m_iHue)        : m_sHue        = _T("0");
	m_iSaturation ? m_sSaturation.Format(_T("%+d"), m_iSaturation) : m_sSaturation = _T("0");

	m_updaterAutoCheckCtrl.SetCheck(s.bUpdaterAutoCheck);
	m_nUpdaterDelay = s.nUpdaterDelay;
	m_updaterDelaySpin.SetRange32(1, 365);

#ifndef _DEBUG
	GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC5)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT1)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN1)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC6)->EnableWindow(FALSE);
#endif

	UpdateData(FALSE);

	return TRUE;
}

BOOL CPPageMisc::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.iBrightness		= m_iBrightness;
	s.iContrast			= m_iContrast;
	s.iHue				= m_iHue;
	s.iSaturation		= m_iSaturation;

	s.bUpdaterAutoCheck	= !!m_updaterAutoCheckCtrl.GetCheck();
	m_nUpdaterDelay		= min(max(1, m_nUpdaterDelay), 365);
	s.nUpdaterDelay		= m_nUpdaterDelay;

	return __super::OnApply();
}

void CPPageMisc::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CMainFrame* pMainFrame = ((CMainFrame*)AfxGetMyApp()->GetMainWnd());
	UpdateData();

	if (*pScrollBar == m_SliBrightness) {
		m_iBrightness = m_SliBrightness.GetPos();
		pMainFrame->SetColorControl(ProcAmp_Brightness, m_iBrightness, m_iContrast, m_iHue, m_iSaturation);
		m_iBrightness ? m_sBrightness.Format(_T("%+d"), m_iBrightness) : m_sBrightness = _T("0");
	}
	else if (*pScrollBar == m_SliContrast) {
		m_iContrast = m_SliContrast.GetPos();
		pMainFrame->SetColorControl(ProcAmp_Contrast, m_iBrightness, m_iContrast, m_iHue, m_iSaturation);
		m_iContrast ? m_sContrast.Format(_T("%+d"), m_iContrast) : m_sContrast = _T("0");
	}
	else if (*pScrollBar == m_SliHue) {
		m_iHue = m_SliHue.GetPos();
		pMainFrame->SetColorControl(ProcAmp_Hue, m_iBrightness, m_iContrast, m_iHue, m_iSaturation);
		m_iHue ? m_sHue.Format(_T("%+d"), m_iHue) : m_sHue = _T("0");
	}
	else if (*pScrollBar == m_SliSaturation) {
		m_iSaturation = m_SliSaturation.GetPos();
		pMainFrame->SetColorControl(ProcAmp_Saturation, m_iBrightness, m_iContrast, m_iHue, m_iSaturation);
		m_iSaturation ? m_sSaturation.Format(_T("%+d"), m_iSaturation) : m_sSaturation = _T("0");
	}

	UpdateData(FALSE);

	SetModified();

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPageMisc::OnBnClickedReset()
{
	CMainFrame* pMainFrame = ((CMainFrame*)AfxGetMyApp()->GetMainWnd());

	m_iBrightness	= pMainFrame->m_ColorCintrol.GetColorControl(ProcAmp_Brightness)->DefaultValue;
	m_iContrast		= pMainFrame->m_ColorCintrol.GetColorControl(ProcAmp_Contrast)->DefaultValue;
	m_iHue			= pMainFrame->m_ColorCintrol.GetColorControl(ProcAmp_Hue)->DefaultValue;
	m_iSaturation	= pMainFrame->m_ColorCintrol.GetColorControl(ProcAmp_Saturation)->DefaultValue;

	m_SliBrightness.SetPos	(m_iBrightness);
	m_SliContrast.SetPos	(m_iContrast);
	m_SliHue.SetPos			(m_iHue);
	m_SliSaturation.SetPos	(m_iSaturation);

	m_iBrightness ? m_sBrightness.Format(_T("%+d"), m_iBrightness) : m_sBrightness = _T("0");
	m_iContrast   ? m_sContrast.Format  (_T("%+d"), m_iContrast)   : m_sContrast   = _T("0");
	m_iHue        ? m_sHue.Format       (_T("%+d"), m_iHue)        : m_sHue        = _T("0");
	m_iSaturation ? m_sSaturation.Format(_T("%+d"), m_iSaturation) : m_sSaturation = _T("0");

	((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetColorControl(ProcAmp_All, m_iBrightness, m_iContrast, m_iHue, m_iSaturation);

	UpdateData(FALSE);

	SetModified();
}

void CPPageMisc::OnUpdateDelayEditBox(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_updaterAutoCheckCtrl.GetCheck() == BST_CHECKED);
}

void CPPageMisc::OnResetSettings()
{
	if (MessageBox(ResStr(IDS_RESET_SETTINGS_WARNING), ResStr(IDS_RESET_SETTINGS), MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		AfxGetAppSettings().fReset = true;
		GetParent()->PostMessage(WM_CLOSE);
	}
}

void CPPageMisc::OnExportSettings()
{
	if (GetParent()->GetDlgItem(ID_APPLY_NOW)->IsWindowEnabled()) {
		int ret = MessageBox(ResStr(IDS_EXPORT_SETTINGS_WARNING), ResStr(IDS_EXPORT_SETTINGS), MB_ICONEXCLAMATION | MB_YESNOCANCEL);

		if (ret == IDCANCEL) {
			return;
		} else if (ret == IDYES) {
			GetParent()->PostMessage(PSM_APPLY);
		}
	}

	AfxGetMyApp()->ExportSettings();
	SetFocus();
}

void CPPageMisc::OnCancel()
{
	AppSettings& s = AfxGetAppSettings();

	((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetColorControl(ProcAmp_All, s.iBrightness, s.iContrast, s.iHue, s.iSaturation);

	__super::OnCancel();
}
