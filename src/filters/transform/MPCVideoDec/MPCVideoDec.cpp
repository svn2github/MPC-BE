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
#include <atlbase.h>
#include <MMReg.h>
#include "../../../DSUtil/ff_log.h"

#ifdef REGISTER_FILTER
	#include <InitGuid.h>
#endif

#include "MPCVideoDec.h"
#include "DXVAAllocator.h"
#include "CpuId.h"
#include "FfmpegContext.h"

#include "../../../DSUtil/DSUtil.h"
#include "../../../DSUtil/SysVersion.h"
#include "../../../DSUtil/MediaTypes.h"
#include "../../../DSUtil/WinAPIUtils.h"
#include "../../parser/MpegSplitter/MpegSplitter.h"
#include "DXVADecoderH264.h"
#include <moreuuids.h>

#include "Version.h"

#pragma warning(push)
#pragma warning(disable: 4005)
extern "C" {
	#include <ffmpeg/libavcodec/avcodec.h>
	#include <ffmpeg/libavutil/pixdesc.h>
	#include <ffmpeg/libavutil/imgutils.h>
}
#pragma warning(pop)

// option names
#define OPT_REGKEY_VideoDec  _T("Software\\MPC-BE Filters\\MPC Video Decoder")
#define OPT_SECTION_VideoDec _T("Filters\\MPC Video Decoder")
#define OPT_ThreadNumber     _T("ThreadNumber")
#define OPT_DiscardMode      _T("DiscardMode")
#define OPT_Deinterlacing    _T("Deinterlacing")
#define OPT_ARMode           _T("ARMode")
#define OPT_DXVACheck        _T("DXVACheckCompatibility")
#define OPT_DisableDXVA_SD   _T("DisableDXVA_SD")
#define OPT_SW_prefix        _T("Sw_")
#define OPT_SwPreset         _T("SwPreset")
#define OPT_SwStandard       _T("SwStandard")
#define OPT_SwRGBLevels      _T("SwRGBLevels")

#define MAX_AUTO_THREADS 16

#pragma region any_constants

#ifdef REGISTER_FILTER
#define OPT_REGKEY_VCodecs   _T("Software\\MPC-BE Filters\\MPC Video Decoder\\Codecs")

static const struct vcodec_t {
	const LPCTSTR          opt_name;
	const unsigned __int64 flag;
}
vcodecs[] = {
	{_T("h264"),		CODEC_H264		},
	{_T("mpeg1"),		CODEC_MPEG1		},
	{_T("mpeg3"),		CODEC_MPEG2		},
	{_T("vc1"),			CODEC_VC1		},
	{_T("msmpeg4"),		CODEC_MSMPEG4	},
	{_T("xvid"),		CODEC_XVID		},
	{_T("divx"),		CODEC_DIVX		},
	{_T("wmv"),			CODEC_WMV		},
	{_T("hevc"),		CODEC_HEVC		},
	{_T("vp356"),		CODEC_VP356		},
	{_T("vp89"),		CODEC_VP89		},
	{_T("theora"),		CODEC_THEORA	},
	{_T("mjpeg"),		CODEC_MJPEG		},
	{_T("dv"),			CODEC_DV		},
	{_T("lossless"),	CODEC_LOSSLESS	},
	{_T("prores"),		CODEC_PRORES	},
	{_T("cllc"),		CODEC_CLLC		},
	{_T("screc"),		CODEC_SCREC		},
	{_T("indeo"),		CODEC_INDEO		},
	{_T("h263"),		CODEC_H263		},
	{_T("svq3"),		CODEC_SVQ3		},
	{_T("realv"),		CODEC_REALV		},
	{_T("dirac"),		CODEC_DIRAC		},
	{_T("binkv"),		CODEC_BINKV		},
	{_T("amvv"),		CODEC_AMVV		},
	{_T("flash"),		CODEC_FLASH		},
	{_T("utvd"),		CODEC_UTVD		},
	{_T("png"),			CODEC_PNG		},
	{_T("uncompressed"),CODEC_UNCOMPRESSED},
	{_T("dnxhd"),		CODEC_DNXHD		},
	// dxva codecs
	{_T("h264_dxva"),	CODEC_H264_DXVA	},
	{_T("mpeg2_dxva"),	CODEC_MPEG2_DXVA},
	{_T("vc1_dxva"),	CODEC_VC1_DXVA	},
	{_T("wmv3_dxva"),	CODEC_WMV3_DXVA	}
};
#endif

#define MAX_SUPPORTED_MODE 5

typedef struct {
	const int		PicEntryNumber_DXVA1;
	const int		PicEntryNumber_DXVA2;
	const UINT		PreferedConfigBitstream;
	const GUID*		Decoder[MAX_SUPPORTED_MODE];
	const WORD		RestrictedMode[MAX_SUPPORTED_MODE];
} DXVA_PARAMS;

typedef struct {
	const CLSID*			clsMinorType;
	const enum AVCodecID	nFFCodec;
	const DXVA_PARAMS*		DXVAModes;

	const int				FFMPEGCode;
	const int				DXVACode;

	int DXVAModeCount() {
		if (!DXVAModes) {
			return 0;
		}
		for (int i = 0; i < MAX_SUPPORTED_MODE; i++) {
			if (DXVAModes->Decoder[i] == &GUID_NULL) {
				return i;
			}
		}
		return MAX_SUPPORTED_MODE;
	}
} FFMPEG_CODECS;

// DXVA modes supported for Mpeg2
DXVA_PARAMS		DXVA_Mpeg2 = {
	16,		// PicEntryNumber - DXVA1
	24,		// PicEntryNumber - DXVA2
	1,		// PreferedConfigBitstream
	{ &DXVA2_ModeMPEG2_VLD, &GUID_NULL },
	{ DXVA_RESTRICTED_MODE_MPEG2_D, 0 } // Restricted mode for DXVA1?
};

// DXVA modes supported for H264
DXVA_PARAMS		DXVA_H264 = {
	16,		// PicEntryNumber - DXVA1
	24,		// PicEntryNumber - DXVA2
	2,		// PreferedConfigBitstream
	{ &DXVA2_ModeH264_E, &DXVA2_ModeH264_F, &DXVA_Intel_H264_ClearVideo, &GUID_NULL },
	{ DXVA_RESTRICTED_MODE_H264_E, 0}
};

// DXVA modes supported for VC1
DXVA_PARAMS		DXVA_VC1 = {
	16,		// PicEntryNumber - DXVA1
	24,		// PicEntryNumber - DXVA2
	1,		// PreferedConfigBitstream
	{ &DXVA2_ModeVC1_D, &GUID_NULL },
	{ DXVA_RESTRICTED_MODE_VC1_D, 0}
};

FFMPEG_CODECS		ffCodecs[] = {
	// Flash video
	{ &MEDIASUBTYPE_FLV1, AV_CODEC_ID_FLV1, NULL, FFM_FLV4, -1 },
	{ &MEDIASUBTYPE_flv1, AV_CODEC_ID_FLV1, NULL, FFM_FLV4, -1 },
	{ &MEDIASUBTYPE_FLV4, AV_CODEC_ID_VP6F, NULL, FFM_FLV4, -1 },
	{ &MEDIASUBTYPE_flv4, AV_CODEC_ID_VP6F, NULL, FFM_FLV4, -1 },
	{ &MEDIASUBTYPE_VP6F, AV_CODEC_ID_VP6F, NULL, FFM_FLV4, -1 },
	{ &MEDIASUBTYPE_vp6f, AV_CODEC_ID_VP6F, NULL, FFM_FLV4, -1 },

	// VP3
	{ &MEDIASUBTYPE_VP30, AV_CODEC_ID_VP3,  NULL, FFM_VP356, -1 },
	{ &MEDIASUBTYPE_VP31, AV_CODEC_ID_VP3,  NULL, FFM_VP356, -1 },

	// VP5
	{ &MEDIASUBTYPE_VP50, AV_CODEC_ID_VP5,  NULL, FFM_VP356, -1 },
	{ &MEDIASUBTYPE_vp50, AV_CODEC_ID_VP5,  NULL, FFM_VP356, -1 },

	// VP6
	{ &MEDIASUBTYPE_VP60, AV_CODEC_ID_VP6,  NULL, FFM_VP356, -1 },
	{ &MEDIASUBTYPE_vp60, AV_CODEC_ID_VP6,  NULL, FFM_VP356, -1 },
	{ &MEDIASUBTYPE_VP61, AV_CODEC_ID_VP6,  NULL, FFM_VP356, -1 },
	{ &MEDIASUBTYPE_vp61, AV_CODEC_ID_VP6,  NULL, FFM_VP356, -1 },
	{ &MEDIASUBTYPE_VP62, AV_CODEC_ID_VP6,  NULL, FFM_VP356, -1 },
	{ &MEDIASUBTYPE_vp62, AV_CODEC_ID_VP6,  NULL, FFM_VP356, -1 },
	{ &MEDIASUBTYPE_VP6A, AV_CODEC_ID_VP6A, NULL, FFM_VP356, -1 },
	{ &MEDIASUBTYPE_vp6a, AV_CODEC_ID_VP6A, NULL, FFM_VP356, -1 },

	// VP8
	{ &MEDIASUBTYPE_VP80, AV_CODEC_ID_VP8, NULL, FFM_VP8, -1 },

	// VP9
	{ &MEDIASUBTYPE_VP90, AV_CODEC_ID_VP9, NULL, FFM_VP8, -1 },

	// Xvid
	{ &MEDIASUBTYPE_XVID, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_xvid, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_XVIX, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_xvix, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },

	// DivX
	{ &MEDIASUBTYPE_DX50, AV_CODEC_ID_MPEG4, NULL, FFM_DIVX, -1 },
	{ &MEDIASUBTYPE_dx50, AV_CODEC_ID_MPEG4, NULL, FFM_DIVX, -1 },
	{ &MEDIASUBTYPE_DIVX, AV_CODEC_ID_MPEG4, NULL, FFM_DIVX, -1 },
	{ &MEDIASUBTYPE_divx, AV_CODEC_ID_MPEG4, NULL, FFM_DIVX, -1 },
	{ &MEDIASUBTYPE_Divx, AV_CODEC_ID_MPEG4, NULL, FFM_DIVX, -1 },

	// WMV1/2/3
	{ &MEDIASUBTYPE_WMV1, AV_CODEC_ID_WMV1, NULL, FFM_WMV, -1 },
	{ &MEDIASUBTYPE_wmv1, AV_CODEC_ID_WMV1, NULL, FFM_WMV, -1 },
	{ &MEDIASUBTYPE_WMV2, AV_CODEC_ID_WMV2, NULL, FFM_WMV, -1 },
	{ &MEDIASUBTYPE_wmv2, AV_CODEC_ID_WMV2, NULL, FFM_WMV, -1 },
	{ &MEDIASUBTYPE_WMV3, AV_CODEC_ID_WMV3, &DXVA_VC1, FFM_WMV, TRA_DXVA_WMV3 },
	{ &MEDIASUBTYPE_wmv3, AV_CODEC_ID_WMV3, &DXVA_VC1, FFM_WMV, TRA_DXVA_WMV3 },
	// WMVP
	{ &MEDIASUBTYPE_WMVP, AV_CODEC_ID_WMV3IMAGE, NULL, FFM_WMV, -1 },

	// MPEG-2
	{ &MEDIASUBTYPE_MPEG2_VIDEO, AV_CODEC_ID_MPEG2VIDEO, &DXVA_Mpeg2, FFM_MPEG2, TRA_DXVA_MPEG2 },
	{ &MEDIASUBTYPE_MPG2,		 AV_CODEC_ID_MPEG2VIDEO, &DXVA_Mpeg2, FFM_MPEG2, TRA_DXVA_MPEG2 },

	// MPEG-1
	{ &MEDIASUBTYPE_MPEG1Packet,  AV_CODEC_ID_MPEG1VIDEO, NULL, FFM_MPEG1, -1 },
	{ &MEDIASUBTYPE_MPEG1Payload, AV_CODEC_ID_MPEG1VIDEO, NULL, FFM_MPEG1, -1 },

	// MSMPEG-4
	{ &MEDIASUBTYPE_DIV3, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_div3, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_DVX3, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_dvx3, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_MP43, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_mp43, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_COL1, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_col1, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_DIV4, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_div4, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_DIV5, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_div5, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_DIV6, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_div6, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_AP41, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_ap41, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_MPG3, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_mpg3, AV_CODEC_ID_MSMPEG4V3, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_DIV2, AV_CODEC_ID_MSMPEG4V2, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_div2, AV_CODEC_ID_MSMPEG4V2, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_MP42, AV_CODEC_ID_MSMPEG4V2, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_mp42, AV_CODEC_ID_MSMPEG4V2, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_MPG4, AV_CODEC_ID_MSMPEG4V1, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_mpg4, AV_CODEC_ID_MSMPEG4V1, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_DIV1, AV_CODEC_ID_MSMPEG4V1, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_div1, AV_CODEC_ID_MSMPEG4V1, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_MP41, AV_CODEC_ID_MSMPEG4V1, NULL, FFM_MSMPEG4, -1 },
	{ &MEDIASUBTYPE_mp41, AV_CODEC_ID_MSMPEG4V1, NULL, FFM_MSMPEG4, -1 },

	// AMV Video
	{ &MEDIASUBTYPE_AMVV, AV_CODEC_ID_AMV, NULL, FFM_AMVV, -1 },

	// MJPEG
	{ &MEDIASUBTYPE_MJPG,   AV_CODEC_ID_MJPEG,    NULL, FFM_MJPEG, -1 },
	{ &MEDIASUBTYPE_QTJpeg, AV_CODEC_ID_MJPEG,    NULL, FFM_MJPEG, -1 },
	{ &MEDIASUBTYPE_MJPA,   AV_CODEC_ID_MJPEG,    NULL, FFM_MJPEG, -1 },
	{ &MEDIASUBTYPE_MJPB,   AV_CODEC_ID_MJPEGB,	  NULL, FFM_MJPEG, -1 },
	{ &MEDIASUBTYPE_MJP2,   AV_CODEC_ID_JPEG2000, NULL, FFM_MJPEG, -1 },
	{ &MEDIASUBTYPE_MJ2C,   AV_CODEC_ID_JPEG2000, NULL, FFM_MJPEG, -1 },
	
	// DV VIDEO
	{ &MEDIASUBTYPE_dvsl, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	{ &MEDIASUBTYPE_dvsd, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	{ &MEDIASUBTYPE_dvhd, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	{ &MEDIASUBTYPE_dv25, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	{ &MEDIASUBTYPE_dv50, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	{ &MEDIASUBTYPE_dvh1, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	{ &MEDIASUBTYPE_CDVH, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	{ &MEDIASUBTYPE_CDVC, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	// Quicktime DV sybtypes (used in LAV Splitter)
	{ &MEDIASUBTYPE_DVCP, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	{ &MEDIASUBTYPE_DVPP, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	{ &MEDIASUBTYPE_DV5P, AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },
	{ &MEDIASUBTYPE_DVC,  AV_CODEC_ID_DVVIDEO, NULL, FFM_DV, -1 },

	// CSCD
	{ &MEDIASUBTYPE_CSCD, AV_CODEC_ID_CSCD,		   NULL, FFM_SCREC, -1 },
	// TSCC
	{ &MEDIASUBTYPE_TSCC, AV_CODEC_ID_TSCC,		   NULL, FFM_SCREC, -1 },
	// TSCC2
	{ &MEDIASUBTYPE_TSCC2, AV_CODEC_ID_TSCC2,	   NULL, FFM_SCREC, -1 },
	// VMnc
	{ &MEDIASUBTYPE_VMnc, AV_CODEC_ID_VMNC,		   NULL, FFM_SCREC, -1 },
	// QTRLE
	{ &MEDIASUBTYPE_QTRle, AV_CODEC_ID_QTRLE,	   NULL, FFM_SCREC, -1 },
	// CINEPAK
	{ &MEDIASUBTYPE_CVID, AV_CODEC_ID_CINEPAK,	   NULL, FFM_SCREC, -1 },
	// FLASHSV1
	{ &MEDIASUBTYPE_FLASHSV1, AV_CODEC_ID_FLASHSV, NULL, FFM_SCREC, -1 },
	// FLASHSV2
	//	{ &MEDIASUBTYPE_FLASHSV2,  CODEC_ID_FLASHSV2, NULL, FFM_SCREC, -1 },
	// FRAPS
	{ &MEDIASUBTYPE_FPS1, AV_CODEC_ID_FRAPS,	   NULL, FFM_SCREC, -1 },
	// MSS1
	{ &MEDIASUBTYPE_MSS1, AV_CODEC_ID_MSS1,		   NULL, FFM_SCREC, -1 },
	// MSS2
	{ &MEDIASUBTYPE_MSS2, AV_CODEC_ID_MSS2,		   NULL, FFM_SCREC, -1 },
	// MSA1
	{ &MEDIASUBTYPE_MSA1, AV_CODEC_ID_MSA1,		   NULL, FFM_SCREC, -1 },
	// MTS2
	{ &MEDIASUBTYPE_MTS2, AV_CODEC_ID_MTS2,		   NULL, FFM_SCREC, -1 },
	// G2M
	{ &MEDIASUBTYPE_G2M, AV_CODEC_ID_G2M,		   NULL, FFM_SCREC, -1 },
	// CRAM - Microsoft Video 1
	{ &MEDIASUBTYPE_CRAM, AV_CODEC_ID_MSVIDEO1,	   NULL, FFM_SCREC, -1 },
	//
	{ &MEDIASUBTYPE_8BPS, AV_CODEC_ID_8BPS,		   NULL, FFM_SCREC, -1 },

	// UtVideo
	{ &MEDIASUBTYPE_UTVD_ULRG, AV_CODEC_ID_UTVIDEO, NULL, FFM_UTVD, -1 },
	{ &MEDIASUBTYPE_UTVD_ULRA, AV_CODEC_ID_UTVIDEO, NULL, FFM_UTVD, -1 },
	{ &MEDIASUBTYPE_UTVD_ULY0, AV_CODEC_ID_UTVIDEO, NULL, FFM_UTVD, -1 },
	{ &MEDIASUBTYPE_UTVD_ULY2, AV_CODEC_ID_UTVIDEO, NULL, FFM_UTVD, -1 },

	// DIRAC
	{ &MEDIASUBTYPE_DRAC, AV_CODEC_ID_DIRAC, NULL, FFM_DIRAC, -1 },

	// Lossless Video
	{ &MEDIASUBTYPE_HuffYUV,  AV_CODEC_ID_HUFFYUV,  NULL, FFM_LOSSLESS, -1 },
	{ &MEDIASUBTYPE_Lagarith, AV_CODEC_ID_LAGARITH, NULL, FFM_LOSSLESS, -1 },
	{ &MEDIASUBTYPE_FFVH,     AV_CODEC_ID_FFVHUFF,  NULL, FFM_LOSSLESS, -1 },
	{ &MEDIASUBTYPE_FFV1,     AV_CODEC_ID_FFV1,     NULL, FFM_LOSSLESS, -1 },

	// Indeo 3/4/5
	{ &MEDIASUBTYPE_IV31, AV_CODEC_ID_INDEO3, NULL, FFM_INDEO, -1 },
	{ &MEDIASUBTYPE_IV32, AV_CODEC_ID_INDEO3, NULL, FFM_INDEO, -1 },
	{ &MEDIASUBTYPE_IV41, AV_CODEC_ID_INDEO4, NULL, FFM_INDEO, -1 },
	{ &MEDIASUBTYPE_IV50, AV_CODEC_ID_INDEO5, NULL, FFM_INDEO, -1 },

	// H264/AVC
	{ &MEDIASUBTYPE_H264,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_h264,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_X264,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_x264,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_VSSH,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_vssh,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_DAVC,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_davc,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_PAVC,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_pavc,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_AVC1,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_avc1,	  AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },
	{ &MEDIASUBTYPE_H264_bis, AV_CODEC_ID_H264, &DXVA_H264, FFM_H264, TRA_DXVA_H264 },

	// SVQ3
	{ &MEDIASUBTYPE_SVQ3, AV_CODEC_ID_SVQ3, NULL, FFM_SVQ3, -1 },
	// SVQ1
	{ &MEDIASUBTYPE_SVQ1, AV_CODEC_ID_SVQ1, NULL, FFM_SVQ3, -1 },

	// H263
	{ &MEDIASUBTYPE_H263, AV_CODEC_ID_H263, NULL, FFM_H263, -1 },
	{ &MEDIASUBTYPE_h263, AV_CODEC_ID_H263, NULL, FFM_H263, -1 },
	{ &MEDIASUBTYPE_S263, AV_CODEC_ID_H263, NULL, FFM_H263, -1 },
	{ &MEDIASUBTYPE_s263, AV_CODEC_ID_H263, NULL, FFM_H263, -1 },

	// Real Video
	{ &MEDIASUBTYPE_RV10, AV_CODEC_ID_RV10, NULL, FFM_RV, -1 },
	{ &MEDIASUBTYPE_RV20, AV_CODEC_ID_RV20, NULL, FFM_RV, -1 },
	{ &MEDIASUBTYPE_RV30, AV_CODEC_ID_RV30, NULL, FFM_RV, -1 },
	{ &MEDIASUBTYPE_RV40, AV_CODEC_ID_RV40, NULL, FFM_RV, -1 },

	// Theora
	{ &MEDIASUBTYPE_THEORA, AV_CODEC_ID_THEORA, NULL, FFM_THEORA, -1 },
	{ &MEDIASUBTYPE_theora, AV_CODEC_ID_THEORA, NULL, FFM_THEORA, -1 },

	// WVC1
	{ &MEDIASUBTYPE_WVC1, AV_CODEC_ID_VC1, &DXVA_VC1, FFM_VC1, TRA_DXVA_VC1 },
	{ &MEDIASUBTYPE_wvc1, AV_CODEC_ID_VC1, &DXVA_VC1, FFM_VC1, TRA_DXVA_VC1 },

	// WMVA
	{ &MEDIASUBTYPE_WMVA, AV_CODEC_ID_VC1, &DXVA_VC1, FFM_VC1, TRA_DXVA_VC1 },

	// WVP2
	{ &MEDIASUBTYPE_WVP2, AV_CODEC_ID_VC1IMAGE, NULL, FFM_VC1, -1 },

	// Apple ProRes
	{ &MEDIASUBTYPE_apch, AV_CODEC_ID_PRORES, NULL, FFM_PRORES, -1 },
	{ &MEDIASUBTYPE_apcn, AV_CODEC_ID_PRORES, NULL, FFM_PRORES, -1 },
	{ &MEDIASUBTYPE_apcs, AV_CODEC_ID_PRORES, NULL, FFM_PRORES, -1 },
	{ &MEDIASUBTYPE_apco, AV_CODEC_ID_PRORES, NULL, FFM_PRORES, -1 },
	{ &MEDIASUBTYPE_ap4h, AV_CODEC_ID_PRORES, NULL, FFM_PRORES, -1 },
	{ &MEDIASUBTYPE_icpf, AV_CODEC_ID_PRORES, NULL, FFM_PRORES, -1 },
	{ &MEDIASUBTYPE_icod, AV_CODEC_ID_AIC,    NULL, FFM_PRORES, -1 },

	// Bink Video
	{ &MEDIASUBTYPE_BINKVI, AV_CODEC_ID_BINKVIDEO, NULL, FFM_BINKV, -1 },
	{ &MEDIASUBTYPE_BINKVB, AV_CODEC_ID_BINKVIDEO, NULL, FFM_BINKV, -1 },

	// PNG
	{ &MEDIASUBTYPE_PNG, AV_CODEC_ID_PNG, NULL, FFM_PNG, -1 },

	// Canopus Lossless
	{ &MEDIASUBTYPE_CLLC, AV_CODEC_ID_CLLC, NULL, FFM_CLLC, -1 },

	// HEVC
	{ &MEDIASUBTYPE_HEVC, AV_CODEC_ID_HEVC, NULL, FFM_HEVC, -1 },
	{ &MEDIASUBTYPE_HVC1, AV_CODEC_ID_HEVC, NULL, FFM_HEVC, -1 },
//	{ &MEDIASUBTYPE_HM91, AV_CODEC_ID_HEVC, NULL, FFM_HEVC, -1 },
	{ &MEDIASUBTYPE_HM10, AV_CODEC_ID_HEVC, NULL, FFM_HEVC, -1 },
//	{ &MEDIASUBTYPE_HM12, AV_CODEC_ID_HEVC, NULL, FFM_HEVC, -1 },

	// Avid DNxHD
	{ &MEDIASUBTYPE_AVdn, AV_CODEC_ID_DNXHD, NULL, FFM_DNXHD, -1 },

	// Other MPEG-4
	{ &MEDIASUBTYPE_MP4V, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_mp4v, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_M4S2, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_m4s2, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_MP4S, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_mp4s, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_3IV1, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_3iv1, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_3IV2, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_3iv2, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_3IVX, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_3ivx, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_BLZ0, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_blz0, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_DM4V, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_dm4v, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_FFDS, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_ffds, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_FVFW, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_fvfw, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_DXGM, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_dxgm, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_FMP4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_fmp4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_HDX4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_hdx4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_LMP4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_lmp4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_NDIG, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_ndig, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_RMP4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_rmp4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_SMP4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_smp4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_SEDG, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_sedg, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_UMP4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_ump4, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_WV1F, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },
	{ &MEDIASUBTYPE_wv1f, AV_CODEC_ID_MPEG4, NULL, FFM_XVID, -1 },

	// QT uncompressed video
	{ &MEDIASUBTYPE_v210, AV_CODEC_ID_V210, NULL, FFM_UNCOMPRESSED, -1 },
	{ &MEDIASUBTYPE_V410, AV_CODEC_ID_V410, NULL, FFM_UNCOMPRESSED, -1 },

	// uncompressed video
	{ &MEDIASUBTYPE_Y800, AV_CODEC_ID_RAWVIDEO, NULL, FFM_UNCOMPRESSED, -1 },
	{ &MEDIASUBTYPE_I420, AV_CODEC_ID_RAWVIDEO, NULL, FFM_UNCOMPRESSED, -1 },
	{ &MEDIASUBTYPE_Y41B, AV_CODEC_ID_RAWVIDEO, NULL, FFM_UNCOMPRESSED, -1 },
	{ &MEDIASUBTYPE_Y42B, AV_CODEC_ID_RAWVIDEO, NULL, FFM_UNCOMPRESSED, -1 },
	{ &MEDIASUBTYPE_444P, AV_CODEC_ID_RAWVIDEO, NULL, FFM_UNCOMPRESSED, -1 },
	{ &MEDIASUBTYPE_cyuv, AV_CODEC_ID_RAWVIDEO, NULL, FFM_UNCOMPRESSED, -1 },
	{ &MEDIASUBTYPE_yuv2, AV_CODEC_ID_RAWVIDEO, NULL, FFM_UNCOMPRESSED, -1 },
};

/* Important: the order should be exactly the same as in ffCodecs[] */
const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
	// Flash video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FLV1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_flv1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FLV4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_flv4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP6F },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp6f },

	// VP3
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP30 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP31 },

	// VP5
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP50 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp50 },

	// VP6
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP60 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp60 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP61 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp61 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP62 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp62 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP6A },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp6a },

	// VP8
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP80 },

	// VP9
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP90 },

	// Xvid
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_XVID },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_xvid },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_XVIX },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_xvix },

	// DivX
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DX50 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dx50 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIVX },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_divx },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_Divx },

	// WMV1/2/3
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WMV1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_wmv1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WMV2 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_wmv2 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WMV3 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_wmv3 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WMVP },

	// MPEG-2
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MPEG2_VIDEO },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MPG2        },

	// MPEG-1
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MPEG1Packet  },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MPEG1Payload },

	// MSMPEG-4
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV3 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div3 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DVX3 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dvx3 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MP43 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mp43 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_COL1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_col1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV5 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div5 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV6 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div6 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_AP41 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ap41 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MPG3 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mpg3 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV2 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div2 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MP42 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mp42 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MPG4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mpg4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MP41 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mp41 },

	// AMV Video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_AMVV },

	// MJPEG
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MJPG   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_QTJpeg },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MJPA   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MJPB   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MJP2   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MJ2C   },

	// DV VIDEO
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dvsl },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dvsd },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dvhd },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dv25 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dv50 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dvh1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_CDVH },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_CDVC },
	// Quicktime DV sybtypes (used in LAV Splitter)
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DVCP },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DVPP },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DV5P },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DVC  },

	// CSCD
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_CSCD },

	// TSCC
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_TSCC },

	// TSCC2
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_TSCC2 },

	// VMnc
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VMnc },

	// QTRLE
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_QTRle },

	// CINEPAK
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_CVID },

	// FLASHSV1
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FLASHSV1 },

	// FLASHSV2
