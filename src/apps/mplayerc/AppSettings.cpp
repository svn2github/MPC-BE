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
#include "AppSettings.h"
#include <atlpath.h>
#include "MiniDump.h"
#include "../../DSUtil/SysVersion.h"
#include "../../DSUtil/WinAPIUtils.h"

CAppSettings::CAppSettings()
	: fInitialized(false)
	, MRU(0, _T("Recent File List"), _T("File%d"), 20)
	, MRUDub(0, _T("Recent Dub List"), _T("Dub%d"), 20)
	, hAccel(NULL)
	, nCmdlnWebServerPort(-1)
	, fShowDebugInfo(false)
	, fIsFSWindow(false)
	, fShadersNeedSave(false)
	, fReset(false)
	, scalefont(1.0)
	, p_MainWnd(NULL)
{
	// Internal source filters
	SrcFiltersKeys[SRC_AMR]					= _T("src_amr");
	SrcFiltersKeys[SRC_AVI]					= _T("src_avi");
	SrcFiltersKeys[SRC_APE]					= _T("src_ape");
	SrcFiltersKeys[SRC_CDDA]				= _T("src_cdda");
	SrcFiltersKeys[SRC_CDXA]				= _T("src_cdxa");
	SrcFiltersKeys[SRC_DSM]					= _T("src_dsm");
	SrcFiltersKeys[SRC_DTSAC3]				= _T("src_dtsac3");
	SrcFiltersKeys[SRC_VTS]					= _T("src_vts");
	SrcFiltersKeys[SRC_FLIC]				= _T("src_flic");
	SrcFiltersKeys[SRC_FLAC]				= _T("src_flac");
	SrcFiltersKeys[SRC_FLV]					= _T("src_flv");
	SrcFiltersKeys[SRC_MATROSKA]			= _T("src_matroska");
	SrcFiltersKeys[SRC_MP4]					= _T("src_mp4");
	SrcFiltersKeys[SRC_MPA]					= _T("src_mpa");
	SrcFiltersKeys[SRC_MPEG]				= _T("src_mpeg");
	SrcFiltersKeys[SRC_MUSEPACK]			= _T("src_musepack");
	SrcFiltersKeys[SRC_OGG]					= _T("src_ogg");
	SrcFiltersKeys[SRC_RAWVIDEO]			= _T("src_rawvideo");
	SrcFiltersKeys[SRC_REAL]				= _T("src_real");
	SrcFiltersKeys[SRC_ROQ]					= _T("src_roq");
	SrcFiltersKeys[SRC_SHOUTCAST]			= _T("src_shoutcast");
	SrcFiltersKeys[SRC_TAK]					= _T("src_tak");
	SrcFiltersKeys[SRC_TTA]					= _T("src_tta");
	SrcFiltersKeys[SRC_WAV]					= _T("src_wav");
	SrcFiltersKeys[SRC_WAVPACK]				= _T("src_wavpack");
	SrcFiltersKeys[SRC_UDP]					= _T("src_udp");

	// Internal DXVA decoders
	DXVAFiltersKeys[VDEC_DXVA_H264]			= _T("vdec_dxva_h264");
	DXVAFiltersKeys[VDEC_DXVA_MPEG2]		= _T("vdec_dxva_mpeg2");
	DXVAFiltersKeys[VDEC_DXVA_VC1]			= _T("vdec_dxva_vc1");
	DXVAFiltersKeys[VDEC_DXVA_WMV3]			= _T("vdec_dxva_wmv3");

	// Internal video decoders
	VideoFiltersKeys[VDEC_AMV]				= _T("vdec_amv");
	VideoFiltersKeys[VDEC_PRORES]			= _T("vdec_prores");
	VideoFiltersKeys[VDEC_DNXHD]			= _T("vdec_dnxhd");
	VideoFiltersKeys[VDEC_BINK]				= _T("vdec_bink");
	VideoFiltersKeys[VDEC_CANOPUS]			= _T("vdec_canopus");
	VideoFiltersKeys[VDEC_CINEPAK]			= _T("vdec_cinepak");
	VideoFiltersKeys[VDEC_DIRAC]			= _T("vdec_dirac");
	VideoFiltersKeys[VDEC_DIVX]				= _T("vdec_divx");
	VideoFiltersKeys[VDEC_DV]				= _T("vdec_dv");
	VideoFiltersKeys[VDEC_FLV]				= _T("vdec_flv");
	VideoFiltersKeys[VDEC_H263]				= _T("vdec_h263");
	VideoFiltersKeys[VDEC_H264]				= _T("vdec_h264");
	VideoFiltersKeys[VDEC_HEVC]				= _T("vdec_hevc");
	VideoFiltersKeys[VDEC_INDEO]			= _T("vdec_indeo");
	VideoFiltersKeys[VDEC_LOSSLESS]			= _T("vdec_lossless");
	VideoFiltersKeys[VDEC_MJPEG]			= _T("vdec_mjpeg");
	VideoFiltersKeys[VDEC_MPEG1]			= _T("vdec_mpeg1");
	VideoFiltersKeys[VDEC_MPEG2]			= _T("vdec_mpeg2");
	VideoFiltersKeys[VDEC_LIBMPEG2_MPEG1]	= _T("vdec_libmpeg2_mpeg1");
	VideoFiltersKeys[VDEC_LIBMPEG2_MPEG2]	= _T("vdec_libmpeg2_mpeg2");
	VideoFiltersKeys[VDEC_MSMPEG4]			= _T("vdec_msmpeg4");
	VideoFiltersKeys[VDEC_PNG]				= _T("vdec_png");
	VideoFiltersKeys[VDEC_QT]				= _T("vdec_qt");
	VideoFiltersKeys[VDEC_SCREEN]			= _T("vdec_screen");
	VideoFiltersKeys[VDEC_SVQ]				= _T("vdec_svq");
	VideoFiltersKeys[VDEC_THEORA]			= _T("vdec_theora");
	VideoFiltersKeys[VDEC_UT]				= _T("vdec_ut");
	VideoFiltersKeys[VDEC_VC1]				= _T("vdec_vc1");
	VideoFiltersKeys[VDEC_VP356]			= _T("vdec_vp356");
	VideoFiltersKeys[VDEC_VP789]			= _T("vdec_vp789");
	VideoFiltersKeys[VDEC_WMV]				= _T("vdec_wmv");
	VideoFiltersKeys[VDEC_XVID]				= _T("vdec_xvid");
	VideoFiltersKeys[VDEC_REAL]				= _T("vdec_real");
	VideoFiltersKeys[VDEC_UNCOMPRESSED]		= _T("vdec_uncompressed");

	// Internal audio decoders
	AudioFiltersKeys[ADEC_AAC]				= _T("adec_aac");
	AudioFiltersKeys[ADEC_AC3]				= _T("adec_ac3");
	AudioFiltersKeys[ADEC_ALAC]				= _T("adec_alac");
	AudioFiltersKeys[ADEC_ALS]				= _T("adec_als");
	AudioFiltersKeys[ADEC_AMR]				= _T("adec_amr");
	AudioFiltersKeys[ADEC_APE]				= _T("adec_ape");
	AudioFiltersKeys[ADEC_BINK]				= _T("adec_bink");
	AudioFiltersKeys[ADEC_TRUESPEECH]		= _T("adec_truespeech");
	AudioFiltersKeys[ADEC_DTS]				= _T("adec_dts");
	AudioFiltersKeys[ADEC_FLAC]				= _T("adec_flac");
	AudioFiltersKeys[ADEC_INDEO]			= _T("adec_indeo");
	AudioFiltersKeys[ADEC_LPCM]				= _T("adec_lpcm");
	AudioFiltersKeys[ADEC_MPA]				= _T("adec_mpa");
	AudioFiltersKeys[ADEC_MUSEPACK]			= _T("adec_musepack");
	AudioFiltersKeys[ADEC_NELLY]			= _T("adec_nelly");
	AudioFiltersKeys[ADEC_OPUS]				= _T("adec_opus");
	AudioFiltersKeys[ADEC_PS2]				= _T("adec_ps2");
	AudioFiltersKeys[ADEC_QDM2]				= _T("adec_qdm2");
	AudioFiltersKeys[ADEC_REAL]				= _T("adec_real");
	AudioFiltersKeys[ADEC_SPEEX]			= _T("adec_speex");
	AudioFiltersKeys[ADEC_TAK]				= _T("adec_tak");
	AudioFiltersKeys[ADEC_TTA]				= _T("adec_tta");
	AudioFiltersKeys[ADEC_VORBIS]			= _T("adec_vorbis");
	AudioFiltersKeys[ADEC_VOXWARE]			= _T("adec_voxware");
	AudioFiltersKeys[ADEC_WAVPACK]			= _T("adec_wavpack");
	AudioFiltersKeys[ADEC_WMA]				= _T("adec_wma");
	AudioFiltersKeys[ADEC_WMA9]				= _T("adec_wma9");
	AudioFiltersKeys[ADEC_WMALOSSLESS]		= _T("adec_wmalossless");
	AudioFiltersKeys[ADEC_WMAVOICE]			= _T("adec_wmavoice");
	AudioFiltersKeys[ADEC_PCM_ADPCM]		= _T("adec_pcm_adpcm");
}

