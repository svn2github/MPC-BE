/*
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


#include "stdafx.h"
#include "DVBSub.h"
#include "../DSUtil/GolombBuffer.h"

#if (0)		// Set to 1 to activate DVB subtitles traces
	#define TRACE_DVB	TRACE
#else
	#define TRACE_DVB	__noop
#endif

#define BUFFER_CHUNK_GROW		0x1000

CDVBSub::CDVBSub(void)
	: CBaseSub(ST_DVB)
{
	m_nBufferReadPos	= 0;
	m_nBufferWritePos	= 0;
	m_nBufferSize		= 0;
	m_pBuffer			= NULL;
}

CDVBSub::~CDVBSub(void)
{
	Reset();
	SAFE_DELETE(m_pBuffer);
}

CDVBSub::DVB_PAGE* CDVBSub::FindPage(REFERENCE_TIME rt)
{
	POSITION pos = m_Pages.GetHeadPosition();

	while (pos) {
		DVB_PAGE* pPage = m_Pages.GetAt (pos);

		if (rt >= pPage->rtStart && rt < pPage->rtStop) {
			return pPage;
		}

		m_Pages.GetNext(pos);
	}

	return NULL;
}

CDVBSub::DVB_REGION* CDVBSub::FindRegion(DVB_PAGE* pPage, BYTE bRegionId)
{
	if (pPage != NULL) {
		for (int i=0; i<pPage->RegionCount; i++) {
			if (pPage->Regions[i].Id == bRegionId) {
				return &pPage->Regions[i];
			}
		}
	}
	return NULL;
}

CDVBSub::DVB_CLUT* CDVBSub::FindClut(DVB_PAGE* pPage, BYTE bClutId)
{
	if (pPage != NULL) {
		POSITION pos = pPage->Clut.GetHeadPosition();

		while (pos) {
			DVB_CLUT Clut = pPage->Clut.GetAt(pos);

			if (Clut.id == bClutId) {
				return &pPage->Clut.GetAt(pos);
			}
			pPage->Clut.GetNext(pos);
		}
	}
	return NULL;
}

CompositionObject* CDVBSub::FindObject(DVB_PAGE* pPage, SHORT sObjectId)
{
	if (pPage != NULL) {
		POSITION pos = pPage->Objects.GetHeadPosition();

		while (pos) {
			CompositionObject* pObject = pPage->Objects.GetAt (pos);

			if (pObject->m_object_id_ref == sObjectId) {
				return pObject;
			}

			pPage->Objects.GetNext(pos);
		}
	}
	return NULL;
}

HRESULT CDVBSub::AddToBuffer(BYTE* pData, int nSize)
{
	bool bFirstChunk = (*((LONG*)pData) & 0x00FFFFFF) == 0x000f0020;	// DVB sub start with 0x20 0x00 0x0F ...

	if (m_nBufferWritePos > 0 || bFirstChunk) {
		if (bFirstChunk) {
			m_nBufferWritePos	= 0;
			m_nBufferReadPos	= 0;
		}

		if (m_nBufferWritePos+nSize > m_nBufferSize) {
			if (m_nBufferWritePos+nSize > 20*BUFFER_CHUNK_GROW) {
				// Too big to be a DVB sub !
				TRACE_DVB (_T("DVB - Too much data receive...\n"));
				ASSERT (FALSE);

				Reset();
				return E_INVALIDARG;
			}

			BYTE* pPrev		= m_pBuffer;
			m_nBufferSize	= max (m_nBufferWritePos+nSize, m_nBufferSize+BUFFER_CHUNK_GROW);
			m_pBuffer		= DNew BYTE[m_nBufferSize];
			if (pPrev != NULL) {
				memcpy_s (m_pBuffer, m_nBufferSize, pPrev, m_nBufferWritePos);
				SAFE_DELETE (pPrev);
			}
		}
		memcpy_s (m_pBuffer+m_nBufferWritePos, m_nBufferSize, pData, nSize);
		m_nBufferWritePos += nSize;
		return S_OK;
	}
	return S_FALSE;
}

#define MARKER if(gb.BitRead(1) != 1) {ASSERT(0); return(E_FAIL);}

HRESULT CDVBSub::ParseSample(IMediaSample* pSample)
{
	CheckPointer (pSample, E_POINTER);
	HRESULT				hr;
	BYTE*				pData = NULL;
	int					nSize;
	DVB_SEGMENT_TYPE	nCurSegment;

	hr = pSample->GetPointer(&pData);
	if (FAILED(hr) || pData == NULL) {
		return hr;
	}
	nSize = pSample->GetActualDataLength();

	if (*((LONG*)pData) == 0xBD010000) {
		CGolombBuffer	gb (pData, nSize);

		gb.SkipBytes(4);
		WORD	wLength	= (WORD)gb.BitRead(16);
		UNREFERENCED_PARAMETER(wLength);

		if (gb.BitRead(2) != 2) {
			return E_FAIL;    // type
		}

		gb.BitRead(2);	// scrambling
		gb.BitRead(1);	// priority
		gb.BitRead(1);	// alignment
		gb.BitRead(1);	// copyright
		gb.BitRead(1);	// original
		BYTE fpts = (BYTE)gb.BitRead(1);	// fpts
		BYTE fdts = (BYTE)gb.BitRead(1);	// fdts
		gb.BitRead(1);	// escr
		gb.BitRead(1);	// esrate
		gb.BitRead(1);	// dsmtrickmode
		gb.BitRead(1);	// morecopyright
		gb.BitRead(1);	// crc
		gb.BitRead(1);	// extension
		gb.BitRead(8);	// hdrlen

		if (fpts) {
			BYTE b = (BYTE)gb.BitRead(4);
			if (!(fdts && b == 3 || !fdts && b == 2)) {
				ASSERT(0);
				return(E_FAIL);
			}

			REFERENCE_TIME	pts = 0;
			pts |= gb.BitRead(3) << 30;
			MARKER; // 32..30
			pts |= gb.BitRead(15) << 15;
			MARKER; // 29..15
			pts |= gb.BitRead(15);
			MARKER; // 14..0
			pts = 10000*pts/90;

			m_rtStart	= pts;
			m_rtStop	= pts+1;
		} else {
			m_rtStart	= INVALID_TIME;
			m_rtStop	= INVALID_TIME;
		}

		nSize -= 14;
		pData += 14;
		pSample->GetTime(&m_rtStart, &m_rtStop);
		pSample->GetMediaTime(&m_rtStart, &m_rtStop);
	} else if (SUCCEEDED (pSample->GetTime(&m_rtStart, &m_rtStop))) {
		pSample->SetTime(&m_rtStart, &m_rtStop);
	}

	if (AddToBuffer (pData, nSize) == S_OK) {
		CGolombBuffer	gb (m_pBuffer+m_nBufferReadPos, m_nBufferWritePos-m_nBufferReadPos);
		int				nLastPos = 0;

		while (!gb.IsEOF()) {
			if (gb.ReadByte() == 0x0F) {
				WORD	wPageId;
				WORD	wSegLength;

				nCurSegment	= (DVB_SEGMENT_TYPE) gb.ReadByte();
				wPageId		= gb.ReadShort();
				wSegLength	= gb.ReadShort();

				if (gb.RemainingSize() < wSegLength) {
					hr = S_FALSE;
					break;
				}

				TRACE_DVB (_T("DVB - ParseSample, Segment = [%ws], PageId = [%d], SegLength/Buffer = [%d]/[%d]\n"), GetSegmentType(nCurSegment), wPageId, wSegLength, gb.RemainingSize());

				switch (nCurSegment) {
					case PAGE : {
						CAutoPtr<DVB_PAGE>	pPage;
						ParsePage(gb, wSegLength, pPage);

						if (pPage->PageState == DPS_ACQUISITION || pPage->PageState == DPS_MODE) {
							TRACE_DVB (_T("DVB - Page start\n"));

							if (m_pCurrentPage != NULL) {
								m_pCurrentPage->rtStop = min(m_pCurrentPage->rtStop, m_rtStart);
								TRACE_DVB (_T("DVB - store Page : %ws => %ws\n"), ReftimeToString(m_pCurrentPage->rtStart), ReftimeToString(m_pCurrentPage->rtStop));
								m_Pages.AddTail (m_pCurrentPage.Detach());
							}
							UpdateTimeStamp(m_rtStart);

							m_pCurrentPage = pPage;
							m_pCurrentPage->rtStart	= m_rtStart;
							m_pCurrentPage->rtStop	= m_pCurrentPage->rtStart + m_pCurrentPage->PageTimeOut * UNITS; // TODO - need to limit the duration of the segment

							TRACE_DVB (_T("DVB - Page started : %ws, TimeOut = %d\n"), ReftimeToString(m_pCurrentPage->rtStart), m_pCurrentPage->PageTimeOut);
						} else if (pPage->PageState == DPS_NORMAL) {
							TRACE_DVB (_T("DVB - Page update\n"));

							if (m_pCurrentPage && !m_pCurrentPage->RegionCount && pPage->RegionCount) {
								m_pCurrentPage = pPage;
								m_pCurrentPage->rtStart	= m_rtStart;
								m_pCurrentPage->rtStop	= m_pCurrentPage->rtStart + m_pCurrentPage->PageTimeOut * UNITS; // TODO - need to limit the duration of the segment

								TRACE_DVB (_T("DVB - Page started[update] : %ws, TimeOut = %d\n"), ReftimeToString(m_pCurrentPage->rtStart), m_pCurrentPage->PageTimeOut);
							}
						}
					}
					break;
					case REGION :
						ParseRegion(gb, wSegLength);
						TRACE_DVB (_T("DVB - Region\n"));
						break;
					case CLUT :
						ParseClut(gb, wSegLength);
						TRACE_DVB (_T("DVB - Clut\n"));
						break;
					case OBJECT :
						ParseObject(gb, wSegLength);
						TRACE_DVB (_T("DVB - Object\n"));
						break;
					case DISPLAY :
						ParseDisplay(gb, wSegLength);
						TRACE_DVB (_T("DVB - Display\n"));
						break;
					case END_OF_DISPLAY :
						/*
						if (m_pCurrentPage != NULL && (m_pCurrentPage->rtStart != m_rtStart)) {
							m_pCurrentPage->rtStop = max(m_pCurrentPage->rtStop, m_rtStart);
							TRACE_DVB (_T("DVB - End display : %ws => %ws\n"), ReftimeToString(m_pCurrentPage->rtStart), ReftimeToString(m_pCurrentPage->rtStop));
							m_Pages.AddTail (m_pCurrentPage.Detach());
						}
						*/
						TRACE_DVB (_T("DVB - End display\n"));
						break;
					default :
						TRACE_DVB (_T("DVB - unknown Segment\n"));
						break;
				}
				nLastPos = gb.GetPos();
			}
		}
		m_nBufferReadPos += nLastPos;
	}

	return hr;
}