//	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FLASHSV2 },

	// Fraps
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FPS1 },

	// MSS1
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MSS1 },

	// MSS2
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MSS2 },

	// MSA1
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MSA1 },

	// MTS2
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MTS2 },

	// G2M
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_G2M },

	// CRAM - Microsoft Video 1
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_CRAM },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_8BPS },

	// UtVideo
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_UTVD_ULRG },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_UTVD_ULRA },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_UTVD_ULY0 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_UTVD_ULY2 },

	// DIRAC
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DRAC },

	// Lossless Video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_HuffYUV  },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_Lagarith },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FFVH     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FFV1     },

	// Indeo 3/4/5
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_IV31 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_IV32 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_IV41 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_IV50 },

	// H264/AVC
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_H264     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_h264     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_X264     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_x264     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VSSH     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vssh     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DAVC     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_davc     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_PAVC     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_pavc     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_AVC1     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_avc1     },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_H264_bis },

	// SVQ3
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_SVQ3 },

	// SVQ1
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_SVQ1 },

	// H263
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_H263 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_h263 },

	{ &MEDIATYPE_Video, &MEDIASUBTYPE_S263 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_s263 },

	// Real video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RV10 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RV20 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RV30 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RV40 },

	// Theora
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_THEORA },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_theora },

	// VC1
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WVC1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_wvc1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WMVA },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WVP2 },

	// Apple ProRes
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_apch },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_apcn },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_apcs },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_apco },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ap4h },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_icpf },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_icod },

	// Bink Video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_BINKVI },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_BINKVB },

	// PNG
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_PNG },

	// Canopus Lossless
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_CLLC },

	// HEVC
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_HEVC },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_HVC1 },
//	{ &MEDIATYPE_Video, &MEDIASUBTYPE_HM91 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_HM10 },
//	{ &MEDIATYPE_Video, &MEDIASUBTYPE_HM12 },

	// Avid DNxHD
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_AVdn },

	// Other MPEG-4
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MP4V },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mp4v },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_M4S2 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_m4s2 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MP4S },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mp4s },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3IV1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3iv1 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3IV2 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3iv2 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3IVX },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3ivx },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_BLZ0 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_blz0 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DM4V },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dm4v },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FFDS },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ffds },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FVFW },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_fvfw },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DXGM },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dxgm },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FMP4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_fmp4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_HDX4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_hdx4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_LMP4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_lmp4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_NDIG },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ndig },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RMP4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_rmp4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_SMP4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_smp4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_SEDG },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_sedg },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_UMP4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ump4 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WV1F },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_wv1f },

	// QT uncompressed video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_v210 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_V410 },

	// uncompressed video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_Y800 },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_I420 }, // YUV 4:2:0 Planar
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_Y41B }, // YUV 4:1:1 Planar
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_Y42B }, // YUV 4:2:2 Planar
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_444P }, // YUV 4:4:4 Planar
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_cyuv }, // UYVY flipped vertically
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_yuv2 }, // modified YUY2, used in QuickTime
};

#pragma endregion any_constants

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NV12},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_YV12},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_YUY2},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_YV16},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_AYUV},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_YV24},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_P010},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_P210},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_Y410},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_P016},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_P216},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_Y416},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_RGB32},
};

VIDEO_OUTPUT_FORMATS DXVAFormats[] = { // DXVA2
	{&MEDIASUBTYPE_NV12,  1, 12, FCC('dxva')},
	{&MEDIASUBTYPE_NV12,  1, 12, FCC('DXVA')},
	{&MEDIASUBTYPE_NV12,  1, 12, FCC('DxVA')},
	{&MEDIASUBTYPE_NV12,  1, 12, FCC('DXvA')}
};

#ifdef REGISTER_FILTER