void CAppSettings::CreateCommands()
{
#define ADDCMD(cmd) wmcmds.AddTail(wmcmd##cmd)
	ADDCMD((ID_FILE_OPENQUICK,					'Q', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_0));
	ADDCMD((ID_FILE_OPENMEDIA,					'O', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_OPEN_FILE));
	ADDCMD((ID_FILE_OPENDVD,					'D', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_OPEN_DVD));
	ADDCMD((ID_FILE_OPENDEVICE,					'V', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_OPEN_DEVICE));
	ADDCMD((ID_FILE_REOPEN,						'E', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_REOPEN));

	ADDCMD((ID_FILE_SAVE_COPY,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_SAVE_AS));
	ADDCMD((ID_FILE_SAVE_IMAGE,					'I', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_SAVE_IMAGE));
	ADDCMD((ID_FILE_SAVE_IMAGE_AUTO,		  VK_F5, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_6));
	ADDCMD((ID_FILE_SAVE_THUMBNAILS,			  0, FVIRTKEY|FNOINVERT,				IDS_FILE_SAVE_THUMBNAILS));

	ADDCMD((ID_FILE_LOAD_SUBTITLE,				'L', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_LOAD_SUBTITLE));
	ADDCMD((ID_FILE_SAVE_SUBTITLE,				'S', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_SAVE_SUBTITLE));
	ADDCMD((ID_FILE_CLOSEPLAYLIST,				'C', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_CLOSE));
	ADDCMD((ID_FILE_PROPERTIES,				 VK_F10, FVIRTKEY|FSHIFT|FNOINVERT,			IDS_AG_PROPERTIES));
	ADDCMD((ID_FILE_EXIT,						'X', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_EXIT));
	ADDCMD((ID_PLAY_PLAYPAUSE,			   VK_SPACE, FVIRTKEY|FNOINVERT,				IDS_AG_PLAYPAUSE,	APPCOMMAND_MEDIA_PLAY_PAUSE, wmcmd::LDOWN, wmcmd::LDOWN));
	ADDCMD((ID_PLAY_PLAY,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_PLAY,		APPCOMMAND_MEDIA_PLAY));
	ADDCMD((ID_PLAY_PAUSE,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_PAUSE,		APPCOMMAND_MEDIA_PAUSE));
	ADDCMD((ID_PLAY_STOP,			  VK_OEM_PERIOD, FVIRTKEY|FNOINVERT,				IDS_AG_STOP,		APPCOMMAND_MEDIA_STOP));

	ADDCMD((ID_MENU_NAVIGATE_SUBTITLES,			'S', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_MENU_SUBTITLELANG));
	ADDCMD((ID_MENU_NAVIGATE_AUDIO,				'A', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_MENU_AUDIOLANG));
	ADDCMD((ID_MENU_NAVIGATE_JUMPTO,			'G', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_MENU_JUMPTO));

	ADDCMD((ID_PLAY_FRAMESTEP,			   VK_RIGHT, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_FRAMESTEP));
	ADDCMD((ID_PLAY_FRAMESTEPCANCEL,		VK_LEFT, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_16));
	ADDCMD((ID_PLAY_GOTO,						'G', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_GO_TO));
	ADDCMD((ID_PLAY_INCRATE,				  VK_UP, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_INCREASE_RATE));
	ADDCMD((ID_PLAY_DECRATE,				VK_DOWN, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_DECREASE_RATE));
	ADDCMD((ID_PLAY_RESETRATE,					'R', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_RESET_RATE));
	ADDCMD((ID_PLAY_INCAUDDELAY,			 VK_ADD, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_21));
	ADDCMD((ID_PLAY_DECAUDDELAY,		VK_SUBTRACT, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_22));
	ADDCMD((ID_PLAY_SEEKFORWARDSMALL,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_23));
	ADDCMD((ID_PLAY_SEEKBACKWARDSMALL,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_24));
	ADDCMD((ID_PLAY_SEEKFORWARDMED,		   VK_RIGHT, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_25));
	ADDCMD((ID_PLAY_SEEKBACKWARDMED,		VK_LEFT, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_26));
	ADDCMD((ID_PLAY_SEEKFORWARDLARGE,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_27));
	ADDCMD((ID_PLAY_SEEKBACKWARDLARGE,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_28));
	ADDCMD((ID_PLAY_SEEKKEYFORWARD,		   VK_RIGHT, FVIRTKEY|FSHIFT|FNOINVERT,			IDS_MPLAYERC_29));
	ADDCMD((ID_PLAY_SEEKKEYBACKWARD,		VK_LEFT, FVIRTKEY|FSHIFT|FNOINVERT,			IDS_MPLAYERC_30));
	ADDCMD((ID_NAVIGATE_SKIPFORWARD,		VK_NEXT, FVIRTKEY|FNOINVERT,				IDS_AG_NEXT,		APPCOMMAND_MEDIA_NEXTTRACK, wmcmd::X2DOWN, wmcmd::X2DOWN));
	ADDCMD((ID_NAVIGATE_SKIPBACK,		   VK_PRIOR, FVIRTKEY|FNOINVERT,				IDS_AG_PREVIOUS,	APPCOMMAND_MEDIA_PREVIOUSTRACK, wmcmd::X1DOWN, wmcmd::X1DOWN));
	ADDCMD((ID_NAVIGATE_SKIPFORWARDFILE,	VK_NEXT, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_NEXT_FILE));
	ADDCMD((ID_NAVIGATE_SKIPBACKFILE,	   VK_PRIOR, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PREVIOUS_FILE));
	ADDCMD((ID_NAVIGATE_TUNERSCAN,				'T', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_NAVIGATE_TUNERSCAN));
	ADDCMD((ID_FAVORITES_QUICKADDFAVORITE,		'Q', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_FAVORITES_QUICKADDFAVORITE));
	ADDCMD((ID_VIEW_CAPTIONMENU,				'0', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_CAPTION));
	ADDCMD((ID_VIEW_SEEKER,						'1', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_SEEKER));
	ADDCMD((ID_VIEW_CONTROLS,					'2', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_CONTROLS));
	ADDCMD((ID_VIEW_INFORMATION,				'3', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_INFO));
	ADDCMD((ID_VIEW_STATISTICS,					'4', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_STATS));
	ADDCMD((ID_VIEW_STATUS,						'5', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_STATUS));
	ADDCMD((ID_VIEW_SUBRESYNC,					'6', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_SUBRESYNC));
	ADDCMD((ID_VIEW_PLAYLIST,					'7', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_PLAYLIST));
	ADDCMD((ID_VIEW_CAPTURE,					'8', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_CAPTURE));
	ADDCMD((ID_VIEW_SHADEREDITOR,				'9', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_SHADER));
	ADDCMD((ID_VIEW_PRESETS_MINIMAL,			'1', FVIRTKEY|FNOINVERT,				IDS_AG_VIEW_MINIMAL));
	ADDCMD((ID_VIEW_PRESETS_COMPACT,			'2', FVIRTKEY|FNOINVERT,				IDS_AG_VIEW_COMPACT));
	ADDCMD((ID_VIEW_PRESETS_NORMAL,				'3', FVIRTKEY|FNOINVERT,				IDS_AG_VIEW_NORMAL));
	ADDCMD((ID_VIEW_FULLSCREEN,			  VK_RETURN, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_FULLSCREEN, 0, wmcmd::LDBLCLK, wmcmd::LDBLCLK));
	ADDCMD((ID_VIEW_FULLSCREEN_SECONDARY, VK_RETURN, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_39));
	ADDCMD((ID_VIEW_ZOOM_50,					'1', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_50));
	ADDCMD((ID_VIEW_ZOOM_100,					'2', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_100));
	ADDCMD((ID_VIEW_ZOOM_200,					'3', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_200));
	ADDCMD((ID_VIEW_ZOOM_AUTOFIT,				'4', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_AUTO_FIT));
	ADDCMD((ID_ASPECTRATIO_NEXT,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_NEXT_AR_PRESET));
	ADDCMD((ID_VIEW_VF_HALF,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_HALF));
	ADDCMD((ID_VIEW_VF_NORMAL,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_NORMAL));
	ADDCMD((ID_VIEW_VF_DOUBLE,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_DOUBLE));
	ADDCMD((ID_VIEW_VF_STRETCH,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_STRETCH));
	ADDCMD((ID_VIEW_VF_FROMINSIDE,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_INSIDE));
	ADDCMD((ID_VIEW_VF_ZOOM1,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_ZOOM1));
	ADDCMD((ID_VIEW_VF_ZOOM2,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_ZOOM2));
	ADDCMD((ID_VIEW_VF_FROMOUTSIDE,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_OUTSIDE));
	ADDCMD((ID_VIEW_VF_SWITCHZOOM,				'P', FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_SWITCHZOOM));
	ADDCMD((ID_ONTOP_ALWAYS,					'A', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_ALWAYS_ON_TOP));
	ADDCMD((ID_VIEW_RESET,				 VK_NUMPAD5, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_RESET));
	ADDCMD((ID_VIEW_INCSIZE,			 VK_NUMPAD9, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_INC_SIZE));
	ADDCMD((ID_VIEW_INCWIDTH,			 VK_NUMPAD6, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_INC_WIDTH));
	ADDCMD((ID_VIEW_INCHEIGHT,			 VK_NUMPAD8, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_47));
	ADDCMD((ID_VIEW_DECSIZE,			 VK_NUMPAD1, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_DEC_SIZE));
	ADDCMD((ID_VIEW_DECWIDTH,			 VK_NUMPAD4, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_DEC_WIDTH));
	ADDCMD((ID_VIEW_DECHEIGHT,			 VK_NUMPAD2, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_50));
	ADDCMD((ID_PANSCAN_CENTER,			 VK_NUMPAD5, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_CENTER));
	ADDCMD((ID_PANSCAN_MOVELEFT,		 VK_NUMPAD4, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_LEFT));
	ADDCMD((ID_PANSCAN_MOVERIGHT,		 VK_NUMPAD6, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_RIGHT));
	ADDCMD((ID_PANSCAN_MOVEUP,			 VK_NUMPAD8, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_UP));
	ADDCMD((ID_PANSCAN_MOVEDOWN,		 VK_NUMPAD2, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_DOWN));
	ADDCMD((ID_PANSCAN_MOVEUPLEFT,		 VK_NUMPAD7, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_UPLEFT));
	ADDCMD((ID_PANSCAN_MOVEUPRIGHT,		 VK_NUMPAD9, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_UPRIGHT));
	ADDCMD((ID_PANSCAN_MOVEDOWNLEFT,	 VK_NUMPAD1, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_DOWNLEFT));
	ADDCMD((ID_PANSCAN_MOVEDOWNRIGHT,	 VK_NUMPAD3, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_59));
	ADDCMD((ID_PANSCAN_ROTATEXP,		 VK_NUMPAD8, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEX_P));
	ADDCMD((ID_PANSCAN_ROTATEXM,		 VK_NUMPAD2, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEX_M));
	ADDCMD((ID_PANSCAN_ROTATEYP,		 VK_NUMPAD4, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEY_P));
	ADDCMD((ID_PANSCAN_ROTATEYM,		 VK_NUMPAD6, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEY_M));
	ADDCMD((ID_PANSCAN_ROTATEZP,		 VK_NUMPAD1, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEZ_P));
	ADDCMD((ID_PANSCAN_ROTATEZM,		 VK_NUMPAD3, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEZ_M));
	ADDCMD((ID_VOLUME_UP,					  VK_UP, FVIRTKEY|FNOINVERT,				IDS_AG_VOLUME_UP,   0, wmcmd::WUP, wmcmd::WUP));
	ADDCMD((ID_VOLUME_DOWN,					VK_DOWN, FVIRTKEY|FNOINVERT,				IDS_AG_VOLUME_DOWN, 0, wmcmd::WDOWN, wmcmd::WDOWN));
	ADDCMD((ID_VOLUME_MUTE,						'M', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_VOLUME_MUTE, 0));
	ADDCMD((ID_VOLUME_BOOST_INC,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_INC));
	ADDCMD((ID_VOLUME_BOOST_DEC,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_DEC));
	ADDCMD((ID_VOLUME_BOOST_MIN,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_MIN));
	ADDCMD((ID_VOLUME_BOOST_MAX,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_MAX));
	ADDCMD((ID_CUSTOM_CHANNEL_MAPPING,			  0, FVIRTKEY|FNOINVERT,				IDS_CUSTOM_CHANNEL_MAPPING));
	ADDCMD((ID_NORMALIZE,						  0, FVIRTKEY|FNOINVERT,				IDS_NORMALIZE));
	ADDCMD((ID_REGAIN_VOLUME,					  0, FVIRTKEY|FNOINVERT,				IDS_REGAIN_VOLUME));
	ADDCMD((ID_COLOR_BRIGHTNESS_INC,			  0, FVIRTKEY|FNOINVERT,				IDS_BRIGHTNESS_INC));
	ADDCMD((ID_COLOR_BRIGHTNESS_DEC,			  0, FVIRTKEY|FNOINVERT,				IDS_BRIGHTNESS_DEC));
	ADDCMD((ID_COLOR_CONTRAST_INC,				  0, FVIRTKEY|FNOINVERT,				IDS_CONTRAST_INC));
	ADDCMD((ID_COLOR_CONTRAST_DEC,				  0, FVIRTKEY|FNOINVERT,				IDS_CONTRAST_DEC));
	//ADDCMD((ID_COLOR_HUE_INC,					  0, FVIRTKEY|FNOINVERT,				IDS_HUE_INC)); // nobody need this feature
	//ADDCMD((ID_COLOR_HUE_DEC,					  0, FVIRTKEY|FNOINVERT,				IDS_HUE_DEC)); // nobody need this feature
	ADDCMD((ID_COLOR_SATURATION_INC,			  0, FVIRTKEY|FNOINVERT,				IDS_SATURATION_INC));
	ADDCMD((ID_COLOR_SATURATION_DEC,			  0, FVIRTKEY|FNOINVERT,				IDS_SATURATION_DEC));
	ADDCMD((ID_COLOR_RESET,						  0, FVIRTKEY|FNOINVERT,				IDS_RESET_COLOR));
	ADDCMD((ID_NAVIGATE_TITLEMENU,				'T', FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_63));
	ADDCMD((ID_NAVIGATE_ROOTMENU,				'R', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_ROOT_MENU));
	ADDCMD((ID_NAVIGATE_SUBPICTUREMENU,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_65));
	ADDCMD((ID_NAVIGATE_AUDIOMENU,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_66));
	ADDCMD((ID_NAVIGATE_ANGLEMENU,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_67));
	ADDCMD((ID_NAVIGATE_CHAPTERMENU,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_68));
	ADDCMD((ID_NAVIGATE_MENU_LEFT,			VK_LEFT, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_MENU_LEFT));
	ADDCMD((ID_NAVIGATE_MENU_RIGHT,		   VK_RIGHT, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_70));
	ADDCMD((ID_NAVIGATE_MENU_UP,			  VK_UP, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_MENU_UP));
	ADDCMD((ID_NAVIGATE_MENU_DOWN,			VK_DOWN, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_MENU_DOWN));
	ADDCMD((ID_NAVIGATE_MENU_ACTIVATE,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_73));
	ADDCMD((ID_NAVIGATE_MENU_BACK,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_DVD_MENU_BACK));
	ADDCMD((ID_NAVIGATE_MENU_LEAVE,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_75));
	ADDCMD((ID_BOSS,							'B', FVIRTKEY|FNOINVERT,				IDS_AG_BOSS_KEY));
	ADDCMD((ID_MENU_PLAYER_SHORT,			VK_APPS, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_77, 0, wmcmd::RUP, wmcmd::RUP));
	ADDCMD((ID_MENU_PLAYER_LONG,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_78));
	ADDCMD((ID_MENU_FILTERS,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_FILTERS_MENU));
	ADDCMD((ID_VIEW_OPTIONS,					'O', FVIRTKEY|FNOINVERT,				IDS_AG_OPTIONS));
	ADDCMD((ID_STREAM_AUDIO_NEXT,				'A', FVIRTKEY|FNOINVERT,				IDS_AG_NEXT_AUDIO));
	ADDCMD((ID_STREAM_AUDIO_PREV,				'A', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_AG_PREV_AUDIO));
	ADDCMD((ID_STREAM_SUB_NEXT,					'S', FVIRTKEY|FNOINVERT,				IDS_AG_NEXT_SUBTITLE));
	ADDCMD((ID_STREAM_SUB_PREV,					'S', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_AG_PREV_SUBTITLE));
	ADDCMD((ID_STREAM_SUB_ONOFF,				'W', FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_85));
	ADDCMD((ID_SUBTITLES_SUBITEM_START+3,		  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_86));
	// subtitle position
	ADDCMD((ID_SUB_POS_UP,					  VK_UP, FVIRTKEY|FCONTROL|FSHIFT|FNOINVERT,	IDS_SUB_POS_UP));
	ADDCMD((ID_SUB_POS_DOWN,				VK_DOWN, FVIRTKEY|FCONTROL|FSHIFT|FNOINVERT,	IDS_SUB_POS_DOWN));
	ADDCMD((ID_SUB_POS_LEFT,				VK_LEFT, FVIRTKEY|FCONTROL|FSHIFT|FNOINVERT,	IDS_SUB_POS_LEFT));
	ADDCMD((ID_SUB_POS_RIGHT,			   VK_RIGHT, FVIRTKEY|FCONTROL|FSHIFT|FNOINVERT,	IDS_SUB_POS_RIGHT));
	ADDCMD((ID_SUB_POS_RESTORE,			  VK_DELETE, FVIRTKEY|FCONTROL|FSHIFT|FNOINVERT,	IDS_SUB_POS_RESTORE));
	//
	ADDCMD((ID_FILE_ISDB_DOWNLOAD,				'D', FVIRTKEY|FNOINVERT,				IDS_DOWNLOAD_SUBS));
	ADDCMD((ID_OGM_AUDIO_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_87));
	ADDCMD((ID_OGM_AUDIO_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_88));
	ADDCMD((ID_OGM_SUB_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_89));
	ADDCMD((ID_OGM_SUB_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_90));
	ADDCMD((ID_DVD_ANGLE_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_91));
	ADDCMD((ID_DVD_ANGLE_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_92));
	ADDCMD((ID_DVD_AUDIO_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_93));
	ADDCMD((ID_DVD_AUDIO_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_94));
	ADDCMD((ID_DVD_SUB_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_95));
	ADDCMD((ID_DVD_SUB_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_96));
	ADDCMD((ID_DVD_SUB_ONOFF,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_97));
	ADDCMD((ID_VIEW_TEARING_TEST,				'T', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TEARING_TEST));
	ADDCMD((ID_VIEW_REMAINING_TIME,				'I', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_98));
	ADDCMD((ID_OSD_LOCAL_TIME,					'I', FVIRTKEY|FNOINVERT|FNOINVERT,		IDS_AG_OSD_LOCAL_TIME));
	ADDCMD((ID_OSD_FILE_NAME,					'I', FVIRTKEY|FSHIFT|FNOINVERT,		    IDS_AG_OSD_FILE_NAME));
	ADDCMD((ID_SHADERS_TOGGLE,					'P', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AT_TOGGLE_SHADER));
	ADDCMD((ID_SHADERS_TOGGLE_SCREENSPACE,		'P', FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AT_TOGGLE_SHADERSCREENSPACE));
	ADDCMD((ID_D3DFULLSCREEN_TOGGLE,			'F', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_99));
	ADDCMD((ID_GOTO_PREV_SUB,					'Y', FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_100, APPCOMMAND_BROWSER_BACKWARD));
	ADDCMD((ID_GOTO_NEXT_SUB,					'U', FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_101,  APPCOMMAND_BROWSER_FORWARD));
	ADDCMD((ID_SHIFT_SUB_DOWN,				VK_NEXT, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_102));
	ADDCMD((ID_SHIFT_SUB_UP,			   VK_PRIOR, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_103));
	ADDCMD((ID_VIEW_DISPLAYSTATS,				'J', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_DISPLAY_STATS));
	ADDCMD((ID_VIEW_RESETSTATS,					'R', FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_RESET_STATS));
	ADDCMD((ID_VIEW_VSYNC,						'V', FVIRTKEY|FNOINVERT,				IDS_AG_VSYNC));
	ADDCMD((ID_VIEW_ENABLEFRAMETIMECORRECTION,  'C', FVIRTKEY|FNOINVERT,				IDS_AG_ENABLEFRAMETIMECORRECTION));
	ADDCMD((ID_VIEW_VSYNCACCURATE,				'V', FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_VSYNCACCURATE));
	ADDCMD((ID_VIEW_VSYNCOFFSET_DECREASE,	  VK_UP, FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_VSYNCOFFSET_DECREASE));
	ADDCMD((ID_VIEW_VSYNCOFFSET_INCREASE,	VK_DOWN, FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_VSYNCOFFSET_INCREASE));
	ADDCMD((ID_SUB_DELAY_DOWN,				  VK_F1, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_104));
	ADDCMD((ID_SUB_DELAY_UP,				  VK_F2, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_105));

	ADDCMD((ID_AFTERPLAYBACK_CLOSE,				  0, FVIRTKEY|FNOINVERT,				IDS_AFTERPLAYBACK_CLOSE));
	ADDCMD((ID_AFTERPLAYBACK_STANDBY,			  0, FVIRTKEY|FNOINVERT,				IDS_AFTERPLAYBACK_STANDBY));
	ADDCMD((ID_AFTERPLAYBACK_HIBERNATE,			  0, FVIRTKEY|FNOINVERT,				IDS_AFTERPLAYBACK_HIBERNATE));
	ADDCMD((ID_AFTERPLAYBACK_SHUTDOWN,			  0, FVIRTKEY|FNOINVERT,				IDS_AFTERPLAYBACK_SHUTDOWN));
	ADDCMD((ID_AFTERPLAYBACK_LOGOFF,			  0, FVIRTKEY|FNOINVERT,				IDS_AFTERPLAYBACK_LOGOFF));
	ADDCMD((ID_AFTERPLAYBACK_LOCK,				  0, FVIRTKEY|FNOINVERT,				IDS_AFTERPLAYBACK_LOCK));
	ADDCMD((ID_AFTERPLAYBACK_EXIT,				  0, FVIRTKEY|FNOINVERT,				IDS_AFTERPLAYBACK_EXIT));
	ADDCMD((ID_AFTERPLAYBACK_DONOTHING,			  0, FVIRTKEY|FNOINVERT,				IDS_AFTERPLAYBACK_DONOTHING));
	ADDCMD((ID_AFTERPLAYBACK_NEXT,				  0, FVIRTKEY|FNOINVERT,				IDS_AFTERPLAYBACK_NEXT));

	ADDCMD((ID_VIEW_EDITLISTEDITOR,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_TOGGLE_EDITLISTEDITOR));
	ADDCMD((ID_EDL_IN,							  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDL_IN));
	ADDCMD((ID_EDL_OUT,							  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDL_OUT));
	ADDCMD((ID_EDL_NEWCLIP,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDL_NEW_CLIP));
	ADDCMD((ID_EDL_SAVE,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDL_SAVE));
	ADDCMD((ID_WINDOW_TO_PRIMARYSCREEN,			'1', FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_WINDOW_TO_PRIMARYSCREEN));

	ResetPositions();
#undef ADDCMD
}

