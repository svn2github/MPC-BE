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

#define HAVE_AV_CONFIG_H

#include <Windows.h>
#include <WinNT.h>
#include <vfwmsgs.h>
#include <sys/timeb.h>
#include <time.h> // for the _time64 workaround
#include "../../../DSUtil/SysVersion.h"
#include "ffmpegContext.h"
#include <math.h>

extern "C" {
	#include <ffmpeg/libavcodec/dsputil.h>
	#include <ffmpeg/libavcodec/avcodec.h>
// This is kind of an hack but it avoids using a C++ keyword as a struct member name
#define class classFFMPEG
	#include <ffmpeg/libavcodec/mpegvideo.h>
#undef class
	#include <ffmpeg/libavcodec/golomb.h>
// hack since "h264.h" is using "new" as a variable
#define new newFFMPEG
	#include <ffmpeg/libavcodec/h264.h>
#undef new
	#include <ffmpeg/libavcodec/h264data.h>
	#include <ffmpeg/libavcodec/vc1.h>
	#include <ffmpeg/libavcodec/mpeg12.h>
}

static const WORD PCID_NVIDIA_VP5 [] = {
	// http://us.download.nvidia.com/XFree86/Linux-x86_64/331.38/README/supportedchips.html
	// http://pci-ids.ucw.cz/read/PC/10de
	// VP5, Nvidia VDPAU Feature Set D: GF119, GK104, GK106, GK107, GK110, GK208
	0x0FC2, // GeForce GT 630 (GK107) (not officially supported or typo, 4k tested)
	0x0FC6, // GeForce GTX 650
	0x0FCD, // GeForce GT 755M
	0x0FD1, // GeForce GT 650M
	0x0FD2, // GeForce GT 640M
	0x0FD4, // GeForce GTX 660M
	0x0FD5, // GeForce GT 650M
	0x0FD8, // GeForce GT 640M
	0x0FD9, // GeForce GT 645M
	0x0FDF, // GeForce GT 740M
	0x0FE0, // GeForce GTX 660M
	0x0FE1, // GeForce GT 730M
	0x0FE2, // GeForce GT 745M
	0x0FE3, // GeForce GT 745M
	0x0FE4, // GeForce GT 750M
	0x0FE9, // GeForce GT 750M
	0x0FEA, // GeForce GT 755M
	0x0FEF, // GRID K340
	0x0FF2, // GRID K1
	0x0FF6, // Quadro K1100M
	0x0FF8, // Quadro K500M
	0x0FF9, // Quadro K2000D
	0x0FFA, // Quadro K600
	0x0FFB, // Quadro K2000M
	0x0FFC, // Quadro K1000M
	0x0FFD, // NVS 510
	0x0FFE, // Quadro K2000
	0x0FFF, // Quadro 410
	0x1004, // GeForce GTX 780
	0x1005, // GeForce GTX TITAN
	0x100A, // GeForce GTX 780 Ti
	0x1021, // Tesla K20Xm
	0x1022, // Tesla K20c
	0x1023, // Tesla K40m
	0x1024, // Tesla K40c
	0x1026, // Tesla K20s
	0x1027, // Tesla K40st
	0x1028, // Tesla K20m
	0x1029, // Tesla K40s
	0x103A, // Quadro K6000
	0x1040, // GeForce GT 520 (GF119) (not officially supported or typo, 4k tested)
	0x1042, // GeForce 510
	0x1048, // GeForce 605
	0x104A, // GeForce GT 610 (fully tested)
	0x104B, // GeForce GT 625 (OEM)
	0x1050, // GeForce GT 520M (GF119) (not officially supported or typo)
	0x1051, // GeForce GT 520MX
	0x1052, // GeForce GT 520M (GF119) (not officially supported or typo)
	0x1054, // GeForce 410M
	0x1055, // GeForce 410M
	0x1056, // NVS 4200M
	0x1057, // NVS 4200M
	0x105B, // GeForce 705M
	0x107C, // NVS 315
	0x107D, // NVS 310
	0x1180, // GeForce GTX 680
	0x1183, // GeForce GTX 660 Ti (fully tested)
	0x1184, // GeForce GTX 770
	0x1185, // GeForce GTX 660
	0x1187, // GeForce GTX 760
	0x1188, // GeForce GTX 690
	0x1189, // GeForce GTX 670
	0x118A, // GRID K520
	0x118E, // GeForce GTX 760 (192-bit)
	0x118F, // Tesla K10
	0x1193, // GeForce GTX 760 Ti OEM
	0x119D, // GeForce GTX 775M
	0x119E, // GeForce GTX 780M
	0x119F, // GeForce GTX 780M
	0x11A0, // GeForce GTX 680M
	0x11A1, // GeForce GTX 670MX
	0x11A2, // GeForce GTX 675MX
	0x11A3, // GeForce GTX 680MX
	0x11A7, // GeForce GTX 675MX
	0x11B6, // Quadro K3100M
	0x11B7, // Quadro K4100M
	0x11B8, // Quadro K5100M
	0x11BA, // Quadro K5000
	0x11BC, // Quadro K5000M
	0x11BD, // Quadro K4000M
	0x11BE, // Quadro K3000M
	0x11BF, // GRID K2
	0x11C0, // GeForce GTX 660
	0x11C2, // GeForce GTX 650 Ti BOOST
	0x11C3, // GeForce GTX 650 Ti
	0x11C4, // GeForce GTX 645
	0x11C6, // GeForce GTX 650 Ti
	0x11C8, // GeForce GTX 650
	0x11E0, // GeForce GTX 770M
	0x11E1, // GeForce GTX 765M
	0x11E2, // GeForce GTX 765M
	0x11E3, // GeForce GTX 760M
	0x11FA, // Quadro K4000
	0x11FC, // Quadro K2100M
	0x1280, // GeForce GT 635
	0x1282, // GeForce GT 640 rev. 2 (not officially supported or typo, fully tested)
	0x1290, // GeForce GT 730M
	0x1291, // GeForce GT 735M
	0x1292, // GeForce GT 740M
	0x1293, // GeForce GT 730M
	0x12B9, // Quadro K610M
	0x12BA, // Quadro K510M
};

