/*
 * 
 * (C) 2013 see Authors.txt
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

#include "IMPCVideoDec.h"

#pragma warning(push)
#pragma warning(disable: 4005)
#include <stdint.h>
#pragma warning(pop)

const MPCPixelFormat YUV420_8[PixFmt_count]  = {PixFmt_NV12, PixFmt_YV12, PixFmt_YUY2, PixFmt_AYUV, PixFmt_YV24, PixFmt_P010, PixFmt_P016, PixFmt_P210, PixFmt_P216, PixFmt_Y410, PixFmt_Y416, PixFmt_RGB32};
const MPCPixelFormat YUV422_8[PixFmt_count]  = {PixFmt_YUY2, PixFmt_AYUV, PixFmt_YV24, PixFmt_NV12, PixFmt_YV12, PixFmt_P210, PixFmt_P216, PixFmt_Y410, PixFmt_P010, PixFmt_P016, PixFmt_Y416, PixFmt_RGB32};
const MPCPixelFormat YUV444_8[PixFmt_count]  = {PixFmt_AYUV, PixFmt_YV24, PixFmt_YUY2, PixFmt_NV12, PixFmt_YV12, PixFmt_Y410, PixFmt_P210, PixFmt_P216, PixFmt_P010, PixFmt_P016, PixFmt_Y416, PixFmt_RGB32};

const MPCPixelFormat YUV420_10[PixFmt_count] = {PixFmt_P010, PixFmt_P016, PixFmt_P210, PixFmt_P216, PixFmt_Y410, PixFmt_Y416, PixFmt_NV12, PixFmt_YV12, PixFmt_YUY2, PixFmt_AYUV, PixFmt_YV24, PixFmt_RGB32};
const MPCPixelFormat YUV422_10[PixFmt_count] = {PixFmt_P210, PixFmt_P216, PixFmt_Y410, PixFmt_P010, PixFmt_P016, PixFmt_Y416, PixFmt_YUY2, PixFmt_AYUV, PixFmt_YV24, PixFmt_NV12, PixFmt_YV12, PixFmt_RGB32};
const MPCPixelFormat YUV444_10[PixFmt_count] = {PixFmt_Y410, PixFmt_P210, PixFmt_P216, PixFmt_P010, PixFmt_P016, PixFmt_Y416, PixFmt_AYUV, PixFmt_YV24, PixFmt_YUY2, PixFmt_NV12, PixFmt_YV12, PixFmt_RGB32};

const MPCPixelFormat YUV420_16[PixFmt_count] = {PixFmt_P016, PixFmt_P216, PixFmt_Y416, PixFmt_P010, PixFmt_P210, PixFmt_Y410, PixFmt_NV12, PixFmt_YV12, PixFmt_YUY2, PixFmt_AYUV, PixFmt_YV24, PixFmt_RGB32};
const MPCPixelFormat YUV422_16[PixFmt_count] = {PixFmt_P216, PixFmt_Y416, PixFmt_P016, PixFmt_P210, PixFmt_Y410, PixFmt_P010, PixFmt_YUY2, PixFmt_AYUV, PixFmt_YV24, PixFmt_NV12, PixFmt_YV12, PixFmt_RGB32};
const MPCPixelFormat YUV444_16[PixFmt_count] = {PixFmt_Y416, PixFmt_P216, PixFmt_P016, PixFmt_Y410, PixFmt_P210, PixFmt_P010, PixFmt_AYUV, PixFmt_YV24, PixFmt_YUY2, PixFmt_NV12, PixFmt_YV12, PixFmt_RGB32};

const MPCPixelFormat RGB_8[PixFmt_count]     = {PixFmt_RGB32, PixFmt_AYUV, PixFmt_YV24, PixFmt_YUY2, PixFmt_NV12, PixFmt_YV12, PixFmt_Y410, PixFmt_P210, PixFmt_P216, PixFmt_P010, PixFmt_P016, PixFmt_Y416};

typedef struct {
	const LPCTSTR				name;
	const DWORD					biCompression;
	const GUID*					subtype;
	const int					bpp;
	const int					codedbytes;
	const int					planes;
	const int					planeHeight[4];
	const int					planeWidth[4];
	const enum AVPixelFormat	av_pix_fmt;
	const uint8_t				chroma_w;
	const uint8_t				chroma_h;
	const int					actual_bpp;
	const int					luma_bits;
} SW_OUT_FMT;

const SW_OUT_FMT* GetSWOF(int pixfmt);
LPCTSTR GetChromaSubsamplingStr(AVPixelFormat av_pix_fmt);
int GetLumaBits(AVPixelFormat av_pix_fmt);

// CFormatConverter

struct AVFrame;
struct SwsContext;

enum MPCPixFmtType {
	PFType_unspecified,
	PFType_RGB,
	PFType_YUV420,
	PFType_YUV422,
	PFType_YUV444,
	PFType_YUV420Px,
	PFType_YUV422Px,
	PFType_YUV444Px,
};

typedef struct {
	// basic properties
	enum AVPixelFormat	avpixfmt;
	int					width;
	int					height;
	// additional properties
	int					lumabits;
	MPCPixFmtType		pftype;
	enum AVColorSpace	colorspace;
	enum AVColorRange	colorrange;
} FrameProps;

class CFormatConverter
{
#define CONV_FUNC_PARAMS const uint8_t* const src[4], const int srcStride[4], uint8_t* dst[], int width, int height, int dstStride[]

protected:
	SwsContext*			m_pSwsContext;
	FrameProps			m_FProps;

	MPCPixelFormat		m_out_pixfmt;

	int					m_swsFlags;
	bool				m_autocolorspace;
	int					m_colorspace;
	int					m_dstRGBRange;

	int					m_dstStride;
	int					m_planeHeight;

	size_t				m_nAlignedBufferSize;
	uint8_t*			m_pAlignedBuffer;

	int					m_nCPUFlag;

	bool Init();
	void UpdateDetails();
	void SetConvertFunc();

	// from LAV Filters
	HRESULT ConvertToAYUV(CONV_FUNC_PARAMS);
	HRESULT ConvertToPX1X(CONV_FUNC_PARAMS, int chromaVertical);
	HRESULT ConvertToY410(CONV_FUNC_PARAMS);
	HRESULT ConvertToY416(CONV_FUNC_PARAMS);
	
	// Conversion function pointer
	typedef HRESULT (CFormatConverter::*ConverterFn)(CONV_FUNC_PARAMS);
	ConverterFn pConvertFn;

	HRESULT ConvertGeneric(CONV_FUNC_PARAMS);
	// optimized function
	HRESULT convert_yuv444_y410(CONV_FUNC_PARAMS);
	HRESULT convert_yuv444_ayuv(CONV_FUNC_PARAMS);
	HRESULT convert_yuv444_ayuv_dither_le(CONV_FUNC_PARAMS);
	HRESULT convert_yuv420_px1x_le(CONV_FUNC_PARAMS);
	HRESULT convert_yuv420_yuy2(CONV_FUNC_PARAMS);
	HRESULT convert_yuv422_yuy2_uyvy_dither_le(CONV_FUNC_PARAMS);

public:
	CFormatConverter();
	~CFormatConverter();

	void UpdateOutput(MPCPixelFormat out_pixfmt, int dstStride, int planeHeight);
	void UpdateOutput2(DWORD biCompression, LONG biWidth, LONG biHeight);
	void SetOptions(int preset, int standard, int rgblevels);

	MPCPixelFormat GetOutPixFormat() { return m_out_pixfmt; }

	int  Converting(BYTE* dst, AVFrame* pFrame);

	void Cleanup();
};
