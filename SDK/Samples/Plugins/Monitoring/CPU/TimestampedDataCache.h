// TimestampedDataCache.h: interface for the CTextTablesList class.
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#pragma once
/////////////////////////////////////////////////////////////////////////////
#include "afxtempl.h"
/////////////////////////////////////////////////////////////////////////////
typedef struct TIMESTAMPED_DATA
{
	DWORD	dwData;
	DWORD	dwTimestamp;
} TIMESTAMPED_DATA, *LPTIMESTAMPED_DATA;
/////////////////////////////////////////////////////////////////////////////
class CTimestampedDataCache : public CMap<DWORD, DWORD, LPTIMESTAMPED_DATA, LPTIMESTAMPED_DATA>
{
public:
	LPTIMESTAMPED_DATA SetData(DWORD dwKey, DWORD dwTimestamp, DWORD dwData);
	LPTIMESTAMPED_DATA GetData(DWORD dwKey, DWORD dwTimestamp);

	void Destroy();
	
	CTimestampedDataCache();
	virtual ~CTimestampedDataCache();
};
/////////////////////////////////////////////////////////////////////////////