#include "../../filters/ffmpeg_fix.cpp"

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesIn),  sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilters[] = {
	{&__uuidof(CMPCVideoDecFilter), MPCVideoDecName, MERIT_NORMAL + 1, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
	{sudFilters[0].strName, &__uuidof(CMPCVideoDecFilter), CreateInstance<CMPCVideoDecFilter>, NULL, &sudFilters[0]},
	{L"CMPCVideoDecPropertyPage", &__uuidof(CMPCVideoDecSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CMPCVideoDecSettingsWnd> >},
	{L"CMPCVideoDecPropertyPage2", &__uuidof(CMPCVideoDecCodecWnd), CreateInstance<CInternalPropertyPageTempl<CMPCVideoDecCodecWnd> >},
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

#include "../../filters/Filters.h"

CFilterApp theApp;

#endif

BOOL CALLBACK EnumFindProcessWnd (HWND hwnd, LPARAM lParam)
{
	DWORD	procid = 0;
	TCHAR	WindowClass[40];
	GetWindowThreadProcessId(hwnd, &procid);
	GetClassName(hwnd, WindowClass, _countof(WindowClass));

	if (procid == GetCurrentProcessId() && _tcscmp(WindowClass, _T(MPC_WND_CLASS_NAME)) == 0) {
		HWND* pWnd = (HWND*) lParam;
		*pWnd = hwnd;
		return FALSE;
	}
	return TRUE;
}

// CMPCVideoDecFilter

CMPCVideoDecFilter::CMPCVideoDecFilter(LPUNKNOWN lpunk, HRESULT* phr)
	: CBaseVideoFilter(NAME("MPC - Video decoder"), lpunk, phr, __uuidof(this))
	, m_nThreadNumber(0)
	, m_nDiscardMode(AVDISCARD_DEFAULT)
	, m_nDeinterlacing(AUTO)
	, m_nARMode(2)
	, m_nDXVACheckCompatibility(1)
	, m_nDXVA_SD(0)
	, m_nSwPreset(2)
	, m_nSwStandard(2)
	, m_nSwRGBLevels(0)
	, m_pAVCodec(NULL)
	, m_pAVCtx(NULL)
	, m_pFrame(NULL)
	, m_pParser(NULL)
	, m_nCodecNb(-1)
	, m_nCodecId(AV_CODEC_ID_NONE)
	, m_bReorderBFrame(true)
	, m_nPosB(1)
	, m_bWaitKeyFrame(false)
	, m_DXVADecoderGUID(GUID_NULL)
	, m_nActiveCodecs(CODECS_ALL)
	, m_rtAvrTimePerFrame(0)
	, m_rtLastStop(0)
	, m_rtPrevStop(0)
	, m_rtStartCache(INVALID_TIME)
	, m_nWorkaroundBug(FF_BUG_AUTODETECT)
	, m_nErrorConcealment(FF_EC_DEBLOCK | FF_EC_GUESS_MVS)
	, m_bDXVACompatible(true)
	, m_pFFBuffer(NULL)
	, m_nFFBufferSize(0)
	, m_pFFBuffer2(NULL)
	, m_nFFBufferSize2(0)
	, m_nOutputWidth(0)
	, m_nOutputHeight(0)
	, m_nARX(0)
	, m_nARY(0)
	, m_bUseDXVA(true)
	, m_bUseFFmpeg(true)
	, m_pDXVADecoder(NULL)
	, m_pVideoOutputFormat(NULL)
	, m_nVideoOutputCount(0)
	, m_hDevice(INVALID_HANDLE_VALUE)
	, m_bWaitingForKeyFrame(TRUE)
	, m_bIsEVO(FALSE)
	, m_nFrameType(PICT_FRAME)
	, m_PixelFormat(AV_PIX_FMT_NONE)
{
	for (int i = 0; i < PixFmt_count; i++) {
		if (i == PixFmt_AYUV) {
			m_fPixFmts[i] = false;
		} else {
			m_fPixFmts[i] = true;
		}
	}

	if (phr) {
		*phr = S_OK;
	}

	if (m_pOutput)	{
		delete m_pOutput;
	}
	m_pOutput = DNew CVideoDecOutputPin(NAME("CVideoDecOutputPin"), this, phr, L"Output");
	if (!m_pOutput) {
		*phr = E_OUTOFMEMORY;
	}

	memset(&m_DDPixelFormat, 0, sizeof(m_DDPixelFormat));

	memset(&m_DXVAFilters, false, sizeof(m_DXVAFilters));
	memset(&m_FFmpegFilters, false, sizeof(m_FFmpegFilters));
	
	m_pCpuId = DNew CCpuId();

#ifdef REGISTER_FILTER
	CRegKey key;
	ULONG len = 255;
	if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, OPT_REGKEY_VideoDec, KEY_READ)) {
		DWORD dw;
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_ThreadNumber, dw)) {
			m_nThreadNumber = dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_DiscardMode, dw)) {
			m_nDiscardMode = dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_Deinterlacing, dw)) {
			m_nDeinterlacing = (MPC_DEINTERLACING_FLAGS)dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_ARMode, dw)) {
			m_nARMode = dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_DXVACheck, dw)) {
			m_nDXVACheckCompatibility = dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_DisableDXVA_SD, dw)) {
			m_nDXVA_SD = dw;
		}

		for (int i = 0; i < PixFmt_count; i++) {
			CString optname = OPT_SW_prefix;
			optname += GetSWOF(i)->name;
			if (ERROR_SUCCESS == key.QueryDWORDValue(optname, dw)) {
				m_fPixFmts[i] = !!dw;
			}
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_SwPreset, dw)) {
			m_nSwPreset = dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_SwStandard, dw)) {
			m_nSwStandard = dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(OPT_SwRGBLevels, dw)) {
			m_nSwRGBLevels = dw;
		}
	}
	if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, OPT_REGKEY_VCodecs, KEY_READ)) {
		m_nActiveCodecs = 0;
		for (size_t i = 0; i < _countof(vcodecs); i++) {
			DWORD dw = 1;
			key.QueryDWORDValue(vcodecs[i].opt_name, dw);
			if (dw) {
				m_nActiveCodecs |= vcodecs[i].flag;
			}
		}
	}
#else
	m_nThreadNumber				= AfxGetApp()->GetProfileInt(OPT_SECTION_VideoDec, OPT_ThreadNumber, m_nThreadNumber);
	m_nDiscardMode				= AfxGetApp()->GetProfileInt(OPT_SECTION_VideoDec, OPT_DiscardMode, m_nDiscardMode);
	m_nDeinterlacing			= (MPC_DEINTERLACING_FLAGS)AfxGetApp()->GetProfileInt(OPT_SECTION_VideoDec, OPT_Deinterlacing, m_nDeinterlacing);
	m_nARMode					= AfxGetApp()->GetProfileInt(OPT_SECTION_VideoDec, OPT_ARMode, m_nARMode);

	m_nDXVACheckCompatibility	= AfxGetApp()->GetProfileInt(OPT_SECTION_VideoDec, OPT_DXVACheck, m_nDXVACheckCompatibility);
	m_nDXVA_SD					= AfxGetApp()->GetProfileInt(OPT_SECTION_VideoDec, OPT_DisableDXVA_SD, m_nDXVA_SD);

	for (int i = 0; i < PixFmt_count; i++) {
		CString optname = OPT_SW_prefix;
		optname += GetSWOF(i)->name;
		m_fPixFmts[i] = !!AfxGetApp()->GetProfileInt(OPT_SECTION_VideoDec, optname, m_fPixFmts[i]);
	}
	m_nSwPreset					= AfxGetApp()->GetProfileInt(OPT_SECTION_VideoDec, OPT_SwPreset, m_nSwPreset);
	m_nSwStandard				= AfxGetApp()->GetProfileInt(OPT_SECTION_VideoDec, OPT_SwStandard, m_nSwStandard);
	m_nSwRGBLevels				= AfxGetApp()->GetProfileInt(OPT_SECTION_VideoDec, OPT_SwRGBLevels, m_nSwRGBLevels);
#endif

	m_nDXVACheckCompatibility = max(0, min(m_nDXVACheckCompatibility, 3));

	if (m_nDeinterlacing > PROGRESSIVE) {
		m_nDeinterlacing = AUTO;
	}
	if (m_nSwRGBLevels != 1) {
		m_nSwRGBLevels = 0;
	}

	avcodec_register_all();
	av_log_set_callback(ff_log);
	m_FormatConverter.SetOptions(m_nSwPreset, m_nSwStandard, m_nSwRGBLevels);

	HWND hWnd = NULL;
	EnumWindows(EnumFindProcessWnd, (LPARAM)&hWnd);
	DetectVideoCard(hWnd);

#ifdef _DEBUG
	// Check codec definition table
	size_t nCodecs		= _countof(ffCodecs);
	size_t nPinTypes	= _countof(sudPinTypesIn);
	ASSERT(nCodecs == nPinTypes);
	for (size_t i = 0; i < nPinTypes; i++) {
		ASSERT(ffCodecs[i].clsMinorType == sudPinTypesIn[i].clsMinorType);
	}
#endif
}

CMPCVideoDecFilter::~CMPCVideoDecFilter()
{
	Cleanup();

	SAFE_DELETE(m_pCpuId);
}

void CMPCVideoDecFilter::DetectVideoCard(HWND hWnd)
{
	IDirect3D9* pD3D9				= NULL;
	m_nPCIVendor					= 0;
	m_nPCIDevice					= 0;
	m_VideoDriverVersion.HighPart	= 0;
	m_VideoDriverVersion.LowPart	= 0;

	pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (pD3D9) {
		D3DADAPTER_IDENTIFIER9 adapterIdentifier;
		if (pD3D9->GetAdapterIdentifier(GetAdapter(pD3D9, hWnd), 0, &adapterIdentifier) == S_OK) {
			m_nPCIVendor = adapterIdentifier.VendorId;
			m_nPCIDevice = adapterIdentifier.DeviceId;
			m_VideoDriverVersion	= adapterIdentifier.DriverVersion;
			m_strDeviceDescription	= adapterIdentifier.Description;
			m_strDeviceDescription.AppendFormat (_T(" (%04X:%04X)"), m_nPCIVendor, m_nPCIDevice);
		}
		pD3D9->Release();
	}
}

bool CMPCVideoDecFilter::IsVideoInterlaced()
{
	// NOT A BUG : always tell DirectShow it's interlaced - progressive flags set in SetTypeSpecificFlags() function
	return true;
};

REFERENCE_TIME CMPCVideoDecFilter::GetDuration()
{
	REFERENCE_TIME AvgTimePerFrame = m_rtAvrTimePerFrame;
	if ((m_nCodecId == AV_CODEC_ID_MPEG2VIDEO || m_nCodecId == AV_CODEC_ID_MPEG1VIDEO)
		|| m_rtAvrTimePerFrame < 166666) // fps > 60 ... try to get fps value from ffmpeg
	{
		if (m_pAVCtx->time_base.den && m_pAVCtx->time_base.num) {
			AvgTimePerFrame = (UNITS * m_pAVCtx->time_base.num / m_pAVCtx->time_base.den) * m_pAVCtx->ticks_per_frame;
		}
	}

	return max(AvgTimePerFrame, m_rtAvrTimePerFrame);
}

#define AVRTIMEPERFRAME_PULLDOWN 417083
void CMPCVideoDecFilter::UpdateFrameTime(REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop, bool pulldown_flag)
{
	REFERENCE_TIME AvgTimePerFrame = GetDuration();
	bool m_PullDownFlag = pulldown_flag && AvgTimePerFrame == 333666;
	REFERENCE_TIME m_rtFrameDuration = m_PullDownFlag ? AVRTIMEPERFRAME_PULLDOWN : (AvgTimePerFrame * (m_pFrame->repeat_pict ? 3 : 2)  / 2);

	if ((rtStart == INVALID_TIME) || (m_PullDownFlag && m_rtPrevStop && (rtStart <= m_rtPrevStop))) {
		rtStart = m_rtLastStop;
	}

	rtStop			= rtStart + (m_rtFrameDuration / m_dRate);
	m_rtLastStop	= rtStop;
}

void CMPCVideoDecFilter::GetOutputSize(int& w, int& h, int& arx, int& ary, int& RealWidth, int& RealHeight)
{
	RealWidth	= PictWidth();
	RealHeight	= PictHeight();
	w			= PictWidthRounded();
	h			= PictHeightRounded();
}

int CMPCVideoDecFilter::PictWidth()
{
	return m_pAVCtx->width;
}

int CMPCVideoDecFilter::PictHeight()
{
	return m_pAVCtx->height;
}

int CMPCVideoDecFilter::PictWidthRounded()
{
	// Picture height should be rounded to 16 for DXVA
	if (!m_nOutputWidth || m_nOutputWidth < m_pAVCtx->coded_width) {
		m_nOutputWidth = m_pAVCtx->coded_width;
	}
	return ((m_nOutputWidth + 15) & ~15); // rounding to 16
}

int CMPCVideoDecFilter::PictHeightRounded()
{
	// Picture height should be rounded to 16 for DXVA
	if (!m_nOutputHeight || m_nOutputHeight < m_pAVCtx->coded_height) {
		m_nOutputHeight = m_pAVCtx->coded_height;
	}
	return ((m_nOutputHeight + 15) & ~15); // rounding to 16
}

static bool IsFFMPEGEnabled(FFMPEG_CODECS ffcodec, const bool FFmpegFilters[FFM_LAST + !FFM_LAST])
{
	if (ffcodec.FFMPEGCode < 0 || ffcodec.FFMPEGCode >= FFM_LAST) {
		return false;
	}

	return FFmpegFilters[ffcodec.FFMPEGCode];
}

static bool IsDXVAEnabled(FFMPEG_CODECS ffcodec, const bool DXVAFilters[TRA_DXVA_LAST + !TRA_DXVA_LAST])
{
	if (ffcodec.DXVACode < 0 || ffcodec.DXVACode >= TRA_DXVA_LAST) {
		return false;
	}

	return DXVAFilters[ffcodec.DXVACode];
}

int CMPCVideoDecFilter::FindCodec(const CMediaType* mtIn, bool bForced)
{
	m_bUseFFmpeg	= false;
	m_bUseDXVA		= false;
	for (size_t i = 0; i < _countof(ffCodecs); i++)
		if (mtIn->subtype == *ffCodecs[i].clsMinorType) {
#ifndef REGISTER_FILTER
			m_bUseFFmpeg	= bForced || IsFFMPEGEnabled(ffCodecs[i], m_FFmpegFilters);
			m_bUseDXVA		= bForced || IsDXVAEnabled(ffCodecs[i], m_DXVAFilters);
			return ((m_bUseDXVA || m_bUseFFmpeg) ? i : -1);
#else
			bool	bCodecActivated = false;
			m_bUseFFmpeg			= true;
			switch (ffCodecs[i].nFFCodec) {
				case AV_CODEC_ID_FLV1 :
				case AV_CODEC_ID_VP6F :
					bCodecActivated = (m_nActiveCodecs & CODEC_FLASH) != 0;
					break;
				case AV_CODEC_ID_MPEG4 :
					if ((*ffCodecs[i].clsMinorType == MEDIASUBTYPE_DX50) ||		// DivX
							(*ffCodecs[i].clsMinorType == MEDIASUBTYPE_dx50) ||
							(*ffCodecs[i].clsMinorType == MEDIASUBTYPE_DIVX) ||
							(*ffCodecs[i].clsMinorType == MEDIASUBTYPE_divx) ||
							(*ffCodecs[i].clsMinorType == MEDIASUBTYPE_Divx) ) {
						bCodecActivated = (m_nActiveCodecs & CODEC_DIVX) != 0;
					} else {
						bCodecActivated = (m_nActiveCodecs & CODEC_XVID) != 0;	// Xvid/MPEG-4
					}
					break;
				case AV_CODEC_ID_WMV1 :
				case AV_CODEC_ID_WMV2 :
				case AV_CODEC_ID_WMV3IMAGE :
					bCodecActivated = (m_nActiveCodecs & CODEC_WMV) != 0;
					break;
				case AV_CODEC_ID_WMV3 :
					m_bUseDXVA		= (m_nActiveCodecs & CODEC_WMV3_DXVA) != 0;
					m_bUseFFmpeg	= (m_nActiveCodecs & CODEC_WMV) != 0;
					bCodecActivated	= m_bUseDXVA || m_bUseFFmpeg;
					break;
				case AV_CODEC_ID_MSMPEG4V3 :
				case AV_CODEC_ID_MSMPEG4V2 :
				case AV_CODEC_ID_MSMPEG4V1 :
					bCodecActivated = (m_nActiveCodecs & CODEC_MSMPEG4) != 0;
					break;
				case AV_CODEC_ID_H264 :
					m_bUseDXVA		= (m_nActiveCodecs & CODEC_H264_DXVA) != 0;
					m_bUseFFmpeg	= (m_nActiveCodecs & CODEC_H264) != 0;
					bCodecActivated	= m_bUseDXVA || m_bUseFFmpeg;
					break;
				case AV_CODEC_ID_SVQ3 :
				case AV_CODEC_ID_SVQ1 :
					bCodecActivated = (m_nActiveCodecs & CODEC_SVQ3) != 0;
					break;
				case AV_CODEC_ID_H263 :
					bCodecActivated = (m_nActiveCodecs & CODEC_H263) != 0;
					break;
				case AV_CODEC_ID_DIRAC  :
					bCodecActivated = (m_nActiveCodecs & CODEC_DIRAC) != 0;
					break;
				case AV_CODEC_ID_DVVIDEO  :
					bCodecActivated = (m_nActiveCodecs & CODEC_DV) != 0;
					break;
				case AV_CODEC_ID_THEORA :
					bCodecActivated = (m_nActiveCodecs & CODEC_THEORA) != 0;
					break;
				case AV_CODEC_ID_VC1 :
					m_bUseDXVA		= (m_nActiveCodecs & CODEC_VC1_DXVA) != 0;
					m_bUseFFmpeg	= (m_nActiveCodecs & CODEC_VC1) != 0;
					bCodecActivated	= m_bUseDXVA || m_bUseFFmpeg;
					break;
				case AV_CODEC_ID_VC1IMAGE :
					bCodecActivated = (m_nActiveCodecs & CODEC_VC1) != 0;
					break;
				case AV_CODEC_ID_AMV :
					bCodecActivated = (m_nActiveCodecs & CODEC_AMVV) != 0;
					break;
				case AV_CODEC_ID_LAGARITH :
					bCodecActivated = (m_nActiveCodecs & CODEC_LOSSLESS) != 0;
					break;
				case AV_CODEC_ID_VP3  :
				case AV_CODEC_ID_VP5  :
				case AV_CODEC_ID_VP6  :
				case AV_CODEC_ID_VP6A :
					bCodecActivated = (m_nActiveCodecs & CODEC_VP356) != 0;
					break;
				case AV_CODEC_ID_VP8  :
				case AV_CODEC_ID_VP9  :
					bCodecActivated = (m_nActiveCodecs & CODEC_VP89) != 0;
					break;
				case AV_CODEC_ID_MJPEG  :
				case AV_CODEC_ID_MJPEGB :
					bCodecActivated = (m_nActiveCodecs & CODEC_MJPEG) != 0;
					break;
				case AV_CODEC_ID_INDEO3 :
				case AV_CODEC_ID_INDEO4 :
				case AV_CODEC_ID_INDEO5 :
					bCodecActivated = (m_nActiveCodecs & CODEC_INDEO) != 0;
					break;
				case AV_CODEC_ID_UTVIDEO :
					bCodecActivated = (m_nActiveCodecs & CODEC_UTVD) != 0;
					break;
				case AV_CODEC_ID_CSCD    :
				case AV_CODEC_ID_QTRLE   :
				case AV_CODEC_ID_TSCC    :
				case AV_CODEC_ID_TSCC2   :
				case AV_CODEC_ID_VMNC    :
				case AV_CODEC_ID_CINEPAK :
					bCodecActivated = (m_nActiveCodecs & CODEC_SCREC) != 0;
					break;
				case AV_CODEC_ID_RV10 :
				case AV_CODEC_ID_RV20 :
				case AV_CODEC_ID_RV30 :
				case AV_CODEC_ID_RV40 :
					bCodecActivated = (m_nActiveCodecs & CODEC_REALV) != 0;
					break;
				case AV_CODEC_ID_MPEG2VIDEO :
					m_bUseDXVA		= (m_nActiveCodecs & CODEC_MPEG2_DXVA) != 0;
					m_bUseFFmpeg	= (m_nActiveCodecs & CODEC_MPEG2) != 0;
					bCodecActivated	= m_bUseDXVA || m_bUseFFmpeg;
					break;
				case AV_CODEC_ID_MPEG1VIDEO :
					bCodecActivated = (m_nActiveCodecs & CODEC_MPEG1) != 0;
					break;
				case AV_CODEC_ID_PRORES :
					bCodecActivated = (m_nActiveCodecs & CODEC_PRORES) != 0;
					break;
				case AV_CODEC_ID_BINKVIDEO :
					bCodecActivated = (m_nActiveCodecs & CODEC_BINKV) != 0;
					break;
				case AV_CODEC_ID_PNG :
					bCodecActivated = (m_nActiveCodecs & CODEC_PNG) != 0;
					break;
				case AV_CODEC_ID_CLLC :
					bCodecActivated = (m_nActiveCodecs & CODEC_CLLC) != 0;
					break;
				case AV_CODEC_ID_V210 :
				case AV_CODEC_ID_V410 :
				case AV_CODEC_ID_RAWVIDEO :
					bCodecActivated = (m_nActiveCodecs & CODEC_UNCOMPRESSED) != 0;
					break;
				case AV_CODEC_ID_HEVC :
					bCodecActivated = (m_nActiveCodecs & CODEC_HEVC) != 0;
					break;
				case AV_CODEC_ID_DNXHD :
					bCodecActivated = (m_nActiveCodecs & CODEC_DNXHD) != 0;
					break;
			}

			if (!bCodecActivated && !bForced) {
				m_bUseFFmpeg = false;
			}
			return ((bForced || bCodecActivated) ? i : -1);
#endif
		}

	return -1;
}