static const WORD PCID_ATI_UVD [] = {
	0x94C7, // ATI Radeon HD 2350
	0x94C1, // ATI Radeon HD 2400 XT
	0x94CC, // ATI Radeon HD 2400 Series
	0x958A, // ATI Radeon HD 2600 X2 Series
	0x9588, // ATI Radeon HD 2600 XT
	0x9405, // ATI Radeon HD 2900 GT
	0x9400, // ATI Radeon HD 2900 XT
	0x9611, // ATI Radeon 3100 Graphics
	0x9610, // ATI Radeon HD 3200 Graphics
	0x9614, // ATI Radeon HD 3300 Graphics
	0x95C0, // ATI Radeon HD 3400 Series (and others)
	0x95C5, // ATI Radeon HD 3400 Series (and others)
	0x95C4, // ATI Radeon HD 3400 Series (and others)
	0x94C3, // ATI Radeon HD 3410
	0x9589, // ATI Radeon HD 3600 Series (and others)
	0x9598, // ATI Radeon HD 3600 Series (and others)
	0x9591, // ATI Radeon HD 3600 Series (and others)
	0x9501, // ATI Radeon HD 3800 Series (and others)
	0x9505, // ATI Radeon HD 3800 Series (and others)
	0x9507, // ATI Radeon HD 3830
	0x9513, // ATI Radeon HD 3850 X2
	0x9515, // ATI Radeon HD 3850 AGP
	0x950F, // ATI Radeon HD 3850 X2
};

static const WORD PCID_INTEL_4K [] = {
	// IvyBridge
	0x0152, // Intel HD Graphics 2500        (4k tested)
	0x0156, // Intel HD Graphics 2500 Mobile
	0x015A, // Intel HD Graphics P2500
	0x0162, // Intel HD Graphics 4000        (fully tested)
	0x0166, // Intel HD Graphics 4000 Mobile (not tested)
	0x016A, // Intel HD Graphics P4000       (not tested)
	// Haswell (not tested)
	0x0412, // Intel HD Graphics HD4600
	0x0416, // Intel HD Graphics HD4600 Mobile
	0x041A, // Intel HD Graphics P4600/P4700
	0x0A16, // Intel HD Graphics Family
	0x0A1E, // Intel HD Graphics Family
	0x0A26, // Intel HD Graphics 5000
	0x0A2E, // Intel Iris Graphics 5100
	0x0D22, // Intel Iris Graphics 5200
	0x0D26, // Intel Iris Graphics 5200
};