void CDVBSub::Render(SubPicDesc& spd, REFERENCE_TIME rt, RECT& bbox)
{
	DVB_PAGE* pPage = FindPage (rt);

	if (pPage != NULL) {
		pPage->Rendered = true;
		for (int i=0; i<pPage->RegionCount; i++) {
			CDVBSub::DVB_REGION* pRegion = &pPage->Regions[i];
			for (int j=0; j<pRegion->ObjectCount; j++) {
				CompositionObject* pObject = FindObject (pPage, pRegion->Objects[j].object_id);
				if (pObject) {
					SHORT nX, nY;
					nX = pRegion->HorizAddr + pRegion->Objects[j].object_horizontal_position;
					nY = pRegion->VertAddr  + pRegion->Objects[j].object_vertical_position;
					pObject->m_width  = pRegion->width;
					pObject->m_height = pRegion->height;
					CDVBSub::DVB_CLUT* pClut = FindClut(pPage, pRegion->CLUT_id);
					if (pClut != NULL) {
						pObject->SetPalette(pClut->Size, pClut->Palette, m_Display.width > 720);
					}

					TRACE_DVB (_T("CDVBSub::Render() : size = %ld, %d:%d, ObjRes = %dx%d, SPDRes = %dx%d, %I64d = %ws, [%ws => %ws]\n"),
									pObject->GetRLEDataSize(),
									nX, nY,
									pObject->m_width, pObject->m_height, spd.w, spd.h,
									rt, ReftimeToString(rt),
									ReftimeToString(pPage->rtStart), ReftimeToString(pPage->rtStop));

					
					pObject->RenderDvb(spd, nX, nY);
				}
			}
		}

		bbox.left	= 0;
		bbox.top	= 0;
		bbox.right	= m_Display.width < spd.w ? m_Display.width : spd.w;
		ASSERT(spd.h >= 0);
		bbox.bottom	= m_Display.height < spd.h ? m_Display.height : spd.h;
	}
}

