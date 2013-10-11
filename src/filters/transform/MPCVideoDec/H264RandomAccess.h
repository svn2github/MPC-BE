/*
 * 
 *      Copyright (C) 2010-2013 Hendrik Leppkes
 *      http://www.1f0.de
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  Adaptation for MPC-BE (C) 2013 Alexandr Vodiannikov aka "Aleksoid1978" (Aleksoid1978@mail.ru)
 *
 */

#pragma once

struct AVFrame;
class CGolombBuffer;

class CH264RandomAccess
{
public:
	CH264RandomAccess();
	~CH264RandomAccess();

	void flush(int threadCount);
	BOOL searchRecoveryPoint(const BYTE *buf, size_t buf_size);
	void judgeFrameUsability(AVFrame *pFrame, int *got_picture_ptr);

	void SetAVCNALSize(int avcNALSize) { m_AVCNALSize = avcNALSize; }

private:
	int decode_sei_recovery_point(CGolombBuffer *pGB);
	int parseForRecoveryPoint(const BYTE *buf, size_t buf_size, int *recoveryFrameCount);

private:
	int m_RecoveryMode; // 0: OK; 1: Searching; 2: Found; 3: 
	int m_RecoveryFrameCount;
	int m_RecoveryPOC;
	int m_ThreadDelay;

	int m_AVCNALSize;
};
