//////////////////////////////////////////////////////////////////////////
//
// WTL Port Of the MFCGridCtrl by Nicola Tufarelli
//
// rel. 0.0.1 25-04-2005
// WTL version of the control
//

//////////////////////////////////////////////////////////////////////
// InPlaceEdit.h : header file
//
// MFC Grid Control - inplace editing class
//
// Written by Chris Maunder <cmaunder@mail.com>
// Copyright (c) 1998-2002. All Rights Reserved.
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

#ifndef __INPLACEEDIT_H__
#define __INPLACEEDIT_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#ifndef GVN_ENDLABELEDIT
#define GVN_ENDLABELEDIT        LVN_ENDLABELEDIT     // LVN_FIRST-6
#endif

#ifndef UNUSED_ALWAYS
#define UNUSED_ALWAYS(x) x
#endif


//////////////////////////////////////////////////////////////////////////
class CInPlaceEdit : public CWindowImpl< CInPlaceEdit, CEdit, CControlWinTraits >
{
public:
	CInPlaceEdit()
		:
		m_hFont(NULL)
	{
	}

	DECLARE_WND_SUPERCLASS(_T("WTL_InPlaceEdit"), GetWndClassName())  

	typedef CWindowImpl< CInPlaceEdit, CEdit, CControlWinTraits >  __baseClass;

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	BEGIN_MSG_MAP(CInPlaceEdit)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
	END_MSG_MAP()

	// Operations
	BOOL SubclassWindow(HWND hWnd);

	// Implementation

	void _Init()
	{
		ATLASSERT(IsWindow());
		//ATLASSERT((GetStyle() & (ES_AUTOHSCROLL|ES_UPPERCASE|ES_LOWERCASE|ES_PASSWORD|ES_READONLY|ES_MULTILINE))==0);

		//ModifyStyle(ES_AUTOHSCROLL,0);
	}

	HWND Create(HWND hWndParent, CRect& rect, DWORD dwStyle, UINT nID,
				int nRow, int nColumn, CString sInitText, UINT nFirstChar)
	{
		m_sInitText     = sInitText;
		m_nRow          = nRow;
		m_nColumn       = nColumn;
		m_nLastChar     = 0; 
		m_bExitOnArrows = (nFirstChar != VK_LBUTTON);    // If mouse click brought us here,
		// then no exit on arrows

		m_Rect = rect;  // For bizarre CE bug.

		DWORD dwEditStyle = WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | dwStyle;
		m_hWnd = __baseClass::Create(hWndParent, rect, NULL, dwEditStyle, 0, nID);

		CWindow parent = GetParent();
		if (m_hFont == NULL)
			SetFont(parent.GetFont());
		else
			SetFont(m_hFont);

		SetWindowText(sInitText);
		SetFocus();

		switch (nFirstChar)
		{
			case VK_LBUTTON: 
			case VK_RETURN:
				SetSel((int)_tcslen(m_sInitText), -1);
				return m_hWnd;
			case VK_BACK:
				SetSel((int)_tcslen(m_sInitText), -1);
				break;
			case VK_TAB:
			case VK_DOWN: 
			case VK_UP:   
			case VK_RIGHT:
			case VK_LEFT:  
			case VK_NEXT:  
			case VK_PRIOR: 
			case VK_HOME:
			case VK_SPACE:
			case VK_END:
				SetSel(0,-1);
				return m_hWnd;
			default:
				SetSel(0,-1);
		}

		// Added by KiteFly. When entering DBCS chars into cells the first char was being lost
		// SenMessage changed to PostMessage (John Lagerquist)
		if( nFirstChar < 0x80)
			PostMessage(WM_CHAR, nFirstChar);   
		else
			PostMessage(WM_IME_CHAR, nFirstChar);

		return m_hWnd;
	}

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	// Message Handlers

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
		_Init();
		return lRes;
	}

	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		UNUSED_ALWAYS(uMsg);
		UNUSED_ALWAYS(lParam);
		UNUSED_ALWAYS(wParam);

		EndEdit();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRes = 0;
		if (::GetFocus() != m_hWnd)
			return DefWindowProc(uMsg, wParam, lParam);

		TCHAR ch = (TCHAR)wParam;
		if (ch == VK_TAB || ch == VK_RETURN)
		{
			m_nLastChar = ch;
			::SetFocus(GetParent());    // This will destroy this window
			return lRes;
		}

		if (ch == VK_ESCAPE) 
		{
			SetWindowText(m_sInitText);    // restore previous text
			m_nLastChar = ch;
			::SetFocus(GetParent());    // This will destroy this window
			return lRes;
		}

		lRes = DefWindowProc(uMsg, wParam, lParam);

		// Resize edit control if needed
		// Get text extent
		CComBSTR bs;
		CString str;
		GetWindowText(&bs);

		str = bs;

		// add some extra buffer
		str += _T("  ");

		CWindowDC dc(m_hWnd);
		CFont FontDC = dc.SelectFont(GetFont());
		CSize size;
		dc.GetTextExtent(str, str.GetLength(), &size);
		dc.SelectFont(FontDC);

		// Get client rect
		CRect ParentRect;
		::GetClientRect(GetParent(), &ParentRect );

		// Check whether control needs to be resized
		// and whether there is space to grow
		if (size.cx > m_Rect.Width())
		{
			if( size.cx + m_Rect.left < ParentRect.right )
				m_Rect.right = m_Rect.left + size.cx;
			else
				m_Rect.right = ParentRect.right;
			MoveWindow(&m_Rect);
		}

		bHandled = TRUE;
		return lRes;
	}

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		UNUSED_ALWAYS(uMsg);
		UNUSED_ALWAYS(lParam);

		TCHAR ch = (TCHAR)wParam;

		if ((ch == VK_PRIOR || ch == VK_NEXT ||
			ch == VK_DOWN  || ch == VK_UP   ||
			ch == VK_RIGHT || ch == VK_LEFT) &&
			(m_bExitOnArrows || (::GetKeyState(VK_CONTROL) & 0x8000)))
		{
			m_nLastChar = ch;
			::SetFocus(GetParent());
			return 0;
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		return DefWindowProc(uMsg, wParam, lParam) | DLGC_WANTALLKEYS;
	}

	// Operations
public:
	void EndEdit();
	HFONT m_hFont;

private:
	int     m_nRow;
	int     m_nColumn;
	CString m_sInitText;
	UINT    m_nLastChar;
	BOOL    m_bExitOnArrows;
	CRect   m_Rect;
};


#endif //__INPLACEEDIT_H__