CAppSettings::~CAppSettings()
{
	if (hAccel) {
		DestroyAcceleratorTable(hAccel);
	}
}

bool CAppSettings::IsD3DFullscreen() const
{
	if (iDSVideoRendererType == VIDRNDT_DS_VMR9RENDERLESS
			|| iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM
			|| iDSVideoRendererType == VIDRNDT_DS_SYNC) {
		return fD3DFullscreen || (nCLSwitches & CLSW_D3DFULLSCREEN);
	}

	return false;
}

CString CAppSettings::SelectedAudioRenderer() const
{
	CString strResult;
	if (AfxGetMyApp()->m_AudioRendererDisplayName_CL.GetLength() > 0) {
		strResult = AfxGetMyApp()->m_AudioRendererDisplayName_CL;
	} else {
		strResult = AfxGetAppSettings().strAudioRendererDisplayName;
	}

	return strResult;
}

void CAppSettings::ResetPositions()
{
	nCurrentDvdPosition		= -1;
	nCurrentFilePosition	= -1;
}

DVD_POSITION* CAppSettings::CurrentDVDPosition()
{
	if (nCurrentDvdPosition != -1) {
		return &DvdPosition[nCurrentDvdPosition];
	} else {
		return NULL;
	}
}

bool CAppSettings::NewDvd(ULONGLONG llDVDGuid)
{
	// Look for the DVD position
	for (int i = 0; i < MAX_DVD_POSITION; i++) {
		if (DvdPosition[i].llDVDGuid == llDVDGuid) {
			nCurrentDvdPosition = i;
			return false;
		}
	}

	// If DVD is unknown, we put it first
	for (int i = MAX_DVD_POSITION - 1; i > 0; i--) {
		memcpy(&DvdPosition[i], &DvdPosition[i-1], sizeof(DVD_POSITION));
	}
	DvdPosition[0].llDVDGuid	= llDVDGuid;
	nCurrentDvdPosition			= 0;
	return true;
}

FILE_POSITION* CAppSettings::CurrentFilePosition()
{
	if (nCurrentFilePosition != -1) {
		return &FilePosition[nCurrentFilePosition];
	} else {
		return NULL;
	}
}

bool CAppSettings::NewFile(LPCTSTR strFileName)
{
	// Look for the file position
	for (int i = 0; i < MAX_FILE_POSITION; i++) {
		if (FilePosition[i].strFile == strFileName) {
			nCurrentFilePosition = i;
			return false;
		}
	}

	// If it is unknown, we put it first
	for (int i = MAX_FILE_POSITION - 1; i > 0; i--) {
		FilePosition[i].strFile		= FilePosition[i-1].strFile;
		FilePosition[i].llPosition	= FilePosition[i-1].llPosition;
	}
	FilePosition[0].strFile		= strFileName;
	FilePosition[0].llPosition	= 0;
	nCurrentFilePosition		= 0;
	return true;
}

void CAppSettings::DeserializeHex (LPCTSTR strVal, BYTE* pBuffer, int nBufSize) const
{
	long lRes;

	for (int i = 0; i < nBufSize; i++) {
		_stscanf_s(strVal+(i*2), L"%02x", &lRes);
		pBuffer[i] = (BYTE)lRes;
	}
}

CString CAppSettings::SerializeHex (BYTE* pBuffer, int nBufSize) const
{
	CString strTemp;
	CString strResult;

	for (int i = 0; i < nBufSize; i++) {
		strTemp.Format(L"%02x", pBuffer[i]);
		strResult += strTemp;
	}

	return strResult;
}

