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

#include "stdafx.h"
#include "PPageAccelTbl.h"


struct APP_COMMAND {
	UINT		appcmd;
	LPCTSTR		cmdname;
};

APP_COMMAND	g_CommandList[] = {
	{0,									_T("")},
	{APPCOMMAND_BROWSER_BACKWARD,		_T("BROWSER_BACKWARD")},
	{APPCOMMAND_BROWSER_FORWARD,		_T("BROWSER_FORWARD")},
	{APPCOMMAND_BROWSER_REFRESH,		_T("BROWSER_REFRESH")},
	{APPCOMMAND_BROWSER_STOP,			_T("BROWSER_STOP")},
	{APPCOMMAND_BROWSER_SEARCH,			_T("BROWSER_SEARCH")},
	{APPCOMMAND_BROWSER_FAVORITES,		_T("BROWSER_FAVORITES")},
	{APPCOMMAND_BROWSER_HOME,			_T("BROWSER_HOME")},
	{APPCOMMAND_VOLUME_MUTE,			_T("VOLUME_MUTE")},
	{APPCOMMAND_VOLUME_DOWN,			_T("VOLUME_DOWN")},
	{APPCOMMAND_VOLUME_UP,				_T("VOLUME_UP")},
	{APPCOMMAND_MEDIA_NEXTTRACK,		_T("MEDIA_NEXTTRACK")},
	{APPCOMMAND_MEDIA_PREVIOUSTRACK,	_T("MEDIA_PREVIOUSTRACK")},
	{APPCOMMAND_MEDIA_STOP,				_T("MEDIA_STOP")},
	{APPCOMMAND_MEDIA_PLAY_PAUSE,		_T("MEDIA_PLAY_PAUSE")},
	{APPCOMMAND_LAUNCH_MAIL,			_T("LAUNCH_MAIL")},
	{APPCOMMAND_LAUNCH_MEDIA_SELECT,	_T("LAUNCH_MEDIA_SELECT")},
	{APPCOMMAND_LAUNCH_APP1,			_T("LAUNCH_APP1")},
	{APPCOMMAND_LAUNCH_APP2,			_T("LAUNCH_APP2")},
	{APPCOMMAND_BASS_DOWN,				_T("BASS_DOWN")},
	{APPCOMMAND_BASS_BOOST,				_T("BASS_BOOST")},
	{APPCOMMAND_BASS_UP,				_T("BASS_UP")},
	{APPCOMMAND_TREBLE_DOWN,			_T("TREBLE_DOWN")},
	{APPCOMMAND_TREBLE_UP,				_T("TREBLE_UP")},
	{APPCOMMAND_MICROPHONE_VOLUME_MUTE, _T("MICROPHONE_VOLUME_MUTE")},
	{APPCOMMAND_MICROPHONE_VOLUME_DOWN, _T("MICROPHONE_VOLUME_DOWN")},
	{APPCOMMAND_MICROPHONE_VOLUME_UP,	_T("MICROPHONE_VOLUME_UP")},
	{APPCOMMAND_HELP,					_T("HELP")},
	{APPCOMMAND_FIND,					_T("FIND")},
	{APPCOMMAND_NEW,					_T("NEW")},
	{APPCOMMAND_OPEN,					_T("OPEN")},
	{APPCOMMAND_CLOSE,					_T("CLOSE")},
	{APPCOMMAND_SAVE,					_T("SAVE")},
	{APPCOMMAND_PRINT,					_T("PRINT")},
	{APPCOMMAND_UNDO,					_T("UNDO")},
	{APPCOMMAND_REDO,					_T("REDO")},
	{APPCOMMAND_COPY,					_T("COPY")},
	{APPCOMMAND_CUT,					_T("CUT")},
	{APPCOMMAND_PASTE,					_T("PASTE")},
	{APPCOMMAND_REPLY_TO_MAIL,			_T("REPLY_TO_MAIL")},
	{APPCOMMAND_FORWARD_MAIL,			_T("FORWARD_MAIL")},
	{APPCOMMAND_SEND_MAIL,				_T("SEND_MAIL")},
	{APPCOMMAND_SPELL_CHECK,			_T("SPELL_CHECK")},
	{APPCOMMAND_DICTATE_OR_COMMAND_CONTROL_TOGGLE, _T("DICTATE_OR_COMMAND_CONTROL_TOGGLE")},
	{APPCOMMAND_MIC_ON_OFF_TOGGLE,		_T("MIC_ON_OFF_TOGGLE")},
	{APPCOMMAND_CORRECTION_LIST,		_T("CORRECTION_LIST")},
	{APPCOMMAND_MEDIA_PLAY,				_T("MEDIA_PLAY")},
	{APPCOMMAND_MEDIA_PAUSE,			_T("MEDIA_PAUSE")},
	{APPCOMMAND_MEDIA_RECORD,			_T("MEDIA_RECORD")},
	{APPCOMMAND_MEDIA_FAST_FORWARD,		_T("MEDIA_FAST_FORWARD")},
	{APPCOMMAND_MEDIA_REWIND,			_T("MEDIA_REWIND")},
	{APPCOMMAND_MEDIA_CHANNEL_UP,		_T("MEDIA_CHANNEL_UP")},
	{APPCOMMAND_MEDIA_CHANNEL_DOWN,		_T("MEDIA_CHANNEL_DOWN")},
	{APPCOMMAND_DELETE,					_T("DELETE")},
	{APPCOMMAND_DWM_FLIP3D,				_T("DWM_FLIP3D")},
	{MCE_DETAILS,						_T("MCE_DETAILS")},
	{MCE_GUIDE,							_T("MCE_GUIDE")},
	{MCE_TVJUMP,						_T("MCE_TVJUMP")},
	{MCE_STANDBY,						_T("MCE_STANDBY")},
	{MCE_OEM1,							_T("MCE_OEM1")},
	{MCE_OEM2,							_T("MCE_OEM2")},
	{MCE_MYTV,							_T("MCE_MYTV")},
	{MCE_MYVIDEOS,						_T("MCE_MYVIDEOS")},
	{MCE_MYPICTURES,					_T("MCE_MYPICTURES")},
	{MCE_MYMUSIC,						_T("MCE_MYMUSIC")},
	{MCE_RECORDEDTV,					_T("MCE_RECORDEDTV")},
	{MCE_DVDANGLE,						_T("MCE_DVDANGLE")},
	{MCE_DVDAUDIO,						_T("MCE_DVDAUDIO")},
	{MCE_DVDMENU,						_T("MCE_DVDMENU")},
	{MCE_DVDSUBTITLE,					_T("MCE_DVDSUBTITLE")},
	{MCE_RED,							_T("MCE_RED")},
	{MCE_GREEN,							_T("MCE_GREEN")},
	{MCE_YELLOW,						_T("MCE_YELLOW")},
	{MCE_BLUE,							_T("MCE_BLUE")},
	{MCE_MEDIA_NEXTTRACK,				_T("MCE_MEDIA_NEXTTRACK")},
	{MCE_MEDIA_PREVIOUSTRACK,			_T("MCE_MEDIA_PREVIOUSTRACK")}
};

