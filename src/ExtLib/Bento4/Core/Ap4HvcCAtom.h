/*****************************************************************
|
|    AP4 - hvcC Atom
|
|    Copyright 2002 Gilles Boccon-Gibod & Julien Boeuf
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

#ifndef _AP4_HVCC_ATOM_H_
#define _AP4_HVCC_ATOM_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4Atom.h"
#include "Ap4Types.h"
#include "Ap4Array.h"
#include "Ap4DataBuffer.h"

/*----------------------------------------------------------------------
|       AP4_HvcCAtom
+---------------------------------------------------------------------*/
class AP4_HvcCAtom : public AP4_Atom
{
public:
	AP4_HvcCAtom(AP4_Size         size,
                 AP4_ByteStream&  stream);

    AP4_Result WriteFields(AP4_ByteStream& stream) { return AP4_FAILURE; }

    const AP4_DataBuffer* GetDecoderInfo() const { return &m_DecoderInfo; }

private:
    AP4_DataBuffer m_DecoderInfo;
};

#endif // _AP4_HVCC_ATOM_H_