void CAppSettings::SaveSettings()
{
	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp);

	if (!fInitialized) {
		return;
	}

	PerfomanceSettings.SaveSettings();
	FiltersPrioritySettings.SaveSettings();

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECAPTIONMENU, iCaptionMenuMode);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDENAVIGATION, fHideNavigation);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CONTROLSTATE, nCS);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULTVIDEOFRAME, iDefaultVideoSize);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPASPECTRATIO, fKeepAspectRatio);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COMPMONDESKARDIFF, fCompMonDeskARDiff);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUME, nVolume);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_BALANCE, nBalance);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MUTE, fMute);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOOPNUM, nLoops);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOOP, fLoopForever);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REWIND, fRewind);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ZOOM, iZoomLevel);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUME_STEP, nVolumeStep);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPEED_STEP, nSpeedStep);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MULTIINST, iMultipleInst);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTSTYLE, iTitleBarTextStyle);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTTITLE, fTitleBarTextTitle);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ONTOP, iOnTop);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TRAYICON, fTrayIcon);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOZOOM, fRememberZoomLevel);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLS, fShowBarsWhenFullScreen);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLSTIMEOUT, nShowBarsWhenFullScreenTimeOut);
	pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_FULLSCREENRES, (BYTE*)&AutoChangeFullscrRes, sizeof(AutoChangeFullscrRes));
	pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("AccelTblColWidth"), (BYTE*)&AccelTblColWidth, sizeof(AccelTblColWidth));
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATTHEEND, fExitFullScreenAtTheEnd);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATFOCUSLOST, fExitFullScreenAtFocusLost);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RESTORERESAFTEREXIT, fRestoreResAfterExit);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWPOS, fRememberWindowPos);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWSIZE, fRememberWindowSize);
	if (fSavePnSZoom) {
		CString str;
		str.Format(_T("%.3f,%.3f"), dZoomX, dZoomY);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_PANSCANZOOM, str);
	} else {
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_PANSCANZOOM, NULL);
	}
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SNAPTODESKTOPEDGES, fSnapToDesktopEdges);
	pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_LASTWINDOWRECT, (BYTE*)&rcLastWindowPos, sizeof(rcLastWindowPos));
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTWINDOWTYPE, (nLastWindowType == SIZE_MINIMIZED) ? SIZE_RESTORED : nLastWindowType);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_X, sizeAspectRatio.cx);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_Y, sizeAspectRatio.cy);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPHISTORY, fKeepHistory);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, iDSVideoRendererType);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, iRMVideoRendererType);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, iQTVideoRendererType);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SHUFFLEPLAYLISTITEMS, bShufflePlaylistItems);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERPLAYLISTITEMS, bRememberPlaylistItems);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDEPLAYLISTFULLSCREEN, bHidePlaylistFullScreen);
	pApp->WriteProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_REMEMBERPOS, bFavRememberPos);
	pApp->WriteProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_RELATIVEDRIVE, bFavRelativeDrive);

	UpdateRenderersData(true);

	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIORENDERERTYPE, strAudioRendererDisplayName);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SECONDAUDIORENDERER, strSecondAudioRendererDisplayName);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DUALAUDIOOUTPUT, fDualAudioOutput);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADAUDIO, fAutoloadAudio);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITIZEEXTERNALAUDIO, fPrioritizeExternalAudio);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOPATHS, strAudioPaths);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADSUBTITLES, fAutoloadSubtitles);

	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANGORDER, CString(strSubtitlesLanguageOrder));
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOSLANGORDER, CString(strAudiosLanguageOrder));
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_INTERNALSELECTTRACKLOGIC, fUseInternalSelectTrackLogic);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_BLOCKVSFILTER, fBlockVSFilter);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWORKERTHREADFOROPENING, fEnableWorkerThreadForOpening);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REPORTFAILEDPINS, fReportFailedPins);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RTSPHANDLER, iRtspHandler);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RTSPFILEEXTFIRST, fRtspFileExtFirst);

	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_DVDPATH, strDVDPath);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USEDVDPATH, fUseDVDPath);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MENULANG, idMenuLang);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOLANG, idAudioLang);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANG, idSubtitlesLang);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CLOSEDCAPTIONS, fClosedCaptions);

	CString style;
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SPLOGFONT, style <<= subdefstyle);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPOVERRIDEPLACEMENT, fOverridePlacement);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPHORPOS, nHorPos);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPVERPOS, nVerPos);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBDELAYINTERVAL, nSubDelayInterval);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLESUBTITLES, fEnableSubtitles);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FORCEDSUBTITLES, fForcedSubtitles);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITIZEEXTERNALSUBTITLES, fPrioritizeExternalSubtitles);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEINTERNALSUBTITLES, fDisableInternalSubtitles);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTORELOADEXTSUBTITLES, fAutoReloadExtSubtitles);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USE_SUBRESYNC, fUseSybresync);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLEPATHS, strSubtitlePaths);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USEDEFAULTSUBTITLESSTYLE, fUseDefaultSubtitlesStyle);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOSWITCHER, fEnableAudioSwitcher);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOTIMESHIFT, fAudioTimeShift);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOTIMESHIFT, iAudioTimeShift);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CUSTOMCHANNELMAPPING, fCustomChannelMapping);
	pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_SPEAKERTOCHANNELMAPPING, (BYTE*)pSpeakerToChannelMap, sizeof(pSpeakerToChannelMap));
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZE, fAudioNormalize);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZERECOVER, fAudioNormalizeRecover);

	CString strTemp;
	strTemp.Format( _T("%.1f"), dAudioBoost_dB);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, strTemp);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPEAKERCHANNELS, nSpeakerChannels);

	// Multi-monitor code
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_FULLSCREENMONITOR, CString(strFullScreenMonitor));
	// DeviceID
	pApp->WriteProfileString(IDS_R_SETTINGS, _T("FullScreenMonitorID"), CString(strFullScreenMonitorID));

	// Prevent Minimize when in Fullscreen mode on non default monitor
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_PREVENT_MINIMIZE, fPreventMinimize);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_WIN7TASKBAR, fUseWin7TaskBar);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_EXIT_AFTER_PB, fExitAfterPlayback);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NEXT_AFTER_PB, fNextInDirAfterPlayback);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NEXT_AFTER_PB_LOOPED, fNextInDirAfterPlaybackLooped);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NO_SEARCH_IN_FOLDER, fDontUseSearchInFolder);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USE_TIME_TOOLTIP, fUseTimeTooltip);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TIME_TOOLTIP_POSITION, nTimeTooltipPosition);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_OSD_SIZE, nOSDSize);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_MPC_OSD_FONT, strOSDFont);

	// Associated types with icon or not...
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASSOCIATED_WITH_ICON, fAssociatedWithIcons);
	// file/dir context menu
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CONTEXTFILES, bSetContextFiles);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CONTEXTDIR, bSetContextDir);

	// Last Open Dir
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_DIR, strLastOpenDir);
	// Last Saved Playlist Dir
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LAST_SAVED_PLAYLIST_DIR, strLastSavedPlaylistDir);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_D3DFULLSCREEN, fD3DFullscreen);
	//pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MONITOR_AUTOREFRESHRATE, fMonitorAutoRefreshRate);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_BRIGHTNESS, iBrightness);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_CONTRAST, iContrast);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_HUE, iHue);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_SATURATION, iSaturation);

	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLIST, strShaderList);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLISTSCREENSPACE, strShaderListScreenSpace);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADER, (int)fToggleShader);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADERSSCREENSPACE, (int)fToggleShaderScreenSpace);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SHOWOSD, (int)fShowOSD);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEEDLEDITOR, (int)fEnableEDLEditor);

	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LANGUAGE, CMPlayerCApp::languageResources[iLanguage].strcode);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FASTSEEK_KEYFRAME, (int)fFastSeek);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MINI_DUMP, (int)fMiniDump);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LCD_SUPPORT, (int)fLCDSupport);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SMARTSEEK, (int)fSmartSeek);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CHAPTER_MARKER, (int)fChapterMarker);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USE_FLYBAR, (int)fFlybar);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USE_FLYBAR_ONTOP, (int)fFlybarOnTop);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("OSDFontShadow"), (int)fFontShadow);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("OSDFontAA"), (int)fFontAA);

	// Save analog capture settings
	pApp->WriteProfileInt   (IDS_R_SETTINGS, IDS_RS_DEFAULT_CAPTURE, iDefaultCaptureDevice);
	pApp->WriteProfileString(IDS_R_CAPTURE, IDS_RS_VIDEO_DISP_NAME, strAnalogVideo);
	pApp->WriteProfileString(IDS_R_CAPTURE, IDS_RS_AUDIO_DISP_NAME, strAnalogAudio);
	pApp->WriteProfileInt   (IDS_R_CAPTURE, IDS_RS_COUNTRY,		 iAnalogCountry);

	// Save digital capture settings (BDA)
	pApp->WriteProfileString(IDS_R_DVB, IDS_RS_BDA_NETWORKPROVIDER, strBDANetworkProvider);
	pApp->WriteProfileString(IDS_R_DVB, IDS_RS_BDA_TUNER, strBDATuner);
	pApp->WriteProfileString(IDS_R_DVB, IDS_RS_BDA_RECEIVER, strBDAReceiver);
	//pApp->WriteProfileString(IDS_R_DVB, IDS_RS_BDA_STANDARD, strBDAStandard);
	pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_SCAN_FREQ_START, iBDAScanFreqStart);
	pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_SCAN_FREQ_END, iBDAScanFreqEnd);
	pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_BANDWIDTH, iBDABandwidth);
	pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_USE_OFFSET, fBDAUseOffset);
	pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_OFFSET, iBDAOffset);
	pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_BDA_IGNORE_ENCRYPTED_CHANNELS, fBDAIgnoreEncryptedChannels);
	pApp->WriteProfileInt(IDS_R_DVB, IDS_RS_DVB_LAST_CHANNEL, nDVBLastChannel);

	int			iChannel = 0;
	POSITION	pos = m_DVBChannels.GetHeadPosition();
	while (pos) {
		CString			strTemp;
		CString			strChannel;
		CDVBChannel&	Channel = m_DVBChannels.GetNext(pos);
		strTemp.Format(_T("%d"), iChannel);
		pApp->WriteProfileString(IDS_R_DVB, strTemp, Channel.ToString());
		iChannel++;
	}

	// playback positions for last played DVDs
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DVDPOS, (int)fRememberDVDPos);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOS, (int)fRememberFilePos);
	if (fKeepHistory) {
		for (int i = 0; i < MAX_DVD_POSITION; i++) {
			CString strDVDPos;
			CString strValue;

			strDVDPos.Format(_T("DVD Position %d"), i);
			strValue = SerializeHex((BYTE*)&DvdPosition[i], sizeof(DVD_POSITION));
			pApp->WriteProfileString(IDS_R_SETTINGS, strDVDPos, strValue);
		}

		// playback positions for last played files
		for (int i = 0; i < MAX_FILE_POSITION; i++) {
			CString strFilePos;
			CString strValue;

			strFilePos.Format(_T("File Name %d"), i);
			pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, FilePosition[i].strFile);
			strFilePos.Format(_T("File Position %d"), i);
			strValue.Format(_T("%I64d"), FilePosition[i].llPosition);
			pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, strValue);
		}
	}

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DVD_START_MAIN_TITLE, (int)fStartMainTitle);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFULLSCREEN, (int)fLastFullScreen);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_INTREALMEDIA, fIntRealMedia);
	//pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIARENDERLESS, fRealMediaRenderless);
	//pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_QUICKTIMERENDERER, iQuickTimeRenderer);
	//pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIAFPS, *((DWORD*)&dRealMediaQuickTimeFPS));

	pApp->WriteProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, NULL, NULL);
	for (int i = 0, j = m_pnspresets.GetCount(); i < j; i++) {
		CString str;
		str.Format(_T("Preset%d"), i);
		pApp->WriteProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, str, m_pnspresets[i]);
	}

	pApp->WriteProfileString(IDS_R_COMMANDS, NULL, NULL);
	pos = wmcmds.GetHeadPosition();
	for (int i = 0; pos;) {
		wmcmd& wc = wmcmds.GetNext(pos);
		if (wc.IsModified()) {
			CString str;
			str.Format(_T("CommandMod%d"), i);
			CString str2;
			str2.Format(_T("%d %x %x %s %d %u %u %u"),
						wc.cmd, wc.fVirt, wc.key,
						_T("\"") + CString(wc.rmcmd) +  _T("\""), wc.rmrepcnt,
						wc.mouse, wc.appcmd, wc.mouseFS);
			pApp->WriteProfileString(IDS_R_COMMANDS, str, str2);
			i++;
		}
	}

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WINLIRC, fWinLirc);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WINLIRCADDR, strWinLircAddr);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_UICE, fUIce);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_UICEADDR, strUIceAddr);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_GLOBALMEDIA, fGlobalMedia);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEXPTOOLBARS, fDisableXPToolbars);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ThemeBrightness"), nThemeBrightness);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ThemeRed"), nThemeRed);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ThemeGreen"), nThemeGreen);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ThemeBlue"), nThemeBlue);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("OSDTransparent"), nOSDTransparent);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("OSDBorder"), nOSDBorder);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("FileNameOnSeekBar"), fFileNameOnSeekBar);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CLRFACEABGR, clrFaceABGR);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CLROUTLINEABGR, clrOutlineABGR);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("OSDFontColor"), clrFontABGR);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("OSDGrad1Color"), clrGrad1ABGR);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("OSDGrad2Color"), clrGrad2ABGR);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTS, nJumpDistS);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTM, nJumpDistM);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTL, nJumpDistL);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LIMITWINDOWPROPORTIONS, fLimitWindowProportions);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTUSEDPAGE, nLastUsedPage);

	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("DlgPropX"), iDlgPropX);
	pApp->WriteProfileInt(IDS_R_SETTINGS, _T("DlgPropY"), iDlgPropY);

	m_Formats.UpdateData(true);

	// Internal filters
	for (int f = 0; f < SRC_LAST; f++) {
		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, SrcFiltersKeys[f], SrcFilters[f]);
	}
	for (int f = 0; f < VDEC_DXVA_LAST; f++) {
		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, DXVAFiltersKeys[f], DXVAFilters[f]);
	}
	for (int f = 0; f < VDEC_LAST; f++) {
		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, VideoFiltersKeys[f], VideoFilters[f]);
	}
	for (int f = 0; f < ADEC_LAST; f++) {
		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, AudioFiltersKeys[f], AudioFilters[f]);
	}

	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LOGOFILE, strLogoFileName);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOID, nLogoId);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOEXT, fLogoExternal);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECDROMSSUBMENU, fHideCDROMsSubMenu);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITY, dwPriority);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LAUNCHFULLSCREEN, fLaunchfullscreen);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWEBSERVER, fEnableWebServer);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPORT, nWebServerPort);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERQUALITY, nWebServerQuality);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPRINTDEBUGINFO, fWebServerPrintDebugInfo);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERUSECOMPRESSION, fWebServerUseCompression);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERLOCALHOSTONLY, fWebServerLocalhostOnly);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBROOT, strWebRoot);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBDEFINDEX, strWebDefIndex);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBSERVERCGI, strWebServerCGI);

	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTPATH, strSnapShotPath);
	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTEXT, strSnapShotExt);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBROWS, iThumbRows);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBCOLS, iThumbCols);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBWIDTH, iThumbWidth);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBQUALITY, iThumbQuality);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBLEVELPNG, iThumbLevelPNG);

	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_ISDB, strISDb);

	pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_FILTER_DIR, strLastOpenFilterDir);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_YOUTUBE_TAG, iYoutubeTag);
	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_YOUTUBE_SOURCE, iYoutubeSource);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMAINING_TIME, fRemainingTime);

	pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFILEINFOPAGE, nLastFileInfoPage);

	if (pApp->m_pszRegistryKey) {
		// WINBUG: on win2k this would crash WritePrivateProfileString
		pApp->WriteProfileInt(_T(""), _T(""), pApp->GetProfileInt(_T(""), _T(""), 0) ? 0 : 1);
	}

	if (fShadersNeedSave) { // This is a large data block. Save it only when really necessary.
		SaveShaders();
		fShadersNeedSave = false;
	}
}

void CAppSettings::SaveExternalFilters()
{
	// External Filter settings are saved for a long time. Use only when really necessary.
	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp);

	if (!fInitialized) {
		return;
	}

	for (unsigned int i = 0; ; i++) {
		CString key;
		key.Format(_T("%s\\%04u"), IDS_R_FILTERS, i);
		int j = pApp->GetProfileInt(key, _T("Enabled"), -1);
		pApp->WriteProfileString(key, NULL, NULL);
		if (j < 0) {
			break;
		}
	}
	pApp->WriteProfileString(IDS_R_FILTERS, NULL, NULL);

	unsigned int k = 0;
	POSITION pos = m_filters.GetHeadPosition();
	while (pos) {
		FilterOverride* f = m_filters.GetNext(pos);

		if (f->fTemporary) {
			continue;
		}

		CString key;
		key.Format(_T("%s\\%04u"), IDS_R_FILTERS, k);

		pApp->WriteProfileInt(key, _T("SourceType"), (int)f->type);
		pApp->WriteProfileInt(key, _T("Enabled"), (int)!f->fDisabled);
		if (f->type == FilterOverride::REGISTERED) {
			pApp->WriteProfileString(key, _T("DisplayName"), CString(f->dispname));
			pApp->WriteProfileString(key, _T("Name"), f->name);
			if (f->clsid == CLSID_NULL) {
				CComPtr<IBaseFilter> pBF;
				CStringW FriendlyName;
				if (CreateFilter(f->dispname, &pBF, FriendlyName)) {
					f->clsid = GetCLSID(pBF);
				}
			}
			pApp->WriteProfileString(key, _T("CLSID"), CStringFromGUID(f->clsid));
		} else if (f->type == FilterOverride::EXTERNAL) {
			pApp->WriteProfileString(key, _T("Path"), f->path);
			pApp->WriteProfileString(key, _T("Name"), f->name);
			pApp->WriteProfileString(key, _T("CLSID"), CStringFromGUID(f->clsid));
		}
		POSITION pos2 = f->backup.GetHeadPosition();
		for (unsigned int i = 0; pos2; i++) {
			CString val;
			val.Format(_T("org%04u"), i);
			pApp->WriteProfileString(key, val, CStringFromGUID(f->backup.GetNext(pos2)));
		}
		pos2 = f->guids.GetHeadPosition();
		for (unsigned int i = 0; pos2; i++) {
			CString val;
			val.Format(_T("mod%04u"), i);
			pApp->WriteProfileString(key, val, CStringFromGUID(f->guids.GetNext(pos2)));
		}
		pApp->WriteProfileInt(key, _T("LoadType"), f->iLoadType);
		pApp->WriteProfileInt(key, _T("Merit"), f->dwMerit);

		k++;
	}
}