// CPPageAccelTbl dialog

IMPLEMENT_DYNAMIC(CPPageAccelTbl, CPPageBase)
CPPageAccelTbl::CPPageAccelTbl()
	: CPPageBase(CPPageAccelTbl::IDD, CPPageAccelTbl::IDD)
	, m_list(0)
	, m_counter(0)
	, m_fWinLirc(FALSE)
	, m_WinLircLink(_T("http://winlirc.sourceforge.net/"))
	, m_fUIce(FALSE)
	, m_UIceLink(_T("http://www.mediatexx.com/"))
	, m_fGlobalMedia(FALSE)
{
}

CPPageAccelTbl::~CPPageAccelTbl()
{
}

BOOL CPPageAccelTbl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN
			&& (pMsg->hwnd == m_WinLircEdit.m_hWnd || pMsg->hwnd == m_UIceEdit.m_hWnd)) {
		OnApply();
		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}


void CPPageAccelTbl::SetupList()
{
	for (int row = 0; row < m_list.GetItemCount(); row++) {
		wmcmd& wc = m_wmcmds.GetAt((POSITION)m_list.GetItemData(row));

		CString hotkey;
		HotkeyModToString(wc.key, wc.fVirt, hotkey);
		m_list.SetItemText(row, COL_KEY, hotkey);

		CString id;
		id.Format(_T("%d"), wc.cmd);
		m_list.SetItemText(row, COL_ID, id);

		m_list.SetItemText(row, COL_MOUSE, MakeMouseButtonLabel(wc.mouse));

		m_list.SetItemText(row, COL_MOUSE_FS, MakeMouseButtonLabel(wc.mouseFS));

		m_list.SetItemText(row, COL_APPCMD, MakeAppCommandLabel(wc.appcmd));

		m_list.SetItemText(row, COL_RMCMD, CString(wc.rmcmd));

		CString repcnt;
		repcnt.Format(_T("%d"), wc.rmrepcnt);
		m_list.SetItemText(row, COL_RMREPCNT, repcnt);
	}

	if (!AfxGetAppSettings().AccelTblColWidth.bEnable) {
		int contentSize;
		for (int nCol = COL_CMD; nCol <= COL_RMREPCNT; nCol++) {
			m_list.SetColumnWidth(nCol, LVSCW_AUTOSIZE);
			contentSize = m_list.GetColumnWidth(nCol);
			m_list.SetColumnWidth(nCol, LVSCW_AUTOSIZE_USEHEADER);
			if (contentSize > m_list.GetColumnWidth(nCol)) {
				m_list.SetColumnWidth(nCol, LVSCW_AUTOSIZE);
			}
		}
	}
}

CString CPPageAccelTbl::MakeAccelModLabel(BYTE fVirt)
{
	CString str;
	if (fVirt&FCONTROL) {
		if (!str.IsEmpty()) {
			str += _T(" + ");
		}
		str += _T("Ctrl");
	}
	if (fVirt&FALT) {
		if (!str.IsEmpty()) {
			str += _T(" + ");
		}
		str += _T("Alt");
	}
	if (fVirt&FSHIFT) {
		if (!str.IsEmpty()) {
			str += _T(" + ");
		}
		str += _T("Shift");
	}
	if (str.IsEmpty()) {
		str = ResStr(IDS_AG_NONE);
	}
	return(str);
}

