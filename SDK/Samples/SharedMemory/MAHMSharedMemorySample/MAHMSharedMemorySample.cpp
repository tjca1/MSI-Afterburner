// MAHMSharedMemorySample.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MAHMSharedMemorySample.h"
#include "MAHMSharedMemorySampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMAHMSharedMemorySampleApp

BEGIN_MESSAGE_MAP(CMAHMSharedMemorySampleApp, CWinApp)
	//{{AFX_MSG_MAP(CMAHMSharedMemorySampleApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMAHMSharedMemorySampleApp construction

CMAHMSharedMemorySampleApp::CMAHMSharedMemorySampleApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMAHMSharedMemorySampleApp object

CMAHMSharedMemorySampleApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMAHMSharedMemorySampleApp initialization

BOOL CMAHMSharedMemorySampleApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	CMAHMSharedMemorySampleDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
