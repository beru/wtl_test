//////////////////////////////////////////////////////////////////////////
//
// WTL Port Of the MFCGridCtrl by Nicola Tufarelli
//
// rel. 0.0.1 25-04-2005
// WTL version of the control
//

////////////////////////////////////////////////////////////////////////////
// TitleTip.cpp : implementation file
//
// Based on code by Zafir Anjum
//
// Adapted by Chris Maunder <cmaunder@mail.com>
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
// For use with CGridCtrl v2.20+
//
// History
//         10 Apr 1999  Now accepts a LOGFONT pointer and 
//                      a tracking rect in Show(...)  (Chris Maunder)
//         18 Apr 1999  Resource leak in Show fixed by Daniel Gehriger
//          8 Mar 2000  Added double-click fix found on codeguru
//                      web site but forgot / can't find who contributed it
//         28 Mar 2000  Aqiruse (marked with //FNA)
//                      Titletips now use cell color
//         18 Jun 2000  Delayed window creation added
//
/////////////////////////////////////////////////////////////////////////////
 
#include "stdafx.h"
#include "gridctrl.h"

#ifndef GRIDCONTROL_NO_TITLETIPS

#include "TitleTip.h"


/////////////////////////////////////////////////////////////////////////////
// CTitleTip

CTitleTip::CTitleTip() : __baseClass()
{
    m_dwLastLButtonDown = ULONG_MAX;
    m_dwDblClickMsecs   = GetDoubleClickTime();
    m_bCreated          = FALSE;
    m_ParentWnd			= NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CTitleTip message handlers

BOOL CTitleTip::Create(HWND hParentWnd)
{
	ATLASSERT(::IsWindow(hParentWnd));

    // Already created?
    if (m_bCreated)
        return TRUE;

	DWORD dwStyle = WS_BORDER | WS_POPUP; 
	DWORD dwExStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
	m_ParentWnd = hParentWnd;

	m_hWnd = __baseClass::Create(m_ParentWnd, rcDefault, NULL, dwStyle, dwExStyle);

	m_bCreated = IsWindow();

	_Init();

    return m_bCreated;
}

BOOL CTitleTip::DestroyWindow() 
{
    m_bCreated = FALSE;
	
	return __baseClass::DestroyWindow();
}

// Show 		 - Show the titletip if needed
// rectTitle	 - The rectangle within which the original 
//				    title is constrained - in client coordinates
// lpszTitleText - The text to be displayed
// xoffset		 - Number of pixel that the text is offset from
//				   left border of the cell
void CTitleTip::Show(CRect rectTitle,
					 LPCTSTR lpszTitleText,
					 int xoffset /*=0*/,
                     LPRECT lpHoverRect /*=NULL*/,
                     const LOGFONT* lpLogFont /*=NULL*/,
                     COLORREF crTextClr /* CLR_DEFAULT */,
                     COLORREF crBackClr /* CLR_DEFAULT */)
{
    if (!IsWindow())
        Create(m_ParentWnd);

	ATLASSERT(IsWindow() );

    if (rectTitle.IsRectEmpty())
        return;

	// If titletip is already displayed, don't do anything.
	if (IsWindowVisible())
		return;

    m_rectHover = (lpHoverRect != NULL) ? lpHoverRect : rectTitle;
    m_rectHover.right++;
	m_rectHover.bottom++;

	m_ParentWnd.ClientToScreen( m_rectHover );
    ScreenToClient( m_rectHover );

	// Do not display the titletip is app does not have focus
	if (GetFocus() == NULL)
		return;

	// Define the rectangle outside which the titletip will be hidden.
	// We add a buffer of one pixel around the rectangle
	m_rectTitle.top    = - 1;
	m_rectTitle.left   = - xoffset - 1;
	m_rectTitle.right  = rectTitle.Width() - xoffset;
	m_rectTitle.bottom = rectTitle.Height() + 1;

	// Determine the width of the text
	m_ParentWnd.ClientToScreen( rectTitle );

	CClientDC dc(m_hWnd);
	CString strTitle = _T("");
    strTitle += _T(" ");
    strTitle += lpszTitleText; 
    strTitle += _T(" ");

	CFont font, oldFont = NULL;
    if (lpLogFont)
    {
        font.CreateFontIndirect(lpLogFont);
	    oldFont = dc.SelectFont( font );
    }
    else
    {
        // use same font as ctrl
	    oldFont = dc.SelectFont( m_ParentWnd.GetFont() );
    }

	CSize size;
	dc.GetTextExtent(strTitle, -1, &size);

    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);
    size.cx += tm.tmOverhang;

	CRect rectDisplay = rectTitle;
	rectDisplay.left += xoffset;
	rectDisplay.right = rectDisplay.left + size.cx + xoffset;
    
    // Do not display if the text fits within available space
    if ( rectDisplay.right > rectTitle.right-xoffset )
    {
		// Show the titletip
        SetWindowPos(HWND_TOP, rectDisplay.left, rectDisplay.top, 
            rectDisplay.Width(), rectDisplay.Height(), 
            SWP_SHOWWINDOW|SWP_NOACTIVATE );
        
        // FNA - handle colors correctly
        if (crBackClr != CLR_DEFAULT)
        {
		    CBrush backBrush;
			backBrush.CreateSolidBrush(crBackClr);
		    CBrush oldBrush = dc.SelectBrush(backBrush);
		    CRect rect;
		    dc.GetClipBox(&rect);     // Erase the area needed 

		    dc.PatBlt(rect.left, rect.top, rect.Width(), rect.Height(),  PATCOPY);
		    dc.SelectBrush(oldBrush);
	    }
        // Set color
        if (crTextClr != CLR_DEFAULT)//FNA
            dc.SetTextColor(crTextClr);//FA

        dc.SetBkMode( TRANSPARENT );
        dc.TextOut( 0, 0, strTitle );
        SetCapture();
    }
    
    dc.SelectFont( oldFont );
}