CString CPPageAccelTbl::MakeAccelVkeyLabel(WORD key, bool fVirtKey)
{
	// Reference page for Virtual-Key Codes: http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.100%29.aspx
	CString str;

	switch (key) {
		case VK_LBUTTON:
			str = _T("VK_LBUTTON");
			break;
		case VK_RBUTTON:
			str = _T("VK_RBUTTON");
			break;
		case VK_CANCEL:
			str = _T("VK_CANCEL");
			break;
		case VK_MBUTTON:
			str = _T("VK_MBUTTON");
			break;
		case VK_XBUTTON1:
			str = _T("VK_XBUTTON1");
			break;
		case VK_XBUTTON2:
			str = _T("VK_XBUTTON2");
			break;
		case VK_BACK:
			str = _T("VK_BACK");
			break;
		case VK_TAB:
			str = _T("VK_TAB");
			break;
		case VK_CLEAR:
			str = _T("VK_CLEAR");
			break;
		case VK_RETURN:
			str = _T("VK_RETURN");
			break;
		case VK_SHIFT:
			str = _T("VK_SHIFT");
			break;
		case VK_CONTROL:
			str = _T("VK_CONTROL");
			break;
		case VK_MENU:
			str = _T("VK_MENU");
			break;
		case VK_PAUSE:
			str = _T("VK_PAUSE");
			break;
		case VK_CAPITAL:
			str = _T("VK_CAPITAL");
			break;
			//	case VK_KANA: str = _T("VK_KANA"); break;
			//	case VK_HANGEUL: str = _T("VK_HANGEUL"); break;
		case VK_HANGUL:
			str = _T("VK_HANGUL");
			break;
		case VK_JUNJA:
			str = _T("VK_JUNJA");
			break;
		case VK_FINAL:
			str = _T("VK_FINAL");
			break;
			//	case VK_HANJA: str = _T("VK_HANJA"); break;
		case VK_KANJI:
			str = _T("VK_KANJI");
			break;
		case VK_ESCAPE:
			str = _T("VK_ESCAPE");
			break;
		case VK_CONVERT:
			str = _T("VK_CONVERT");
			break;
		case VK_NONCONVERT:
			str = _T("VK_NONCONVERT");
			break;
		case VK_ACCEPT:
			str = _T("VK_ACCEPT");
			break;
		case VK_MODECHANGE:
			str = _T("VK_MODECHANGE");
			break;
		case VK_SPACE:
			str = _T("VK_SPACE");
			break;
		case VK_PRIOR:
			str = _T("VK_PRIOR");
			break;
		case VK_NEXT:
			str = _T("VK_NEXT");
			break;
		case VK_END:
			str = _T("VK_END");
			break;
		case VK_HOME:
			str = _T("VK_HOME");
			break;
		case VK_LEFT:
			str = _T("VK_LEFT");
			break;
		case VK_UP:
			str = _T("VK_UP");
			break;
		case VK_RIGHT:
			str = _T("VK_RIGHT");
			break;
		case VK_DOWN:
			str = _T("VK_DOWN");
			break;
		case VK_SELECT:
			str = _T("VK_SELECT");
			break;
		case VK_PRINT:
			str = _T("VK_PRINT");
			break;
		case VK_EXECUTE:
			str = _T("VK_EXECUTE");
			break;
		case VK_SNAPSHOT:
			str = _T("VK_SNAPSHOT");
			break;
		case VK_INSERT:
			str = _T("VK_INSERT");
			break;
		case VK_DELETE:
			str = _T("VK_DELETE");
			break;
		case VK_HELP:
			str = _T("VK_HELP");
			break;
		case VK_LWIN:
			str = _T("VK_LWIN");
			break;
		case VK_RWIN:
			str = _T("VK_RWIN");
			break;
		case VK_APPS:
			str = _T("VK_APPS");
			break;
		case VK_SLEEP:
			str = _T("VK_SLEEP");
			break;
		case VK_NUMPAD0:
			str = _T("VK_NUMPAD0");
			break;
		case VK_NUMPAD1:
			str = _T("VK_NUMPAD1");
			break;
		case VK_NUMPAD2:
			str = _T("VK_NUMPAD2");
			break;
		case VK_NUMPAD3:
			str = _T("VK_NUMPAD3");
			break;
		case VK_NUMPAD4:
			str = _T("VK_NUMPAD4");
			break;
		case VK_NUMPAD5:
			str = _T("VK_NUMPAD5");
			break;
		case VK_NUMPAD6:
			str = _T("VK_NUMPAD6");
			break;
		case VK_NUMPAD7:
			str = _T("VK_NUMPAD7");
			break;
		case VK_NUMPAD8:
			str = _T("VK_NUMPAD8");
			break;
		case VK_NUMPAD9:
			str = _T("VK_NUMPAD9");
			break;
		case VK_MULTIPLY:
			str = _T("VK_MULTIPLY");
			break;
		case VK_ADD:
			str = _T("VK_ADD");
			break;
		case VK_SEPARATOR:
			str = _T("VK_SEPARATOR");
			break;
		case VK_SUBTRACT:
			str = _T("VK_SUBTRACT");
			break;
		case VK_DECIMAL:
			str = _T("VK_DECIMAL");
			break;
		case VK_DIVIDE:
			str = _T("VK_DIVIDE");
			break;
		case VK_F1:
			str = _T("VK_F1");
			break;
		case VK_F2:
			str = _T("VK_F2");
			break;
		case VK_F3:
			str = _T("VK_F3");
			break;
		case VK_F4:
			str = _T("VK_F4");
			break;
		case VK_F5:
			str = _T("VK_F5");
			break;
		case VK_F6:
			str = _T("VK_F6");
			break;
		case VK_F7:
			str = _T("VK_F7");
			break;
		case VK_F8:
			str = _T("VK_F8");
			break;
		case VK_F9:
			str = _T("VK_F9");
			break;
		case VK_F10:
			str = _T("VK_F10");
			break;
		case VK_F11:
			str = _T("VK_F11");
			break;
		case VK_F12:
			str = _T("VK_F12");
			break;
		case VK_F13:
			str = _T("VK_F13");
			break;
		case VK_F14:
			str = _T("VK_F14");
			break;
		case VK_F15:
			str = _T("VK_F15");
			break;
		case VK_F16:
			str = _T("VK_F16");
			break;
		case VK_F17:
			str = _T("VK_F17");
			break;
		case VK_F18:
			str = _T("VK_F18");
			break;
		case VK_F19:
			str = _T("VK_F19");
			break;
		case VK_F20:
			str = _T("VK_F20");
			break;
		case VK_F21:
			str = _T("VK_F21");
			break;
		case VK_F22:
			str = _T("VK_F22");
			break;
		case VK_F23:
			str = _T("VK_F23");
			break;
		case VK_F24:
			str = _T("VK_F24");
			break;
		case VK_NUMLOCK:
			str = _T("VK_NUMLOCK");
			break;
		case VK_SCROLL:
			str = _T("VK_SCROLL");
			break;
			//	case VK_OEM_NEC_EQUAL: str = _T("VK_OEM_NEC_EQUAL"); break;
		case VK_OEM_FJ_JISHO:
			str = _T("VK_OEM_FJ_JISHO");
			break;
		case VK_OEM_FJ_MASSHOU:
			str = _T("VK_OEM_FJ_MASSHOU");
			break;
		case VK_OEM_FJ_TOUROKU:
			str = _T("VK_OEM_FJ_TOUROKU");
			break;
		case VK_OEM_FJ_LOYA:
			str = _T("VK_OEM_FJ_LOYA");
			break;
		case VK_OEM_FJ_ROYA:
			str = _T("VK_OEM_FJ_ROYA");
			break;
		case VK_LSHIFT:
			str = _T("VK_LSHIFT");
			break;
		case VK_RSHIFT:
			str = _T("VK_RSHIFT");
			break;
		case VK_LCONTROL:
			str = _T("VK_LCONTROL");
			break;
		case VK_RCONTROL:
			str = _T("VK_RCONTROL");
			break;
		case VK_LMENU:
			str = _T("VK_LMENU");
			break;
		case VK_RMENU:
			str = _T("VK_RMENU");
			break;
		case VK_BROWSER_BACK:
			str = _T("VK_BROWSER_BACK");
			break;
		case VK_BROWSER_FORWARD:
			str = _T("VK_BROWSER_FORWARD");
			break;
		case VK_BROWSER_REFRESH:
			str = _T("VK_BROWSER_REFRESH");
			break;
		case VK_BROWSER_STOP:
			str = _T("VK_BROWSER_STOP");
			break;
		case VK_BROWSER_SEARCH:
			str = _T("VK_BROWSER_SEARCH");
			break;
		case VK_BROWSER_FAVORITES:
			str = _T("VK_BROWSER_FAVORITES");
			break;
		case VK_BROWSER_HOME:
			str = _T("VK_BROWSER_HOME");
			break;
		case VK_VOLUME_MUTE:
			str = _T("VK_VOLUME_MUTE");
			break;
		case VK_VOLUME_DOWN:
			str = _T("VK_VOLUME_DOWN");
			break;
		case VK_VOLUME_UP:
			str = _T("VK_VOLUME_UP");
			break;
		case VK_MEDIA_NEXT_TRACK:
			str = _T("VK_MEDIA_NEXT_TRACK");
			break;
		case VK_MEDIA_PREV_TRACK:
			str = _T("VK_MEDIA_PREV_TRACK");
			break;
		case VK_MEDIA_STOP:
			str = _T("VK_MEDIA_STOP");
			break;
		case VK_MEDIA_PLAY_PAUSE:
			str = _T("VK_MEDIA_PLAY_PAUSE");
			break;
		case VK_LAUNCH_MAIL:
			str = _T("VK_LAUNCH_MAIL");
			break;
		case VK_LAUNCH_MEDIA_SELECT:
			str = _T("VK_LAUNCH_MEDIA_SELECT");
			break;
		case VK_LAUNCH_APP1:
			str = _T("VK_LAUNCH_APP1");
			break;
		case VK_LAUNCH_APP2:
			str = _T("VK_LAUNCH_APP2");
			break;
		case VK_OEM_1:
			str = _T("VK_OEM_1");
			break;
		case VK_OEM_PLUS:
			str = _T("VK_OEM_PLUS");
			break;
		case VK_OEM_COMMA:
			str = _T("VK_OEM_COMMA");
			break;
		case VK_OEM_MINUS:
			str = _T("VK_OEM_MINUS");
			break;
		case VK_OEM_PERIOD:
			str = _T("VK_OEM_PERIOD");
			break;
		case VK_OEM_2:
			str = _T("VK_OEM_2");
			break;
		case VK_OEM_3:
			str = _T("VK_OEM_3");
			break;
		case VK_OEM_4:
			str = _T("VK_OEM_4");
			break;
		case VK_OEM_5:
			str = _T("VK_OEM_5");
			break;
		case VK_OEM_6:
			str = _T("VK_OEM_6");
			break;
		case VK_OEM_7:
			str = _T("VK_OEM_7");
			break;
		case VK_OEM_8:
			str = _T("VK_OEM_8");
			break;
		case VK_OEM_AX:
			str = _T("VK_OEM_AX");
			break;
		case VK_OEM_102:
			str = _T("VK_OEM_102");
			break;
		case VK_ICO_HELP:
			str = _T("VK_ICO_HELP");
			break;
		case VK_ICO_00:
			str = _T("VK_ICO_00");
			break;
		case VK_PROCESSKEY:
			str = _T("VK_PROCESSKEY");
			break;
		case VK_ICO_CLEAR:
			str = _T("VK_ICO_CLEAR");
			break;
		case VK_PACKET:
			str = _T("VK_PACKET");
			break;
		case VK_OEM_RESET:
			str = _T("VK_OEM_RESET");
			break;
		case VK_OEM_JUMP:
			str = _T("VK_OEM_JUMP");
			break;
		case VK_OEM_PA1:
			str = _T("VK_OEM_PA1");
			break;
		case VK_OEM_PA2:
			str = _T("VK_OEM_PA2");
			break;
		case VK_OEM_PA3:
			str = _T("VK_OEM_PA3");
			break;
		case VK_OEM_WSCTRL:
			str = _T("VK_OEM_WSCTRL");
			break;
		case VK_OEM_CUSEL:
			str = _T("VK_OEM_CUSEL");
			break;
		case VK_OEM_ATTN:
			str = _T("VK_OEM_ATTN");
			break;
		case VK_OEM_FINISH:
			str = _T("VK_OEM_FINISH");
			break;
		case VK_OEM_COPY:
			str = _T("VK_OEM_COPY");
			break;
		case VK_OEM_AUTO:
			str = _T("VK_OEM_AUTO");
			break;
		case VK_OEM_ENLW:
			str = _T("VK_OEM_ENLW");
			break;
		case VK_OEM_BACKTAB:
			str = _T("VK_OEM_BACKTAB");
			break;
		case VK_ATTN:
			str = _T("VK_ATTN");
			break;
		case VK_CRSEL:
			str = _T("VK_CRSEL");
			break;
		case VK_EXSEL:
			str = _T("VK_EXSEL");
			break;
		case VK_EREOF:
			str = _T("VK_EREOF");
			break;
		case VK_PLAY:
			str = _T("VK_PLAY");
			break;
		case VK_ZOOM:
			str = _T("VK_ZOOM");
			break;
		case VK_NONAME:
			str = _T("VK_NONAME");
			break;
		case VK_PA1:
			str = _T("VK_PA1");
			break;
		case VK_OEM_CLEAR:
			str = _T("VK_OEM_CLEAR");
			break;
		case 0x07:
		case 0x0E:
		case 0x0F:
		case 0x16:
		case 0x1A:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F:
		case 0x40:
			str.Format(_T("Undefined (0x%02x)"), (TCHAR)key);
			break;
		case 0x0A:
		case 0x0B:
		case 0x5E:
		case 0xB8:
		case 0xB9:
		case 0xC1:
		case 0xC2:
		case 0xC3:
		case 0xC4:
		case 0xC5:
		case 0xC6:
		case 0xC7:
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF:
		case 0xD0:
		case 0xD1:
		case 0xD2:
		case 0xD3:
		case 0xD4:
		case 0xD5:
		case 0xD6:
		case 0xD7:
		case 0xE0:
			str.Format(_T("Reserved (0x%02x)"), (TCHAR)key);
			break;
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F:
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xE8:
			str.Format(_T("Unassigned (0x%02x)"), (TCHAR)key);
			break;
		case 0xFF:
			str = _T("Multimedia keys");
			break;
		default:
			//	if ('0' <= key && key <= '9' || 'A' <= key && key <= 'Z')
			str.Format(_T("%c"), (TCHAR)key);
			break;
	}

	return(str);
}