void CAppSettings::LoadSettings(bool bForce/* = false*/)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp);

	UINT len;
	BYTE* ptr = NULL;

	if (fInitialized && !bForce) {
		return;
	}

	iDXVer = 0;
	CRegKey dxver;
	if (ERROR_SUCCESS == dxver.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\DirectX"), KEY_READ)) {
		CString str;
		ULONG len = 64;
		if (ERROR_SUCCESS == dxver.QueryStringValue(_T("Version"), str.GetBuffer(len), &len)) {
			str.ReleaseBuffer(len);
			int ver[4];
			_stscanf_s(str, _T("%d.%d.%d.%d"), ver+0, ver+1, ver+2, ver+3);
			iDXVer = ver[1];
		}
	}

	// Set interface language first!
	CString langcode = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LANGUAGE);
	iLanguage = CMPlayerCApp::GetLanguageIndex(langcode);
	if (iLanguage < 0) {
		iLanguage = CMPlayerCApp::GetDefLanguage();
	}
	CMPlayerCApp::SetLanguage(iLanguage);

	CreateCommands();

	PerfomanceSettings.LoadSettings();
	FiltersPrioritySettings.LoadSettings();

	iCaptionMenuMode = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECAPTIONMENU, MODE_SHOWCAPTIONMENU);
	fHideNavigation = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDENAVIGATION, 0);
	nCS = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CONTROLSTATE, CS_SEEKBAR|CS_TOOLBAR|CS_STATUSBAR);
	iDefaultVideoSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULTVIDEOFRAME, DVS_FROMINSIDE);
	fKeepAspectRatio = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPASPECTRATIO, TRUE);
	fCompMonDeskARDiff = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COMPMONDESKARDIFF, FALSE);
	nVolume = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUME, 100);
	nBalance = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_BALANCE, 0);
	fMute = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MUTE, 0);
	nLoops = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOOPNUM, 1);
	fLoopForever = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOOP, 0);
	fRewind = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REWIND, FALSE);
	iZoomLevel = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ZOOM, 1);
	nVolumeStep = max(1, min(10, pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUME_STEP, 5)));
	nSpeedStep = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPEED_STEP, 0);
	iDSVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, (IsWinVistaOrLater() ? (CMPlayerCApp::HasEVR() ? VIDRNDT_DS_EVR_CUSTOM : VIDRNDT_DS_DEFAULT) : VIDRNDT_DS_VMR7WINDOWED) );
	iRMVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, VIDRNDT_RM_DEFAULT);
	iQTVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, VIDRNDT_QT_DEFAULT);

	UpdateRenderersData(false);

	strAudioRendererDisplayName			= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIORENDERERTYPE);
	strSecondAudioRendererDisplayName	= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SECONDAUDIORENDERER);
	fDualAudioOutput					= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DUALAUDIOOUTPUT, FALSE);

	fAutoloadAudio = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADAUDIO, TRUE);
	fPrioritizeExternalAudio = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITIZEEXTERNALAUDIO, FALSE);
	strAudioPaths = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOPATHS, DEFAULT_AUDIO_PATHS);
	fAutoloadSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADSUBTITLES, TRUE);

	strSubtitlesLanguageOrder		= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANGORDER);
	strAudiosLanguageOrder			= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOSLANGORDER);
	fUseInternalSelectTrackLogic	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_INTERNALSELECTTRACKLOGIC, TRUE);

	fBlockVSFilter = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_BLOCKVSFILTER, TRUE);
	fEnableWorkerThreadForOpening = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWORKERTHREADFOROPENING, TRUE);
	fReportFailedPins = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REPORTFAILEDPINS, TRUE);

	iRtspHandler		= (engine_t)AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RTSPHANDLER, (int)RealMedia);
	fRtspFileExtFirst	= !!AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RTSPFILEEXTFIRST, TRUE);

	iMultipleInst = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MULTIINST, 1);
	if (iMultipleInst < 0 || iMultipleInst > 2) {
		iMultipleInst = 1;
	}
	iTitleBarTextStyle = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTSTYLE, 1);
	fTitleBarTextTitle = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTTITLE, FALSE);
	iOnTop = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ONTOP, 0);
	fTrayIcon = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TRAYICON, 0);
	fRememberZoomLevel = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOZOOM, 0);
	fShowBarsWhenFullScreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLS, 1);
	nShowBarsWhenFullScreenTimeOut = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLSTIMEOUT, 0);

	//Multi-monitor code
	strFullScreenMonitor = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_FULLSCREENMONITOR);
	// DeviceID
	strFullScreenMonitorID = pApp->GetProfileString(IDS_R_SETTINGS, _T("FullScreenMonitorID"));

	// Prevent Minimize when in Fullscreen mode on non default monitor
	fPreventMinimize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_PREVENT_MINIMIZE, 0);

	fUseWin7TaskBar = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_WIN7TASKBAR, 1);

	fExitAfterPlayback				= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_EXIT_AFTER_PB, 0);
	fNextInDirAfterPlayback			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NEXT_AFTER_PB, 0);
	fNextInDirAfterPlaybackLooped	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NEXT_AFTER_PB_LOOPED, 0);
	fDontUseSearchInFolder			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NO_SEARCH_IN_FOLDER, 0);

	fUseTimeTooltip = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USE_TIME_TOOLTIP, TRUE);
	nTimeTooltipPosition = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TIME_TOOLTIP_POSITION, TIME_TOOLTIP_ABOVE_SEEKBAR);
	nOSDSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_OSD_SIZE, IsWinVistaOrLater() ? 18 : 20);
	strOSDFont= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_MPC_OSD_FONT, IsWinVistaOrLater() ? _T("Segoe UI") : _T("Arial"));

	// Associated types with icon or not...
	fAssociatedWithIcons	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASSOCIATED_WITH_ICON, 1);
	// file/dir context menu
	bSetContextFiles		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CONTEXTFILES, 0);
	bSetContextDir			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CONTEXTDIR, 0);

	// Last Open Dir
	strLastOpenDir = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_DIR, L"C:\\");
	// Last Saved Playlist Dir
	strLastSavedPlaylistDir = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LAST_SAVED_PLAYLIST_DIR, L"C:\\");

	if (pApp->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_FULLSCREENRES, &ptr, &len)) {
		if (len == sizeof(AChFR)) {
			memcpy(&AutoChangeFullscrRes, ptr, sizeof(AChFR));
		} else {
			AutoChangeFullscrRes.bEnabled = 0;
		}
		delete [] ptr;
	} else {
		AutoChangeFullscrRes.bEnabled = 0;
	}

	if (pApp->GetProfileBinary(IDS_R_SETTINGS, _T("AccelTblColWidth"), &ptr, &len)) {
		if (len == sizeof(AccelTbl)) {
			memcpy(&AccelTblColWidth, ptr, sizeof(AccelTbl));
		} else {
			AccelTblColWidth.bEnable = false;
		}
		delete [] ptr;
	} else {
		AccelTblColWidth.bEnable = false;
	}

	fExitFullScreenAtTheEnd = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATTHEEND, 1);
	fExitFullScreenAtFocusLost = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATFOCUSLOST, 0);
	fRestoreResAfterExit = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RESTORERESAFTEREXIT, 1);
	fRememberWindowPos = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWPOS, 0);
	fRememberWindowSize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWSIZE, 0);
	CString str = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_PANSCANZOOM);
	if (_stscanf_s(str, _T("%f,%f"), &dZoomX, &dZoomY) == 2 &&
			dZoomX >= 0.196 && dZoomX <= 3.06 && // 0.196 = 0.2 / 1.02
			dZoomY >= 0.196 && dZoomY <= 3.06) { // 3.06 = 3 * 1.02
		fSavePnSZoom = true;
	} else {
		fSavePnSZoom = false;
		dZoomX = 1.0;
		dZoomY = 1.0;
	}
	fSnapToDesktopEdges = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SNAPTODESKTOPEDGES, 0);
	sizeAspectRatio.cx = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_X, 0);
	sizeAspectRatio.cy = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_Y, 0);
	fKeepHistory = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPHISTORY, 1);
	if (pApp->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_LASTWINDOWRECT, &ptr, &len)) {
		if (len == sizeof(CRect)) {
			memcpy(&rcLastWindowPos, ptr, sizeof(CRect));
		} else {
			fRememberWindowPos = false;
		}
		delete [] ptr;
	} else {
		fRememberWindowPos = false;
	}
	nLastWindowType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTWINDOWTYPE, SIZE_RESTORED);

	bShufflePlaylistItems = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SHUFFLEPLAYLISTITEMS, FALSE);
	bRememberPlaylistItems = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERPLAYLISTITEMS, TRUE);
	bHidePlaylistFullScreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDEPLAYLISTFULLSCREEN, FALSE);
	bFavRememberPos = !!pApp->GetProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_REMEMBERPOS, TRUE);
	bFavRelativeDrive = !!pApp->GetProfileInt(IDS_R_FAVORITES, IDS_RS_FAV_RELATIVEDRIVE, FALSE);

	strDVDPath = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_DVDPATH);
	fUseDVDPath = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEDVDPATH, 0);

	LANGID langID = GetUserDefaultUILanguage();
	idMenuLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MENULANG, langID);
	idAudioLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOLANG, langID);
	idSubtitlesLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANG, langID);

	fClosedCaptions = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CLOSEDCAPTIONS, 0);
	// TODO: rename subdefstyle -> defStyle, IDS_RS_SPLOGFONT -> IDS_RS_SPSTYLE
	{
		CString temp = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SPLOGFONT);
		subdefstyle <<= temp;
		if (temp.IsEmpty()) {
			subdefstyle.relativeTo = 1;    //default "Position subtitles relative to the video frame" option is checked
		}
	}
	fOverridePlacement = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPOVERRIDEPLACEMENT, 0);
	nHorPos = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPHORPOS, 50);
	nVerPos = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPVERPOS, 90);
	nSubDelayInterval = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBDELAYINTERVAL, 500);

	fEnableSubtitles				= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLESUBTITLES, TRUE);
	fForcedSubtitles				= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FORCEDSUBTITLES, FALSE);
	fPrioritizeExternalSubtitles	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITIZEEXTERNALSUBTITLES, TRUE);
	fDisableInternalSubtitles		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEINTERNALSUBTITLES, FALSE);
	fAutoReloadExtSubtitles			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTORELOADEXTSUBTITLES, FALSE);
	fUseSybresync					= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USE_SUBRESYNC, FALSE);
	strSubtitlePaths				= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLEPATHS, DEFAULT_SUBTITLE_PATHS);

	fUseDefaultSubtitlesStyle		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEDEFAULTSUBTITLESSTYLE, FALSE);
	fEnableAudioSwitcher			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOSWITCHER, TRUE);
	fAudioTimeShift					= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOTIMESHIFT, 0);
	iAudioTimeShift					= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOTIMESHIFT, 0);
	fCustomChannelMapping			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CUSTOMCHANNELMAPPING, 0);

	BOOL bResult = pApp->GetProfileBinary( IDS_R_SETTINGS, IDS_RS_SPEAKERTOCHANNELMAPPING, &ptr, &len );
	if (bResult && len == sizeof(pSpeakerToChannelMap)) {
		memcpy(pSpeakerToChannelMap, ptr, sizeof(pSpeakerToChannelMap));
	} else {
		memset(pSpeakerToChannelMap, 0, sizeof(pSpeakerToChannelMap));
		for (int j = 0; j < 18; j++) {
			for (int i = 0; i <= j; i++) {
				pSpeakerToChannelMap[j][i] = 1 << i;
			}
		}

		pSpeakerToChannelMap[0][0] = 1<<0;
		pSpeakerToChannelMap[0][1] = 1<<0;

		pSpeakerToChannelMap[3][0] = 1<<0;
		pSpeakerToChannelMap[3][1] = 1<<1;
		pSpeakerToChannelMap[3][2] = 0;
		pSpeakerToChannelMap[3][3] = 0;
		pSpeakerToChannelMap[3][4] = 1<<2;
		pSpeakerToChannelMap[3][5] = 1<<3;

		pSpeakerToChannelMap[4][0] = 1<<0;
		pSpeakerToChannelMap[4][1] = 1<<1;
		pSpeakerToChannelMap[4][2] = 1<<2;
		pSpeakerToChannelMap[4][3] = 0;
		pSpeakerToChannelMap[4][4] = 1<<3;
		pSpeakerToChannelMap[4][5] = 1<<4;
	}
	if (bResult) {
		delete [] ptr;
	}

	fAudioNormalize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZE, FALSE);
	fAudioNormalizeRecover = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZERECOVER, TRUE);
	dAudioBoost_dB = (float)_tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, _T("0")));
	if (dAudioBoost_dB < 0 || dAudioBoost_dB > 10) {
		dAudioBoost_dB = 0;
	}

	nSpeakerChannels = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPEAKERCHANNELS, 2);

	{
		for (unsigned int i = 0; ; i++) {
			CString key;
			key.Format(_T("%s\\%04u"), IDS_R_FILTERS, i);

			CAutoPtr<FilterOverride> f(DNew FilterOverride);

			f->fDisabled = !pApp->GetProfileInt(key, _T("Enabled"), 0);

			UINT j = pApp->GetProfileInt(key, _T("SourceType"), -1);
			if (j == 0) {
				f->type = FilterOverride::REGISTERED;
				f->dispname = CStringW(pApp->GetProfileString(key, _T("DisplayName")));
				f->name = pApp->GetProfileString(key, _T("Name"), _T(""));
				f->clsid = GUIDFromCString(pApp->GetProfileString(key, _T("CLSID")));
				if (f->clsid == CLSID_NULL) {
					CComPtr<IBaseFilter> pBF;
					CStringW FriendlyName;
					if (CreateFilter(f->dispname, &pBF, FriendlyName)) {
						f->clsid = GetCLSID(pBF);
						pApp->WriteProfileString(key, _T("CLSID"), CStringFromGUID(f->clsid));
					}
				}
			} else if (j == 1) {
				f->type = FilterOverride::EXTERNAL;
				f->path = pApp->GetProfileString(key, _T("Path"));
				f->name = pApp->GetProfileString(key, _T("Name"));
				f->clsid = GUIDFromCString(pApp->GetProfileString(key, _T("CLSID")));
			} else {
				pApp->WriteProfileString(key, NULL, 0);
				break;
			}

			f->backup.RemoveAll();
			for (unsigned int i = 0; ; i++) {
				CString val;
				val.Format(_T("org%04u"), i);
				CString guid = pApp->GetProfileString(key, val);
				if (guid.IsEmpty()) {
					break;
				}
				f->backup.AddTail(GUIDFromCString(guid));
			}

			f->guids.RemoveAll();
			for (unsigned int i = 0; ; i++) {
				CString val;
				val.Format(_T("mod%04u"), i);
				CString guid = pApp->GetProfileString(key, val);
				if (guid.IsEmpty()) {
					break;
				}
				f->guids.AddTail(GUIDFromCString(guid));
			}

			f->iLoadType = (int)pApp->GetProfileInt(key, _T("LoadType"), -1);
			if (f->iLoadType < 0) {
				break;
			}

			f->dwMerit = pApp->GetProfileInt(key, _T("Merit"), MERIT_DO_NOT_USE+1);

			m_filters.AddTail(f);
		}
	}

	fIntRealMedia = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_INTREALMEDIA, 0);
	//fRealMediaRenderless = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIARENDERLESS, 0);
	//iQuickTimeRenderer = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_QUICKTIMERENDERER, 2);
	//dRealMediaQuickTimeFPS = 25.0;
	//*((DWORD*)&dRealMediaQuickTimeFPS) = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIAFPS, *((DWORD*)&dRealMediaQuickTimeFPS));

	m_pnspresets.RemoveAll();
	for (int i = 0; i < (ID_PANNSCAN_PRESETS_END - ID_PANNSCAN_PRESETS_START); i++) {
		CString str;
		str.Format(_T("Preset%d"), i);
		str = pApp->GetProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, str);
		if (str.IsEmpty()) {
			break;
		}
		m_pnspresets.Add(str);
	}

	if (m_pnspresets.IsEmpty()) {
		double _4p3		= 4.0/3.0;
		double _16p9	= 16.0/9.0;
		double _235p1	= 2.35/1.0;
		//double _185p1	= 1.85/1.0;
		//UNREFERENCED_PARAMETER(_185p1);

		CString str;
		str.Format(ResStr(IDS_SCALE_16_9), 0.5, 0.5, 1, _16p9/_4p3);
		m_pnspresets.Add(str);
		str.Format(ResStr(IDS_SCALE_WIDESCREEN), 0.5, 0.5, _16p9/_4p3, _16p9/_4p3);
		m_pnspresets.Add(str);
		str.Format(ResStr(IDS_SCALE_ULTRAWIDE), 0.5, 0.5, _235p1/_4p3, _235p1/_4p3);
		m_pnspresets.Add(str);
	}

	for (int i = 0; i < wmcmds.GetCount(); i++) {
		CString str;
		str.Format(_T("CommandMod%d"), i);
		str = pApp->GetProfileString(IDS_R_COMMANDS, str);
		if (str.IsEmpty()) {
			break;
		}
		int cmd, fVirt, key, repcnt;
		UINT mouse, mouseFS, appcmd;
		TCHAR buff[128];
		int n;
		if (5 > (n = _stscanf_s(str, _T("%d %x %x %s %d %u %u %u"), &cmd, &fVirt, &key, buff, _countof(buff), &repcnt, &mouse, &appcmd, &mouseFS))) {
			break;
		}
		if (POSITION pos = wmcmds.Find(cmd)) {
			wmcmd& wc = wmcmds.GetAt(pos);
			wc.cmd = cmd;
			wc.fVirt = fVirt;
			wc.key = key;
			if (n >= 6) {
				wc.mouse = mouse;
			}
			if (n >= 7) {
				wc.appcmd = appcmd;
			}
			// If there is no distinct bindings for windowed and
			// fullscreen modes we use the same for both.
			wc.mouseFS = (n >= 8) ? mouseFS : wc.mouse;
			wc.rmcmd = CStringA(buff).Trim('\"');
			wc.rmrepcnt = repcnt;
		}
	}

	CAtlArray<ACCEL> pAccel;
	pAccel.SetCount(wmcmds.GetCount());
	POSITION pos = wmcmds.GetHeadPosition();
	for (int i = 0; pos; i++) {
		pAccel[i] = wmcmds.GetNext(pos);
	}
	hAccel = CreateAcceleratorTable(pAccel.GetData(), pAccel.GetCount());

	strWinLircAddr = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WINLIRCADDR, _T("127.0.0.1:8765"));
	fWinLirc = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WINLIRC, 0);
	strUIceAddr = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_UICEADDR, _T("127.0.0.1:1234"));
	fUIce = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_UICE, 0);
	fGlobalMedia = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_GLOBALMEDIA, 1);

	fDisableXPToolbars = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEXPTOOLBARS, 1);
	nThemeBrightness = pApp->GetProfileInt(IDS_R_SETTINGS, _T("ThemeBrightness"), 15);
	nThemeRed = pApp->GetProfileInt(IDS_R_SETTINGS, _T("ThemeRed"), 256);
	nThemeGreen = pApp->GetProfileInt(IDS_R_SETTINGS, _T("ThemeGreen"), 256);
	nThemeBlue = pApp->GetProfileInt(IDS_R_SETTINGS, _T("ThemeBlue"), 256);
	nOSDTransparent = pApp->GetProfileInt(IDS_R_SETTINGS, _T("OSDTransparent"), 100);
	nOSDBorder = pApp->GetProfileInt(IDS_R_SETTINGS, _T("OSDBorder"), 1);
	fFileNameOnSeekBar = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("FileNameOnSeekBar"), TRUE);

	clrFaceABGR = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CLRFACEABGR, 0x00ffffff);
	clrOutlineABGR = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CLROUTLINEABGR, 0x00868686);
	clrFontABGR = pApp->GetProfileInt(IDS_R_SETTINGS, _T("OSDFontColor"), 0x00E0E0E0);
	clrGrad1ABGR = pApp->GetProfileInt(IDS_R_SETTINGS, _T("OSDGrad1Color"), 0x00302820);
	clrGrad2ABGR = pApp->GetProfileInt(IDS_R_SETTINGS, _T("OSDGrad2Color"), 0x00302820);

	nJumpDistS = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTS, DEFAULT_JUMPDISTANCE_1);
	nJumpDistM = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTM, DEFAULT_JUMPDISTANCE_2);
	nJumpDistL = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTL, DEFAULT_JUMPDISTANCE_3);
	fLimitWindowProportions = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LIMITWINDOWPROPORTIONS, FALSE);

	iDlgPropX = pApp->GetProfileInt(IDS_R_SETTINGS, _T("DlgPropX"), 0);
	iDlgPropY = pApp->GetProfileInt(IDS_R_SETTINGS, _T("DlgPropY"), 0);

	m_Formats.UpdateData(false);

	// Internal filters
	for (int f = 0; f < SRC_LAST; f++) {
		SrcFilters[f] = !!pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, SrcFiltersKeys[f], 1);
	}
	for (int f = 0; f < VDEC_DXVA_LAST; f++) {
		DXVAFilters[f] = !!pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, DXVAFiltersKeys[f], 1);
	}
	for (int f = 0; f < VDEC_LAST; f++) {
		VideoFilters[f] = !!pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, VideoFiltersKeys[f], 1);
	}
	for (int f = 0; f < ADEC_LAST; f++) {
		AudioFilters[f] = !!pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, AudioFiltersKeys[f], 1);
	}

	strLogoFileName = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LOGOFILE);
	nLogoId = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOID, DEF_LOGO);
	fLogoExternal = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOEXT, 0);

	fHideCDROMsSubMenu = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECDROMSSUBMENU, 0);

	dwPriority = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITY, NORMAL_PRIORITY_CLASS);
	::SetPriorityClass(::GetCurrentProcess(), dwPriority);
	fLaunchfullscreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LAUNCHFULLSCREEN, FALSE);

	fEnableWebServer = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWEBSERVER, FALSE);
	nWebServerPort = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPORT, 13579);
	nWebServerQuality = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERQUALITY, 85);
	fWebServerPrintDebugInfo = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPRINTDEBUGINFO, FALSE);
	fWebServerUseCompression = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERUSECOMPRESSION, TRUE);
	fWebServerLocalhostOnly = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERLOCALHOSTONLY, FALSE);
	strWebRoot = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBROOT, _T("*./webroot"));
	strWebDefIndex = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBDEFINDEX, _T("index.html;index.php"));
	strWebServerCGI = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBSERVERCGI);

	CString MyPictures;

	CRegKey key;
	// grrrrr
	// if (!SHGetSpecialFolderPath(NULL, MyPictures.GetBufferSetLength(_MAX_PATH), CSIDL_MYPICTURES, TRUE)) MyPictures.Empty();
	// else MyPictures.ReleaseBuffer();
	if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ)) {
		ULONG len = _MAX_PATH;
		if (ERROR_SUCCESS == key.QueryStringValue(_T("My Pictures"), MyPictures.GetBuffer(_MAX_PATH), &len)) {
			MyPictures.ReleaseBufferSetLength(len);
		} else {
			MyPictures.Empty();
		}
	}
	strSnapShotPath = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTPATH, MyPictures);
	strSnapShotExt = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTEXT, _T(".jpg"));

	iThumbRows		= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBROWS, 4);
	iThumbCols		= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBCOLS, 4);
	iThumbWidth		= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBWIDTH, 1024);
	iThumbQuality	= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBQUALITY, 85);
	iThumbLevelPNG	= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBLEVELPNG, 7);

	strISDb = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_ISDB, _T("www.opensubtitles.org/isdb"));

	nLastUsedPage = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTUSEDPAGE, 0);

	LoadShaders();

	fD3DFullscreen	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_D3DFULLSCREEN, FALSE);
	//fMonitorAutoRefreshRate	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MONITOR_AUTOREFRESHRATE, FALSE);

	iBrightness		= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_BRIGHTNESS, 0);
	iContrast		= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_CONTRAST, 0);
	iHue			= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_HUE, 0);
	iSaturation		= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COLOR_SATURATION, 0);

	strShaderList				= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLIST);
	strShaderListScreenSpace	= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLISTSCREENSPACE);
	fToggleShader				= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADER, 0);
	fToggleShaderScreenSpace	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADERSSCREENSPACE, 0);

	fShowOSD			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SHOWOSD, 1);
	fEnableEDLEditor	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEEDLEDITOR, FALSE);
	fFastSeek			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FASTSEEK_KEYFRAME, TRUE);
	fMiniDump			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MINI_DUMP, FALSE);
	CMiniDump::SetState(fMiniDump);

	fLCDSupport		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LCD_SUPPORT, FALSE);
	fSmartSeek		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SMARTSEEK, FALSE);
	fChapterMarker	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CHAPTER_MARKER, FALSE);
	fFlybar			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USE_FLYBAR, TRUE);
	fFlybarOnTop	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USE_FLYBAR_ONTOP, FALSE);
	fFontShadow		= !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("OSDFontShadow"), FALSE);
	fFontAA			= !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("OSDFontAA"), TRUE);

	// Save analog capture settings
	iDefaultCaptureDevice	= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULT_CAPTURE, 0);
	strAnalogVideo			= pApp->GetProfileString(IDS_R_CAPTURE, IDS_RS_VIDEO_DISP_NAME, _T("dummy"));
	strAnalogAudio			= pApp->GetProfileString(IDS_R_CAPTURE, IDS_RS_AUDIO_DISP_NAME, _T("dummy"));
	iAnalogCountry			= pApp->GetProfileInt(IDS_R_CAPTURE, IDS_RS_COUNTRY, 1);

	strBDANetworkProvider	= pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_NETWORKPROVIDER);
	strBDATuner				= pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_TUNER);
	strBDAReceiver			= pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_RECEIVER);
	//sBDAStandard			= pApp->GetProfileString(IDS_R_DVB, IDS_RS_BDA_STANDARD);
	iBDAScanFreqStart		= pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_SCAN_FREQ_START, 474000);
	iBDAScanFreqEnd			= pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_SCAN_FREQ_END, 858000);
	iBDABandwidth			= pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_BANDWIDTH, 8);
	fBDAUseOffset			= !!pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_USE_OFFSET, 0);
	iBDAOffset				= pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_OFFSET, 166);
	nDVBLastChannel			= pApp->GetProfileInt(IDS_R_DVB, IDS_RS_DVB_LAST_CHANNEL, 1);
	fBDAIgnoreEncryptedChannels = !!pApp->GetProfileInt(IDS_R_DVB, IDS_RS_BDA_IGNORE_ENCRYPTED_CHANNELS, 0);

	for (int iChannel = 0; ; iChannel++) {
		CString strTemp;
		CString strChannel;
		CDVBChannel	Channel;
		strTemp.Format(_T("%d"), iChannel);
		strChannel = pApp->GetProfileString(IDS_R_DVB, strTemp);
		if (strChannel.IsEmpty()) {
			break;
		}
		Channel.FromString(strChannel);
		m_DVBChannels.AddTail (Channel);
	}

	// playback positions for last played DVDs
	fRememberDVDPos		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DVDPOS, 0);
	nCurrentDvdPosition = -1;
	memset(DvdPosition, 0, sizeof(DvdPosition));
	for (int i = 0; i < MAX_DVD_POSITION; i++) {
		CString		strDVDPos;
		CString		strValue;

		strDVDPos.Format(_T("DVD Position %d"), i);
		strValue = pApp->GetProfileString(IDS_R_SETTINGS, strDVDPos);
		if (strValue.GetLength()/2 == sizeof(DVD_POSITION)) {
			DeserializeHex(strValue, (BYTE*)&DvdPosition[i], sizeof(DVD_POSITION));
		}
	}

	// playback positions for last played files
	fRememberFilePos		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOS, 0);
	nCurrentFilePosition	= -1;
	for (int i = 0; i < MAX_FILE_POSITION; i++) {
		CString		strFilePos;
		CString		strValue;

		strFilePos.Format(_T("File Name %d"), i);
		FilePosition[i].strFile = pApp->GetProfileString(IDS_R_SETTINGS, strFilePos);

		strFilePos.Format(_T("File Position %d"), i);
		strValue = pApp->GetProfileString(IDS_R_SETTINGS, strFilePos);
		FilePosition[i].llPosition = _tstoi64(strValue);
	}

	fStartMainTitle			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DVD_START_MAIN_TITLE, 0);
	fNormalStartDVD			= true;
	fLastFullScreen			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFULLSCREEN, 0);

	fRemainingTime			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMAINING_TIME, FALSE);

	strLastOpenFilterDir	= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_FILTER_DIR);

	iYoutubeTag				= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_YOUTUBE_TAG, 0);
	iYoutubeSource			= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_YOUTUBE_SOURCE, 0);

	nLastFileInfoPage		= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFILEINFOPAGE, 0);

	if (fLaunchfullscreen && !IsD3DFullscreen()) {
		nCLSwitches |= CLSW_FULLSCREEN;
	}

	fInitialized = true;
}

