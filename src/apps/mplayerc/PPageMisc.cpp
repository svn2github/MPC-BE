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
#include "MainFrm.h"
#include "PPageOutput.h"
#include <moreuuids.h>
#include "PPageMisc.h"
#include <psapi.h>


// CPPageMisc dialog

IMPLEMENT_DYNAMIC(CPPageMisc, CPPageBase)
CPPageMisc::CPPageMisc()
	: CPPageBase(CPPageMisc::IDD, CPPageMisc::IDD)
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
}

BEGIN_MESSAGE_MAP(CPPageMisc, CPPageBase)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RESET, OnBnClickedReset)
	ON_BN_CLICKED(IDC_RESET_SETTINGS, OnResetSettings)
	ON_BN_CLICKED(IDC_EXPORT_SETTINGS, OnExportSettings)
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
	m_SliBrightness.SetRange		(-100, 100, true);
	m_SliBrightness.SetTic			(0);
	m_SliBrightness.SetPos			(m_iBrightness);

	m_SliContrast.EnableWindow		(TRUE);
	m_SliContrast.SetRange			(-100, 100, true);
	m_SliContrast.SetTic			(0);
	m_SliContrast.SetPos			(m_iContrast);

	m_SliHue.EnableWindow			(TRUE);
	m_SliHue.SetRange				(-180, 180, true);
	m_SliHue.SetTic					(0);
	m_SliHue.SetPos					(m_iHue);

	m_SliSaturation.EnableWindow	(TRUE);
	m_SliSaturation.SetRange		(-100, 100, true);
	m_SliSaturation.SetTic			(0);
	m_SliSaturation.SetPos			(m_iSaturation);

	m_iBrightness ? m_sBrightness.Format(_T("%+d"), m_iBrightness) : m_sBrightness = _T("0");
	m_iContrast   ? m_sContrast.Format  (_T("%+d"), m_iContrast)   : m_sContrast   = _T("0");
	m_iHue        ? m_sHue.Format       (_T("%+d"), m_iHue)        : m_sHue        = _T("0");
	m_iSaturation ? m_sSaturation.Format(_T("%+d"), m_iSaturation) : m_sSaturation = _T("0");

	UpdateData(FALSE);

	return TRUE;
}

BOOL CPPageMisc::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.iBrightness				= m_iBrightness;
	s.iContrast					= m_iContrast;
	s.iHue						= m_iHue;
	s.iSaturation				= m_iSaturation;

	return __super::OnApply();
}

void CPPageMisc::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateData();

	if (*pScrollBar == m_SliBrightness) {
		m_iBrightness = m_SliBrightness.GetPos();
		((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetColorControl(ProcAmp_Brightness, m_iBrightness, m_iContrast, m_iHue, m_iSaturation);
		m_iBrightness ? m_sBrightness.Format(_T("%+d"), m_iBrightness) : m_sBrightness = _T("0");
	}
	else if (*pScrollBar == m_SliContrast) {
		m_iContrast = m_SliContrast.GetPos();
		((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetColorControl(ProcAmp_Contrast, m_iBrightness, m_iContrast, m_iHue, m_iSaturation);
		m_iContrast ? m_sContrast.Format(_T("%+d"), m_iContrast) : m_sContrast = _T("0");
	}
	else if (*pScrollBar == m_SliHue) {
		m_iHue = m_SliHue.GetPos();
		((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetColorControl(ProcAmp_Hue, m_iBrightness, m_iContrast, m_iHue, m_iSaturation);
		m_iHue ? m_sHue.Format(_T("%+d"), m_iHue) : m_sHue = _T("0");
	}
	else if (*pScrollBar == m_SliSaturation) {
		m_iSaturation = m_SliSaturation.GetPos();
		((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetColorControl(ProcAmp_Saturation, m_iBrightness, m_iContrast, m_iHue, m_iSaturation);
		m_iSaturation ? m_sSaturation.Format(_T("%+d"), m_iSaturation) : m_sSaturation = _T("0");
	}

	UpdateData(FALSE);

	SetModified();

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPageMisc::OnBnClickedReset()
{
	m_iBrightness	= AfxGetMyApp()->GetColorControl(ProcAmp_Brightness)->DefaultValue;
	m_iContrast		= AfxGetMyApp()->GetColorControl(ProcAmp_Contrast)->DefaultValue;
	m_iHue			= AfxGetMyApp()->GetColorControl(ProcAmp_Hue)->DefaultValue;
	m_iSaturation	= AfxGetMyApp()->GetColorControl(ProcAmp_Saturation)->DefaultValue;

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