CString CPPageAccelTbl::MakeAccelShortcutLabel(UINT id)
{
	CList<wmcmd>& wmcmds = AfxGetAppSettings().wmcmds;
	POSITION pos = wmcmds.GetHeadPosition();
	while (pos) {
		ACCEL& a = wmcmds.GetNext(pos);
		if (a.cmd == id) {
			return(MakeAccelShortcutLabel(a));
		}
	}

	return(_T(""));
}

CString CPPageAccelTbl::MakeAccelShortcutLabel(ACCEL& a)
{
	// Reference page for Virtual-Key Codes: http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.100%29.aspx
	CString str;

	switch (a.key) {
		case VK_LBUTTON:
			str = _T("LBtn");
			break;
		case VK_RBUTTON:
			str = _T("RBtn");
			break;
		case VK_CANCEL:
			str = _T("Cancel");
			break;
		case VK_MBUTTON:
			str = _T("MBtn");
			break;
		case VK_XBUTTON1:
			str = _T("X1Btn");
			break;
		case VK_XBUTTON2:
			str = _T("X2Btn");
			break;
		case VK_BACK:
			str = _T("Back");
			break;
		case VK_TAB:
			str = _T("Tab");
			break;
		case VK_CLEAR:
			str = _T("Clear");
			break;
		case VK_RETURN:
			str = _T("Enter");
			break;
		case VK_SHIFT:
			str = _T("Shift");
			break;
		case VK_CONTROL:
			str = _T("Ctrl");
			break;
		case VK_MENU:
			str = _T("Alt");
			break;
		case VK_PAUSE:
			str = _T("Pause");
			break;
		case VK_CAPITAL:
			str = _T("Capital");
			break;
			//	case VK_KANA: str = _T("Kana"); break;
			//	case VK_HANGEUL: str = _T("Hangeul"); break;
		case VK_HANGUL:
			str = _T("Hangul");
			break;
		case VK_JUNJA:
			str = _T("Junja");
			break;
		case VK_FINAL:
			str = _T("Final");
			break;
			//	case VK_HANJA: str = _T("Hanja"); break;
		case VK_KANJI:
			str = _T("Kanji");
			break;
		case VK_ESCAPE:
			str = _T("Escape");
			break;
		case VK_CONVERT:
			str = _T("Convert");
			break;
		case VK_NONCONVERT:
			str = _T("Non Convert");
			break;
		case VK_ACCEPT:
			str = _T("Accept");
			break;
		case VK_MODECHANGE:
			str = _T("Mode Change");
			break;
		case VK_SPACE:
			str = _T("Space");
			break;
		case VK_PRIOR:
			str = _T("PgUp");
			break;
		case VK_NEXT:
			str = _T("PgDn");
			break;
		case VK_END:
			str = _T("End");
			break;
		case VK_HOME:
			str = _T("Home");
			break;
		case VK_LEFT:
			str = _T("Left");
			break;
		case VK_UP:
			str = _T("Up");
			break;
		case VK_RIGHT:
			str = _T("Right");
			break;
		case VK_DOWN:
			str = _T("Down");
			break;
		case VK_SELECT:
			str = _T("Select");
			break;
		case VK_PRINT:
			str = _T("Print");
			break;
		case VK_EXECUTE:
			str = _T("Execute");
			break;
		case VK_SNAPSHOT:
			str = _T("Snapshot");
			break;
		case VK_INSERT:
			str = _T("Insert");
			break;
		case VK_DELETE:
			str = _T("Delete");
			break;
		case VK_HELP:
			str = _T("Help");
			break;
		case VK_LWIN:
			str = _T("LWin");
			break;
		case VK_RWIN:
			str = _T("RWin");
			break;
		case VK_APPS:
			str = _T("Apps");
			break;
		case VK_SLEEP:
			str = _T("Sleep");
			break;
		case VK_NUMPAD0:
			str = _T("Numpad 0");
			break;
		case VK_NUMPAD1:
			str = _T("Numpad 1");
			break;
		case VK_NUMPAD2:
			str = _T("Numpad 2");
			break;
		case VK_NUMPAD3:
			str = _T("Numpad 3");
			break;
		case VK_NUMPAD4:
			str = _T("Numpad 4");
			break;
		case VK_NUMPAD5:
			str = _T("Numpad 5");
			break;
		case VK_NUMPAD6:
			str = _T("Numpad 6");
			break;
		case VK_NUMPAD7:
			str = _T("Numpad 7");
			break;
		case VK_NUMPAD8:
			str = _T("Numpad 8");
			break;
		case VK_NUMPAD9:
			str = _T("Numpad 9");
			break;
		case VK_MULTIPLY:
			str = _T("Multiply");
			break;
		case VK_ADD:
			str = _T("Add");
			break;
		case VK_SEPARATOR:
			str = _T("Separator");
			break;
		case VK_SUBTRACT:
			str = _T("Subtract");
			break;
		case VK_DECIMAL:
			str = _T("Decimal");
			break;
		case VK_DIVIDE:
			str = _T("Divide");
			break;
		case VK_F1:
			str = _T("F1");
			break;
		case VK_F2:
			str = _T("F2");
			break;
		case VK_F3:
			str = _T("F3");
			break;
		case VK_F4:
			str = _T("F4");
			break;
		case VK_F5:
			str = _T("F5");
			break;
		case VK_F6:
			str = _T("F6");
			break;
		case VK_F7:
			str = _T("F7");
			break;
		case VK_F8:
			str = _T("F8");
			break;
		case VK_F9:
			str = _T("F9");
			break;
		case VK_F10:
			str = _T("F10");
			break;
		case VK_F11:
			str = _T("F11");
			break;
		case VK_F12:
			str = _T("F12");
			break;
		case VK_F13:
			str = _T("F13");
			break;
		case VK_F14:
			str = _T("F14");
			break;
		case VK_F15:
			str = _T("F15");
			break;
		case VK_F16:
			str = _T("F16");
			break;
		case VK_F17:
			str = _T("F17");
			break;
		case VK_F18:
			str = _T("F18");
			break;
		case VK_F19:
			str = _T("F19");
			break;
		case VK_F20:
			str = _T("F20");
			break;
		case VK_F21:
			str = _T("F21");
			break;
		case VK_F22:
			str = _T("F22");
			break;
		case VK_F23:
			str = _T("F23");
			break;
		case VK_F24:
			str = _T("F24");
			break;
		case VK_NUMLOCK:
			str = _T("Numlock");
			break;
		case VK_SCROLL:
			str = _T("Scroll");
			break;
			//	case VK_OEM_NEC_EQUAL: str = _T("OEM NEC Equal"); break;
		case VK_OEM_FJ_JISHO:
			str = _T("OEM FJ Jisho");
			break;
		case VK_OEM_FJ_MASSHOU:
			str = _T("OEM FJ Msshou");
			break;
		case VK_OEM_FJ_TOUROKU:
			str = _T("OEM FJ Touroku");
			break;
		case VK_OEM_FJ_LOYA:
			str = _T("OEM FJ Loya");
			break;
		case VK_OEM_FJ_ROYA:
			str = _T("OEM FJ Roya");
			break;
		case VK_LSHIFT:
			str = _T("LShift");
			break;
		case VK_RSHIFT:
			str = _T("RShift");
			break;
		case VK_LCONTROL:
			str = _T("LCtrl");
			break;
		case VK_RCONTROL:
			str = _T("RCtrl");
			break;
		case VK_LMENU:
			str = _T("LAlt");
			break;
		case VK_RMENU:
			str = _T("RAlt");
			break;
		case VK_BROWSER_BACK:
			str = _T("Browser Back");
			break;
		case VK_BROWSER_FORWARD:
			str = _T("Browser Forward");
			break;
		case VK_BROWSER_REFRESH:
			str = _T("Browser Refresh");
			break;
		case VK_BROWSER_STOP:
			str = _T("Browser Stop");
			break;
		case VK_BROWSER_SEARCH:
			str = _T("Browser Search");
			break;
		case VK_BROWSER_FAVORITES:
			str = _T("Browser Favorites");
			break;
		case VK_BROWSER_HOME:
			str = _T("Browser Home");
			break;
		case VK_VOLUME_MUTE:
			str = _T("Volume Mute");
			break;
		case VK_VOLUME_DOWN:
			str = _T("Volume Down");
			break;
		case VK_VOLUME_UP:
			str = _T("Volume Up");
			break;
		case VK_MEDIA_NEXT_TRACK:
			str = _T("Media Next Track");
			break;
		case VK_MEDIA_PREV_TRACK:
			str = _T("Media Prev Track");
			break;
		case VK_MEDIA_STOP:
			str = _T("Media Stop");
			break;
		case VK_MEDIA_PLAY_PAUSE:
			str = _T("Media Play/Pause");
			break;
		case VK_LAUNCH_MAIL:
			str = _T("Launch Mail");
			break;
		case VK_LAUNCH_MEDIA_SELECT:
			str = _T("Launch Media Select");
			break;
		case VK_LAUNCH_APP1:
			str = _T("Launch App1");
			break;
		case VK_LAUNCH_APP2:
			str = _T("Launch App2");
			break;
		case VK_OEM_1:
			str = _T("OEM 1");
			break;
		case VK_OEM_PLUS:
			str = _T("Plus");
			break;
		case VK_OEM_COMMA:
			str = _T("Comma");
			break;
		case VK_OEM_MINUS:
			str = _T("Minus");
			break;
		case VK_OEM_PERIOD:
			str = _T("Period");
			break;
		case VK_OEM_2:
			str = _T("OEM 2");
			break;
		case VK_OEM_3:
			str = _T("OEM 3");
			break;
		case VK_OEM_4:
			str = _T("OEM 4");
			break;
		case VK_OEM_5:
			str = _T("OEM 5");
			break;
		case VK_OEM_6:
			str = _T("OEM 6");
			break;
		case VK_OEM_7:
			str = _T("OEM 7");
			break;
		case VK_OEM_8:
			str = _T("OEM 8");
			break;
		case VK_OEM_AX:
			str = _T("OEM AX");
			break;
		case VK_OEM_102:
			str = _T("OEM 102");
			break;
		case VK_ICO_HELP:
			str = _T("ICO Help");
			break;
		case VK_ICO_00:
			str = _T("ICO 00");
			break;
		case VK_PROCESSKEY:
			str = _T("Process Key");
			break;
		case VK_ICO_CLEAR:
			str = _T("ICO Clear");
			break;
		case VK_PACKET:
			str = _T("Packet");
			break;
		case VK_OEM_RESET:
			str = _T("OEM Reset");
			break;
		case VK_OEM_JUMP:
			str = _T("OEM Jump");
			break;
		case VK_OEM_PA1:
			str = _T("OEM PA1");
			break;
		case VK_OEM_PA2:
			str = _T("OEM PA2");
			break;
		case VK_OEM_PA3:
			str = _T("OEM PA3");
			break;
		case VK_OEM_WSCTRL:
			str = _T("OEM WSCtrl");
			break;
		case VK_OEM_CUSEL:
			str = _T("OEM CUSEL");
			break;
		case VK_OEM_ATTN:
			str = _T("OEM ATTN");
			break;
		case VK_OEM_FINISH:
			str = _T("OEM Finish");
			break;
		case VK_OEM_COPY:
			str = _T("OEM Copy");
			break;
		case VK_OEM_AUTO:
			str = _T("OEM Auto");
			break;
		case VK_OEM_ENLW:
			str = _T("OEM ENLW");
			break;
		case VK_OEM_BACKTAB:
			str = _T("OEM Backtab");
			break;
		case VK_ATTN:
			str = _T("ATTN");
			break;
		case VK_CRSEL:
			str = _T("CRSEL");
			break;
		case VK_EXSEL:
			str = _T("EXSEL");
			break;
		case VK_EREOF:
			str = _T("EREOF");
			break;
		case VK_PLAY:
			str = _T("Play");
			break;
		case VK_ZOOM:
			str = _T("Zoom");
			break;
		case VK_NONAME:
			str = _T("Noname");
			break;
		case VK_PA1:
			str = _T("PA1");
			break;
		case VK_OEM_CLEAR:
			str = _T("OEM Clear");
			break;
		case 0x07:
		case 0x0E:
		case 0x0F:
		case 0x16:
		case 0x1A:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F:
		case 0x40:
			str.Format(_T("Undefined (0x%02x)"), (TCHAR)a.key);
			break;
		case 0x0A:
		case 0x0B:
		case 0x5E:
		case 0xB8:
		case 0xB9:
		case 0xC1:
		case 0xC2:
		case 0xC3:
		case 0xC4:
		case 0xC5:
		case 0xC6:
		case 0xC7:
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF:
		case 0xD0:
		case 0xD1:
		case 0xD2:
		case 0xD3:
		case 0xD4:
		case 0xD5:
		case 0xD6:
		case 0xD7:
		case 0xE0:
			str.Format(_T("Reserved (0x%02x)"), (TCHAR)a.key);
			break;
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F:
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xE8:
			str.Format(_T("Unassigned (0x%02x)"), (TCHAR)a.key);
			break;
		case 0xFF:
			str = _T("Multimedia keys");
			break;
		default:
			//	if ('0' <= a.key && a.key <= '9' || 'A' <= a.key && a.key <= 'Z')
			str.Format(_T("%c"), (TCHAR)a.key);
			break;
	}

	if (a.fVirt&(FCONTROL|FALT|FSHIFT)) {
		str = MakeAccelModLabel(a.fVirt) + _T(" + ") + str;
	}

	str.Replace(_T(" + "), _T("+"));

	return(str);
}