static bool CheckPCID(DWORD pcid, const WORD* pPCIDs, size_t count)
{
	WORD wPCID = (WORD)pcid;
	for (size_t i = 0; i < count; i++) {
		if (wPCID == pPCIDs[i]) {
			return true;
		}
	}

	return false;
}

bool IsATIUVD(DWORD nPCIVendor, DWORD nPCIDevice)
{
	return (nPCIVendor == PCIV_ATI && CheckPCID(nPCIDevice, PCID_ATI_UVD, _countof(PCID_ATI_UVD)));
}

// === H264 functions
// returns TRUE if version is equal to or higher than A.B.C.D, returns FALSE otherwise
BOOL DriverVersionCheck(LARGE_INTEGER VideoDriverVersion, int A, int B, int C, int D)
{
	if (HIWORD(VideoDriverVersion.HighPart) > A) {
		return TRUE;
	} else if (HIWORD(VideoDriverVersion.HighPart) == A) {
		if (LOWORD(VideoDriverVersion.HighPart) > B) {
			return TRUE;
		} else if (LOWORD(VideoDriverVersion.HighPart) == B) {
			if (HIWORD(VideoDriverVersion.LowPart) > C) {
				return TRUE;
			} else if (HIWORD(VideoDriverVersion.LowPart) == C) {
				if (LOWORD(VideoDriverVersion.LowPart) >= D) {
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

static unsigned __int64 GetFileVersion(LPCTSTR lptstrFilename)
{
	unsigned __int64 ret = 0;

	DWORD buff[4] = { 0 };
	VS_FIXEDFILEINFO* pvsf = (VS_FIXEDFILEINFO*)buff;
	DWORD d; // a variable that GetFileVersionInfoSize sets to zero (but why is it needed ?????????????????????????????? :)
	DWORD len = GetFileVersionInfoSize((LPTSTR)lptstrFilename, &d);

	if (len) {
		TCHAR* b1 = new TCHAR[len];
		if (b1) {
			UINT uLen;
			if (GetFileVersionInfo((LPTSTR)lptstrFilename, 0, len, b1) && VerQueryValue(b1, L"\\", (void**)&pvsf, &uLen)) {
				ret = ((unsigned __int64)pvsf->dwFileVersionMS << 32) | pvsf->dwFileVersionLS;
			}

			delete [] b1;
		}
	}

	return ret;
}

int FFH264CheckCompatibility(int nWidth, int nHeight, struct AVCodecContext* pAVCtx,
							 DWORD nPCIVendor, DWORD nPCIDevice, LARGE_INTEGER VideoDriverVersion, bool nIsAtiDXVACompatible)
{
	H264Context* pContext		= (H264Context*)pAVCtx->priv_data;

	int video_is_level51		= 0;
	int no_level51_support		= 1;
	int too_much_ref_frames		= 0;
	int max_ref_frames_dpb41	= min(11, 8388608/(nWidth * nHeight) );

	PPS* cur_pps				= &pContext->pps;
	SPS* cur_sps				= cur_pps != NULL ? pContext->sps_buffers[cur_pps->sps_id] : NULL;

	if (cur_sps != NULL) {
		if (cur_sps->bit_depth_luma > 8 || cur_sps->chroma_format_idc > 1) {
			return DXVA_HIGH_BIT;
		}

		if (cur_sps->profile_idc > 100) {
			return DXVA_PROFILE_HIGHER_THAN_HIGH;
		}

		video_is_level51			= cur_sps->level_idc >= 51 ? 1 : 0;
		int max_ref_frames			= max_ref_frames_dpb41; // default value is calculate

		if (nPCIVendor == PCIV_nVidia) {
			// nVidia cards support level 5.1 since drivers v6.14.11.7800 for XP and drivers v7.15.11.7800 for Vista/7
			if (IsWinVistaOrLater()) {
				if (DriverVersionCheck(VideoDriverVersion, 7, 15, 11, 7800)) {
					no_level51_support = 0;

					// max ref frames is 16 for HD and 11 otherwise
					// max_ref_frames = (nWidth >= 1280) ? 16 : 11;
					max_ref_frames = 16;
				}
			} else {
				if (DriverVersionCheck(VideoDriverVersion, 6, 14, 11, 7800)) {
					no_level51_support = 0;

					// max ref frames is 14
					max_ref_frames = 14;
				}
			}
		} else if (nPCIVendor == PCIV_S3_Graphics || nPCIVendor == PCIV_Intel) {
			no_level51_support = 0;
			max_ref_frames = 16;
		} else if (nPCIVendor == PCIV_ATI && nIsAtiDXVACompatible) {
			TCHAR path[MAX_PATH];
			GetSystemDirectory(path, MAX_PATH);
			wcscat(path, L"\\drivers\\atikmdag.sys\0");
			unsigned __int64 f_version = GetFileVersion(path);

			if (f_version) {
				LARGE_INTEGER VideoDriverVersion;
				VideoDriverVersion.QuadPart = f_version;

				if (IsWinVistaOrLater()) {
					// file version 8.1.1.1016 - Catalyst 10.4, WinVista & Win7
					if (DriverVersionCheck(VideoDriverVersion, 8, 1, 1, 1016)) {
						no_level51_support = 0;
						max_ref_frames = 16;
					}				
				} else {
					// TODO - need file version for Catalyst 10.4 under WinXP
				}
			
			} else {
				// driver version 8.14.1.6105 - Catalyst 10.4; TODO - verify this information
				if (DriverVersionCheck(VideoDriverVersion, 8, 14, 1, 6105)) {
					no_level51_support = 0;
					max_ref_frames = 16;
				}
			}
		}

		// Check maximum allowed number reference frames
		if (cur_sps->ref_frame_count > max_ref_frames) {
			too_much_ref_frames = 1;
		}
	}

	int Flags = 0;
	if (video_is_level51 * no_level51_support) {
		Flags |= DXVA_UNSUPPORTED_LEVEL;
	}
	if (too_much_ref_frames) {
		Flags |= DXVA_TOO_MANY_REF_FRAMES;
	}

	return Flags;
}

void FFH264SetDxvaParams(struct AVCodecContext* pAVCtx, void* DXVA_Context)
{
	H264Context* h			= (H264Context*)pAVCtx->priv_data;
	h->dxva_context			= DXVA_Context;
}

// === VC1 functions
void FFVC1SetDxvaParams(struct AVCodecContext* pAVCtx, void* pPicParams, void* pSliceInfo)
{
	VC1Context* vc1			= (VC1Context*)pAVCtx->priv_data;
	vc1->pPictureParameters	= pPicParams;
	vc1->pSliceInfo			= pSliceInfo;
}

// === Mpeg2 functions
int	MPEG2CheckCompatibility(struct AVCodecContext* pAVCtx)
{
	MpegEncContext*	s = (MpegEncContext*)pAVCtx->priv_data;

	// restore codec_id value, it's can be changed to AV_CODEC_ID_MPEG1VIDEO on .wtv/.dvr-ms + StreamBufferSource (possible broken extradata)
	pAVCtx->codec_id = AV_CODEC_ID_MPEG2VIDEO;

	return (s->chroma_format<2);
}

void FFMPEG2SetDxvaParams(struct AVCodecContext* pAVCtx, void* pDXVA_Context)
{
	MpegEncContext* s	= (MpegEncContext*)pAVCtx->priv_data;
	s->dxva_context		= pDXVA_Context;
}

// === Common functions
HRESULT FFDecodeFrame(struct AVCodecContext* pAVCtx, struct AVFrame* pFrame,
					  BYTE* pBuffer, UINT nSize, REFERENCE_TIME rtStart,
					  int* got_picture, UINT* nFrameSize/* = NULL*/)
{
	HRESULT	hr = E_FAIL;
	if (pBuffer) {
		AVPacket		avpkt;
		av_init_packet(&avpkt);
		avpkt.data		= pBuffer;
		avpkt.size		= nSize;
		avpkt.pts		= rtStart;
		avpkt.flags		= AV_PKT_FLAG_KEY;
		int used_bytes	= avcodec_decode_video2(pAVCtx, pFrame, got_picture, &avpkt);
		
#if defined(_DEBUG) && 0
		av_log(pAVCtx, AV_LOG_INFO, "FFDecodeFrame() : %d, %d\n", used_bytes, got_picture);
#endif

		if (used_bytes < 0) {
			return hr;
		}

		if (nFrameSize && pAVCtx->codec_id == AV_CODEC_ID_VC1) {
			VC1Context* vc1	= (VC1Context*)pAVCtx->priv_data;
			*nFrameSize		= vc1->second_field_offset;
		}

		hr = S_OK;
	}

	return hr;
}

inline MpegEncContext* GetMpegEncContext(struct AVCodecContext* pAVCtx)
{
	MpegEncContext*	s = NULL;

	switch (pAVCtx->codec_id) {
		case AV_CODEC_ID_VC1 :
		case AV_CODEC_ID_H264 :
		case AV_CODEC_ID_MPEG2VIDEO:
			s 	= (MpegEncContext*)pAVCtx->priv_data;
			break;
	}
	return s;
}

BOOL FFGetAlternateScan(struct AVCodecContext* pAVCtx)
{
	MpegEncContext* s = GetMpegEncContext(pAVCtx);

	return (s != NULL) ? s->alternate_scan : 0;
}

UINT FFGetMBCount(struct AVCodecContext* pAVCtx)
{
	UINT MBCount = 0;
	switch (pAVCtx->codec_id) {
		case AV_CODEC_ID_H264 : {
				H264Context* h	= (H264Context*)pAVCtx->priv_data;
				MBCount			= h->mb_width * h->mb_height;
			}
			break;
		case AV_CODEC_ID_MPEG2VIDEO: {
				MpegEncContext* s	= (MpegEncContext*)pAVCtx->priv_data;
				const int is_field	= s->picture_structure != PICT_FRAME;
				MBCount				= s->mb_width * (s->mb_height >> is_field);
			}
			break;
	}

	return MBCount;
}

void FFGetFrameProps(struct AVCodecContext* pAVCtx, struct AVFrame* pFrame, int& width, int& height)
{
	switch (pAVCtx->codec_id) {
	case AV_CODEC_ID_H264:
		{
			H264Context* h = (H264Context*)pAVCtx->priv_data;
			SPS* cur_sps = &h->sps;

			if (pAVCtx->extradata_size) {
				// When this code is needed? Or is it outdated?
				int				got_picture	= 0;
				AVPacket		avpkt;
				av_init_packet(&avpkt);
				avpkt.data		= pAVCtx->extradata;
				avpkt.size		= pAVCtx->extradata_size;
				avpkt.flags		= AV_PKT_FLAG_KEY;
				int used_bytes	= avcodec_decode_video2(pAVCtx, pFrame, &got_picture, &avpkt);
			}

			if (cur_sps) {
				width	= cur_sps->mb_width * 16;
				height	= cur_sps->mb_height * (2 - cur_sps->frame_mbs_only_flag) * 16;
			}
		}
		break;
	case AV_CODEC_ID_MPEG1VIDEO:
	case AV_CODEC_ID_MPEG2VIDEO:
		{
			MpegEncContext*	s = (MpegEncContext*)pAVCtx->priv_data;

			if (pAVCtx->extradata_size) {
				// need to try decode extradata to fill MpegEncContext structure.
				int				got_picture	= 0;
				AVPacket		avpkt;
				av_init_packet(&avpkt);
				avpkt.data		= pAVCtx->extradata;
				avpkt.size		= pAVCtx->extradata_size;
				avpkt.flags		= AV_PKT_FLAG_KEY;
				int used_bytes	= avcodec_decode_video2(pAVCtx, pFrame, &got_picture, &avpkt);
			}

			width	= FFALIGN(s->width, 16);
			height	= FFALIGN(s->height, 16);
			if (!s->progressive_sequence) {
				height = int((s->height + 31) / 32 * 2) * 16;
			}

			if (pAVCtx->pix_fmt == AV_PIX_FMT_NONE) {
				if(s->chroma_format < 2) {
					pAVCtx->pix_fmt = AV_PIX_FMT_YUV420P;
				} else if(s->chroma_format == 2) {
					pAVCtx->pix_fmt = AV_PIX_FMT_YUV422P;
				} else {
					pAVCtx->pix_fmt = AV_PIX_FMT_YUV444P;
				}
			}
		}
		break;
	case AV_CODEC_ID_LAGARITH:
		if (pAVCtx->pix_fmt == AV_PIX_FMT_NONE && pAVCtx->extradata_size >= 4) {
			switch (*(DWORD*)pAVCtx->extradata) {
			case 0:
				if (pAVCtx->bits_per_coded_sample == 32) {
					pAVCtx->pix_fmt = AV_PIX_FMT_RGBA;
				} else if (pAVCtx->bits_per_coded_sample == 24) {
					pAVCtx->pix_fmt = AV_PIX_FMT_RGB24;
				}
				break;
			case 1:
				pAVCtx->pix_fmt = AV_PIX_FMT_YUV422P;
				break;
			case 2:
				pAVCtx->pix_fmt = AV_PIX_FMT_YUV420P;
				break;
			}
		}
		break;
	case AV_CODEC_ID_PRORES:
		if (pAVCtx->pix_fmt == AV_PIX_FMT_NONE && pAVCtx->extradata_size >= 8) {
			switch (*(DWORD*)(pAVCtx->extradata + 4)) {
			case 'hcpa': // Apple ProRes 422 High Quality
			case 'ncpa': // Apple ProRes 422 Standard Definition
			case 'scpa': // Apple ProRes 422 LT
			case 'ocpa': // Apple ProRes 422 Proxy
				pAVCtx->pix_fmt = AV_PIX_FMT_YUV422P10LE;
				break;
			case 'h4pa': // Apple ProRes 4444
				pAVCtx->pix_fmt = pAVCtx->bits_per_coded_sample == 32 ? AV_PIX_FMT_YUVA444P10LE : AV_PIX_FMT_YUV444P10LE;
				break;
			}
		}
		break;
	case AV_CODEC_ID_MJPEG:
	case AV_CODEC_ID_DNXHD:
		if (pAVCtx->pix_fmt == AV_PIX_FMT_NONE) {
			av_log(pAVCtx, AV_LOG_INFO, "WARNING! : pAVCtx->pix_fmt == AV_PIX_FMT_NONE\n");
			pAVCtx->pix_fmt = AV_PIX_FMT_YUV422P; // bad hack
		}
		break;
	case AV_CODEC_ID_HEVC: // TODO
	default:
		if (pAVCtx->pix_fmt == AV_PIX_FMT_NONE) {
			av_log(pAVCtx, AV_LOG_INFO, "WARNING! : pAVCtx->pix_fmt == AV_PIX_FMT_NONE\n");
			pAVCtx->pix_fmt = AV_PIX_FMT_YUV420P; // bad hack
		}
	}
}

#define CHECK_AVC_L52_SIZE(w, h) ((w) <= 4096 && (h) <= 4096 && (w) * (h) <= 36864 * 16 * 16)
BOOL DXVACheckFramesize(int width, int height, DWORD nPCIVendor, DWORD nPCIDevice)
{
	width = (width + 15) & ~15; // (width + 15) / 16 * 16;
	height = (height + 15) & ~15; // (height + 15) / 16 * 16;

	if (nPCIVendor == PCIV_nVidia) {
		if (CheckPCID(nPCIDevice, PCID_NVIDIA_VP5, _countof(PCID_NVIDIA_VP5)) && width <= 4096 && height <= 4096 && width * height <= 4080 * 4080) {
			// tested H.264 on VP5 (GT 610, GTX 660 Ti)
			// 4080x4080 = 65025 macroblocks
			return TRUE;
		} else if (width <= 2032 && height <= 2032 && width * height <= 8190 * 16 * 16) {
			// tested H.264, VC-1 and MPEG-2 on VP4 (feature set C) (G210M, GT220)
			return TRUE;
		}
	} else if (nPCIVendor == PCIV_Intel && nPCIDevice == PCID_Intel_HD4000) {
		//if (width <= 4096 && height <= 4096 && width * height <= 56672 * 16 * 16) {
		if (width <= 4096 && height <= 4096) { // driver v.9.17.10.2867
			// complete test was performed
			return TRUE;
		}
	} else if (nPCIVendor == PCIV_Intel && CheckPCID(nPCIDevice, PCID_INTEL_4K, _countof(PCID_INTEL_4K))) {
		if (CHECK_AVC_L52_SIZE(width, height)) {
			// tested some media files with AVC Livel 5.1
			// complete test was NOT performed
			return TRUE;
		}
	} else if (width <= 1920 && height <= 1088) {
		return TRUE;
	}

	return FALSE;
}
