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
#include "PPageSubtitles.h"
#include "ISDb.h"

// CPPageSubtitles dialog

IMPLEMENT_DYNAMIC(CPPageSubtitles, CPPageBase)

CPPageSubtitles::CPPageSubtitles()
	: CPPageBase(CPPageSubtitles::IDD, CPPageSubtitles::IDD)
	, m_fAutoloadSubtitles(FALSE)
	, m_fPrioritizeExternalSubtitles(FALSE)
	, m_fDisableInternalSubtitles(FALSE)
	, m_fAutoReloadExtSubtitles(FALSE)
	, m_fUseSybresync(FALSE)
{

}

CPPageSubtitles::~CPPageSubtitles()
{
}

void CPPageSubtitles::DoDataExchange(CDataExchange* pDX)
{
	CPPageBase::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK4, m_fAutoloadSubtitles);
	DDX_Check(pDX, IDC_CHECK1, m_fPrioritizeExternalSubtitles);
	DDX_Check(pDX, IDC_CHECK2, m_fDisableInternalSubtitles);
	DDX_Check(pDX, IDC_CHECK3, m_fAutoReloadExtSubtitles);
	DDX_Check(pDX, IDC_CHECK_SUBRESYNC, m_fUseSybresync);
	DDX_Text(pDX, IDC_EDIT1, m_szAutoloadPaths);
	DDX_Control(pDX, IDC_COMBO1, m_ISDbCombo);
	DDX_CBString(pDX, IDC_COMBO1, m_ISDb);
}

BOOL CPPageSubtitles::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	m_fAutoloadSubtitles			= s.fAutoloadSubtitles;
	m_fPrioritizeExternalSubtitles	= s.fPrioritizeExternalSubtitles;
	m_fDisableInternalSubtitles		= s.fDisableInternalSubtitles;
	m_fAutoReloadExtSubtitles		= s.fAutoReloadExtSubtitles;
	m_fUseSybresync					= s.fUseSybresync;
	m_szAutoloadPaths				= s.strSubtitlePaths;

	m_ISDb = s.strISDb;
	m_ISDbCombo.AddString(m_ISDb);

	if (m_ISDb.CompareNoCase(_T("www.opensubtitles.org/isdb"))) {
		m_ISDbCombo.AddString(_T("www.opensubtitles.org/isdb"));
	}

	UpdateData(FALSE);

	CreateToolTip();

	return TRUE;
}

BOOL CPPageSubtitles::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.fAutoloadSubtitles			= !!m_fAutoloadSubtitles;
	s.fPrioritizeExternalSubtitles	= !!m_fPrioritizeExternalSubtitles;
	s.fDisableInternalSubtitles		= !!m_fDisableInternalSubtitles;
	s.fAutoReloadExtSubtitles		= !!m_fAutoReloadExtSubtitles;
	s.fUseSybresync					= !!m_fUseSybresync;
	s.strSubtitlePaths				= m_szAutoloadPaths;

	s.strISDb = m_ISDb;
	s.strISDb.TrimRight('/');

	return __super::OnApply();
}

BEGIN_MESSAGE_MAP(CPPageSubtitles, CPPageBase)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateButton2)
	ON_CBN_EDITCHANGE(IDC_COMBO1, OnURLModified)
	//ON_UPDATE_COMMAND_UI(IDC_CHECK1, OnUpdateISRSelect2)
	ON_UPDATE_COMMAND_UI(IDC_CHECK2, OnUpdateISRSelect2)
	ON_UPDATE_COMMAND_UI(IDC_CHECK3, OnUpdateISRSelect2)
	ON_UPDATE_COMMAND_UI(IDC_CHECK4, OnUpdateISRSelect)
	ON_UPDATE_COMMAND_UI(IDC_CHECK_SUBRESYNC, OnUpdateISRSelect2)
END_MESSAGE_MAP()

void CPPageSubtitles::OnBnClickedButton1()
{
	m_szAutoloadPaths = DEFAULT_SUBTITLE_PATHS;

	UpdateData(FALSE);

	SetModified();
}

void CPPageSubtitles::OnBnClickedButton2()
{
	CString ISDb, ver, msg, str;

	m_ISDbCombo.GetWindowText(ISDb);
	ISDb.TrimRight('/');

	ver.Format(_T("ISDb v%d"), ISDb_PROTOCOL_VERSION);

	CWebTextFile wtf;
	UINT nIconType = MB_ICONEXCLAMATION;

	if (wtf.Open(_T("http://") + ISDb + _T("/test.php")) && wtf.ReadString(str) && str == ver) {
		msg = ResStr(IDS_PPSDB_URLCORRECT);
		nIconType = MB_ICONINFORMATION;
	} else if (str.Find(_T("ISDb v")) == 0) {
		msg = ResStr(IDS_PPSDB_PROTOCOLERR);
	} else {
		msg = ResStr(IDS_PPSDB_BADURL);
	}

	AfxMessageBox(msg, nIconType | MB_OK);
}

void CPPageSubtitles::OnUpdateButton2(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_ISDbCombo.GetWindowTextLength() > 0);
}

void CPPageSubtitles::OnURLModified()
{
	SetModified();
}

void CPPageSubtitles::OnUpdateISRSelect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(AfxGetAppSettings().IsISRSelect());
}

void CPPageSubtitles::OnUpdateISRSelect2(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(AfxGetAppSettings().IsISRSelect() && IsDlgButtonChecked(IDC_CHECK4));
}