CString CPPageAccelTbl::MakeMouseButtonLabel(UINT mouse)
{
	CString ret;
	switch (mouse) {
		case wmcmd::NONE:
		default:
			ret = ResStr(IDS_AG_NONE);
			break;
		case wmcmd::LDOWN:
			ret = _T("Left Down");
			break;
		case wmcmd::LUP:
			ret = _T("Left Up");
			break;
		case wmcmd::LDBLCLK:
			ret = _T("Left DblClk");
			break;
		case wmcmd::MDOWN:
			ret = _T("Middle Down");
			break;
		case wmcmd::MUP:
			ret = _T("Middle Up");
			break;
		case wmcmd::MDBLCLK:
			ret = _T("Middle DblClk");
			break;
		case wmcmd::RDOWN:
			ret = _T("Right Down");
			break;
		case wmcmd::RUP:
			ret = _T("Right Up");
			break;
		case wmcmd::RDBLCLK:
			ret = _T("Right DblClk");
			break;
		case wmcmd::X1DOWN:
			ret = _T("X1 Down");
			break;
		case wmcmd::X1UP:
			ret = _T("X1 Up");
			break;
		case wmcmd::X1DBLCLK:
			ret = _T("X1 DblClk");
			break;
		case wmcmd::X2DOWN:
			ret = _T("X2 Down");
			break;
		case wmcmd::X2UP:
			ret = _T("X2 Up");
			break;
		case wmcmd::X2DBLCLK:
			ret = _T("X2 DblClk");
			break;
		case wmcmd::WUP:
			ret = _T("Wheel Up");
			break;
		case wmcmd::WDOWN:
			ret = _T("Wheel Down");
			break;
	}
	return ret;
}

