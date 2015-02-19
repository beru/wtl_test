//////////////////////////////////////////////////////////////////////////
//
// WTL Port Of the MFCGridCtrl by Nicola Tufarelli
//
// rel. 0.0.1 25-04-2005
// WTL version of the control
//


//////////////////////////////////////////////////////////////////////////
// InPlaceEdit.cpp : implementation file
//
// Adapted by Chris Maunder <cmaunder@mail.com>
// Copyright (c) 1998-2002. All Rights Reserved.
//
// The code contained in this file is based on the original
// CInPlaceEdit from http://www.codeguru.com/listview/edit_subitems.shtml
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name and all copyright 
// notices remains intact. 
//
// An email letting me know how you are using it would be nice as well. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// For use with CGridCtrl v2.10+
//
// History:
//         10 May 1998  Uses GVN_ notifications instead of LVN_,
//                      Sends notification messages to the parent, 
//                      instead of the parent's parent.
//         15 May 1998  There was a problem when editing with the in-place editor, 
//                      there arises a general protection fault in user.exe, with a 
//                      few qualifications:
//                         (1) This only happens with owner-drawn buttons;
//                         (2) This only happens in Win95
//                         (3) This only happens if the handler for the button does not 
//                             create a new window (even an AfxMessageBox will avoid the 
//                             crash)
//                         (4) This will not happen if Spy++ is running.
//                      PreTranslateMessage was added to route messages correctly.
//                      (Matt Weagle found and fixed this problem)
//         26 Jul 1998  Removed the ES_MULTILINE style - that fixed a few probs!
//          6 Aug 1998  Added nID to the constructor param list
//          6 Sep 1998  Space no longer clears selection when starting edit (Franco Bez)
//         10 Apr 1999  Enter, Tab and Esc key prob fixed (Koay Kah Hoe)
//                      Workaround for bizzare "shrinking window" problem in CE
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "GridCtrl.h"
#include "InPlaceEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit

BOOL CInPlaceEdit::SubclassWindow(HWND hWnd)
{
	ATLASSERT(m_hWnd==NULL);
	ATLASSERT(::IsWindow(hWnd));
#ifdef _DEBUG
	// Check class
	TCHAR szBuffer[16] = { 0 };
	if (::GetClassName(hWnd, szBuffer, (sizeof(szBuffer)/sizeof(TCHAR))-1))
	{
		ATLASSERT(::lstrcmpi(szBuffer, CEdit::GetWndClassName()) == 0);
	}
#endif // _DEBUG
	BOOL bRet = __baseClass::SubclassWindow(hWnd);

	if (bRet)
		_Init();

	return bRet;
}

void CInPlaceEdit::EndEdit()
{
	CString str;

	// EFW - BUG FIX - Clicking on a grid scroll bar in a derived class
	// that validates input can cause this to get called multiple times
	// causing assertions because the edit control goes away the first time.
	static BOOL bAlreadyEnding = FALSE;

	if(bAlreadyEnding)
		return;

	bAlreadyEnding = TRUE;

#if ( _WTL_VER < 0x0750 )
	CComBSTR bsTemp;
	GetWindowText(&bsTemp);
	str = bsTemp;
#else
	GetWindowText(str);
#endif

	// Send Notification to parent
	GV_DISPINFO dispinfo;

	dispinfo.hdr.hwndFrom = m_hWnd;
	dispinfo.hdr.idFrom   = GetDlgCtrlID();
	dispinfo.hdr.code     = GVN_ENDLABELEDIT;

	dispinfo.item.mask    = LVIF_TEXT|LVIF_PARAM;
	dispinfo.item.row     = m_nRow;
	dispinfo.item.col     = m_nColumn;
	dispinfo.item.strText  = str;
	dispinfo.item.lParam  = (LPARAM)m_nLastChar;

	SendMessage(GetParent(), WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dispinfo );

	// Close this window (PostNcDestroy will delete this)
	if (IsWindow())
		SendMessage(WM_CLOSE, 0, 0);

	bAlreadyEnding = FALSE;
}