void CAppSettings::SaveShaders()
{
	if (m_shaders.GetCount() > 0) {
		CString path;
		if (AfxGetMyApp()->GetAppSavePath(path)) {
			path += _T("\\Shaders\\");
			// Only create this folder when needed
			if (!::PathFileExists(path)) {
				::CreateDirectory(path, NULL);
			}

			POSITION pos = m_shaders.GetHeadPosition();
			while (pos) {
				const Shader& s = m_shaders.GetNext(pos);
				if (!s.label.IsEmpty()) {
					CString shname = s.label + _T(".psh");
					CStdioFile shfile;
					if (shfile.Open(path + shname, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone | CFile::typeText)) {
						shfile.WriteString(_T("// $MinimumShaderProfile: ") + s.target + _T("\n"));
						shfile.WriteString(s.srcdata);

						shfile.Close();
					}
				}
			}
		}
	}
}

void CAppSettings::LoadShaders()
{
	m_shaders.RemoveAll();

	CString path;
	if (AfxGetMyApp()->GetAppSavePath(path)) {
		path += _T("\\Shaders\\");
		if (!::PathFileExists(path) && !AfxGetMyApp()->IsIniValid()) {
			// profile does not contain "Shaders" folder. try to take shaders from program folder
			GetModuleFileName(AfxGetInstanceHandle(), path.GetBuffer(_MAX_PATH), _MAX_PATH);
			path.ReleaseBuffer();
			path.Truncate(path.ReverseFind('\\'));
			path += _T("\\Shaders\\");
			fShadersNeedSave = true;
		}

		WIN32_FIND_DATA wfd;
		HANDLE hFile = FindFirstFile(path + _T("*.psh"), &wfd);
		if (hFile != INVALID_HANDLE_VALUE) {
			do {
				CString shname = wfd.cFileName;
				CStdioFile shfile;

				if (shfile.Open(path + shname, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {

					Shader s;
					s.label = shname.Left(shname.GetLength() - 4); // filename without extension (.psh)

					CString str;
					shfile.ReadString(str); // read first string
					if (str.Left(25) == _T("// $MinimumShaderProfile:")) {
						s.target = str.Mid(25).Trim(); // shader version property
					} else if (str.Left(18) == _T("// $ShaderVersion:")) {
						// ignore and delete the old properties
						fShadersNeedSave = true;
					} else {
						shfile.SeekToBegin();
					}
					if (s.target != _T("ps_2_0") && s.target != _T("ps_2_a") && s.target != _T("ps_2_sw") && s.target != _T("ps_3_0") && s.target != _T("ps_3_sw")) {
						s.target = _T("ps_2_0");
					}

					while (shfile.ReadString(str)) {
						s.srcdata += str + _T("\n");
					}
					shfile.Close();

					m_shaders.AddTail(s);
				}
			} while (FindNextFile(hFile, &wfd));
			FindClose(hFile);
		}
	}
}

int CAppSettings::GetMultiInst()
{
	int multiinst = AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MULTIINST, 1);
	if (multiinst < 0 || multiinst > 2) {
		multiinst = 1;
	}
	return multiinst;
}

engine_t CAppSettings::GetRtspHandler(bool& lookext)
{
	lookext = fRtspFileExtFirst;
	return iRtspHandler;
}

void CAppSettings::SetRtspHandler(engine_t e, bool lookext)
{
	iRtspHandler		= e;
	fRtspFileExtFirst	= lookext;
}

bool CAppSettings::IsUsingRtspEngine(CString path, engine_t e)
{
	return (GetRtspEngine(path) == e);
}

engine_t CAppSettings::GetRtspEngine(CString path)
{
	path.Trim().MakeLower();

	if (!fRtspFileExtFirst && path.Find(_T("rtsp://")) == 0) {
		return iRtspHandler;
	}

	CString ext = CPath(path).GetExtension();
	ext.MakeLower();

	if (!ext.IsEmpty()) {
		if (path.Find(_T("rtsp://")) == 0) {
			if (ext == _T(".ram") || ext == _T(".rm") || ext == _T(".ra")) {
				return RealMedia;
			}
			if (ext == _T(".qt") || ext == _T(".mov")) {
				return QuickTime;
			}
		}

		for (size_t i = 0; i < m_Formats.GetCount(); i++) {
			CMediaFormatCategory& mfc = m_Formats.GetAt(i);
			if (mfc.FindExt(ext)) {
				return mfc.GetEngineType();
			}
		}
	}

	if (fRtspFileExtFirst && path.Find(_T("rtsp://")) == 0) {
		return iRtspHandler;
	}

	return DirectShow;
}

void CAppSettings::UpdateRenderersData(bool fSave)
{
	CWinApp* pApp = AfxGetApp();
	CRenderersSettings& r = m_RenderersSettings;
	CRenderersSettings::CAdvRendererSettings& ars = r.m_AdvRendSets;

	if (fSave) {
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_APSURACEFUSAGE, r.iAPSurfaceUsage);
		// pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMRSYNCFIX, fVMRSyncFix);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DX9_RESIZER, r.iDX9Resizer);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMRMIXERMODE, r.fVMRMixerMode);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMRMIXERYUV, r.fVMRMixerYUV);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRAlternateVSync"), ars.fVMR9AlterativeVSync);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSyncOffset"), ars.iVMR9VSyncOffset);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSyncAccurate2"), ars.iVMR9VSyncAccurate);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFullscreenGUISupport"), ars.iVMR9FullscreenGUISupport);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSync"), ars.iVMR9VSync);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRDisableDesktopComposition"), ars.iVMRDisableDesktopComposition);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFullFloatingPointProcessing"), ars.iVMR9FullFloatingPointProcessing);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRHalfFloatingPointProcessing"), ars.iVMR9HalfFloatingPointProcessing);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementEnable"), ars.iVMR9ColorManagementEnable);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementInput"), ars.iVMR9ColorManagementInput);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementAmbientLight"), ars.iVMR9ColorManagementAmbientLight);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementIntent"), ars.iVMR9ColorManagementIntent);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVROutputRange"), ars.iEVROutputRange);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVRHighColorRes"), ars.iEVRHighColorResolution);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVRForceInputHighColorRes"), ars.iEVRForceInputHighColorResolution);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVREnableFrameTimeCorrection"), ars.iEVREnableFrameTimeCorrection);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUBeforeVSync"), ars.iVMRFlushGPUBeforeVSync);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUAfterPresent"), ars.iVMRFlushGPUAfterPresent);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUWait"), ars.iVMRFlushGPUWait);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeClock"), ars.bSynchronizeVideo);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeDisplay"), ars.bSynchronizeDisplay);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeNearest"), ars.bSynchronizeNearest);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("LineDelta"), ars.iLineDelta);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ColumnDelta"), ars.iColumnDelta);

		pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("CycleDelta"), (LPBYTE)&(ars.fCycleDelta), sizeof(ars.fCycleDelta));
		pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("TargetSyncOffset"), (LPBYTE)&(ars.fTargetSyncOffset), sizeof(ars.fTargetSyncOffset));
		pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("ControlLimit"), (LPBYTE)&(ars.fControlLimit), sizeof(ars.fControlLimit));

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ResetDevice"), r.fResetDevice);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPCSIZE, r.nSPCSize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPMAXTEXRES, r.nSPMaxTexRes);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_POW2TEX, r.fSPCPow2Tex);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SPCAllowAnimationWhenBuffering"), r.fSPCAllowAnimationWhenBuffering);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EVR_BUFFERS, r.iEvrBuffers);

		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_D3D9RENDERDEVICE, r.D3D9RenderDevice);

		// Stereoscopic Subtitles
		pApp->WriteProfileInt(IDS_R_SETTINGS, L"StereoDisabled", r.bStereoDisabled);
		pApp->WriteProfileInt(IDS_R_SETTINGS, L"SideBySide", r.bSideBySide);
		pApp->WriteProfileInt(IDS_R_SETTINGS, L"TopAndBottom", r.bTopAndBottom);
	} else {
		r.iAPSurfaceUsage = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_APSURACEFUSAGE, (IsWinVistaOrLater() ? VIDRNDT_AP_TEXTURE3D : VIDRNDT_AP_TEXTURE2D));
		//fVMRSyncFix = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMRSYNCFIX, FALSE);
		r.iDX9Resizer = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DX9_RESIZER, 1);
		r.fVMRMixerMode = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMRMIXERMODE, IsWinVistaOrLater() ? TRUE : FALSE);
		r.fVMRMixerYUV = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMRMIXERYUV, IsWinVistaOrLater() ? TRUE : FALSE);

		CRenderersSettings::CAdvRendererSettings DefaultSettings;
		ars.fVMR9AlterativeVSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRAlternateVSync"), DefaultSettings.fVMR9AlterativeVSync);
		ars.iVMR9VSyncOffset = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSyncOffset"), DefaultSettings.iVMR9VSyncOffset);
		ars.iVMR9VSyncAccurate = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSyncAccurate2"), DefaultSettings.iVMR9VSyncAccurate);
		ars.iVMR9FullscreenGUISupport = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFullscreenGUISupport"), DefaultSettings.iVMR9FullscreenGUISupport);
		ars.iEVRHighColorResolution = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVRHighColorRes"), DefaultSettings.iEVRHighColorResolution);
		ars.iEVRForceInputHighColorResolution = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVRForceInputHighColorRes"), DefaultSettings.iEVRForceInputHighColorResolution);
		ars.iEVREnableFrameTimeCorrection = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVREnableFrameTimeCorrection"), DefaultSettings.iEVREnableFrameTimeCorrection);
		ars.iVMR9VSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSync"), DefaultSettings.iVMR9VSync);
		ars.iVMRDisableDesktopComposition = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRDisableDesktopComposition"), DefaultSettings.iVMRDisableDesktopComposition);
		ars.iVMR9FullFloatingPointProcessing = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFullFloatingPointProcessing"), DefaultSettings.iVMR9FullFloatingPointProcessing);
		ars.iVMR9HalfFloatingPointProcessing = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRHalfFloatingPointProcessing"), DefaultSettings.iVMR9HalfFloatingPointProcessing);

		ars.iVMR9ColorManagementEnable = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementEnable"), DefaultSettings.iVMR9ColorManagementEnable);
		ars.iVMR9ColorManagementInput = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementInput"), DefaultSettings.iVMR9ColorManagementInput);
		ars.iVMR9ColorManagementAmbientLight = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementAmbientLight"), DefaultSettings.iVMR9ColorManagementAmbientLight);
		ars.iVMR9ColorManagementIntent = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRColorManagementIntent"), DefaultSettings.iVMR9ColorManagementIntent);

		ars.iEVROutputRange = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVROutputRange"), DefaultSettings.iEVROutputRange);

		ars.iVMRFlushGPUBeforeVSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUBeforeVSync"), DefaultSettings.iVMRFlushGPUBeforeVSync);
		ars.iVMRFlushGPUAfterPresent = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUAfterPresent"), DefaultSettings.iVMRFlushGPUAfterPresent);
		ars.iVMRFlushGPUWait = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUWait"), DefaultSettings.iVMRFlushGPUWait);

		ars.bSynchronizeVideo = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeClock"), DefaultSettings.bSynchronizeVideo);
		ars.bSynchronizeDisplay = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeDisplay"), DefaultSettings.bSynchronizeDisplay);
		ars.bSynchronizeNearest = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeNearest"), DefaultSettings.bSynchronizeNearest);
		ars.iLineDelta = pApp->GetProfileInt(IDS_R_SETTINGS, _T("LineDelta"), DefaultSettings.iLineDelta);
		ars.iColumnDelta = pApp->GetProfileInt(IDS_R_SETTINGS, _T("ColumnDelta"), DefaultSettings.iColumnDelta);

		double *dPtr;
		UINT dSize;
		if (pApp->GetProfileBinary(IDS_R_SETTINGS, _T("CycleDelta"), (LPBYTE*)&dPtr, &dSize)) {
			ars.fCycleDelta = *dPtr;
			delete [] dPtr;
		}

		if (pApp->GetProfileBinary(IDS_R_SETTINGS, _T("TargetSyncOffset"), (LPBYTE*)&dPtr, &dSize)) {
			ars.fTargetSyncOffset = *dPtr;
			delete [] dPtr;
		}
		if (pApp->GetProfileBinary(IDS_R_SETTINGS, _T("ControlLimit"), (LPBYTE*)&dPtr, &dSize)) {
			ars.fControlLimit = *dPtr;
			delete [] dPtr;
		}

		r.fResetDevice = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("ResetDevice"), TRUE);

		r.nSPCSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPCSIZE, 10);
		r.nSPMaxTexRes = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPMAXTEXRES, 1280);
		r.fSPCPow2Tex = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_POW2TEX, TRUE);
		r.fSPCAllowAnimationWhenBuffering = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SPCAllowAnimationWhenBuffering"), TRUE);

		r.iEvrBuffers		= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EVR_BUFFERS, 5);
		r.D3D9RenderDevice	= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_D3D9RENDERDEVICE);

		// Stereoscopic Subtitles
		r.bStereoDisabled	= pApp->GetProfileInt(IDS_R_SETTINGS, L"StereoDisabled", TRUE);
		r.bSideBySide		= pApp->GetProfileInt(IDS_R_SETTINGS, L"SideBySide", FALSE);
		r.bTopAndBottom		= pApp->GetProfileInt(IDS_R_SETTINGS, L"TopAndBottom", FALSE);

		r.bPositionRelative	= false;
	}
}