CString CPPageAccelTbl::MakeAppCommandLabel(UINT id)
{
	for (int i=0; i<_countof(g_CommandList); i++) {
		if (g_CommandList[i].appcmd == id) {
			return CString(g_CommandList[i].cmdname);
		}
	}
	return CString("");
}

void CPPageAccelTbl::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_WinLircAddr);
	DDX_Control(pDX, IDC_EDIT1, m_WinLircEdit);
	DDX_Control(pDX, IDC_STATICLINK, m_WinLircLink);
	DDX_Check(pDX, IDC_CHECK1, m_fWinLirc);
	DDX_Text(pDX, IDC_EDIT2, m_UIceAddr);
	DDX_Control(pDX, IDC_EDIT2, m_UIceEdit);
	DDX_Control(pDX, IDC_STATICLINK2, m_UIceLink);
	DDX_Check(pDX, IDC_CHECK9, m_fUIce);
	DDX_Check(pDX, IDC_CHECK2, m_fGlobalMedia);
}

BEGIN_MESSAGE_MAP(CPPageAccelTbl, CPPageBase)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST1, OnBeginlabeleditList)
	ON_NOTIFY(LVN_DOLABELEDIT, IDC_LIST1, OnDolabeleditList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST1, OnEndlabeleditList)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CPPageAccelTbl message handlers