void CMPCVideoDecFilter::Cleanup()
{
	SAFE_DELETE(m_pDXVADecoder);

	ffmpegCleanup();

	SAFE_DELETE_ARRAY(m_pVideoOutputFormat);

	// Release DXVA ressources
	if (m_hDevice != INVALID_HANDLE_VALUE) {
		m_pDeviceManager->CloseDeviceHandle(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
	}

	m_pDeviceManager		= NULL;
	m_pDecoderService		= NULL;
	m_pDecoderRenderTarget	= NULL;
}

void CMPCVideoDecFilter::ffmpegCleanup()
{
	// Release FFMpeg
	m_pAVCodec = NULL;

	if (m_pParser) {
		av_parser_close(m_pParser);
		m_pParser = NULL;
	}

	if (m_pAVCtx) {
		avcodec_close(m_pAVCtx);
		av_freep(&m_pAVCtx->extradata);
		av_freep(&m_pAVCtx);
	}

	av_frame_free(&m_pFrame);

	av_freep(&m_pFFBuffer);
	m_nFFBufferSize = 0;
	av_freep(&m_pFFBuffer2);
	m_nFFBufferSize2 = 0;

	m_FormatConverter.Cleanup();
	
	m_nCodecNb		= -1;
	m_nCodecId		= AV_CODEC_ID_NONE;
}

STDMETHODIMP CMPCVideoDecFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(IMPCVideoDecFilter)
		QI(ISpecifyPropertyPages)
		QI(ISpecifyPropertyPages2)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CMPCVideoDecFilter::CheckInputType(const CMediaType* mtIn)
{
	for (size_t i = 0; i < _countof(sudPinTypesIn); i++) {
		if ((mtIn->majortype == *sudPinTypesIn[i].clsMajorType) &&
				(mtIn->subtype == *sudPinTypesIn[i].clsMinorType)) {
			return S_OK;
		}
	}

	return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CMPCVideoDecFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	return CheckInputType(mtIn); // TODO - add check output MediaType
}

bool CMPCVideoDecFilter::IsAVI()
{
	static DWORD SYNC = 0;
	if (SYNC == MAKEFOURCC('R','I','F','F')) {
		return true;
	} else if (SYNC != 0) {
		return false;
	}

	CString fname;

	BeginEnumFilters(m_pGraph, pEF, pBF) {
		CComQIPtr<IFileSourceFilter> pFSF = pBF;
		if (pFSF) {
			LPOLESTR pFN = NULL;
			AM_MEDIA_TYPE mt;
			if (SUCCEEDED(pFSF->GetCurFile(&pFN, &mt)) && pFN && *pFN) {
				fname = CString(pFN);
				CoTaskMemFree(pFN);
			}
			break;
		}
	}
	EndEnumFilters

	if (!fname.IsEmpty() && ::PathFileExists(fname)) {
		CFile f;
		CFileException fileException;
		if (!f.Open(fname, CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone, &fileException)) {
			DbgLog((LOG_TRACE, 3, _T("CMPCVideoDecFilter::IsAVI() : Can't open file '%s', error = %u"), fname, fileException.m_cause));
			return false;
		}

		if (f.Read(&SYNC, sizeof(SYNC)) != sizeof(SYNC)) {
			DbgLog((LOG_TRACE, 3, _T("CMPCVideoDecFilter::IsAVI() : Can't read SYNC from file '%s'"), fname));
			return false;
		}

		if (SYNC == MAKEFOURCC('R','I','F','F')) {
			DbgLog((LOG_TRACE, 3, _T("CMPCVideoDecFilter::IsAVI() : '%s' is a valid AVI file"), fname));
			return true;
		}

		DbgLog((LOG_TRACE, 3, _T("CMPCVideoDecFilter::IsAVI() : '%s' is not valid AVI file"), fname));
	}

	return false;
}

HRESULT CMPCVideoDecFilter::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{
	if (direction == PINDIR_INPUT) {
		HRESULT hr = InitDecoder(pmt);
		if (FAILED(hr)) {
			return hr;
		}
	} else if (direction == PINDIR_OUTPUT) {
		BITMAPINFOHEADER bihOut;
		if (!ExtractBIH(&m_pOutput->CurrentMediaType(), &bihOut)) {
			return E_FAIL;
		}
		m_FormatConverter.UpdateOutput2(bihOut.biCompression, bihOut.biWidth, bihOut.biHeight);
	}

	return __super::SetMediaType(direction, pmt);
}

bool CMPCVideoDecFilter::IsDXVASupported()
{
	if (m_nCodecNb != -1) {
		// Does the codec suppport DXVA ?
		if (ffCodecs[m_nCodecNb].DXVAModes != NULL) {
			// Enabled by user ?
			if (m_bUseDXVA) {
				// is the file compatible ?
				if (m_bDXVACompatible) {
					return true;
				}
			}
		}
	}
	return false;
}

HRESULT CMPCVideoDecFilter::FindDecoderConfiguration()
{
	DbgLog((LOG_TRACE, 3, L"CMPCVideoDecFilter::FindDecoderConfiguration(DXVA2)"));

	HRESULT hr = E_FAIL;

	m_DXVADecoderGUID = GUID_NULL;

	if (m_pDecoderService) {
		UINT cDecoderGuids				= 0;
		GUID* pDecoderGuids				= NULL;
		GUID guidDecoder				= GUID_NULL;
		BOOL bFoundDXVA2Configuration	= FALSE;
		BOOL bHasIntelGuid				= FALSE;
		DXVA2_ConfigPictureDecode config;
		ZeroMemory(&config, sizeof(config));

		if (SUCCEEDED(hr = m_pDecoderService->GetDecoderDeviceGuids(&cDecoderGuids, &pDecoderGuids))) {

			//Intel patch for Ivy Bridge and Sandy Bridge
			if (m_nPCIVendor == PCIV_Intel) {
				for (UINT iCnt = 0; iCnt < cDecoderGuids; iCnt++) {
					if (pDecoderGuids[iCnt] == DXVA_Intel_H264_ClearVideo)
						bHasIntelGuid = TRUE;
				}
			}
			// Look for the decoder GUIDs we want.
			for (UINT iGuid = 0; iGuid < cDecoderGuids; iGuid++) {
				// Do we support this mode?
				if (!IsSupportedDecoderMode(&pDecoderGuids[iGuid])) {
					continue;
				}

				DbgLog((LOG_TRACE, 3, L"	=> Attempt : %s", GetDXVAMode(&pDecoderGuids[iGuid])));

				// Find a configuration that we support.
				if (FAILED(hr = FindDXVA2DecoderConfiguration(m_pDecoderService, pDecoderGuids[iGuid], &config, &bFoundDXVA2Configuration))) {
					break;
				}

				// Patch for the Sandy Bridge (prevent crash on Mode_E, fixme later)
				if (m_nPCIVendor == PCIV_Intel && pDecoderGuids[iGuid] == DXVA2_ModeH264_E && bHasIntelGuid) {
					continue;
				}

				if (bFoundDXVA2Configuration) {
					// Found a good configuration. Save the GUID.
					guidDecoder = pDecoderGuids[iGuid];
					DbgLog((LOG_TRACE, 3, L"	=> Use : %s", GetDXVAMode(&guidDecoder)));
					if (!bHasIntelGuid) {
						break;
					}
				}
			}
		}

		if (pDecoderGuids) {
			CoTaskMemFree(pDecoderGuids);
		}
		if (!bFoundDXVA2Configuration) {
			hr = E_FAIL; // Unable to find a configuration.
		}

		if (SUCCEEDED(hr)) {
			m_DXVA2Config		= config;
			m_DXVADecoderGUID	= guidDecoder;
		}
	}

	return hr;
}

#define ATI_IDENTIFY					_T("ATI ")
#define AMD_IDENTIFY					_T("AMD ")
#define RADEON_HD_IDENTIFY				_T("Radeon HD ")
#define RADEON_MOBILITY_HD_IDENTIFY		_T("Mobility Radeon HD ")

HRESULT CMPCVideoDecFilter::InitDecoder(const CMediaType *pmt)
{
	DbgLog((LOG_TRACE, 3, L"CMPCVideoDecFilter::InitDecoder()"));

	bool bReinit = ((m_pAVCtx != NULL));

	ffmpegCleanup();

	int nNewCodec = FindCodec(pmt, bReinit);

	if (nNewCodec == -1) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// Prevent connection to the video decoder - need to support decoding of uncompressed video (v210, V410, Y800, I420)
	if (CComPtr<IBaseFilter> pFilter = GetFilterFromPin(m_pInput->GetConnected()) ) {
		if (IsVideoDecoder(pFilter, true)) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}

	m_nCodecNb	= nNewCodec;
	m_nCodecId	= ffCodecs[nNewCodec].nFFCodec;

	m_bReorderBFrame	= true;
	m_pAVCodec			= avcodec_find_decoder(m_nCodecId);
	CheckPointer(m_pAVCodec, VFW_E_UNSUPPORTED_VIDEO);

	m_pAVCtx = avcodec_alloc_context3(m_pAVCodec);
	CheckPointer(m_pAVCtx, E_POINTER);

	if (m_nCodecId == AV_CODEC_ID_MPEG2VIDEO
			|| m_nCodecId == AV_CODEC_ID_MPEG1VIDEO
			|| pmt->subtype == MEDIASUBTYPE_H264
			|| pmt->subtype == MEDIASUBTYPE_h264
			|| pmt->subtype == MEDIASUBTYPE_X264
			|| pmt->subtype == MEDIASUBTYPE_x264
			|| pmt->subtype == MEDIASUBTYPE_H264_bis
			|| pmt->subtype == MEDIASUBTYPE_HEVC) {
		m_pParser = av_parser_init(m_nCodecId);
	}

	if (bReinit && m_nDecoderMode == MODE_SOFTWARE) {
		m_bUseDXVA = false;
	}

	int nThreadNumber = m_nThreadNumber ? m_nThreadNumber : m_pCpuId->GetProcessorNumber() * 3/2;
	m_pAVCtx->thread_count = max(1, min((IsDXVASupported() || m_nCodecId == AV_CODEC_ID_MPEG4) ? 1 : nThreadNumber, MAX_AUTO_THREADS));

	m_pFrame = av_frame_alloc();
	CheckPointer(m_pFrame, E_POINTER);

	BITMAPINFOHEADER *pBMI = NULL;
	if (pmt->formattype == FORMAT_VideoInfo) {
		VIDEOINFOHEADER* vih	= (VIDEOINFOHEADER*)pmt->pbFormat;
		pBMI					= &vih->bmiHeader;
	} else if (pmt->formattype == FORMAT_VideoInfo2) {
		VIDEOINFOHEADER2* vih2	= (VIDEOINFOHEADER2*)pmt->pbFormat;
		pBMI					= &vih2->bmiHeader;
	} else if (pmt->formattype == FORMAT_MPEGVideo) {
		MPEG1VIDEOINFO* mpgv	= (MPEG1VIDEOINFO*)pmt->pbFormat;
		pBMI					= &mpgv->hdr.bmiHeader;
	} else if (pmt->formattype == FORMAT_MPEG2Video) {
		MPEG2VIDEOINFO* mpg2v	= (MPEG2VIDEOINFO*)pmt->pbFormat;
		pBMI					= &mpg2v->hdr.bmiHeader;

		switch (m_nCodecId) {
			case AV_CODEC_ID_H264:
				m_bReorderBFrame = IsAVI() ? true : false;
				break;
			case AV_CODEC_ID_MPEG4:
				m_bReorderBFrame = false;
				break;
		}
	} else {
		return VFW_E_INVALIDMEDIATYPE;
	}

	if (m_nCodecId == AV_CODEC_ID_RV10
			|| m_nCodecId == AV_CODEC_ID_RV20
			|| m_nCodecId == AV_CODEC_ID_RV30
			|| m_nCodecId == AV_CODEC_ID_RV40
			|| m_nCodecId == AV_CODEC_ID_VP3
			|| m_nCodecId == AV_CODEC_ID_VP8
			|| m_nCodecId == AV_CODEC_ID_VP9
			|| m_nCodecId == AV_CODEC_ID_THEORA
			|| m_nCodecId == AV_CODEC_ID_MPEG2VIDEO
			|| m_nCodecId == AV_CODEC_ID_MPEG1VIDEO
			|| m_nCodecId == AV_CODEC_ID_DIRAC
			|| m_nCodecId == AV_CODEC_ID_UTVIDEO
			|| m_nCodecId == AV_CODEC_ID_HUFFYUV
			|| m_nCodecId == AV_CODEC_ID_FFVHUFF
			|| m_nCodecId == AV_CODEC_ID_HEVC
			|| m_nCodecId == AV_CODEC_ID_DNXHD) {
		m_bReorderBFrame = false;
	}

	m_bWaitKeyFrame =	m_nCodecId == AV_CODEC_ID_VC1
					 || m_nCodecId == AV_CODEC_ID_VC1IMAGE
					 || m_nCodecId == AV_CODEC_ID_WMV3
					 || m_nCodecId == AV_CODEC_ID_WMV3IMAGE
					 || m_nCodecId == AV_CODEC_ID_MPEG2VIDEO
					 || m_nCodecId == AV_CODEC_ID_RV30
					 || m_nCodecId == AV_CODEC_ID_RV40
					 || m_nCodecId == AV_CODEC_ID_VP3
					 || m_nCodecId == AV_CODEC_ID_THEORA
					 || m_nCodecId == AV_CODEC_ID_MPEG4;

	m_pAVCtx->codec_id              = m_nCodecId;
	m_pAVCtx->codec_tag             = pBMI->biCompression ? pBMI->biCompression : pmt->subtype.Data1;
	m_pAVCtx->coded_width           = pBMI->biWidth;
	m_pAVCtx->coded_height          = abs(pBMI->biHeight);
	m_pAVCtx->bits_per_coded_sample = pBMI->biBitCount;
	m_pAVCtx->workaround_bugs       = m_nWorkaroundBug;
	m_pAVCtx->error_concealment     = m_nErrorConcealment;
	m_pAVCtx->err_recognition       = AV_EF_CAREFUL;
	m_pAVCtx->idct_algo             = FF_IDCT_AUTO;
	m_pAVCtx->skip_loop_filter      = (AVDiscard)m_nDiscardMode;
	m_pAVCtx->refcounted_frames		= 1;

	if (m_nCodecId == AV_CODEC_ID_H264 && IsDXVASupported()) {
		m_pAVCtx->flags2			|= CODEC_FLAG2_SHOW_ALL;
	}

	if (m_pAVCtx->codec_tag == MAKEFOURCC('m','p','g','2')) {
		m_pAVCtx->codec_tag = MAKEFOURCC('M','P','E','G');
	}

	AllocExtradata(m_pAVCtx, pmt);

	ExtractAvgTimePerFrame(&m_pInput->CurrentMediaType(), m_rtAvrTimePerFrame);
	int wout, hout;
	ExtractDim(&m_pInput->CurrentMediaType(), wout, hout, m_nARX, m_nARY);
	UNREFERENCED_PARAMETER(wout);
	UNREFERENCED_PARAMETER(hout);

	m_pAVCtx->using_dxva = IsDXVASupported();

	if (avcodec_open2(m_pAVCtx, m_pAVCodec, NULL) < 0) {
		return VFW_E_INVALIDMEDIATYPE;
	}

	FFGetFrameProps(m_pAVCtx, m_pFrame, m_nOutputWidth, m_nOutputHeight);
	m_PixelFormat = m_pAVCtx->pix_fmt;

	if (IsDXVASupported()) {
		do {
			m_bDXVACompatible = false;

			if (!DXVACheckFramesize(PictWidth(), PictHeight(), m_nPCIVendor, m_nPCIDevice)) { // check frame size
				break;
			}

			if (m_nCodecId == AV_CODEC_ID_H264) {
				if (m_nDXVA_SD && PictWidthRounded() < 1280) { // check "Disable DXVA for SD" option
					break;
				}

				bool IsAtiDXVACompatible = false;
				if (m_nPCIVendor == PCIV_ATI) {
					if (!m_strDeviceDescription.Find(ATI_IDENTIFY) || !m_strDeviceDescription.Find(AMD_IDENTIFY)) {
						m_strDeviceDescription.Delete(0, 4);
						TCHAR ati_version = '0';
						if (!m_strDeviceDescription.Find(RADEON_HD_IDENTIFY)) {
							ati_version = m_strDeviceDescription.GetAt(CString(RADEON_HD_IDENTIFY).GetLength());
						} else if (!m_strDeviceDescription.Find(RADEON_MOBILITY_HD_IDENTIFY)) {
							ati_version = m_strDeviceDescription.GetAt(CString(RADEON_MOBILITY_HD_IDENTIFY).GetLength());
						}
						IsAtiDXVACompatible = (_wtoi(&ati_version) >= 4); // HD4xxx/Mobility and above AMD/ATI cards support level 5.1 and ref = 16
					}
				} else if (m_nPCIVendor == PCIV_Intel && !IsWinVistaOrLater() && m_nPCIDevice == 0x8108) {
					break; // Disable support H.264 DXVA on Intel GMA500 in WinXP
				}
				int nCompat = FFH264CheckCompatibility (PictWidthRounded(), PictHeightRounded(), m_pAVCtx, m_nPCIVendor, m_nPCIDevice, m_VideoDriverVersion, IsAtiDXVACompatible);

				if ((nCompat & DXVA_PROFILE_HIGHER_THAN_HIGH) || (nCompat & DXVA_HIGH_BIT)) { // DXVA unsupported
					break;
				}
					
				if (nCompat) {
					bool bDXVACompatible = true;
					switch (m_nDXVACheckCompatibility) {
						case 0:
							bDXVACompatible = false;
							break;
						case 1:
							bDXVACompatible = (nCompat == DXVA_UNSUPPORTED_LEVEL);
							break;
						case 2:
							bDXVACompatible = (nCompat == DXVA_TOO_MANY_REF_FRAMES);
							break;
					}
					if (!bDXVACompatible) {
						break;
					}
				}
			} else if (m_nCodecId == AV_CODEC_ID_MPEG2VIDEO) {
				if (!MPEG2CheckCompatibility(m_pAVCtx)) {
					break;
				}
			} else if (m_nCodecId == AV_CODEC_ID_WMV3) {
				if (PictWidth() <= 720) { // fixes color problem for some wmv files (profile <= MP@ML)
					break;
				}
			}

			m_bDXVACompatible = true;
		} while (false);

		if (!m_bDXVACompatible) {
			HRESULT hr = ReopenVideo();
			if FAILED(hr) {
				return hr;
			}
		}
	}

	av_frame_unref(m_pFrame);

	BuildOutputFormat();

	if (bReinit) {
		SAFE_DELETE(m_pDXVADecoder);

		if (m_nDecoderMode == MODE_DXVA2) {
			if (m_pDXVA2Allocator && IsDXVASupported() && SUCCEEDED(FindDecoderConfiguration())) {
				RecommitAllocator();
			}
		} else if (m_nDecoderMode == MODE_DXVA1) {
			SAFE_DELETE(m_pDXVADecoder);
			if (IsDXVASupported()) {
				ReconnectOutput(PictWidthRounded(), PictHeightRounded(), true, true, GetDuration(), PictWidth(), PictHeight());
				if (m_pDXVADecoder) {
					m_pDXVADecoder->ConfigureDXVA1();
				}
			}
		}
	}

	return S_OK;
}

void CMPCVideoDecFilter::BuildOutputFormat()
{
	SAFE_DELETE_ARRAY(m_pVideoOutputFormat);

	// === New swscaler options
	int nSwIndex[PixFmt_count] = { 0 };
	int nSwCount = 0;

	if (m_pAVCtx->pix_fmt != AV_PIX_FMT_NONE) {
		const AVPixFmtDescriptor* av_pfdesc = av_pix_fmt_desc_get(m_pAVCtx->pix_fmt);
		int lumabits = av_pfdesc->comp->depth_minus1 + 1;

		const MPCPixelFormat* InOutList = NULL;

		if (av_pfdesc->flags & (AV_PIX_FMT_FLAG_RGB|AV_PIX_FMT_FLAG_PAL)) {
			InOutList = RGB_8;
		} else if (av_pfdesc->nb_components >= 3) {
			if (av_pfdesc->log2_chroma_w == 1 && av_pfdesc->log2_chroma_h == 1) { // 4:2:0
				if (lumabits <= 8) {
					InOutList = YUV420_8;
				} else if (lumabits <= 10) {
					InOutList = YUV420_10;
				} else {
					InOutList = YUV420_16;
				}
			} else if (av_pfdesc->log2_chroma_w == 1 && av_pfdesc->log2_chroma_h == 0) { // 4:2:2
				if (lumabits <= 8) {
					InOutList = YUV422_8;
				} else if (lumabits <= 10) {
					InOutList = YUV422_10;
				} else {
					InOutList = YUV422_16;
				}
			} else if (av_pfdesc->log2_chroma_w == 0 && av_pfdesc->log2_chroma_h == 0) { // 4:4:4
				if (lumabits <= 8) {
					InOutList = YUV444_8;
				} else if (lumabits <= 10) {
					InOutList = YUV444_10;
				} else {
					InOutList = YUV444_16;
				}
			}
		}
		
		if (InOutList == NULL) {
			InOutList = YUV420_8;
		}

		for (int i = 0; i < PixFmt_count; i++) {
			int index = InOutList[i];
			if (m_fPixFmts[index]) {
				nSwIndex[nSwCount++] = index;
			}
		}
	}

	if (!m_fPixFmts[PixFmt_YUY2] || nSwCount == 0) {
		// if YUY2 has not been added yet, then add it to the end of the list
		nSwIndex[nSwCount++] = PixFmt_YUY2;
	}

	m_nVideoOutputCount = m_bUseFFmpeg ? nSwCount : 0;
	if (IsDXVASupported()) {
		if (IsWinVistaOrLater()) {
			m_nVideoOutputCount += _countof(DXVAFormats);
		} else {
			m_nVideoOutputCount += ffCodecs[m_nCodecNb].DXVAModeCount();
		}
	}

	m_pVideoOutputFormat = DNew VIDEO_OUTPUT_FORMATS[m_nVideoOutputCount];

	int nPos = 0;
	if (IsDXVASupported()) {
		if (IsWinVistaOrLater()) {
			// Static list for DXVA2
			memcpy(&m_pVideoOutputFormat[nPos], DXVAFormats, sizeof(DXVAFormats));
			nPos += _countof(DXVAFormats);
		} else {
			// Dynamic DXVA media types for DXVA1
			for (int pos = 0; pos < ffCodecs[m_nCodecNb].DXVAModeCount(); pos++) {
				m_pVideoOutputFormat[nPos + pos].subtype		= ffCodecs[m_nCodecNb].DXVAModes->Decoder[nPos];
				m_pVideoOutputFormat[nPos + pos].biCompression	= FCC('dxva');
				m_pVideoOutputFormat[nPos + pos].biBitCount		= 12;
				m_pVideoOutputFormat[nPos + pos].biPlanes		= 1;
			}
			nPos += ffCodecs[m_nCodecNb].DXVAModeCount();
		}
	}

	// Software rendering
	if (m_bUseFFmpeg) {
		for (int i = 0; i < nSwCount; i++) {
			const SW_OUT_FMT* swof = GetSWOF(nSwIndex[i]);
			m_pVideoOutputFormat[nPos + i].subtype			= swof->subtype;
			m_pVideoOutputFormat[nPos + i].biCompression	= swof->biCompression;
			m_pVideoOutputFormat[nPos + i].biBitCount		= swof->bpp;
			m_pVideoOutputFormat[nPos + i].biPlanes			= 1; // This value must be set to 1.
		}
	}
}

int CMPCVideoDecFilter::GetPicEntryNumber()
{
	if (IsDXVASupported()) {
		return IsWinVistaOrLater()
			? ffCodecs[m_nCodecNb].DXVAModes->PicEntryNumber_DXVA2
			: ffCodecs[m_nCodecNb].DXVAModes->PicEntryNumber_DXVA1;
	} else {
		return 0;
	}
}

void CMPCVideoDecFilter::GetOutputFormats(int& nNumber, VIDEO_OUTPUT_FORMATS** ppFormats)
{
	nNumber		= m_nVideoOutputCount;
	*ppFormats	= m_pVideoOutputFormat;
}

void CMPCVideoDecFilter::AllocExtradata(AVCodecContext* pAVCtx, const CMediaType* pmt)
{
	// code from LAV ...
	// Process Extradata
	BYTE *extra				= NULL;
	unsigned int extralen	= 0;
	getExtraData((const BYTE *)pmt->Format(), pmt->FormatType(), pmt->FormatLength(), NULL, &extralen);

	BOOL bH264avc = FALSE;
	if (extralen > 0) {
		DbgLog((LOG_TRACE, 3, L"CMPCVideoDecFilter::AllocExtradata() : processing extradata of %d bytes", extralen));
		// Reconstruct AVC1 extradata format
		if (pmt->formattype == FORMAT_MPEG2Video && (m_pAVCtx->codec_tag == MAKEFOURCC('a','v','c','1') || m_pAVCtx->codec_tag == MAKEFOURCC('A','V','C','1') || m_pAVCtx->codec_tag == MAKEFOURCC('C','C','V','1'))) {
			MPEG2VIDEOINFO *mp2vi = (MPEG2VIDEOINFO *)pmt->Format();
			extralen += 7;
			extra = (uint8_t *)av_mallocz(extralen + FF_INPUT_BUFFER_PADDING_SIZE);
			extra[0] = 1;
			extra[1] = (BYTE)mp2vi->dwProfile;
			extra[2] = 0;
			extra[3] = (BYTE)mp2vi->dwLevel;
			extra[4] = (BYTE)(mp2vi->dwFlags ? mp2vi->dwFlags : 2) - 1;

			// Actually copy the metadata into our new buffer
			unsigned int actual_len;
			getExtraData((const BYTE *)pmt->Format(), pmt->FormatType(), pmt->FormatLength(), extra+6, &actual_len);

			// Count the number of SPS/PPS in them and set the length
			// We'll put them all into one block and add a second block with 0 elements afterwards
			// The parsing logic does not care what type they are, it just expects 2 blocks.
			BYTE *p = extra+6, *end = extra+6+actual_len;
			BOOL bSPS = FALSE, bPPS = FALSE;
			int count = 0;
			while (p+1 < end) {
				unsigned len = (((unsigned)p[0] << 8) | p[1]) + 2;
				if (p + len > end) {
					break;
				}
				if ((p[2] & 0x1F) == 7)
					bSPS = TRUE;
				if ((p[2] & 0x1F) == 8)
					bPPS = TRUE;
				count++;
				p += len;
			}
			extra[5] = count;
			extra[extralen-1] = 0;

			bH264avc = TRUE;
		} else {
			// Just copy extradata for other formats
			extra = (uint8_t *)av_mallocz(extralen + FF_INPUT_BUFFER_PADDING_SIZE);
			getExtraData((const BYTE *)pmt->Format(), pmt->FormatType(), pmt->FormatLength(), extra, NULL);
		}
		// Hack to discard invalid MP4 metadata with AnnexB style video
		if (m_nCodecId == AV_CODEC_ID_H264 && !bH264avc && extra[0] == 1) {
			av_freep(&extra);
			extralen = 0;
		}
		
		if (m_nCodecId == AV_CODEC_ID_HEVC) {
			// try Reconstruct NAL units sequence into NAL Units in Byte-Stream Format
			BYTE* src		= extra;
			BYTE* src_end	= extra + extralen;
			BYTE* dst		= NULL;
			int dst_len		= 0;
			
			int NALCount	= 0;
			while (src + 2 < src_end) {
				int len = (src[0] << 8 | src[1]);
				if (len == 0) {
					break;
				}
				if ((len <= 2) || (src + len + 2 > src_end)) {
					NALCount = 0;
					break;
				}
				src += 2;
				int nat = (src[0] >> 1) & 0x3F;
				if (nat < NAL_UNIT_VPS || nat > NAL_UNIT_PPS) {
					NALCount = 0;
					break;
				}

				dst = (BYTE *)av_realloc_f(dst, dst_len + len + 3 + FF_INPUT_BUFFER_PADDING_SIZE, 1);
				// put startcode 0x000001
				dst[dst_len]		= 0x00;
				dst[dst_len + 1]	= 0x00;
				dst[dst_len + 2]	= 0x01;
				memcpy(dst + dst_len + 3, src, len);
				
				dst_len += len + 3;

				src += len;
				NALCount++;
			}

			if (NALCount > 1) {
				av_freep(&extra);
				extra		= dst;
				extralen	= dst_len;
			} else {
				av_freep(&dst);
			}
		}

		m_pAVCtx->extradata = extra;
		m_pAVCtx->extradata_size = (int)extralen;
	}
}

HRESULT CMPCVideoDecFilter::CompleteConnect(PIN_DIRECTION direction, IPin* pReceivePin)
{
	if (direction == PINDIR_OUTPUT) {
		DetectVideoCard_EVR(pReceivePin);

		if (IsDXVASupported()) {
			if (m_nDecoderMode == MODE_DXVA1) {
				m_pDXVADecoder->ConfigureDXVA1();
			} else if (SUCCEEDED(ConfigureDXVA2(pReceivePin)) && SUCCEEDED(SetEVRForDXVA2(pReceivePin))) {
				m_nDecoderMode = MODE_DXVA2;
			}
		}
		if (m_nDecoderMode == MODE_SOFTWARE && !m_bUseFFmpeg) {
			return VFW_E_INVALIDMEDIATYPE;
		}

		if (m_nDecoderMode == MODE_SOFTWARE && IsDXVASupported()) {
			HRESULT hr;
			if (FAILED(hr = ReopenVideo())) {
				return hr;
			}

			ChangeOutputMediaFormat(2);
		}

		CLSID ClsidSourceFilter = GetCLSID(m_pInput->GetConnected());
		if ((ClsidSourceFilter == __uuidof(CMpegSourceFilter)) || (ClsidSourceFilter == __uuidof(CMpegSplitterFilter))) {
			m_bReorderBFrame = false;

			if (CComPtr<IBaseFilter> pFilter = GetFilterFromPin(m_pInput->GetConnected())) {
				if (CComQIPtr<IMpegSplitterFilter> MpegSplitterFilter = pFilter) {
					m_bIsEVO = (m_nCodecId == AV_CODEC_ID_VC1 && mpeg_ps == MpegSplitterFilter->GetMPEGType());
				}
			}
		}
	}

	return __super::CompleteConnect (direction, pReceivePin);
}

HRESULT CMPCVideoDecFilter::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	if (UseDXVA2()) {
		HRESULT					hr;
		ALLOCATOR_PROPERTIES	Actual;

		if (m_pInput->IsConnected() == FALSE) {
			return E_UNEXPECTED;
		}

		pProperties->cBuffers = GetPicEntryNumber();

		if (FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) {
			return hr;
		}

		return pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
			   ? E_FAIL
			   : NOERROR;
	} else {
		return __super::DecideBufferSize (pAllocator, pProperties);
	}
}

