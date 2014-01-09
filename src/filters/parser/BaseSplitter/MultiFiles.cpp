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
#include "MultiFiles.h"


IMPLEMENT_DYNAMIC(CMultiFiles, CObject)

CMultiFiles::CMultiFiles()
	: m_hFile(INVALID_HANDLE_VALUE)
	, m_llTotalLength(0)
	, m_nCurPart(-1)
	, m_pCurrentPTSOffset(NULL)
{
}

void CMultiFiles::Reset()
{
	m_strFiles.RemoveAll();
	m_FilesSize.RemoveAll();
	m_rtPtsOffsets.RemoveAll();
	m_llTotalLength = 0;
}

BOOL CMultiFiles::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	Reset();
	m_strFiles.Add(lpszFileName);

	return OpenPart(0);
}

BOOL CMultiFiles::OpenFiles(CHdmvClipInfo::CPlaylist& files, UINT nOpenFlags)
{
	POSITION		pos = files.GetHeadPosition();
	LARGE_INTEGER	llSize;
	int				nPos  = 0;
	REFERENCE_TIME	rtDur = 0;

	Reset();
	while (pos) {
		CHdmvClipInfo::PlaylistItem* s = files.GetNext(pos);
		m_strFiles.Add(s->m_strFileName);
		if (!OpenPart(nPos)) {
			return false;
		}

		llSize.QuadPart = 0;
		GetFileSizeEx(m_hFile, &llSize);
		m_llTotalLength += llSize.QuadPart;
		m_FilesSize.Add(llSize.QuadPart);

		m_rtPtsOffsets.Add((s->m_rtStartTime - s->m_rtIn) + files.GetHead()->m_rtIn);
		rtDur += s->Duration();
		nPos++;
	}

	if (files.GetCount() > 1) {
		ClosePart();
	}

	return TRUE;
}

ULONGLONG CMultiFiles::Seek(LONGLONG lOff, UINT nFrom)
{
	LARGE_INTEGER	llNewPos;
	LARGE_INTEGER	llOff;

	if (m_strFiles.GetCount() == 1) {
		llOff.QuadPart = lOff;
		SetFilePointerEx(m_hFile, llOff, &llNewPos, nFrom);

		return llNewPos.QuadPart;
	} else {
		ULONGLONG	lAbsolutePos = GetAbsolutePosition(lOff, nFrom);
		int			nNewPart	 = 0;
		ULONGLONG	llSum		 = 0;

		while (m_FilesSize[nNewPart]+llSum <= lAbsolutePos) {
			llSum += m_FilesSize[nNewPart];
			nNewPart++;
		}

		OpenPart(nNewPart);
		llOff.QuadPart = lAbsolutePos - llSum;
		SetFilePointerEx(m_hFile, llOff, &llNewPos, FILE_BEGIN);

		return llSum + llNewPos.QuadPart;
	}
}

ULONGLONG CMultiFiles::GetAbsolutePosition(LONGLONG lOff, UINT nFrom)
{
	LARGE_INTEGER	llNoMove = {0, 0};
	LARGE_INTEGER	llCurPos;

	switch (nFrom) {
		case begin :
			return lOff;
		case current :
			SetFilePointerEx(m_hFile, llNoMove, &llCurPos, FILE_CURRENT);
			return llCurPos.QuadPart + lOff;
		case end :
			return m_llTotalLength - lOff;
		default:
			return 0;	// just used to quash "not all control paths return a value" warning
	}
}

ULONGLONG CMultiFiles::GetLength() const
{
	if (m_strFiles.GetCount() == 1) {
		LARGE_INTEGER	llSize;
		GetFileSizeEx(m_hFile, &llSize);
		return llSize.QuadPart;
	} else {
		return m_llTotalLength;
	}
}

UINT CMultiFiles::Read(BYTE* lpBuf, UINT nCount)
{
	DWORD dwRead = 0;
	int nCurPart;

	do {
		DWORD nNumberOfBytesRead = 0;
		if (!ReadFile(m_hFile, lpBuf, nCount - dwRead, &nNumberOfBytesRead, NULL)) {
			break;
		}

		dwRead		+= nNumberOfBytesRead;
		nCurPart	= m_nCurPart;

		if (dwRead != nCount && (nCurPart < 0 || (size_t)nCurPart < m_strFiles.GetCount()-1)) {
			OpenPart(nCurPart+1);
			lpBuf	+= dwRead;
		}
	} while (nCount != dwRead && (nCurPart < 0 || (size_t)nCurPart < m_strFiles.GetCount()-1));

	return dwRead;
}

void CMultiFiles::Close()
{
	ClosePart();
	Reset();
}

CMultiFiles::~CMultiFiles()
{
	Close();
}

BOOL CMultiFiles::OpenPart(int nPart)
{
	if (m_nCurPart == nPart) {
		return TRUE;
	} else {
		CString		fn;

		ClosePart();

		fn			= m_strFiles.GetAt(nPart);
		m_hFile		= CreateFile(fn, GENERIC_READ, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (m_hFile != INVALID_HANDLE_VALUE) {
			m_nCurPart	= nPart;
			if (m_pCurrentPTSOffset != NULL) {
				*m_pCurrentPTSOffset = m_rtPtsOffsets[nPart];
			}
		}

		return (m_hFile != INVALID_HANDLE_VALUE);
	}
}

void CMultiFiles::ClosePart()
{
	if (m_hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hFile);
		m_hFile		= INVALID_HANDLE_VALUE;
		m_nCurPart	= -1;
	}
}