static WNDPROC OldControlProc;

static LRESULT CALLBACK ControlProc(HWND control, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_KEYDOWN) {
		if ((LOWORD(wParam)== 'A' || LOWORD(wParam) == 'a')	&&(GetKeyState(VK_CONTROL) < 0)) {
			CPlayerListCtrl *pList = (CPlayerListCtrl*)CWnd::FromHandle(control);

			for (int i = 0, j = pList->GetItemCount(); i < j; i++) {
				pList->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
			}

			return 0;
		}
	}

	return CallWindowProc(OldControlProc, control, message, wParam, lParam); // call control's own windowproc
}

BOOL CPPageAccelTbl::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	m_wmcmds.RemoveAll();
	m_wmcmds.AddTail(&s.wmcmds);
	m_fWinLirc = s.fWinLirc;
	m_WinLircAddr = s.strWinLircAddr;
	m_fUIce = s.fUIce;
	m_UIceAddr = s.strUIceAddr;
	m_fGlobalMedia = s.fGlobalMedia;

	UpdateData(FALSE);

	CRect r;
	GetDlgItem(IDC_PLACEHOLDER)->GetWindowRect(r);
	ScreenToClient(r);

	m_list.CreateEx(
		WS_EX_CLIENTEDGE,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER,
		r, this, IDC_LIST1);

	m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);

	for (int i = 0, j = m_list.GetHeaderCtrl()->GetItemCount(); i < j; i++) {
		m_list.DeleteColumn(0);
	}

	m_list.InsertColumn(COL_CMD, ResStr(IDS_AG_COMMAND), LVCFMT_LEFT, s.AccelTblColWidth.cmd);
	m_list.InsertColumn(COL_KEY, ResStr(IDS_AG_KEY), LVCFMT_LEFT, s.AccelTblColWidth.key);
	m_list.InsertColumn(COL_ID, _T("ID"), LVCFMT_LEFT, s.AccelTblColWidth.id);
	m_list.InsertColumn(COL_MOUSE, ResStr(IDS_AG_MOUSE), LVCFMT_LEFT, s.AccelTblColWidth.mwnd);
	m_list.InsertColumn(COL_MOUSE_FS, ResStr(IDS_AG_MOUSE_FS), LVCFMT_LEFT, s.AccelTblColWidth.mfs);
	m_list.InsertColumn(COL_APPCMD, ResStr(IDS_AG_APP_COMMAND), LVCFMT_LEFT, s.AccelTblColWidth.appcmd);
	m_list.InsertColumn(COL_RMCMD, _T("RemoteCmd"), LVCFMT_LEFT, s.AccelTblColWidth.remcmd);
	m_list.InsertColumn(COL_RMREPCNT, _T("RepCnt"), LVCFMT_CENTER, s.AccelTblColWidth.repcnt);

	POSITION pos = m_wmcmds.GetHeadPosition();
	for (int i = 0; pos; i++) {
		int row = m_list.InsertItem(m_list.GetItemCount(), m_wmcmds.GetAt(pos).GetName(), COL_CMD);
		m_list.SetItemData(row, (DWORD_PTR)pos);
		m_wmcmds.GetNext(pos);
	}

	SetupList();

	// subclass the keylist control
	OldControlProc = (WNDPROC) SetWindowLongPtr(m_list.m_hWnd, GWLP_WNDPROC, (LONG_PTR) ControlProc);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageAccelTbl::OnApply()
{
	AfxGetMyApp()->UnregisterHotkeys();
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.wmcmds.RemoveAll();
	s.wmcmds.AddTail(&m_wmcmds);

	CAtlArray<ACCEL> pAccel;
	pAccel.SetCount(m_wmcmds.GetCount());
	POSITION pos = m_wmcmds.GetHeadPosition();
	for (int i = 0; pos; i++) {
		pAccel[i] = m_wmcmds.GetNext(pos);
	}
	if (s.hAccel) {
		DestroyAcceleratorTable(s.hAccel);
	}
	s.hAccel = CreateAcceleratorTable(pAccel.GetData(), pAccel.GetCount());

	GetParentFrame()->m_hAccelTable = s.hAccel;

	s.fWinLirc = !!m_fWinLirc;
	s.strWinLircAddr = m_WinLircAddr;
	if (s.fWinLirc) {
		s.WinLircClient.Connect(m_WinLircAddr);
	}
	s.fUIce = !!m_fUIce;
	s.strUIceAddr = m_UIceAddr;
	if (s.fUIce) {
		s.UIceClient.Connect(m_UIceAddr);
	}
	s.fGlobalMedia = !!m_fGlobalMedia;

	AfxGetMyApp()->RegisterHotkeys();

	s.AccelTblColWidth.bEnable = true;
	s.AccelTblColWidth.cmd		= m_list.GetColumnWidth(COL_CMD);
	s.AccelTblColWidth.key		= m_list.GetColumnWidth(COL_KEY);
	s.AccelTblColWidth.id		= m_list.GetColumnWidth(COL_ID);
	s.AccelTblColWidth.mwnd		= m_list.GetColumnWidth(COL_MOUSE);
	s.AccelTblColWidth.mfs		= m_list.GetColumnWidth(COL_MOUSE_FS);
	s.AccelTblColWidth.appcmd	= m_list.GetColumnWidth(COL_APPCMD);
	s.AccelTblColWidth.remcmd	= m_list.GetColumnWidth(COL_RMCMD);
	s.AccelTblColWidth.repcnt	= m_list.GetColumnWidth(COL_RMREPCNT);

	return __super::OnApply();
}