HRESULT CMPCVideoDecFilter::BeginFlush()
{
	return __super::BeginFlush();
}

HRESULT CMPCVideoDecFilter::EndFlush()
{
	CAutoLock cAutoLock(&m_csReceive);
	return __super::EndFlush();
}

HRESULT CMPCVideoDecFilter::NewSegment(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, double dRate)
{
	CAutoLock cAutoLock(&m_csReceive);
	
	if (m_pAVCtx) {
		avcodec_flush_buffers(m_pAVCtx);
	}

	if (m_pParser) {
		av_parser_close(m_pParser);
		m_pParser = av_parser_init(m_nCodecId);
	}

	if (m_pDXVADecoder) {
		m_pDXVADecoder->Flush();
	}
	
	memset(&m_BFrames, 0, sizeof(m_BFrames));
	m_nPosB = 1;
	m_dRate	= dRate;

	m_bWaitingForKeyFrame = TRUE;

	rm.video_after_seek	= true;

	m_rtStart		= rtStart;
	m_rtStartCache	= INVALID_TIME;
	m_rtPrevStop	= 0;
	m_rtLastStop	= 0;

	if (m_nCodecId == AV_CODEC_ID_H264 && (m_nDecoderMode == MODE_SOFTWARE || (m_nFrameType != PICT_FRAME && m_nPCIVendor == PCIV_ATI))) {
		InitDecoder(&m_pInput->CurrentMediaType());
	}

	return __super::NewSegment(rtStart, rtStop, dRate);
}

HRESULT CMPCVideoDecFilter::EndOfStream()
{
	CAutoLock cAutoLock(&m_csReceive);

	if (m_nDecoderMode == MODE_SOFTWARE) {
		REFERENCE_TIME rtStart = 0, rtStop = 0;
		SoftwareDecode(NULL, NULL, 0, rtStart, rtStop);
	} else if (m_nDecoderMode == MODE_DXVA2 && m_pDXVADecoder && m_pDXVA2Allocator) { // TODO - need check under WinXP on DXVA1
		m_pDXVADecoder->EndOfStream();
	}

	return __super::EndOfStream();
}

HRESULT CMPCVideoDecFilter::BreakConnect(PIN_DIRECTION dir)
{
	if (dir == PINDIR_INPUT) {
		Cleanup();
	}

	return __super::BreakConnect (dir);
}

