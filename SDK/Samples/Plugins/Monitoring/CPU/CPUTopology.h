// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#pragma once
/////////////////////////////////////////////////////////////////////////////
#ifdef _WIN64
#define MAX_LOGICAL_CPU 64
#else
#define MAX_LOGICAL_CPU 32
#endif
/////////////////////////////////////////////////////////////////////////////
class CCPUTopology
{
public:
	CCPUTopology();
	virtual ~CCPUTopology();

	BOOL	Init();
	void	Uninit();

	DWORD	GetPckg(DWORD dwCpu);
	DWORD	GetCore(DWORD dwCpu);

	LPBYTE	GetInfo();
	DWORD	GetSize();

protected:
	LPBYTE	m_lpInfo;
	DWORD	m_dwSize;

	BYTE	m_pckgMap[MAX_LOGICAL_CPU];
	BYTE	m_coreMap[MAX_LOGICAL_CPU];
};
/////////////////////////////////////////////////////////////////////////////