void CAppSettings::SaveCurrentFilePosition()
{
	CWinApp* pApp = AfxGetApp();
	CString strFilePos;
	CString strValue;
	int i = nCurrentFilePosition;

	strFilePos.Format(_T("File Name %d"), i);
	pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, FilePosition[i].strFile);
	strFilePos.Format(_T("File Position %d"), i);
	strValue.Format(_T("%I64d"), FilePosition[i].llPosition);
	pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, strValue);
}

void CAppSettings::ClearFilePositions()
{
	CWinApp* pApp = AfxGetApp();
	CString strFilePos;
	for (int i = 0; i < MAX_FILE_POSITION; i++) {
		FilePosition[i].strFile.Empty();
		FilePosition[i].llPosition = 0;

		strFilePos.Format(_T("File Name %d"), i);
		pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, L"");
		strFilePos.Format(_T("File Position %d"), i);
		pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, L"");
	}
}

void CAppSettings::SaveCurrentDVDPosition()
{
	CWinApp* pApp = AfxGetApp();
	CString strDVDPos;
	CString strValue;
	int i = nCurrentDvdPosition;

	strDVDPos.Format(_T("DVD Position %d"), i);
	strValue = SerializeHex((BYTE*)&DvdPosition[i], sizeof(DVD_POSITION));
	pApp->WriteProfileString(IDS_R_SETTINGS, strDVDPos, strValue);
}