void CMPCVideoDecFilter::SetTypeSpecificFlags(IMediaSample* pMS)
{
	if (CComQIPtr<IMediaSample2> pMS2 = pMS) {
		AM_SAMPLE2_PROPERTIES props;
		if (SUCCEEDED(pMS2->GetProperties(sizeof(props), (BYTE*)&props))) {
			props.dwTypeSpecificFlags &= ~0x7f;

			switch (m_nDeinterlacing) {
				case AUTO :
					m_nFrameType = PICT_BOTTOM_FIELD;
					if (!m_pFrame->interlaced_frame) {
						props.dwTypeSpecificFlags		|= AM_VIDEO_FLAG_WEAVE;
						m_nFrameType					= PICT_FRAME;
					} else {
						if (m_pFrame->top_field_first) {
							props.dwTypeSpecificFlags	|= AM_VIDEO_FLAG_FIELD1FIRST;
							m_nFrameType				= PICT_TOP_FIELD;
						}
					}
					break;
				case PROGRESSIVE :
					props.dwTypeSpecificFlags	|= AM_VIDEO_FLAG_WEAVE;
					m_nFrameType				= PICT_FRAME;
					break;
				case TOPFIELD :
					props.dwTypeSpecificFlags	|= AM_VIDEO_FLAG_FIELD1FIRST;
					m_nFrameType				= PICT_TOP_FIELD;
					break;
				case BOTTOMFIELD :
					m_nFrameType = PICT_BOTTOM_FIELD;
			}

			switch (m_pFrame->pict_type) {
				case AV_PICTURE_TYPE_I :
				case AV_PICTURE_TYPE_SI :
					props.dwTypeSpecificFlags	|= AM_VIDEO_FLAG_I_SAMPLE;
					break;
				case AV_PICTURE_TYPE_P :
				case AV_PICTURE_TYPE_SP :
					props.dwTypeSpecificFlags	|= AM_VIDEO_FLAG_P_SAMPLE;
					break;
				default :
					props.dwTypeSpecificFlags	|= AM_VIDEO_FLAG_B_SAMPLE;
					break;
			}

			pMS2->SetProperties(sizeof(props), (BYTE*)&props);
		}
	}
}

#define RM_SKIP_BITS(n)	(buffer<<=n)
#define RM_SHOW_BITS(n)	((buffer)>>(32-(n)))
static int rm_fix_timestamp(uint8_t *buf, int64_t timestamp, enum AVCodecID nCodecId, int64_t *kf_base, int *kf_pts)
{
	uint8_t *s = buf + 1 + (*buf+1)*8;
	uint32_t buffer = (s[0]<<24) + (s[1]<<16) + (s[2]<<8) + s[3];
	uint32_t kf = timestamp;
	int pict_type;
	uint32_t orig_kf;

	if (nCodecId == AV_CODEC_ID_RV30) {
		RM_SKIP_BITS(3);
		pict_type = RM_SHOW_BITS(2);
		RM_SKIP_BITS(2 + 7);
	} else {
		RM_SKIP_BITS(1);
		pict_type = RM_SHOW_BITS(2);
		RM_SKIP_BITS(2 + 7 + 3);
	}
	orig_kf = kf = RM_SHOW_BITS(13); // kf= 2*RM_SHOW_BITS(12);
	if (pict_type <= 1) {
		// I frame, sync timestamps:
		*kf_base = (int64_t)timestamp-kf;
		kf = timestamp;
	} else {
		// P/B frame, merge timestamps:
		int64_t tmp = (int64_t)timestamp - *kf_base;
		kf |= tmp&(~0x1fff); // combine with packet timestamp
		if (kf<tmp-4096) {
			kf += 8192;
		} else if (kf>tmp+4096) { // workaround wrap-around problems
			kf -= 8192;
		}
		kf += *kf_base;
	}
	if (pict_type != 3) { // P || I  frame -> swap timestamps
		uint32_t tmp=kf;
		kf = *kf_pts;
		*kf_pts = tmp;
	}

	return kf;
}

static int64_t process_rv_timestamp(RMDemuxContext *rm, enum AVCodecID nCodecId, uint8_t *buf, int64_t timestamp)
{
	if (rm->video_after_seek) {
		rm->kf_base	= 0;
		rm->kf_pts	= timestamp;
		rm->video_after_seek = false;
	}
	return rm_fix_timestamp(buf, timestamp, nCodecId, &rm->kf_base, &rm->kf_pts);
}

