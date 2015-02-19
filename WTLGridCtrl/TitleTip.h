//////////////////////////////////////////////////////////////////////////
//
// WTL Port Of the MFCGridCtrl by Nicola Tufarelli
//
// rel. 0.0.1 25-04-2005
// WTL version of the control
//

/////////////////////////////////////////////////////////////////////////////
// Titletip.h : header file
//
// MFC Grid Control - cell titletips
//
// Written by Chris Maunder <cmaunder@mail.com>
// Copyright (c) 1998-2001. All Rights Reserved.
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
//////////////////////////////////////////////////////////////////////

#ifndef __TITLETIP_H__
#define __TITLETIP_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define TITLETIP_CLASSNAME _T("ZTitleTip")

/////////////////////////////////////////////////////////////////////////////
// CTitleTip window

class CTitleTip : public CWindowImpl<CTitleTip>
{
// Construction
public:
	DECLARE_WND_CLASS(TITLETIP_CLASSNAME)

	typedef CWindowImpl< CTitleTip, CWindow, CControlWinTraits >  __baseClass;

	CTitleTip();
	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CTitleTip)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	END_MSG_MAP()

	BOOL Create(HWND hParentWnd);
	void _Init()
	{
		ATLASSERT(IsWindow());

		m_dwLastLButtonDown = ULONG_MAX;
		m_dwDblClickMsecs   = GetDoubleClickTime();
	}

	// Generated message map functions
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
		_Init();
		return lRes;
	}

// Attributes
public:
    void SetParentWnd(HWND hParentWnd)  { m_ParentWnd = hParentWnd; }
    CWindow* GetParentWnd()             { return &m_ParentWnd;       }

// Operations
public:
	void Show(CRect rectTitle, LPCTSTR lpszTitleText, 
              int xoffset = 0, LPRECT lpHoverRect = NULL, 
              const LOGFONT* lpLogFont = NULL,
              COLORREF crTextClr = CLR_DEFAULT,
			  COLORREF crBackClr = CLR_DEFAULT);

    void Hide();
	BOOL DestroyWindow();

// Implementation
protected:
	CWindow m_ParentWnd;
	CRect	m_rectTitle;
    CRect	m_rectHover;
    DWORD	m_dwLastLButtonDown;
    DWORD	m_dwDblClickMsecs;
    BOOL	m_bCreated;
};


#endif //__TITLETIP_H__