HRESULT CDVBSub::GetTextureSize(POSITION pos, SIZE& MaxTextureSize, SIZE& VideoSize, POINT& VideoTopLeft)
{
	MaxTextureSize.cx = VideoSize.cx = m_Display.width;
	MaxTextureSize.cy = VideoSize.cy = m_Display.height;

	VideoTopLeft.x	= 0;
	VideoTopLeft.y	= 0;

	return S_OK;
}

POSITION CDVBSub::GetStartPosition(REFERENCE_TIME rt, double fps, bool CleanOld)
{
	if (CleanOld) {
		CDVBSub::CleanOld(rt);
	}

	return m_Pages.GetHeadPosition();
}

POSITION CDVBSub::GetNext(POSITION pos)
{
	m_Pages.GetNext(pos);
	return pos;
}


REFERENCE_TIME CDVBSub::GetStart(POSITION nPos)
{
	DVB_PAGE* pPage = m_Pages.GetAt(nPos);
	return pPage != NULL ? pPage->rtStart : INVALID_TIME;
}

REFERENCE_TIME CDVBSub::GetStop(POSITION nPos)
{
	DVB_PAGE* pPage = m_Pages.GetAt(nPos);
	return pPage != NULL ? pPage->rtStop : INVALID_TIME;
}

void CDVBSub::Reset()
{
	m_nBufferReadPos	= 0;
	m_nBufferWritePos	= 0;
	m_pCurrentPage.Free();

	DVB_PAGE* pPage;
	while (m_Pages.GetCount() > 0) {
		pPage = m_Pages.RemoveHead();
		delete pPage;
	}

}