#define PULLDOWN_FLAG (m_nCodecId == AV_CODEC_ID_VC1 && m_bIsEVO && m_rtAvrTimePerFrame == 333666)
HRESULT CMPCVideoDecFilter::SoftwareDecode(IMediaSample* pIn, BYTE* pDataIn, int nSize, REFERENCE_TIME& rtStartIn, REFERENCE_TIME& rtStopIn)
{
	HRESULT			hr = S_OK;
	int				got_picture;
	int				used_bytes;
	BOOL			bFlush = (pDataIn == NULL);

	AVPacket		avpkt;
	av_init_packet(&avpkt);

	while (nSize > 0 || bFlush) {
		REFERENCE_TIME rtStart = rtStartIn, rtStop = rtStopIn;
		
		if (!bFlush) {
			if (nSize + FF_INPUT_BUFFER_PADDING_SIZE > m_nFFBufferSize) {
				m_nFFBufferSize	= nSize + FF_INPUT_BUFFER_PADDING_SIZE;
				m_pFFBuffer		= (BYTE*)av_realloc_f(m_pFFBuffer, m_nFFBufferSize, 1);
				if (!m_pFFBuffer) {
					m_nFFBufferSize = 0;
					return E_OUTOFMEMORY;
				}
			}

			// Required number of additionally allocated bytes at the end of the input bitstream for decoding.
			// This is mainly needed because some optimized bitstream readers read
			// 32 or 64 bit at once and could read over the end.
			// Note: If the first 23 bits of the additional bytes are not 0, then damaged
			// MPEG bitstreams could cause overread and segfault.
			memcpy_sse(m_pFFBuffer, pDataIn, nSize);
			memset(m_pFFBuffer + nSize, 0, FF_INPUT_BUFFER_PADDING_SIZE);

			avpkt.data	= m_pFFBuffer;
			avpkt.size	= nSize;
			avpkt.pts	= rtStartIn;
			avpkt.flags	= AV_PKT_FLAG_KEY;
		} else {
			avpkt.data	= NULL;
			avpkt.size	= 0;
		}

		// all Parser code from LAV ... thanks to it's author
		if (m_pParser) {
			BYTE *pOut		= NULL;
			int pOut_size	= 0;

			used_bytes = av_parser_parse2(m_pParser, m_pAVCtx, &pOut, &pOut_size, avpkt.data, avpkt.size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

			if (used_bytes == 0 && pOut_size == 0 && !bFlush) {
				DbgLog((LOG_TRACE, 3, L"CMPCVideoDecFilter::SoftwareDecode() - could not process buffer, starving?"));
				break;
			} else if (used_bytes > 0) {
				nSize	-= used_bytes;
				pDataIn	+= used_bytes;
			}

			// Update start time cache
			// If more data was read then output, update the cache (incomplete frame)
			// If output is bigger, a frame was completed, update the actual rtStart with the cached value, and then overwrite the cache
			if (used_bytes > pOut_size) {
				if (rtStartIn != INVALID_TIME) {
					m_rtStartCache = rtStartIn;
				}
			} else if (used_bytes == pOut_size || ((used_bytes + 9) == pOut_size)) {
				// Why +9 above?
				// Well, apparently there are some broken MKV muxers that like to mux the MPEG-2 PICTURE_START_CODE block (which is 9 bytes) in the package with the previous frame
				// This would cause the frame timestamps to be delayed by one frame exactly, and cause timestamp reordering to go wrong.
				// So instead of failing on those samples, lets just assume that 9 bytes are that case exactly.
				m_rtStartCache = rtStartIn = INVALID_TIME;
			} else if (pOut_size > used_bytes) {
				rtStart			= m_rtStartCache;
				m_rtStartCache	= rtStartIn;
				// The value was used once, don't use it for multiple frames, that ends up in weird timings
				rtStartIn		= INVALID_TIME;
			}

			if (pOut_size > 0 || bFlush) {

				if (pOut && pOut_size > 0) {
					if (pOut_size + FF_INPUT_BUFFER_PADDING_SIZE > m_nFFBufferSize2) {
						m_nFFBufferSize2	= pOut_size + FF_INPUT_BUFFER_PADDING_SIZE;
						m_pFFBuffer2		= (BYTE *)av_realloc_f(m_pFFBuffer2, m_nFFBufferSize2, 1);
						if (!m_pFFBuffer2) {
							m_nFFBufferSize2 = 0;
							return E_OUTOFMEMORY;
						}
					}
					memcpy(m_pFFBuffer2, pOut, pOut_size);
					memset(m_pFFBuffer2 + pOut_size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

					avpkt.data		= m_pFFBuffer2;
					avpkt.size		= pOut_size;
					avpkt.pts		= rtStart;
					avpkt.duration	= 0;
				} else {
					avpkt.data		= NULL;
					avpkt.size		= 0;
				}

				if (m_nDecoderMode != MODE_SOFTWARE) {
					// use ffmpeg's parser for DXVA decoder - H.264 AnnexB format
					hr = m_pDXVADecoder->DecodeFrame(avpkt.data, avpkt.size, avpkt.pts, avpkt.pts + 1);
					av_frame_unref(m_pFrame);
					continue;
				}

				int ret2 = avcodec_decode_video2(m_pAVCtx, m_pFrame, &got_picture, &avpkt);
				if (ret2 < 0) {
					DbgLog((LOG_TRACE, 3, L"CMPCVideoDecFilter::SoftwareDecode() - decoding failed despite successfull parsing"));
					got_picture = 0;
				}
			} else {
				got_picture = 0;
			}
		} else {
			used_bytes	= avcodec_decode_video2(m_pAVCtx, m_pFrame, &got_picture, &avpkt);
			nSize		= 0;
		}

		if (used_bytes < 0) {
			av_frame_unref(m_pFrame);
			return S_OK;
		}

		if (m_bWaitKeyFrame) {
			if (m_bWaitingForKeyFrame && got_picture) {
				if (m_pFrame->key_frame) {
					m_bWaitingForKeyFrame = FALSE;
				} else {
					got_picture = 0;
				}
			}
		}

		if (!got_picture || !m_pFrame->data[0]) {
			if (!avpkt.size) {
				bFlush = FALSE; // End flushing, no more frames
			}

			av_frame_unref(m_pFrame);
			continue;
		}

		if ((m_nCodecId == AV_CODEC_ID_RV10 || m_nCodecId == AV_CODEC_ID_RV20) && m_pFrame->pict_type == AV_PICTURE_TYPE_B) {
			rtStart = m_rtPrevStop;
		} else if ((m_nCodecId == AV_CODEC_ID_RV30 || m_nCodecId == AV_CODEC_ID_RV40) && avpkt.data) {
			rtStart = m_pFrame->pkt_pts;
			rtStart = (rtStart == INVALID_TIME) ? m_rtPrevStop : (10000i64*process_rv_timestamp(&rm, m_nCodecId, avpkt.data, (rtStart + m_rtStart)/10000i64) - m_rtStart);
		} else if (!PULLDOWN_FLAG) {
			rtStart = m_pFrame->pkt_pts;
		}

		ReorderBFrames(rtStart, rtStop);

		UpdateFrameTime(rtStart, rtStop, PULLDOWN_FLAG);

		m_rtPrevStop = rtStop;

		if ((pIn && pIn->IsPreroll() == S_OK) || rtStart < 0) {
			av_frame_unref(m_pFrame);
			continue;
		}

		if (m_PixelFormat != m_pFrame->format) {
			ChangeOutputMediaFormat(2);
		}
		m_PixelFormat = (AVPixelFormat)m_pFrame->format;

		CComPtr<IMediaSample>	pOut;
		BYTE*					pDataOut = NULL;

		UpdateAspectRatio();
		if (FAILED(hr = GetDeliveryBuffer(m_pAVCtx->width, m_pAVCtx->height, &pOut, GetDuration())) || FAILED(hr = pOut->GetPointer(&pDataOut))) {
			av_frame_unref(m_pFrame);
			continue;
		}

		pOut->SetTime(&rtStart, &rtStop);
		pOut->SetMediaTime(NULL, NULL);

		// Check alignment on rawvideo, which can be off depending on the source file
		AVFrame* pTmpFrame = NULL;
		if (m_nCodecId == AV_CODEC_ID_RAWVIDEO) {
			for (size_t i = 0; i < 4; i++) {
				if ((intptr_t)m_pFrame->data[i] % 16u || m_pFrame->linesize[i] % 16u) {
					// copy the frame, its not aligned properly and would crash later
					pTmpFrame = av_frame_alloc();
					pTmpFrame->format = m_pFrame->format;
					pTmpFrame->width  = m_pFrame->width;
					pTmpFrame->height = m_pFrame->height;
					pTmpFrame->colorspace  = m_pFrame->colorspace;
					pTmpFrame->color_range = m_pFrame->color_range;
					av_frame_get_buffer(pTmpFrame, 16);
					av_image_copy(pTmpFrame->data, pTmpFrame->linesize, (const uint8_t**)m_pFrame->data, m_pFrame->linesize, (AVPixelFormat)m_pFrame->format, m_pFrame->width, m_pFrame->height);
					break;
				}
			}
		}
		if (pTmpFrame) {
			m_FormatConverter.Converting(pDataOut, pTmpFrame);
			av_frame_free(&pTmpFrame);
		} else {
			m_FormatConverter.Converting(pDataOut, m_pFrame);
		}

#if defined(_DEBUG) && 0
		static REFERENCE_TIME	rtLast = 0;
		TRACE ("Deliver : %10I64d - %10I64d   (%10I64d)  {%10I64d}\n", rtStart, rtStop,
			   rtStop - rtStart, rtStart - rtLast);
		rtLast = rtStart;
#endif

		SetTypeSpecificFlags(pOut);
		av_frame_unref(m_pFrame);

		hr = m_pOutput->Deliver(pOut);
	}

	return hr;
}

// change colorspace details/output media format
// 1 - change swscaler colorspace details
// 2 - change output media format
HRESULT CMPCVideoDecFilter::ChangeOutputMediaFormat(int nType)
{
	HRESULT hr = S_OK;

	if (!m_pOutput || !m_pOutput->IsConnected()) {
		return hr;
	}

	// change swscaler colorspace details
	if (nType >= 1) {
		m_FormatConverter.SetOptions(m_nSwPreset, m_nSwStandard, m_nSwRGBLevels);
	}

	// change output media format
	if (nType == 2) {
		CAutoLock cObjectLock(m_pLock);
		BuildOutputFormat();

		CComQIPtr<IPin> pPin = m_pOutput->GetConnected();
		CComQIPtr<IBaseFilter> pBF = GetFilterFromPin(pPin);
		if (IsVideoRenderer(pBF)) {
			hr = NotifyEvent(EC_DISPLAY_CHANGED, (LONG_PTR)(IPin*)pPin, NULL);
			if (S_OK != hr) {
				hr = E_FAIL;
			}
		} else {
			int nNumber;
			VIDEO_OUTPUT_FORMATS* pFormats;
			GetOutputFormats(nNumber, &pFormats);
			for (int i = 0; i < nNumber * 2; i++) {
				CMediaType mt;
				if (SUCCEEDED(GetMediaType(i, &mt))) {
					hr = pPin->QueryAccept(&mt);
					if (hr == S_OK) {
						hr = ReconnectPin(pPin, &mt);
						if (hr == S_OK) {
							return hr;
						}
					}
				}
			}

			return E_FAIL;
		}
	}

	return hr;
}

// reopen video codec - reset the threads count and dxva flag
HRESULT CMPCVideoDecFilter::ReopenVideo()
{
	if (m_pAVCtx) {
		m_bUseDXVA = false;
		avcodec_close(m_pAVCtx);
		m_pAVCtx->using_dxva = false;
		if (m_nCodecId == AV_CODEC_ID_H264) {
			m_pAVCtx->flags2 &= ~CODEC_FLAG2_SHOW_ALL;
		}
		int nThreadNumber = m_nThreadNumber ? m_nThreadNumber : m_pCpuId->GetProcessorNumber() * 3/2;
		m_pAVCtx->thread_count = max(1, min(m_nCodecId == AV_CODEC_ID_MPEG4 ? 1 : nThreadNumber, MAX_AUTO_THREADS));
		
		if (avcodec_open2(m_pAVCtx, m_pAVCodec, NULL) < 0) {
			return VFW_E_INVALIDMEDIATYPE;
		}
	}

	return S_OK;
}

HRESULT CMPCVideoDecFilter::Transform(IMediaSample* pIn)
{
	CAutoLock cAutoLock(&m_csReceive);
	HRESULT			hr;
	BYTE*			pDataIn;
	int				nSize;
	REFERENCE_TIME	rtStart	= INVALID_TIME;
	REFERENCE_TIME	rtStop	= INVALID_TIME;

	if (FAILED(hr = pIn->GetPointer(&pDataIn))) {
		return hr;
	}

	nSize = pIn->GetActualDataLength();
	// Skip empty packet
	if (nSize == 0) {
		return S_OK;
	}

	hr = pIn->GetTime(&rtStart, &rtStop);

	if (FAILED(hr)) {
		rtStart = rtStop = INVALID_TIME;
	}

	if (m_pAVCtx->has_b_frames) {
		m_BFrames[m_nPosB].rtStart	= rtStart;
		m_BFrames[m_nPosB].rtStop	= rtStop;
		m_nPosB						= 1 - m_nPosB;
	}

	switch (m_nDecoderMode) {
		case MODE_SOFTWARE :
			hr = SoftwareDecode(pIn, pDataIn, nSize, rtStart, rtStop);
			break;
		case MODE_DXVA2 :
			CheckPointer(m_pDXVA2Allocator, E_UNEXPECTED);
		case MODE_DXVA1 :
			{
				CheckPointer(m_pDXVADecoder, E_UNEXPECTED);
				UpdateAspectRatio();

				// Change aspect ratio for DXVA1
				// stupid DXVA1 - size for the output MediaType should be the same that size of DXVA surface
				if (m_nDecoderMode == MODE_DXVA1 && ReconnectOutput(PictWidthRounded(), PictHeightRounded(), true, false, GetDuration(), PictWidth(), PictHeight()) == S_OK) {
					m_pDXVADecoder->ConfigureDXVA1();
				}

				int nWidth	= PictWidthRounded();
				int nHeight	= PictHeightRounded();

				if (m_nCodecId == AV_CODEC_ID_H264 && m_pParser) {
					hr = SoftwareDecode(pIn, pDataIn, nSize, rtStart, rtStop);
				} else {
					hr = m_pDXVADecoder->DecodeFrame(pDataIn, nSize, rtStart, rtStop);
					av_frame_unref(m_pFrame);
				}

				if (nWidth != PictWidthRounded() || nHeight != PictHeightRounded()) {
					FindDecoderConfiguration();
					RecommitAllocator();
					ReconnectOutput(PictWidth(), PictHeight());
				}
			}
			break;
		default :
			ASSERT(FALSE);
			hr = E_UNEXPECTED;
	}

	return hr;
}

void CMPCVideoDecFilter::UpdateAspectRatio()
{
	if (m_nARMode) {
		bool bSetAR = true;
		if (m_nARMode == 2) {
			CMediaType& mt = m_pInput->CurrentMediaType();
			if (mt.formattype == FORMAT_VideoInfo2 || mt.formattype == FORMAT_MPEG2_VIDEO || mt.formattype == FORMAT_DiracVideoInfo) {
				VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)mt.pbFormat;
				bSetAR = (!vih2->dwPictAspectRatioX && !vih2->dwPictAspectRatioY);
			}
			if (!bSetAR && (m_nARX && m_nARY)) {
				CSize aspect(m_nARX, m_nARY);
				ReduceDim(aspect);
				SetAspect(aspect);
			}
		}

		if (bSetAR) {
			if (m_pAVCtx && (m_pAVCtx->sample_aspect_ratio.num > 0) && (m_pAVCtx->sample_aspect_ratio.den > 0)) {
				CSize aspect(m_pAVCtx->sample_aspect_ratio.num * m_pAVCtx->width, m_pAVCtx->sample_aspect_ratio.den * m_pAVCtx->height);
				ReduceDim(aspect);
				SetAspect(aspect);
			}
		}
	} else if (m_nARX && m_nARY) {
		CSize aspect(m_nARX, m_nARY);
		ReduceDim(aspect);
		SetAspect(aspect);
	}
}

void CMPCVideoDecFilter::ReorderBFrames(REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
	// Re-order B-frames if needed
	if (m_pAVCtx->has_b_frames && m_bReorderBFrame) {
		rtStart	= m_BFrames[m_nPosB].rtStart;
		rtStop	= m_BFrames[m_nPosB].rtStop;
	}
}

void CMPCVideoDecFilter::FillInVideoDescription(DXVA2_VideoDesc *pDesc)
{
	memset(pDesc, 0, sizeof(DXVA2_VideoDesc));
	pDesc->SampleWidth			= PictWidthRounded();
	pDesc->SampleHeight			= PictHeightRounded();
	pDesc->Format				= D3DFMT_A8R8G8B8;
	pDesc->UABProtectionLevel	= 1;
}

BOOL CMPCVideoDecFilter::IsSupportedDecoderMode(const GUID* mode)
{
	const CString dxvaMode = GetDXVAMode(mode);
	DbgLog((LOG_TRACE, 3, L"CMPCVideoDecFilter::IsSupportedDecoderMode() : %s", dxvaMode));

	if (IsDXVASupported()) {
		for (int i = 0; i < MAX_SUPPORTED_MODE; i++) {
			if (*ffCodecs[m_nCodecNb].DXVAModes->Decoder[i] == GUID_NULL) {
				break;
			} else if (*ffCodecs[m_nCodecNb].DXVAModes->Decoder[i] == *mode) {
				DbgLog((LOG_TRACE, 3, L"	=> Found : %s", dxvaMode));
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CMPCVideoDecFilter::IsSupportedDecoderConfig(const D3DFORMAT nD3DFormat, const DXVA2_ConfigPictureDecode& config, bool& bIsPrefered)
{
	bIsPrefered = (config.ConfigBitstreamRaw == ffCodecs[m_nCodecNb].DXVAModes->PreferedConfigBitstream);
	return (nD3DFormat == MAKEFOURCC('N', 'V', '1', '2') || nD3DFormat == MAKEFOURCC('I', 'M', 'C', '3'));
}

HRESULT CMPCVideoDecFilter::FindDXVA2DecoderConfiguration(IDirectXVideoDecoderService *pDecoderService,
														  const GUID& guidDecoder,
														  DXVA2_ConfigPictureDecode *pSelectedConfig,
														  BOOL *pbFoundDXVA2Configuration)
{
	HRESULT hr = S_OK;
	UINT cFormats = 0;
	UINT cConfigurations = 0;
	bool bIsPrefered = false;

	D3DFORMAT                   *pFormats = NULL;			// size = cFormats
	DXVA2_ConfigPictureDecode   *pConfig = NULL;			// size = cConfigurations

	// Find the valid render target formats for this decoder GUID.
	hr = pDecoderService->GetDecoderRenderTargets(guidDecoder, &cFormats, &pFormats);

	if (SUCCEEDED(hr)) {
		// Look for a format that matches our output format.
		for (UINT iFormat = 0; iFormat < cFormats;  iFormat++) {

			// Fill in the video description. Set the width, height, format, and frame rate.
			FillInVideoDescription(&m_VideoDesc); // Private helper function.
			m_VideoDesc.Format = pFormats[iFormat];

			// Get the available configurations.
			hr = pDecoderService->GetDecoderConfigurations(guidDecoder, &m_VideoDesc, NULL, &cConfigurations, &pConfig);

			if (FAILED(hr)) {
				continue;
			}

			// Find a supported configuration.
			for (UINT iConfig = 0; iConfig < cConfigurations; iConfig++) {
				if (IsSupportedDecoderConfig(pFormats[iFormat], pConfig[iConfig], bIsPrefered)) {
					// This configuration is good.
					if (bIsPrefered || !*pbFoundDXVA2Configuration) {
						*pbFoundDXVA2Configuration = TRUE;
						*pSelectedConfig = pConfig[iConfig];
					}

					if (bIsPrefered) {
						break;
					}
				}
			}

			CoTaskMemFree(pConfig);
		} // End of formats loop.
	}

	CoTaskMemFree(pFormats);

	// Note: It is possible to return S_OK without finding a configuration.
	return hr;
}

HRESULT CMPCVideoDecFilter::ConfigureDXVA2(IPin *pPin)
{
	HRESULT hr = S_OK;

	CComPtr<IMFGetService>					pGetService;
	CComPtr<IDirect3DDeviceManager9>		pDeviceManager;
	CComPtr<IDirectXVideoDecoderService>	pDecoderService;
	HANDLE									hDevice = INVALID_HANDLE_VALUE;

	// Query the pin for IMFGetService.
	hr = pPin->QueryInterface(__uuidof(IMFGetService), (void**)&pGetService);

	// Get the Direct3D device manager.
	if (SUCCEEDED(hr)) {
		hr = pGetService->GetService(
				 MR_VIDEO_ACCELERATION_SERVICE,
				 __uuidof(IDirect3DDeviceManager9),
				 (void**)&pDeviceManager);
	}

	// Open a new device handle.
	if (SUCCEEDED(hr)) {
		hr = pDeviceManager->OpenDeviceHandle(&hDevice);
	}

	// Get the video decoder service.
	if (SUCCEEDED(hr)) {
		hr = pDeviceManager->GetVideoService(
				 hDevice,
				 __uuidof(IDirectXVideoDecoderService),
				 (void**)&pDecoderService);
	}

	if (SUCCEEDED(hr)) {
		m_pDeviceManager	= pDeviceManager;
		m_pDecoderService	= pDecoderService;
		m_hDevice			= hDevice;
		hr					= FindDecoderConfiguration();
	}

	if (FAILED(hr)) {
		if (hDevice != INVALID_HANDLE_VALUE) {
			pDeviceManager->CloseDeviceHandle(hDevice);
		}
		m_pDeviceManager		= NULL;
		m_pDecoderService		= NULL;
		m_hDevice				= INVALID_HANDLE_VALUE;
	}

	return hr;
}

HRESULT CMPCVideoDecFilter::SetEVRForDXVA2(IPin *pPin)
{
    IMFGetService* pGetService;
    HRESULT hr = pPin->QueryInterface(__uuidof(IMFGetService), reinterpret_cast<void**>(&pGetService));
	if (SUCCEEDED(hr)) {
		IDirectXVideoMemoryConfiguration* pVideoConfig;
		hr = pGetService->GetService(MR_VIDEO_ACCELERATION_SERVICE, IID_IDirectXVideoMemoryConfiguration, reinterpret_cast<void**>(&pVideoConfig));
		if (SUCCEEDED(hr)) {
			// Notify the EVR.
			DXVA2_SurfaceType surfaceType;
			DWORD dwTypeIndex = 0;
			for (;;) {
				hr = pVideoConfig->GetAvailableSurfaceTypeByIndex(dwTypeIndex, &surfaceType);
				if (FAILED(hr)) {
					break;
				}
				if (surfaceType == DXVA2_SurfaceType_DecoderRenderTarget) {
					hr = pVideoConfig->SetSurfaceType(DXVA2_SurfaceType_DecoderRenderTarget);
					break;
				}
				++dwTypeIndex;
			}
			pVideoConfig->Release();
		}
		pGetService->Release();
    }
	return hr;
}

HRESULT CMPCVideoDecFilter::CreateDXVA2Decoder(UINT nNumRenderTargets, IDirect3DSurface9** pDecoderRenderTargets)
{
	DbgLog((LOG_TRACE, 3, L"CMPCVideoDecFilter::CreateDXVA2Decoder()"));

	HRESULT							hr;
	CComPtr<IDirectXVideoDecoder>	pDirectXVideoDec;

	m_pDecoderRenderTarget = NULL;

	if (m_pDXVADecoder) {
		m_pDXVADecoder->SetDirectXVideoDec(NULL);
	}

	hr = m_pDecoderService->CreateVideoDecoder(m_DXVADecoderGUID, &m_VideoDesc, &m_DXVA2Config,
			pDecoderRenderTargets, nNumRenderTargets, &pDirectXVideoDec);

	if (SUCCEEDED(hr)) {
		// need recreate dxva decoder after "stop" on Intel HD Graphics
		SAFE_DELETE(m_pDXVADecoder);
		m_pDXVADecoder = CDXVADecoder::CreateDecoder(this, pDirectXVideoDec, &m_DXVADecoderGUID, GetPicEntryNumber(), &m_DXVA2Config);
		if (m_pDXVADecoder) {
			m_pDXVADecoder->SetDirectXVideoDec(pDirectXVideoDec);
		} else {
			hr = E_FAIL;
		}
	}

	return hr;
}

HRESULT CMPCVideoDecFilter::InitAllocator(IMemAllocator **ppAlloc)
{
	HRESULT hr = S_FALSE;
	m_pDXVA2Allocator = DNew CVideoDecDXVAAllocator(this, &hr);
	if (!m_pDXVA2Allocator) {
		return E_OUTOFMEMORY;
	}
	if (FAILED(hr)) {
		SAFE_DELETE(m_pDXVA2Allocator);
		return hr;
	}
	
	// Return the IMemAllocator interface.
	return m_pDXVA2Allocator->QueryInterface(__uuidof(IMemAllocator), (void **)ppAlloc);
}

HRESULT CMPCVideoDecFilter::RecommitAllocator()
{
	HRESULT hr = S_OK;

	if (m_pDXVA2Allocator) {

		hr = m_pDXVA2Allocator->Decommit();
		if (m_pDXVA2Allocator->DecommitInProgress()) {
			DbgLog((LOG_TRACE, 3, L"CVideoDecOutputPin::Recommit() : WARNING! DXVA2 Allocator is still busy, trying to flush downstream"));
			if (m_pDXVA2Allocator) {
				m_pDXVADecoder->EndOfStream();
			}
			if (m_pDXVA2Allocator->DecommitInProgress()) {
				DbgLog((LOG_TRACE, 3, L"CVideoDecOutputPin::Recommit() : WARNING! Flush had no effect, decommit of the allocator still not complete"));
			} else {
				DbgLog((LOG_TRACE, 3, L"CVideoDecOutputPin::Recommit() : Flush was successfull, decommit completed!"));
			}
		}
		hr = m_pDXVA2Allocator->Commit();
	}

	return hr;
}

HRESULT CMPCVideoDecFilter::FindDXVA1DecoderConfiguration(IAMVideoAccelerator* pAMVideoAccelerator, const GUID* guidDecoder, DDPIXELFORMAT* pPixelFormat)
{
	HRESULT			hr				= E_FAIL;
	DWORD			dwFormats		= 0;
	DDPIXELFORMAT*	pPixelFormats	= NULL;


	pAMVideoAccelerator->GetUncompFormatsSupported(guidDecoder, &dwFormats, NULL);
	if (dwFormats > 0) {
		// Find the valid render target formats for this decoder GUID.
		pPixelFormats = DNew DDPIXELFORMAT[dwFormats];
		hr = pAMVideoAccelerator->GetUncompFormatsSupported(guidDecoder, &dwFormats, pPixelFormats);
		if (SUCCEEDED(hr)) {
			// Look for a format that matches our output format.
			for (DWORD iFormat = 0; iFormat < dwFormats; iFormat++) {
				if (pPixelFormats[iFormat].dwFourCC == MAKEFOURCC('N', 'V', '1', '2')) {
					memcpy(pPixelFormat, &pPixelFormats[iFormat], sizeof(DDPIXELFORMAT));
					SAFE_DELETE_ARRAY(pPixelFormats)
					return S_OK;
				}
			}

			SAFE_DELETE_ARRAY(pPixelFormats);
			hr = E_FAIL;
		}
	}

	return hr;
}

void CMPCVideoDecFilter::SetDXVA1Params(const GUID* pGuid, DDPIXELFORMAT* pPixelFormat)
{
	m_DXVADecoderGUID = *pGuid;
	memcpy(&m_DDPixelFormat, pPixelFormat, sizeof(DDPIXELFORMAT));
}

WORD CMPCVideoDecFilter::GetDXVA1RestrictedMode()
{
	if (m_nCodecNb != -1) {
		for (int i=0; i<MAX_SUPPORTED_MODE; i++)
			if (*ffCodecs[m_nCodecNb].DXVAModes->Decoder[i] == m_DXVADecoderGUID) {
				return ffCodecs[m_nCodecNb].DXVAModes->RestrictedMode [i];
			}
	}

	return DXVA_RESTRICTED_MODE_UNRESTRICTED;
}

HRESULT CMPCVideoDecFilter::CreateDXVA1Decoder(IAMVideoAccelerator* pAMVideoAccelerator, const GUID* pDecoderGuid, DWORD dwSurfaceCount)
{
	SAFE_DELETE(m_pDXVADecoder);

	if (!m_bUseDXVA) {
		return E_FAIL;
	}

	m_nDecoderMode		= MODE_DXVA1;
	m_DXVADecoderGUID	= *pDecoderGuid;
	m_pDXVADecoder		= CDXVADecoder::CreateDecoder(this, pAMVideoAccelerator, &m_DXVADecoderGUID, dwSurfaceCount);

	return S_OK;
}

// ISpecifyPropertyPages2

STDMETHODIMP CMPCVideoDecFilter::GetPages(CAUUID* pPages)
{
	CheckPointer(pPages, E_POINTER);

#ifdef REGISTER_FILTER
	pPages->cElems		= 2;
#else
	pPages->cElems		= 1;
#endif
	pPages->pElems		= (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
	pPages->pElems[0]	= __uuidof(CMPCVideoDecSettingsWnd);
#ifdef REGISTER_FILTER
	pPages->pElems[1]	= __uuidof(CMPCVideoDecCodecWnd);
#endif

	return S_OK;
}

STDMETHODIMP CMPCVideoDecFilter::CreatePage(const GUID& guid, IPropertyPage** ppPage)
{
	CheckPointer(ppPage, E_POINTER);

	if (*ppPage != NULL) {
		return E_INVALIDARG;
	}

	HRESULT hr;

	if (guid == __uuidof(CMPCVideoDecSettingsWnd)) {
		(*ppPage = DNew CInternalPropertyPageTempl<CMPCVideoDecSettingsWnd>(NULL, &hr))->AddRef();
	}
#ifdef REGISTER_FILTER
	else if (guid == __uuidof(CMPCVideoDecCodecWnd)) {
		(*ppPage = DNew CInternalPropertyPageTempl<CMPCVideoDecCodecWnd>(NULL, &hr))->AddRef();
	}
#endif

	return *ppPage ? S_OK : E_FAIL;
}

// EVR functions
HRESULT CMPCVideoDecFilter::DetectVideoCard_EVR(IPin *pPin)
{
	IMFGetService* pGetService;
	HRESULT hr = pPin->QueryInterface(__uuidof(IMFGetService), reinterpret_cast<void**>(&pGetService));
	if (SUCCEEDED(hr)) {
		// Try to get the adapter description of the active DirectX 9 device.
		IDirect3DDeviceManager9* pDevMan9;
		hr = pGetService->GetService(MR_VIDEO_ACCELERATION_SERVICE, IID_IDirect3DDeviceManager9, reinterpret_cast<void**>(&pDevMan9));
		if (SUCCEEDED(hr)) {
			HANDLE hDevice;
			hr = pDevMan9->OpenDeviceHandle(&hDevice);
			if (SUCCEEDED(hr)) {
				IDirect3DDevice9* pD3DDev9;
				hr = pDevMan9->LockDevice(hDevice, &pD3DDev9, TRUE);
				if (hr == DXVA2_E_NEW_VIDEO_DEVICE) {
					// Invalid device handle. Try to open a new device handle.
					hr = pDevMan9->CloseDeviceHandle(hDevice);
					if (SUCCEEDED(hr)) {
						hr = pDevMan9->OpenDeviceHandle(&hDevice);
						// Try to lock the device again.
						if (SUCCEEDED(hr)) {
							hr = pDevMan9->LockDevice(hDevice, &pD3DDev9, TRUE);
						}
					}
				}
				if (SUCCEEDED(hr)) {
					D3DDEVICE_CREATION_PARAMETERS DevPar9;
					hr = pD3DDev9->GetCreationParameters(&DevPar9);
					if (SUCCEEDED(hr)) {
						IDirect3D9* pD3D9;
						hr = pD3DDev9->GetDirect3D(&pD3D9);
						if (SUCCEEDED(hr)) {
							D3DADAPTER_IDENTIFIER9 AdapID9;
							hr = pD3D9->GetAdapterIdentifier(DevPar9.AdapterOrdinal, 0, &AdapID9);
							if (SUCCEEDED(hr)) {
								// copy adapter description
								m_nPCIVendor					= AdapID9.VendorId;
								m_nPCIDevice					= AdapID9.DeviceId;
								m_VideoDriverVersion.QuadPart	= AdapID9.DriverVersion.QuadPart;
								m_strDeviceDescription			= AdapID9.Description;
								m_strDeviceDescription.AppendFormat(_T(" (%04hX:%04hX)"), m_nPCIVendor, m_nPCIDevice);
							}
						}
						pD3D9->Release();
					}
					pD3DDev9->Release();
					pDevMan9->UnlockDevice(hDevice, FALSE);
				}
				pDevMan9->CloseDeviceHandle(hDevice);
			}
			pDevMan9->Release();
		}
		pGetService->Release();
	}
	return hr;
}

HRESULT CMPCVideoDecFilter::SetFFMpegCodec(int nCodec, bool bEnabled)
{
	CAutoLock cAutoLock(&m_csProps);

	if (nCodec < 0 || nCodec >= FFM_LAST) {
		return E_FAIL;
	}

	m_FFmpegFilters[nCodec] = bEnabled;
	return S_OK;
}

HRESULT CMPCVideoDecFilter::SetDXVACodec(int nCodec, bool bEnabled)
{
	CAutoLock cAutoLock(&m_csProps);

	if (nCodec < 0 || nCodec >= TRA_DXVA_LAST) {
		return E_FAIL;
	}

	m_DXVAFilters[nCodec] = bEnabled;
	return S_OK;
}

// IFFmpegDecFilter
STDMETHODIMP CMPCVideoDecFilter::SaveSettings()
{
#ifdef REGISTER_FILTER
	CRegKey key;
	if (ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, OPT_REGKEY_VideoDec)) {
		key.SetDWORDValue(OPT_ThreadNumber, m_nThreadNumber);
		key.SetDWORDValue(OPT_DiscardMode, m_nDiscardMode);
		key.SetDWORDValue(OPT_Deinterlacing, (int)m_nDeinterlacing);
		key.SetDWORDValue(OPT_ARMode, m_nARMode);
		key.SetDWORDValue(OPT_DXVACheck, m_nDXVACheckCompatibility);
		key.SetDWORDValue(OPT_DisableDXVA_SD, m_nDXVA_SD);

		// === New swscaler options
		for (int i = 0; i < PixFmt_count; i++) {
			CString optname = OPT_SW_prefix;
			optname += GetSWOF(i)->name;
			key.SetDWORDValue(optname, m_fPixFmts[i]);
		}
		key.SetDWORDValue(OPT_SwPreset, m_nSwPreset);
		key.SetDWORDValue(OPT_SwStandard, m_nSwStandard);
		key.SetDWORDValue(OPT_SwRGBLevels, m_nSwRGBLevels);
		//
	}
	if (ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, OPT_REGKEY_VCodecs)) {
		for (size_t i = 0; i < _countof(vcodecs); i++) {
			DWORD dw = m_nActiveCodecs & vcodecs[i].flag ? 1 : 0;
			key.SetDWORDValue(vcodecs[i].opt_name, dw);
		}
	}
#else
	AfxGetApp()->WriteProfileInt(OPT_SECTION_VideoDec, OPT_ThreadNumber, m_nThreadNumber);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_VideoDec, OPT_DiscardMode, m_nDiscardMode);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_VideoDec, OPT_Deinterlacing, (int)m_nDeinterlacing);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_VideoDec, OPT_ARMode, m_nARMode);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_VideoDec, OPT_DXVACheck, m_nDXVACheckCompatibility);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_VideoDec, OPT_DisableDXVA_SD, m_nDXVA_SD);

	// === New swscaler options
	for (int i = 0; i < PixFmt_count; i++) {
		CString optname = OPT_SW_prefix;
		optname += GetSWOF(i)->name;
		AfxGetApp()->WriteProfileInt(OPT_SECTION_VideoDec, optname, m_fPixFmts[i]);
	}
	AfxGetApp()->WriteProfileInt(OPT_SECTION_VideoDec, OPT_SwPreset, m_nSwPreset);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_VideoDec, OPT_SwStandard, m_nSwStandard);
	AfxGetApp()->WriteProfileInt(OPT_SECTION_VideoDec, OPT_SwRGBLevels, m_nSwRGBLevels);
	//
