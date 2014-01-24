/*
 * (C) 2014 see Authors.txt
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
#include "AboutDlg.h"
#include "../../DSUtil/WinAPIUtils.h"

extern "C" char *GetFFmpegCompiler();
extern "C" char *GetlibavcodecVersion();

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	m_hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
}

CAboutDlg::~CAboutDlg()
{
	if (m_hIcon) {
		DestroyIcon(m_hIcon);
	}
}

BOOL CAboutDlg::OnInitDialog()
{
	UpdateData();

	__super::OnInitDialog();

#ifdef _WIN64
	m_appname += _T(" (64-bit)");
#endif

	m_strVersionNumber = AfxGetMyApp()->m_strVersion;
#if DBOXVersion
	m_strVersionNumber.Append(_T(" (D-BOX)"));
#endif

#if defined(__INTEL_COMPILER)
#if (__INTEL_COMPILER >= 1210)
	m_MPCCompiler = _T("ICL ") MAKE_STR(__INTEL_COMPILER) _T(" Build ") MAKE_STR(__INTEL_COMPILER_BUILD_DATE);
#else
#error Compiler is not supported!
#endif
#elif defined(_MSC_VER)
#if (_MSC_VER == 1800)		// 2013
#if (_MSC_FULL_VER == 180021005)
	m_MPCCompiler = _T("MSVC 2013 Update 1");
#else
	m_MPCCompiler = _T("MSVC 2013");
#endif
#elif (_MSC_VER == 1700)	// 2012
#if (_MSC_FULL_VER == 170061030)
    m_MPCCompiler = _T("MSVC 2012.4");
#elif (_MSC_FULL_VER == 170060930)
    m_MPCCompiler = _T("MSVC 2012.4 RC4");
#elif (_MSC_FULL_VER == 170060830)
    m_MPCCompiler = _T("MSVC 2012.4 RC3");
#elif (_MSC_FULL_VER == 170060610)
    m_MPCCompiler = _T("MSVC 2012.3");
#elif (_MSC_FULL_VER == 170060521)
    m_MPCCompiler = _T("MSVC 2012.3 RC2");
#elif (_MSC_FULL_VER == 170060430)
    m_MPCCompiler = _T("MSVC 2012.3 RC1");
#elif (_MSC_FULL_VER == 170060315)
    m_MPCCompiler = _T("MSVC 2012.2");
#elif (_MSC_FULL_VER == 170051106)
    m_MPCCompiler = _T("MSVC 2012.1");
#elif (_MSC_FULL_VER < 170050727)
	m_MPCCompiler = _T("MSVC 2012 Beta/RC/PR");
#else
	m_MPCCompiler = _T("MSVC 2012");
#endif
#elif (_MSC_VER == 1600)	// 2013
#if (_MSC_FULL_VER >= 160040219)
	m_MPCCompiler = _T("MSVC 2010 SP1");
#else
	m_MPCCompiler = _T("MSVC 2010");
#endif
#elif (_MSC_VER < 1600)
#error Compiler is not supported!
#endif
#else
#error Please add support for your compiler
#endif

#if (__AVX__)
	m_MPCCompiler += _T(" (AVX)");
#elif (__SSSE3__)
	m_MPCCompiler += _T(" (SSSE3)");
#elif (__SSE3__)
	m_MPCCompiler += _T(" (SSE3)");
#elif !defined(_M_X64) && defined(_M_IX86_FP)
#if (_M_IX86_FP == 2)   // /arch:SSE2 was used
	m_MPCCompiler += _T(" (SSE2)");
#elif (_M_IX86_FP == 1) // /arch:SSE was used
	m_MPCCompiler += _T(" (SSE)");
#endif
#endif

#ifdef _DEBUG
	m_MPCCompiler += _T(" Debug");
#endif

	m_strSVNNumber.Format(_T("%d"),MPC_VERSION_REV);
	m_FFmpegCompiler.Format(CA2CT(GetFFmpegCompiler()));
	m_libavcodecVersion.Format(CA2CT(GetlibavcodecVersion()));

	m_AuthorsPath = GetModulePath(false) + _T("\\Authors.txt");

	if (::PathFileExists(m_AuthorsPath)) {
		m_Credits.Replace(_T("Authors.txt"), _T("<a>Authors.txt</a>"));
	}

	if (m_hIcon != NULL) {
		((CStatic*)GetDlgItem(IDC_MAINFRAME_ICON))->SetIcon(m_hIcon);
	}

	UpdateData(FALSE);

	GetDlgItem(IDOK)->SetFocus();

	return FALSE;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP

	DDX_Text(pDX, IDC_STATIC1, m_appname);
	DDX_Text(pDX, IDC_VERSION_NUMBER, m_strVersionNumber);
	DDX_Text(pDX, IDC_SVN_NUMBER, m_strSVNNumber);
	DDX_Text(pDX, IDC_MPC_COMPILER, m_MPCCompiler);
	DDX_Text(pDX, IDC_FFMPEG_COMPILER, m_FFmpegCompiler);
	DDX_Text(pDX, IDC_LIBAVCODEC_VERSION, m_libavcodecVersion);
	DDX_Text(pDX, IDC_AUTHORS_LINK, m_Credits);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	// No message handlers
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_CLICK, IDC_SOURCEFORGE_LINK, OnHomepage)
	ON_NOTIFY(NM_CLICK, IDC_AUTHORS_LINK, OnAuthors)
END_MESSAGE_MAP()

void CAboutDlg::OnHomepage(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(m_hWnd, _T("open"), _T(MPC_VERSION_COMMENTS), NULL, NULL, SW_SHOWDEFAULT);

	*pResult = 0;
}

void CAboutDlg::OnAuthors(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(m_hWnd, _T("open"), m_AuthorsPath, NULL, NULL, SW_SHOWDEFAULT);

	*pResult = 0;
}