HRESULT CDVBSub::ParsePage(CGolombBuffer& gb, WORD wSegLength, CAutoPtr<DVB_PAGE>& pPage)
{
	int		nEnd	= gb.GetPos() + wSegLength;
	int		nPos	= 0;

	pPage.Attach (DNew DVB_PAGE());
	pPage->PageTimeOut			= gb.ReadByte();
	pPage->PageVersionNumber	= (BYTE)gb.BitRead(4);
	pPage->PageState			= (BYTE)gb.BitRead(2);
	pPage->RegionCount			= 0;
	gb.BitRead(2);	// Reserved
	while (gb.GetPos() < nEnd) {
		if (nPos < MAX_REGIONS) {
			pPage->Regions[nPos].Id			= gb.ReadByte();
			gb.ReadByte();					// Reserved
			pPage->Regions[nPos].HorizAddr	= gb.ReadShort();
			pPage->Regions[nPos].VertAddr	= gb.ReadShort();
			pPage->RegionCount++;
		}
		nPos++;
	}

	return S_OK;
}

HRESULT CDVBSub::ParseDisplay(CGolombBuffer& gb, WORD wSegLength)
{
	m_Display.version_number		= (BYTE)gb.BitRead (4);
	m_Display.display_window_flag	= (BYTE)gb.BitRead (1);
	gb.BitRead(3);	// reserved
	m_Display.width					= gb.ReadShort() + 1;
	m_Display.height				= gb.ReadShort() + 1;
	if (m_Display.display_window_flag) {
		m_Display.horizontal_position_minimun	= gb.ReadShort();
		m_Display.horizontal_position_maximum	= gb.ReadShort();
		m_Display.vertical_position_minimun		= gb.ReadShort();
		m_Display.vertical_position_maximum		= gb.ReadShort();
	}

	return S_OK;
}

