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

#pragma once

#include <atlcoll.h>
#include "BaseGraph.h"

class CMediaFormatCategory
{
protected:
	CString m_label, m_description, m_specreqnote;
	CAtlList<CString> m_exts, m_backupexts;
	bool m_fAudioOnly;
	engine_t m_engine;

public:
	CMediaFormatCategory();
	CMediaFormatCategory(
		CString label, CString description, CAtlList<CString>& exts, bool fAudioOnly = false,
		CString specreqnote =  _T(""), engine_t e = DirectShow);
	CMediaFormatCategory(
		CString label, CString description, CString exts, bool fAudioOnly = false,
		CString specreqnote =  _T(""), engine_t e = DirectShow);
	virtual ~CMediaFormatCategory();

	void UpdateData(bool fSave);

	CMediaFormatCategory(const CMediaFormatCategory& mfc);
	CMediaFormatCategory& operator = (const CMediaFormatCategory& mfc);

	void RestoreDefaultExts();
	void SetExts(CAtlList<CString>& exts);
	void SetExts(CString exts);

	bool FindExt(CString ext) {
		return m_exts.Find(ext.TrimLeft(_T('.')).MakeLower()) != NULL;
	}

	CString GetLabel() const {
		return m_label;
	}

	CString GetDescription() const {
		return m_description;
	}
	CString GetFilter();
	CString GetExts(bool fAppendEngine = false);
	CString GetExtsWithPeriod(bool fAppendEngine = false);
	CString GetBackupExtsWithPeriod(bool fAppendEngine = false);
	CString GetSpecReqNote() const {
		return m_specreqnote;
	}
	bool IsAudioOnly() const {
		return m_fAudioOnly;
	}
	engine_t GetEngineType() const {
		return m_engine;
	}
	void SetEngineType(engine_t e) {
		m_engine = e;
	}
};

class CMediaFormats : public CAtlArray<CMediaFormatCategory>
{
public:
	CMediaFormats();
	virtual ~CMediaFormats();

	void UpdateData(bool fSave);

	bool FindExt(CString ext, bool fAudioOnly = false);
	CMediaFormatCategory* FindMediaByExt(CString ext, bool fAudioOnly = false);

	void GetFilter(CString& filter, CAtlArray<CString>& mask);
	void GetAudioFilter(CString& filter, CAtlArray<CString>& mask);
};