void CPPageAccelTbl::OnBnClickedButton1()
{
	m_list.SetFocus();

	for (int i = 0, j = m_list.GetItemCount(); i < j; i++) {
		m_list.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CPPageAccelTbl::OnBnClickedButton2()
{
	m_list.SetFocus();

	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (!pos) {
		return;
	}

	while (pos) {
		int ni = m_list.GetNextSelectedItem(pos);
		POSITION pi = (POSITION)m_list.GetItemData(ni);
		wmcmd& wc = m_wmcmds.GetAt(pi);
		wc.Restore();
	}
	AfxGetAppSettings().AccelTblColWidth.bEnable = false;
	SetupList();

	SetModified();
}

void CPPageAccelTbl::OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	*pResult = FALSE;

	if (pItem->iItem < 0) {
		return;
	}

	if (pItem->iSubItem == COL_KEY || pItem->iSubItem == COL_APPCMD
			|| pItem->iSubItem == COL_MOUSE || pItem->iSubItem == COL_MOUSE_FS
			|| pItem->iSubItem == COL_RMCMD || pItem->iSubItem == COL_RMREPCNT) {
		*pResult = TRUE;
	}
}

static BYTE s_mods[] = {0,FALT,FCONTROL,FSHIFT,FCONTROL|FALT,FCONTROL|FSHIFT,FALT|FSHIFT,FCONTROL|FALT|FSHIFT};

void CPPageAccelTbl::OnDolabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	if (pItem->iItem < 0) {
		*pResult = FALSE;
		return;
	}

	*pResult = TRUE;

	wmcmd& wc = m_wmcmds.GetAt((POSITION)m_list.GetItemData(pItem->iItem));

	CAtlList<CString> sl;
	int nSel = -1;

	switch (pItem->iSubItem) {
		case COL_KEY: {
			m_list.ShowInPlaceWinHotkey(pItem->iItem, pItem->iSubItem);
			CWinHotkeyCtrl* pWinHotkey = (CWinHotkeyCtrl*)m_list.GetDlgItem(IDC_WINHOTKEY1);
			UINT cod = 0, mod = 0;

			if (wc.fVirt & FALT) {
				mod |= MOD_ALT;
			}
			if (wc.fVirt & FCONTROL) {
				mod |= MOD_CONTROL;
			}
			if (wc.fVirt & FSHIFT) {
				mod |= MOD_SHIFT;
			}
			cod = wc.key;
			pWinHotkey->SetWinHotkey(cod, mod);
			break;
		}
		case COL_MOUSE:
			for (UINT i = 0; i < wmcmd::LAST; i++) {
				sl.AddTail(MakeMouseButtonLabel(i));
				if (wc.mouse == i) {
					nSel = i;
				}
			}

			m_list.ShowInPlaceComboBox(pItem->iItem, pItem->iSubItem, sl, nSel);
			break;
		case COL_MOUSE_FS:
			for (UINT i = 0; i < wmcmd::LAST; i++) {
				sl.AddTail(MakeMouseButtonLabel(i));
				if (wc.mouseFS == i) {
					nSel = i;
				}
			}

			m_list.ShowInPlaceComboBox(pItem->iItem, pItem->iSubItem, sl, nSel);
			break;
		case COL_APPCMD:
			for (int i = 0; i < _countof(g_CommandList); i++) {
				sl.AddTail(g_CommandList[i].cmdname);
				if (wc.appcmd == g_CommandList[i].appcmd) {
					nSel = i;
				}
			}

			m_list.ShowInPlaceComboBox(pItem->iItem, pItem->iSubItem, sl, nSel);
			break;
		case COL_RMCMD:
			m_list.ShowInPlaceEdit(pItem->iItem, pItem->iSubItem);
			break;
		case COL_RMREPCNT:
			m_list.ShowInPlaceEdit(pItem->iItem, pItem->iSubItem);
			break;
		default:
			*pResult = FALSE;
			break;
	}
}

void CPPageAccelTbl::OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	*pResult = FALSE;

	if (!m_list.m_fInPlaceDirty) {
		return;
	}

	if (pItem->iItem < 0) {
		return;
	}

	wmcmd& wc = m_wmcmds.GetAt((POSITION)m_list.GetItemData(pItem->iItem));

	switch (pItem->iSubItem) {
		case COL_KEY: {
			UINT cod, mod;
			CWinHotkeyCtrl* pWinHotkey = (CWinHotkeyCtrl*)m_list.GetDlgItem(IDC_WINHOTKEY1);
			pWinHotkey->GetWinHotkey(&cod, &mod);
			wc.fVirt = 0;
			if (mod & MOD_ALT) {
				wc.fVirt |= FALT;
			}
			if (mod & MOD_CONTROL) {
				wc.fVirt |= FCONTROL;
			}
			if (mod & MOD_SHIFT) {
				wc.fVirt |= FSHIFT;
			}
			wc.fVirt |= FVIRTKEY;
			wc.key = cod;

			CString str;
			HotkeyToString(cod, mod, str);
			m_list.SetItemText(pItem->iItem, pItem->iSubItem, str);

			*pResult = TRUE;
		}
		break;
		case COL_APPCMD: {
			int i = pItem->lParam;
			if (i >= 0 && i < _countof(g_CommandList)) {
				wc.appcmd = g_CommandList[i].appcmd;
				m_list.SetItemText(pItem->iItem, COL_APPCMD, pItem->pszText);
				*pResult = TRUE;
			}
		}
		break;
		case COL_MOUSE:
			wc.mouse = pItem->lParam;
			m_list.SetItemText(pItem->iItem, COL_MOUSE, pItem->pszText);
			break;
		case COL_MOUSE_FS:
			wc.mouseFS = pItem->lParam;
			m_list.SetItemText(pItem->iItem, COL_MOUSE_FS, pItem->pszText);
			break;
		case COL_RMCMD:
			wc.rmcmd = CStringA(CString(pItem->pszText)).Trim();
			wc.rmcmd.Replace(' ', '_');
			m_list.SetItemText(pItem->iItem, pItem->iSubItem, CString(wc.rmcmd));
			*pResult = TRUE;
			break;
		case COL_RMREPCNT:
			CString str = CString(pItem->pszText).Trim();
			wc.rmrepcnt = _tcstol(str, NULL, 10);
			str.Format(_T("%d"), wc.rmrepcnt);
			m_list.SetItemText(pItem->iItem, pItem->iSubItem, str);
			*pResult = TRUE;
			break;
	}

	if (*pResult) {
		SetModified();
	}
}

void CPPageAccelTbl::OnTimer(UINT_PTR nIDEvent)
{
	UpdateData();

	if (m_fWinLirc) {
		CString addr;
		m_WinLircEdit.GetWindowText(addr);
		AfxGetAppSettings().WinLircClient.Connect(addr);
	}

	m_WinLircEdit.Invalidate();

	if (m_fUIce) {
		CString addr;
		m_UIceEdit.GetWindowText(addr);
		AfxGetAppSettings().UIceClient.Connect(addr);
	}

	m_UIceEdit.Invalidate();

	m_counter++;

	__super::OnTimer(nIDEvent);
}

HBRUSH CPPageAccelTbl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);

	int status = -1;

	if (*pWnd == m_WinLircEdit) {
		status = AfxGetAppSettings().WinLircClient.GetStatus();
	} else if (*pWnd == m_UIceEdit) {
		status = AfxGetAppSettings().UIceClient.GetStatus();
	}

	if (status == 0 || (status == 2 && (m_counter&1))) {
		pDC->SetTextColor(0x0000ff);
	} else if (status == 1) {
		pDC->SetTextColor(0x008000);
	}

	return hbr;
}

BOOL CPPageAccelTbl::OnSetActive()
{
	SetTimer(1, 1000, NULL);

	return CPPageBase::OnSetActive();
}

BOOL CPPageAccelTbl::OnKillActive()
{
	KillTimer(1);

	return CPPageBase::OnKillActive();
}

void CPPageAccelTbl::OnCancel()
{
	AppSettings& s = AfxGetAppSettings();

	if (!s.fWinLirc) {
		s.WinLircClient.DisConnect();
	}
	if (!s.fUIce) {
		s.UIceClient.DisConnect();
	}

	__super::OnCancel();
}