HRESULT CDVBSub::ParseRegion(CGolombBuffer& gb, WORD wSegLength)
{
	int						nEnd	= gb.GetPos() + wSegLength;
	CDVBSub::DVB_REGION*	pRegion;
	CDVBSub::DVB_REGION		DummyRegion;

	pRegion = FindRegion (m_pCurrentPage, gb.ReadByte());

	if (pRegion == NULL) {
		pRegion = &DummyRegion;
	}

	if (pRegion != NULL) {
		pRegion->version_number			= (BYTE)gb.BitRead(4);
		pRegion->fill_flag				= (BYTE)gb.BitRead(1);
		gb.BitRead(3);	// Reserved
		pRegion->width					= gb.ReadShort();
		pRegion->height					= gb.ReadShort();
		pRegion->level_of_compatibility	= (BYTE)gb.BitRead(3);
		pRegion->depth					= (BYTE)gb.BitRead(3);
		gb.BitRead(2);	// Reserved
		pRegion->CLUT_id				= gb.ReadByte();
		pRegion->_8_bit_pixel_code		= gb.ReadByte();
		pRegion->_4_bit_pixel_code		= (BYTE)gb.BitRead(4);
		pRegion->_2_bit_pixel_code		= (BYTE)gb.BitRead(2);
		gb.BitRead(2);	// Reserved

		pRegion->ObjectCount = 0;
		while (gb.GetPos() < nEnd) {
			DVB_OBJECT*		pObject = &pRegion->Objects[pRegion->ObjectCount];
			pObject->object_id					= gb.ReadShort();
			pObject->object_type				= (BYTE)gb.BitRead(2);
			pObject->object_provider_flag		= (BYTE)gb.BitRead(2);
			pObject->object_horizontal_position	= (SHORT)gb.BitRead(12);
			gb.BitRead(4);	// Reserved
			pObject->object_vertical_position	= (SHORT)gb.BitRead(12);
			if (pObject->object_type == 0x01 || pObject->object_type == 0x02) {
				pObject->foreground_pixel_code	= gb.ReadByte();
				pObject->background_pixel_code	= gb.ReadByte();
			}
			pRegion->ObjectCount++;
		}
	} else {
		gb.SkipBytes (wSegLength-1);
	}

	return S_OK;
}

