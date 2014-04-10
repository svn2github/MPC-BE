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

#pragma once

#include <dxva.h>
#include "DXVADecoder.h"

#define MAX_SLICE		1024 // Max slice number for Mpeg2 streams
#define MAX_BUFF_TIME	20

class CDXVADecoderMpeg2 : public CDXVADecoder
{
public:
	CDXVADecoderMpeg2(CMPCVideoDecFilter* pFilter, IAMVideoAccelerator* pAMVideoAccelerator, const GUID* guidDecoder, DXVAMode nMode, int nPicEntryNumber);
	CDXVADecoderMpeg2(CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, const GUID* guidDecoder, DXVAMode nMode, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config);
	virtual ~CDXVADecoderMpeg2();

	virtual HRESULT			CopyBitstream(BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize, UINT nDXVASize = UINT_MAX);
	virtual HRESULT			DecodeFrame(BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);

private:
	struct DXVA_MPEG2_Context {
		DXVA_PictureParameters	DXVAPicParams;
		DXVA_QmatrixData		DXVAScalingMatrix;
		unsigned				slice_count;
		DXVA_SliceInfo			slice[MAX_SLICE];
		const uint8_t			*bitstream;
		unsigned				bitstream_size;
		int						frame_start;
	};
	struct DXVA_Context {
		unsigned				frame_count;
		DXVA_MPEG2_Context		DXVA_MPEG2Context[2];	
	} m_DXVA_Context;

	UINT					m_nFieldNum;

	void					Init();
	void					UpdatePictureParams();
};
