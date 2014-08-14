/*****************************************************************
|
|    AP4 - Fragment Sample Table
|
|    Copyright 2014 Aleksoid1978
|
 ****************************************************************/
#ifndef _AP4_FRAGMENTSAMPLE_TABLE_H_
#define _AP4_FRAGMENTSAMPLE_TABLE_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4Sample.h"
#include "Ap4Array.h"
#include "Ap4TfhdAtom.h"
#include "Ap4TrexAtom.h"
#include "Ap4TrunAtom.h"

/*----------------------------------------------------------------------
|       Ap4_FragmentSampleTable
+---------------------------------------------------------------------*/
class Ap4_FragmentSampleTable {
public:
    // constructor
	Ap4_FragmentSampleTable();

	// methods
	AP4_Result   AddTrun(AP4_TrunAtom* trun,
						 AP4_TfhdAtom* tfhd,
						 AP4_TrexAtom* trex,
						 AP4_ByteStream& stream,
						 AP4_UI64& dts_origin,
						 AP4_Offset moof_offset,
						 AP4_Offset& mdat_payload_offset);
	AP4_Result   GetSample(AP4_Ordinal index, AP4_Sample& sample);
	AP4_Cardinal GetSampleCount() { return m_FragmentSamples.ItemCount(); }
	AP4_Duration GetDuration()    { return m_Duration; }

	AP4_Result   GetSampleIndexForTimeStamp(AP4_TimeStamp ts,
											AP4_Ordinal& index);

private:
	// members
	AP4_Array<AP4_Sample> m_FragmentSamples;
	AP4_Duration m_Duration;
};

#endif // _AP4_FRAGMENTSAMPLE_TABLE_H_