HRESULT CDVBSub::ParseClut(CGolombBuffer& gb, WORD wSegLength)
{
	HRESULT				hr		= S_OK;
	int					nEnd	= gb.GetPos() + wSegLength;
	BYTE				CLUT_id	= gb.ReadByte();

	if (m_pCurrentPage != NULL) {

		CDVBSub::DVB_CLUT	DummyClut;
		CDVBSub::DVB_CLUT*	pClut = FindClut(m_pCurrentPage, CLUT_id);
		if (pClut == NULL) {
			pClut = &DummyClut;
		}

		pClut->id				= CLUT_id;
		pClut->version_number	= (BYTE)gb.BitRead(4);
		gb.BitRead(4);	// Reserved

		pClut->Size = 0;
		while (gb.GetPos() < nEnd) {
			BYTE entry_id	= gb.ReadByte();
			BYTE _2_bit		= (BYTE)gb.BitRead(1);
			BYTE _4_bit		= (BYTE)gb.BitRead(1);
			BYTE _8_bit		= (BYTE)gb.BitRead(1);
			UNREFERENCED_PARAMETER(_2_bit);
			UNREFERENCED_PARAMETER(_4_bit);
			UNREFERENCED_PARAMETER(_8_bit);
			gb.BitRead(4);	// Reserved

			pClut->Palette[entry_id].entry_id = entry_id;
			if (gb.BitRead(1)) {
				pClut->Palette[entry_id].Y	= gb.ReadByte();
				pClut->Palette[entry_id].Cr	= gb.ReadByte();
				pClut->Palette[entry_id].Cb	= gb.ReadByte();
				pClut->Palette[entry_id].T	= 0xff-gb.ReadByte();
			} else {
				pClut->Palette[entry_id].Y	= (BYTE)gb.BitRead(6)<<2;
				pClut->Palette[entry_id].Cr	= (BYTE)gb.BitRead(4)<<4;
				pClut->Palette[entry_id].Cb	= (BYTE)gb.BitRead(4)<<4;
				pClut->Palette[entry_id].T	= 0xff-((BYTE)gb.BitRead(2)<<6);
			}
			if (!pClut->Palette[entry_id].Y) {
				pClut->Palette[entry_id].Cr	= 0;
				pClut->Palette[entry_id].Cb	= 0;
				pClut->Palette[entry_id].T	= 0;
			}

			pClut->Size = max (pClut->Size, entry_id + 1);
		}

		if (DummyClut.Size) {
			m_pCurrentPage->Clut.AddTail(DummyClut);
		}
	}	

	return hr;
}

HRESULT CDVBSub::ParseObject(CGolombBuffer& gb, WORD wSegLength)
{
	HRESULT hr = E_FAIL;

	if (m_pCurrentPage && wSegLength > 2) {
		CompositionObject*	pObject = DNew CompositionObject();
		BYTE				object_coding_method;

		pObject->m_object_id_ref	= gb.ReadShort();
		pObject->m_version_number	= (BYTE)gb.BitRead(4);

		object_coding_method = (BYTE)gb.BitRead(2);	// object_coding_method
		gb.BitRead(1);	// non_modifying_colour_flag
		gb.BitRead(1);	// reserved

		if (object_coding_method == 0x00) {
			pObject->SetRLEData (gb.GetBufferPos(), wSegLength-3, wSegLength-3);
			gb.SkipBytes(wSegLength-3);
			m_pCurrentPage->Objects.AddTail (pObject);
			hr = S_OK;
		} else {
			delete pObject;
			hr = E_NOTIMPL;
		}
	}

	return hr;
}

HRESULT CDVBSub::UpdateTimeStamp(REFERENCE_TIME rtStop)
{
	HRESULT hr = E_FAIL;
	POSITION pos = m_Pages.GetHeadPosition();
	while (pos) {
		DVB_PAGE* pPage = m_Pages.GetAt (pos);
		if (pPage->rtStop > rtStop) {
			pPage->rtStop = rtStop;
			hr = S_OK;
		}
		m_Pages.GetNext(pos);
	}

	return hr;
}

void CDVBSub::CleanOld(REFERENCE_TIME rt)
{
	// Cleanup old PG
	DVB_PAGE* pPage_old;
	while (m_Pages.GetCount()>0) {
		pPage_old = m_Pages.GetHead();
		if (pPage_old->rtStop < rt) {
			if (!pPage_old->Rendered) {
				TRACE_DVB (_T("DVB - remove unrendered object, %ws => %ws, (rt = %ws)\n"),
							ReftimeToString(pPage_old->rtStart), ReftimeToString(pPage_old->rtStop),
							ReftimeToString(rt));
			}
			m_Pages.RemoveHead();
			delete pPage_old;
		} else {
			break;
		}
	}
}

HRESULT CDVBSub::EndOfStream()
{
	if (m_pCurrentPage != NULL) {
		TRACE_DVB (_T("DVB - EndOfStream() : %ws => %ws\n"), ReftimeToString(m_pCurrentPage->rtStart), ReftimeToString(m_pCurrentPage->rtStop));
		m_Pages.AddTail (m_pCurrentPage.Detach());
	}

	return S_OK;
}