void CTitleTip::Hide()
{
  	if (!IsWindow())
        return;

    if (GetCapture() == m_hWnd)
        ReleaseCapture();

	ShowWindow( SW_HIDE );
}

LRESULT CTitleTip::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UNUSED_ALWAYS(uMsg);
	UNUSED_ALWAYS(bHandled);

	UINT nFlags = (UINT)wParam;
	CPoint point(lParam);

    if (!m_rectHover.PtInRect(point)) 
    {
        Hide();
        
        // Forward the message
        ClientToScreen( &point );
		HWND hWnd = ::WindowFromPoint( point );
        if (hWnd == m_hWnd)
		{
            hWnd = GetParent();
		}

		CWindow wnd(hWnd);
        
        int hittest = (int)wnd.SendMessage(WM_NCHITTEST,0,MAKELONG(point.x,point.y));
        
        if (hittest == HTCLIENT) {
            wnd.ScreenToClient( &point );
            wnd.PostMessage( WM_MOUSEMOVE, nFlags, MAKELONG(point.x,point.y) );
        } else {
            wnd.PostMessage( WM_NCMOUSEMOVE, hittest, MAKELONG(point.x,point.y) );
        }
    }

	return 0;
}

BOOL CTitleTip::PreTranslateMessage(MSG* pMsg) 
{
    // Used to qualify WM_LBUTTONDOWN messages as double-clicks
    DWORD dwTick=0;
    BOOL bDoubleClick=FALSE;

	int hittest;
	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
       // Get tick count since last LButtonDown
        dwTick = GetTickCount();
        bDoubleClick = ((dwTick - m_dwLastLButtonDown) <= m_dwDblClickMsecs);
        m_dwLastLButtonDown = dwTick;
        // NOTE: DO NOT ADD break; STATEMENT HERE! Let code fall through

	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		{
			POINTS pts = MAKEPOINTS( pMsg->lParam );
			POINT  point;
			point.x = pts.x;
			point.y = pts.y;

			ClientToScreen( &point );
			Hide();

			HWND hWnd = ::WindowFromPoint(point);

			if (hWnd == m_hWnd)
			{
				hWnd = GetParent();
			}
			
			if( hWnd == m_hWnd) 
				hWnd = GetParent();

			CWindow wnd(hWnd);

			hittest = (int)wnd.SendMessage(WM_NCHITTEST,0,MAKELONG(point.x,point.y));

			if (hittest == HTCLIENT) 
			{
				wnd.ScreenToClient( &point );
				pMsg->lParam = MAKELONG(point.x,point.y);
			}
			else 
			{
				switch (pMsg->message)
				{
				case WM_LBUTTONDOWN: 
					pMsg->message = WM_NCLBUTTONDOWN;
					break;
				case WM_RBUTTONDOWN: 
					pMsg->message = WM_NCRBUTTONDOWN;
					break;
				case WM_MBUTTONDOWN: 
					pMsg->message = WM_NCMBUTTONDOWN;
					break;
				}

				pMsg->wParam = hittest;
				pMsg->lParam = MAKELONG(point.x,point.y);
			}


			// If this is the 2nd WM_LBUTTONDOWN in x milliseconds,
			// post a WM_LBUTTONDBLCLK message instead of a single click.
			wnd.PostMessage(  bDoubleClick ? WM_LBUTTONDBLCLK : pMsg->message,
								pMsg->wParam,
								pMsg->lParam);
			return TRUE;
		}
		
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			CWindow wndParent = GetParent();
			Hide();
			wndParent.PostMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
			return TRUE;
		}
	}

	if( GetFocus() == NULL )
	{
        Hide();
		return TRUE;
	}

	return TRUE;
}

#endif // GRIDCONTROL_NO_TITLETIPS