void CAppSettings::ClearDVDPositions()
{
	CWinApp* pApp = AfxGetApp();
	CString strDVDPos;
	for (int i = 0; i < MAX_DVD_POSITION; i++) {
		DvdPosition[i].llDVDGuid = 0;
		DvdPosition[i].lTitle = 0;

		strDVDPos.Format(_T("DVD Position %d"), i);
		pApp->WriteProfileString(IDS_R_SETTINGS, strDVDPos, L"");
	}
}

__int64 CAppSettings::ConvertTimeToMSec(CString& time) const
{
	__int64 Sec = 0;
	__int64 mSec = 0;
	__int64 mult = 1;

	int pos = time.GetLength() - 1;
	if (pos < 3) {
		return 0;
	}

	while (pos >= 0) {
		TCHAR ch = time[pos];
		if (ch == '.') {
			mSec = Sec * 1000 / mult;
			Sec = 0;
			mult = 1;
		} else if (ch == ':') {
			mult = mult * 6 / 10;
		} else if (ch >= '0' && ch <= '9') {
			Sec += (ch - '0') * mult;
			mult *= 10;
		} else {
			mSec = Sec = 0;
			break;
		}
		pos--;
	}
	return Sec*1000 + mSec;
}

void CAppSettings::ExtractDVDStartPos(CString& strParam)
{
	int i = 0, j = 0;
	for (CString token = strParam.Tokenize(_T("#"), i);
			j < 3 && !token.IsEmpty();
			token = strParam.Tokenize(_T("#"), i), j++) {
		switch (j) {
			case 0 :
				lDVDTitle = token.IsEmpty() ? 0 : (ULONG)_wtol(token);
				break;
			case 1 :
				if (token.Find(':') > 0) {
					_stscanf_s(token, _T("%02d:%02d:%02d.%03d"), &DVDPosition.bHours, &DVDPosition.bMinutes, &DVDPosition.bSeconds, &DVDPosition.bFrames);
					/* Hack by Ron.  If bFrames >= 30, PlayTime commands fail due to invalid arg */
					DVDPosition.bFrames = 0;
				} else {
					lDVDChapter = token.IsEmpty() ? 0 : (ULONG)_wtol(token);
				}
				break;
		}
	}
}

CString CAppSettings::ParseFileName(CString const& param)
{
	CString fullPathName;

	// Try to transform relative pathname into full pathname
	if (param.Find(_T(":")) < 0) {
		fullPathName.ReleaseBuffer(GetFullPathName(param, MAX_PATH, fullPathName.GetBuffer(MAX_PATH), NULL));

		CFileStatus fs;
		if (!fullPathName.IsEmpty() && CFileGetStatus(fullPathName, fs)) {
			return fullPathName;
		}
	}

	return param;
}

void CAppSettings::ParseCommandLine(CAtlList<CString>& cmdln)
{
	nCLSwitches = 0;
	slFiles.RemoveAll();
	slDubs.RemoveAll();
	slSubs.RemoveAll();
	slFilters.RemoveAll();
	rtStart = INVALID_TIME;
	rtShift = 0;
	lDVDTitle = 0;
	lDVDChapter = 0;
	memset (&DVDPosition, 0, sizeof(DVDPosition));
	iAdminOption = 0;
	sizeFixedWindow.SetSize(0, 0);
	iMonitor = 0;
	hMasterWnd = 0;
	strPnSPreset.Empty();

	POSITION pos = cmdln.GetHeadPosition();
	while (pos) {
		CString param = cmdln.GetNext(pos);
		if (param.IsEmpty()) {
			continue;
		}

		if ((param[0] == '-' || param[0] == '/') && param.GetLength() > 1) {
			CString sw = param.Mid(1).MakeLower();

			if (sw == _T("open")) {
				nCLSwitches |= CLSW_OPEN;
			} else if (sw == _T("play")) {
				nCLSwitches |= CLSW_PLAY;
			} else if (sw == _T("fullscreen")) {
				nCLSwitches |= CLSW_FULLSCREEN;
			} else if (sw == _T("minimized")) {
				nCLSwitches |= CLSW_MINIMIZED;
			} else if (sw == _T("new")) {
				nCLSwitches |= CLSW_NEW;
			} else if (sw == _T("help") || sw == _T("h") || sw == _T("?")) {
				nCLSwitches |= CLSW_HELP;
			} else if (sw == _T("dub") && pos) {
				slDubs.AddTail(ParseFileName(cmdln.GetNext(pos)));
			} else if (sw == _T("dubdelay") && pos) {
				CString strFile	= ParseFileName(cmdln.GetNext(pos));
				int nPos		= strFile.Find (_T("DELAY"));
				if (nPos != -1) {
					rtShift = 10000 * _tstol(strFile.Mid(nPos + 6));
				}
				slDubs.AddTail(strFile);
			} else if (sw == _T("sub") && pos) {
				slSubs.AddTail(ParseFileName(cmdln.GetNext(pos)));
			} else if (sw == _T("filter") && pos) {
				slFilters.AddTail(cmdln.GetNext(pos));
			} else if (sw == _T("dvd")) {
				nCLSwitches |= CLSW_DVD;
			} else if (sw == _T("dvdpos")) {
				ExtractDVDStartPos(cmdln.GetNext(pos));
			} else if (sw == _T("cd")) {
				nCLSwitches |= CLSW_CD;
			} else if (sw == _T("add")) {
				nCLSwitches |= CLSW_ADD;
			} else if (sw == _T("regvid")) {
				nCLSwitches |= CLSW_REGEXTVID;
			} else if (sw == _T("regaud")) {
				nCLSwitches |= CLSW_REGEXTAUD;
			} else if (sw == _T("regpl")) {
				nCLSwitches |= CLSW_REGEXTPL;
			} else if (sw == _T("regall")) {
				nCLSwitches |= (CLSW_REGEXTVID | CLSW_REGEXTAUD | CLSW_REGEXTPL);
			} else if (sw == _T("unregall")) {
				nCLSwitches |= CLSW_UNREGEXT;
			} else if (sw == _T("unregvid")) {
				nCLSwitches |= CLSW_UNREGEXT;    /* keep for compatibility with old versions */
			} else if (sw == _T("unregaud")) {
				nCLSwitches |= CLSW_UNREGEXT;    /* keep for compatibility with old versions */
			} else if (sw == _T("start") && pos) {
				rtStart = 10000i64*_tcstol(cmdln.GetNext(pos), NULL, 10);
				nCLSwitches |= CLSW_STARTVALID;
			} else if (sw == _T("startpos") && pos) {
				rtStart = 10000i64 * ConvertTimeToMSec(cmdln.GetNext(pos));
				nCLSwitches |= CLSW_STARTVALID;
			} else if (sw == _T("nofocus")) {
				nCLSwitches |= CLSW_NOFOCUS;
			} else if (sw == _T("close")) {
				nCLSwitches |= CLSW_CLOSE;
			} else if (sw == _T("standby")) {
				nCLSwitches |= CLSW_STANDBY;
			} else if (sw == _T("hibernate")) {
				nCLSwitches |= CLSW_HIBERNATE;
			} else if (sw == _T("shutdown")) {
				nCLSwitches |= CLSW_SHUTDOWN;
			} else if (sw == _T("logoff")) {
				nCLSwitches |= CLSW_LOGOFF;
			} else if (sw == _T("lock")) {
				nCLSwitches |= CLSW_LOCK;
			} else if (sw == _T("d3dfs")) {
				nCLSwitches |= CLSW_D3DFULLSCREEN;
			} else if (sw == _T("adminoption")) {
				nCLSwitches |= CLSW_ADMINOPTION;
				iAdminOption = _ttoi(cmdln.GetNext(pos));
			} else if (sw == _T("slave") && pos) {
				nCLSwitches |= CLSW_SLAVE;
				hMasterWnd = (HWND)IntToPtr(_ttoi(cmdln.GetNext(pos)));
			} else if (sw == _T("fixedsize") && pos) {
				CAtlList<CString> sl;
				Explode(cmdln.GetNext(pos), sl, ',', 2);
				if (sl.GetCount() == 2) {
					sizeFixedWindow.SetSize(_ttol(sl.GetHead()), _ttol(sl.GetTail()));
					if (sizeFixedWindow.cx > 0 && sizeFixedWindow.cy > 0) {
						nCLSwitches |= CLSW_FIXEDSIZE;
					}
				}
			} else if (sw == _T("monitor") && pos) {
				iMonitor = _tcstol(cmdln.GetNext(pos), NULL, 10);
				nCLSwitches |= CLSW_MONITOR;
			} else if (sw == _T("pns")) {
				strPnSPreset = cmdln.GetNext(pos);
			} else if (sw == _T("webport") && pos) {
				int tmpport = _tcstol(cmdln.GetNext(pos), NULL, 10);
				if ( tmpport >= 0 && tmpport <= 65535 ) {
					nCmdlnWebServerPort = tmpport;
				}
			} else if (sw == _T("debug")) {
				fShowDebugInfo = true;
			} else if (sw == _T("nominidump")) {
				CMiniDump::SetState(false);
			} else if (sw == _T("audiorenderer") && pos) {
				SetAudioRenderer(_ttoi(cmdln.GetNext(pos)));
			} else if (sw == _T("reset")) {
				nCLSwitches |= CLSW_RESET;
			} else {
				nCLSwitches |= CLSW_HELP|CLSW_UNRECOGNIZEDSWITCH;
			}
		} else if (param == _T("-")) { // Special case: standard input
			slFiles.AddTail(_T("pipe:0"));
		} else {
			slFiles.AddTail(ParseFileName(param));
		}
	}
}

void CAppSettings::GetFav(favtype ft, CAtlList<CString>& sl)
{
	sl.RemoveAll();

	CString root;

	switch (ft) {
		case FAV_FILE:
			root = IDS_R_FAVFILES;
			break;
		case FAV_DVD:
			root = IDS_R_FAVDVDS;
			break;
		case FAV_DEVICE:
			root = IDS_R_FAVDEVICES;
			break;
		default:
			return;
	}

	for (int i = 0; ; i++) {
		CString s;
		s.Format(_T("Name%d"), i);
		s = AfxGetApp()->GetProfileString(root, s);
		if (s.IsEmpty()) {
			break;
		}
		sl.AddTail(s);
	}
}

void CAppSettings::SetFav(favtype ft, CAtlList<CString>& sl)
{
	CString root;

	switch (ft) {
		case FAV_FILE:
			root = IDS_R_FAVFILES;
			break;
		case FAV_DVD:
			root = IDS_R_FAVDVDS;
			break;
		case FAV_DEVICE:
			root = IDS_R_FAVDEVICES;
			break;
		default:
			return;
	}

	AfxGetApp()->WriteProfileString(root, NULL, NULL);

	int i = 0;
	POSITION pos = sl.GetHeadPosition();
	while (pos) {
		CString s;
		s.Format(_T("Name%d"), i++);
		AfxGetApp()->WriteProfileString(root, s, sl.GetNext(pos));
	}
}

void CAppSettings::AddFav(favtype ft, CString s)
{
	CAtlList<CString> sl;
	GetFav(ft, sl);
	if (sl.Find(s)) {
		return;
	}
	sl.AddTail(s);
	SetFav(ft, sl);
}

CDVBChannel* CAppSettings::FindChannelByPref(int nPrefNumber)
{
	POSITION	pos = m_DVBChannels.GetHeadPosition();
	while (pos) {
		CDVBChannel&	Channel = m_DVBChannels.GetNext (pos);
		if (Channel.GetPrefNumber() == nPrefNumber) {
			return &Channel;
		}
	}

	return NULL;
}

bool CAppSettings::IsISRSelect() const
{
	return (iDSVideoRendererType == VIDRNDT_DS_VMR7RENDERLESS ||
			iDSVideoRendererType == VIDRNDT_DS_VMR9RENDERLESS ||
			iDSVideoRendererType == VIDRNDT_DS_EVR_CUSTOM ||
			iDSVideoRendererType == VIDRNDT_DS_DXR ||
			iDSVideoRendererType == VIDRNDT_DS_SYNC ||
			iDSVideoRendererType == VIDRNDT_DS_MADVR);
}

bool CAppSettings::IsISRAutoLoadEnabled() const
{
	return fAutoloadSubtitles && IsISRSelect();
}

// Settings::CRecentFileAndURLList
CAppSettings::CRecentFileAndURLList::CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
		LPCTSTR lpszEntryFormat, int nSize,
		int nMaxDispLen)
	: CRecentFileList(nStart, lpszSection, lpszEntryFormat, nSize, nMaxDispLen)
{
}

extern BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
extern BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);

void CAppSettings::CRecentFileAndURLList::Add(LPCTSTR lpszPathName)
{
	ASSERT(m_arrNames != NULL);
	ASSERT(lpszPathName != NULL);
	ASSERT(AfxIsValidString(lpszPathName));

	CString pathName = lpszPathName;
	if (CString(pathName).MakeLower().Find(_T("@device:")) >= 0) {
		return;
	}

	bool fURL = (pathName.Find(_T("://")) >= 0);

	// fully qualify the path name
	if (!fURL) {
		AfxFullPath(pathName.GetBufferSetLength(pathName.GetLength() + 1024), lpszPathName);
		pathName.ReleaseBuffer();
	}

	// update the MRU list, if an existing MRU string matches file name
	int iMRU;
	for (iMRU = 0; iMRU < m_nSize-1; iMRU++) {
		if ((fURL && !_tcscmp(m_arrNames[iMRU], pathName))
				|| AfxComparePath(m_arrNames[iMRU], pathName)) {
			break;    // iMRU will point to matching entry
		}
	}
	// move MRU strings before this one down
	for (; iMRU > 0; iMRU--) {
		ASSERT(iMRU > 0);
		ASSERT(iMRU < m_nSize);
		m_arrNames[iMRU] = m_arrNames[iMRU-1];
	}
	// place this one at the beginning
	m_arrNames[0] = pathName;
}
