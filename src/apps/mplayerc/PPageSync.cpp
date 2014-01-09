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
#include "PPageSync.h"


IMPLEMENT_DYNAMIC(CPPageSync, CPPageBase)

CPPageSync::CPPageSync()
	: CPPageBase(CPPageSync::IDD, CPPageSync::IDD)
	, m_bSynchronizeVideo(0)
	, m_bSynchronizeDisplay(0)
	, m_bSynchronizeNearest(0)
	, m_iLineDelta(0)
	, m_iColumnDelta(0)
	, m_fCycleDelta(0.0012)
	, m_fTargetSyncOffset(10.0)
	, m_fControlLimit(2.0)
{
}

CPPageSync::~CPPageSync()
{
}

void CPPageSync::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_SYNCVIDEO, m_bSynchronizeVideo);
	DDX_Check(pDX, IDC_SYNCDISPLAY, m_bSynchronizeDisplay);
	DDX_Check(pDX, IDC_SYNCNEAREST, m_bSynchronizeNearest);
	DDX_Text(pDX, IDC_CYCLEDELTA, m_fCycleDelta);
	DDX_Text(pDX, IDC_LINEDELTA, m_iLineDelta);
	DDX_Text(pDX, IDC_COLUMNDELTA, m_iColumnDelta);
	DDX_Text(pDX, IDC_TARGETSYNCOFFSET, m_fTargetSyncOffset);
	DDX_Text(pDX, IDC_CONTROLLIMIT, m_fControlLimit);
}

BOOL CPPageSync::OnInitDialog()
{
	__super::OnInitDialog();

	InitDialogPrivate();

	return TRUE;
}

BOOL CPPageSync::OnSetActive()
{
	InitDialogPrivate();

	return __super::OnSetActive();
}

void CPPageSync::InitDialogPrivate()
{
	AppSettings& s = AfxGetAppSettings();

	CMainFrame * pFrame;
	pFrame = (CMainFrame *)(AfxGetApp()->m_pMainWnd);

	if ((s.iDSVideoRendererType == VIDRNDT_DS_SYNC) && (pFrame->GetPlaybackMode() == PM_NONE)) {
		GetDlgItem(IDC_SYNCVIDEO)->EnableWindow(TRUE);
		GetDlgItem(IDC_SYNCDISPLAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_SYNCNEAREST)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_SYNCVIDEO)->EnableWindow(FALSE);
		GetDlgItem(IDC_SYNCDISPLAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_SYNCNEAREST)->EnableWindow(FALSE);
	}

	CRenderersSettings::CAdvRendererSettings& ars = s.m_RenderersSettings.m_AdvRendSets;
	m_bSynchronizeVideo = ars.bSynchronizeVideo;
	m_bSynchronizeDisplay = ars.bSynchronizeDisplay;
	m_bSynchronizeNearest = ars.bSynchronizeNearest;
	m_iLineDelta = ars.iLineDelta;
	m_iColumnDelta = ars.iColumnDelta;
	m_fCycleDelta = ars.fCycleDelta;
	m_fTargetSyncOffset = ars.fTargetSyncOffset;
	m_fControlLimit = ars.fControlLimit;

	UpdateData(FALSE);
}

BOOL CPPageSync::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	CRenderersSettings::CAdvRendererSettings& ars = s.m_RenderersSettings.m_AdvRendSets;
	ars.bSynchronizeVideo = !!m_bSynchronizeVideo;
	ars.bSynchronizeDisplay = !!m_bSynchronizeDisplay;
	ars.bSynchronizeNearest = !!m_bSynchronizeNearest;
	ars.iLineDelta = m_iLineDelta;
	ars.iColumnDelta = m_iColumnDelta;
	ars.fCycleDelta = m_fCycleDelta;
	ars.fTargetSyncOffset = m_fTargetSyncOffset;
	ars.fControlLimit = m_fControlLimit;

	return __super::OnApply();
}

BEGIN_MESSAGE_MAP(CPPageSync, CPPageBase)
	ON_BN_CLICKED(IDC_SYNCVIDEO, OnBnClickedSyncVideo)
	ON_BN_CLICKED(IDC_SYNCDISPLAY, OnBnClickedSyncDisplay)
	ON_BN_CLICKED(IDC_SYNCNEAREST, OnBnClickedSyncNearest)
END_MESSAGE_MAP()

void CPPageSync::OnBnClickedSyncVideo()
{
	m_bSynchronizeVideo = !m_bSynchronizeVideo;

	if (m_bSynchronizeVideo) {
		m_bSynchronizeDisplay = FALSE;
		m_bSynchronizeNearest = FALSE;
	}

	UpdateData(FALSE);

	SetModified();
}

void CPPageSync::OnBnClickedSyncDisplay()
{
	m_bSynchronizeDisplay = !m_bSynchronizeDisplay;

	if (m_bSynchronizeDisplay) {
		m_bSynchronizeVideo = FALSE;
		m_bSynchronizeNearest = FALSE;
	}

	UpdateData(FALSE);

	SetModified();
}

void CPPageSync::OnBnClickedSyncNearest()
{
	m_bSynchronizeNearest = !m_bSynchronizeNearest;

	if (m_bSynchronizeNearest) {
		m_bSynchronizeVideo = FALSE;
		m_bSynchronizeDisplay = FALSE;
	}

	UpdateData(FALSE);

	SetModified();
}