#endif

	return S_OK;
}

// === IMPCVideoDecFilter

STDMETHODIMP CMPCVideoDecFilter::SetThreadNumber(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nThreadNumber = nValue;
	return S_OK;
}

STDMETHODIMP_(int) CMPCVideoDecFilter::GetThreadNumber()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nThreadNumber;
}

STDMETHODIMP CMPCVideoDecFilter::SetDiscardMode(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nDiscardMode = nValue;
	return S_OK;
}

STDMETHODIMP_(int) CMPCVideoDecFilter::GetDiscardMode()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nDiscardMode;
}

STDMETHODIMP CMPCVideoDecFilter::SetDeinterlacing(MPC_DEINTERLACING_FLAGS nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nDeinterlacing = nValue;
	return S_OK;
}

STDMETHODIMP_(MPC_DEINTERLACING_FLAGS) CMPCVideoDecFilter::GetDeinterlacing()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nDeinterlacing;
}

STDMETHODIMP_(GUID*) CMPCVideoDecFilter::GetDXVADecoderGuid()
{
	if (m_pGraph == NULL) {
		return NULL;
	} else {
		return &m_DXVADecoderGUID;
	}
}

STDMETHODIMP CMPCVideoDecFilter::SetActiveCodecs(ULONGLONG nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nActiveCodecs = nValue;
	return S_OK;
}

STDMETHODIMP_(ULONGLONG) CMPCVideoDecFilter::GetActiveCodecs()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nActiveCodecs;
}

STDMETHODIMP CMPCVideoDecFilter::SetARMode(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nARMode = nValue;
	return S_OK;
}

STDMETHODIMP_(int) CMPCVideoDecFilter::GetARMode()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nARMode;
}

STDMETHODIMP CMPCVideoDecFilter::SetDXVACheckCompatibility(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nDXVACheckCompatibility = nValue;
	return S_OK;
}

STDMETHODIMP_(int) CMPCVideoDecFilter::GetDXVACheckCompatibility()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nDXVACheckCompatibility;
}

STDMETHODIMP CMPCVideoDecFilter::SetDXVA_SD(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nDXVA_SD = nValue;
	return S_OK;
}

STDMETHODIMP_(int) CMPCVideoDecFilter::GetDXVA_SD()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nDXVA_SD;
}

// === New swscaler options
STDMETHODIMP CMPCVideoDecFilter::SetSwRefresh(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);

	if (nValue && m_pAVCtx && m_nDecoderMode == MODE_SOFTWARE) {
		ChangeOutputMediaFormat(nValue);
	}
	return S_OK;
}

STDMETHODIMP CMPCVideoDecFilter::SetSwPixelFormat(MPCPixelFormat pf, bool enable)
{
	CAutoLock cAutoLock(&m_csProps);
	if (pf < 0 && pf >= PixFmt_count) {
		return E_INVALIDARG;
	}

	m_fPixFmts[pf] = enable;
	return S_OK;
}

STDMETHODIMP_(bool) CMPCVideoDecFilter::GetSwPixelFormat(MPCPixelFormat pf)
{
	CAutoLock cAutoLock(&m_csProps);

	if (pf < 0 && pf >= PixFmt_count) {
		return false;
	}

	return m_fPixFmts[pf];
}

STDMETHODIMP CMPCVideoDecFilter::SetSwPreset(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nSwPreset = nValue;
	return S_OK;
}
STDMETHODIMP_(int) CMPCVideoDecFilter::GetSwPreset()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nSwPreset;
}

STDMETHODIMP CMPCVideoDecFilter::SetSwStandard(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nSwStandard = nValue;
	return S_OK;
}
STDMETHODIMP_(int) CMPCVideoDecFilter::GetSwStandard()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nSwStandard;
}

STDMETHODIMP CMPCVideoDecFilter::SetSwRGBLevels(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nSwRGBLevels = nValue;
	return S_OK;
}
STDMETHODIMP_(int) CMPCVideoDecFilter::GetSwRGBLevels()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nSwRGBLevels;
}

STDMETHODIMP_(int) CMPCVideoDecFilter::GetColorSpaceConversion()
{
	CAutoLock cAutoLock(&m_csProps);

	if (!m_pAVCtx) {
		return -1; // no decoder
	}

	if (m_nDecoderMode != MODE_SOFTWARE || m_pAVCtx->pix_fmt == AV_PIX_FMT_NONE || m_FormatConverter.GetOutPixFormat() == PixFmt_None) {
		return -2; // no conversion
	}
	
	bool in_rgb		= !!(av_pix_fmt_desc_get(m_pAVCtx->pix_fmt)->flags & (AV_PIX_FMT_FLAG_RGB|AV_PIX_FMT_FLAG_PAL));
	bool out_rgb	= (m_FormatConverter.GetOutPixFormat() == PixFmt_RGB32);
	if (in_rgb < out_rgb) {
		return 1; // YUV->RGB conversion
	}
	if (in_rgb > out_rgb) {
		return 2; // RGB->YUV conversion
	}

	return 0; // YUV->YUV or RGB->RGB conversion
}

STDMETHODIMP CMPCVideoDecFilter::GetOutputMediaType(CMediaType* pmt)
{
	CAutoLock cAutoLock(&m_csProps);
	CopyMediaType(pmt, &m_pOutput->CurrentMediaType());

	return S_OK;
}

STDMETHODIMP_(CString) CMPCVideoDecFilter::GetInformation(MPCInfo index)
{
	CAutoLock cAutoLock(&m_csProps);
	CString infostr;

	switch (index) {
	case INFO_MPCVersion:
		infostr.Format(_T("v%d.%d.%d.%d (build %d)"),MPC_VERSION_MAJOR,MPC_VERSION_MINOR,MPC_VERSION_PATCH,MPC_VERSION_STATUS,MPC_VERSION_REV);
		break;
	case INFO_InputFormat:
		if (m_pAVCtx) {
			infostr = m_pAVCtx->codec->name;
			if (const AVPixFmtDescriptor* pfdesc = av_pix_fmt_desc_get(m_pAVCtx->pix_fmt)) {
				if (pfdesc->flags & (AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_PAL)) {
					infostr.AppendFormat(_T(", %d-bit"), GetLumaBits(m_pAVCtx->pix_fmt));
				}
				else {
					infostr.AppendFormat(_T(", %d-bit %s"), GetLumaBits(m_pAVCtx->pix_fmt), GetChromaSubsamplingStr(m_pAVCtx->pix_fmt));
				}
			}
		}
		break;
	case INFO_FrameSize:
		if (m_w && m_h) {
			int sarx = m_arx * m_h;
			int sary = m_ary * m_w;
			ReduceDim(sarx, sary);
			infostr.Format(_T("%dx%d, SAR %d:%d, DAR %d:%d"), m_w, m_h, sarx, sary, m_arx, m_ary);
		}
		break;
	case INFO_OutputFormat:
		if (GUID* DxvaGuid = GetDXVADecoderGuid()) {
			if (*DxvaGuid != GUID_NULL) {
				infostr.Format(_T("DXVA (%s)"), GetDXVAMode(DxvaGuid));
				break;
			}
		}
		if (const SW_OUT_FMT* swof = GetSWOF(m_FormatConverter.GetOutPixFormat())) {
			infostr.Format(_T("%s (%d-bit %s)"), swof->name, swof->luma_bits, GetChromaSubsamplingStr(swof->av_pix_fmt));
		}
		break;
	case INFO_GraphicsAdapter:
		infostr = m_strDeviceDescription;
		break;
	}

	return infostr;
}

STDMETHODIMP_(int) CMPCVideoDecFilter::GetFrameType()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nFrameType;
}

CVideoDecOutputPin::CVideoDecOutputPin(TCHAR* pObjectName, CBaseVideoFilter* pFilter, HRESULT* phr, LPCWSTR pName)
	: CBaseVideoOutputPin(pObjectName, pFilter, phr, pName)
{
	m_pVideoDecFilter		= static_cast<CMPCVideoDecFilter*>(pFilter);
	m_dwDXVA1SurfaceCount	= 0;
	m_GuidDecoderDXVA1		= GUID_NULL;
	memset(&m_ddUncompPixelFormat, 0, sizeof(m_ddUncompPixelFormat));
}

CVideoDecOutputPin::~CVideoDecOutputPin(void)
{
}

HRESULT CVideoDecOutputPin::InitAllocator(IMemAllocator **ppAlloc)
{
	if (m_pVideoDecFilter->UseDXVA2()) {
		return m_pVideoDecFilter->InitAllocator(ppAlloc);
	} else {
		return __super::InitAllocator(ppAlloc);
	}
}

STDMETHODIMP CVideoDecOutputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(IAMVideoAcceleratorNotify)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

// === IAMVideoAcceleratorNotify
STDMETHODIMP CVideoDecOutputPin::GetUncompSurfacesInfo(const GUID *pGuid, LPAMVAUncompBufferInfo pUncompBufferInfo)
{
	HRESULT hr = E_INVALIDARG;

	if (m_pVideoDecFilter->IsSupportedDecoderMode(pGuid)) {
		CComQIPtr<IAMVideoAccelerator> pAMVideoAccelerator = GetConnected();

		if (pAMVideoAccelerator) {
			pUncompBufferInfo->dwMaxNumSurfaces		= m_pVideoDecFilter->GetPicEntryNumber();
			pUncompBufferInfo->dwMinNumSurfaces		= m_pVideoDecFilter->GetPicEntryNumber();

			hr = m_pVideoDecFilter->FindDXVA1DecoderConfiguration(pAMVideoAccelerator, pGuid, &pUncompBufferInfo->ddUncompPixelFormat);
			if (SUCCEEDED(hr)) {
				memcpy(&m_ddUncompPixelFormat, &pUncompBufferInfo->ddUncompPixelFormat, sizeof(DDPIXELFORMAT));
				m_GuidDecoderDXVA1 = *pGuid;
			}
		}
	}

	return hr;
}

STDMETHODIMP CVideoDecOutputPin::SetUncompSurfacesInfo(DWORD dwActualUncompSurfacesAllocated)
{
	m_dwDXVA1SurfaceCount = dwActualUncompSurfacesAllocated;
	return S_OK;
}

STDMETHODIMP CVideoDecOutputPin::GetCreateVideoAcceleratorData(const GUID *pGuid, LPDWORD pdwSizeMiscData, LPVOID *ppMiscData)
{
	HRESULT								hr						= E_UNEXPECTED;
	AMVAUncompDataInfo					UncompInfo;
	AMVACompBufferInfo					CompInfo[30];
	DWORD								dwNumTypesCompBuffers	= _countof(CompInfo);
	CComQIPtr<IAMVideoAccelerator>		pAMVideoAccelerator		= GetConnected();
	DXVA_ConnectMode*					pConnectMode;

	if (pAMVideoAccelerator) {
		memcpy(&UncompInfo.ddUncompPixelFormat, &m_ddUncompPixelFormat, sizeof(DDPIXELFORMAT));
		UncompInfo.dwUncompWidth		= m_pVideoDecFilter->PictWidthRounded();
		UncompInfo.dwUncompHeight		= m_pVideoDecFilter->PictHeightRounded();
		hr = pAMVideoAccelerator->GetCompBufferInfo(&m_GuidDecoderDXVA1, &UncompInfo, &dwNumTypesCompBuffers, CompInfo);

		if (SUCCEEDED(hr)) {
			hr = m_pVideoDecFilter->CreateDXVA1Decoder(pAMVideoAccelerator, pGuid, m_dwDXVA1SurfaceCount);

			if (SUCCEEDED(hr)) {
				m_pVideoDecFilter->SetDXVA1Params(&m_GuidDecoderDXVA1, &m_ddUncompPixelFormat);

				pConnectMode					= (DXVA_ConnectMode*)CoTaskMemAlloc (sizeof(DXVA_ConnectMode));
				pConnectMode->guidMode			= m_GuidDecoderDXVA1;
				pConnectMode->wRestrictedMode	= m_pVideoDecFilter->GetDXVA1RestrictedMode();
				*pdwSizeMiscData				= sizeof(DXVA_ConnectMode);
				*ppMiscData						= pConnectMode;
			}
		}
	}

	return hr;
}

//
void GetFormatList(CAtlList<SUPPORTED_FORMATS>& fmts)
{
	fmts.RemoveAll();

	for (size_t i = 0; i < _countof(ffCodecs); i++) {
		SUPPORTED_FORMATS fmt = {sudPinTypesIn[i].clsMajorType, ffCodecs[i].clsMinorType, ffCodecs[i].FFMPEGCode, ffCodecs[i].DXVACode};
		fmts.AddTail(fmt);
	}
}
