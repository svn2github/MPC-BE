/*
 * (C) 2003-2006 Gabest
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
#include "SaveTextFileDialog.h"
#include "../../DSUtil/SysVersion.h"

// CSaveTextFileDialog

IMPLEMENT_DYNAMIC(CSaveTextFileDialog, CFileDialog)
CSaveTextFileDialog::CSaveTextFileDialog(
	CTextFile::enc e,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
	LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(FALSE, lpszDefExt, lpszFileName,
				OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR,
				lpszFilter, pParentWnd),
	m_e(e)
{
	if (IsWinVistaOrLater()) {

		IFileDialogCustomize* pfdc = GetIFileDialogCustomize();

		pfdc->StartVisualGroup(IDS_TEXTFILE_ENC, ResStr(IDS_TEXTFILE_ENC));
		pfdc->AddComboBox(IDC_COMBO1);
		pfdc->AddControlItem(IDC_COMBO1, CTextFile::ASCII, _T("ANSI"));
		pfdc->AddControlItem(IDC_COMBO1, CTextFile::LE16, _T("Unicode 16-LE"));
		pfdc->AddControlItem(IDC_COMBO1, CTextFile::BE16, _T("Unicode 16-BE"));
		pfdc->AddControlItem(IDC_COMBO1, CTextFile::UTF8, _T("UTF-8"));
		pfdc->SetSelectedControlItem(IDC_COMBO1, m_e);
		pfdc->EndVisualGroup();
		pfdc->MakeProminent(IDS_TEXTFILE_ENC);

		pfdc->Release();
	} else {
		SetTemplate(0, IDD_SAVETEXTFILEDIALOGTEMPL);
	}
}

CSaveTextFileDialog::~CSaveTextFileDialog()
{
}

void CSaveTextFileDialog::DoDataExchange(CDataExchange* pDX)
{
	if (!IsWinVistaOrLater()) {
		DDX_Control(pDX, IDC_COMBO1, m_encoding);
	}

	__super::DoDataExchange(pDX);
}

BOOL CSaveTextFileDialog::OnInitDialog()
{
	__super::OnInitDialog();

	if (!IsWinVistaOrLater()) {
		m_encoding.SetItemData(m_encoding.AddString(_T("ANSI")), CTextFile::ASCII);
		m_encoding.SetItemData(m_encoding.AddString(_T("Unicode 16-LE")), CTextFile::LE16);
		m_encoding.SetItemData(m_encoding.AddString(_T("Unicode 16-BE")), CTextFile::BE16);
		m_encoding.SetItemData(m_encoding.AddString(_T("UTF-8")), CTextFile::UTF8);

		switch (m_e) {
			default:
			case CTextFile::ASCII:
				m_encoding.SetCurSel(0);
				break;
			case CTextFile::LE16:
				m_encoding.SetCurSel(1);
				break;
			case CTextFile::BE16:
				m_encoding.SetCurSel(2);
				break;
			case CTextFile::UTF8:
				m_encoding.SetCurSel(3);
				break;
		}
	}

	return TRUE;
}

BEGIN_MESSAGE_MAP(CSaveTextFileDialog, CFileDialog)
END_MESSAGE_MAP()

// CSaveTextFileDialog message handlers

BOOL CSaveTextFileDialog::OnFileNameOK()
{
	if (IsWinVistaOrLater()) {
		DWORD result;
		IFileDialogCustomize* pfdc = GetIFileDialogCustomize();
		pfdc->GetSelectedControlItem(IDC_COMBO1, &result);
		pfdc->Release();
		m_e = (CTextFile::enc)result;
	} else {
		m_e = (CTextFile::enc)m_encoding.GetItemData(m_encoding.GetCurSel());
	}

	return __super::OnFileNameOK();
}
