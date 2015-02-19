//////////////////////////////////////////////////////////////////////////
//
// WTL Port Of the MFCGridCtrl by Nicola Tufarelli
//
// rel. 0.0.1 25-04-2005
// Mere translation: ATLASSERT for ASSERT, CWindow for CWnd
//					 DECLARE_DYNCREATE deleted

/////////////////////////////////////////////////////////////////////////////
// GridCell.h : header file
//
// MFC Grid Control - Grid cell class header file
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
// For use with CGridCtrl v2.20+
//
//////////////////////////////////////////////////////////////////////

#ifndef __GRIDCELL_H__
#define __GRIDCELL_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CGridCtrl;
#include "GridCellBase.h"
#include "InPlaceEdit.h"

// Each cell contains one of these. Fields "row" and "column" are not stored since we
// will usually have acces to them in other ways, and they are an extra 8 bytes per
// cell that is probably unnecessary.

class CGridCell : public CGridCellBase
{
    friend class CGridCtrl;

// Construction/Destruction
public:
    CGridCell();
    virtual ~CGridCell();

// Attributes
public:
    void operator=(const CGridCell& cell);

    virtual void  SetText(LPCTSTR szText)        { m_strText = szText;  }                       
    virtual void  SetImage(int nImage)           { m_nImage = nImage;   }                        
    virtual void  SetData(LPARAM lParam)         { m_lParam = lParam;   }      
    virtual void  SetGrid(CGridCtrl* pGrid)      { m_pGrid = pGrid;     }                          
    // virtual void SetState(const DWORD nState);  -  use base class version   
    virtual void  SetFormat(DWORD nFormat)       { m_nFormat = nFormat; }                      
    virtual void  SetTextClr(COLORREF clr)       { m_crFgClr = clr;     }                          
    virtual void  SetBackClr(COLORREF clr)       { m_crBkClr = clr;     }                          
    virtual void  SetFont(const LOGFONT* plf);
    virtual void  SetMargin(UINT nMargin)        { m_nMargin = nMargin; }
    virtual CWindow* GetEditWnd() const             { return m_pEditWnd;   }
    virtual void  SetCoords(int /*nRow*/, int /*nCol*/) {}  // don't need to know the row and
                                                            // column for base implementation

    virtual LPCTSTR     GetText() const             { return (m_strText.IsEmpty())? _T("") : LPCTSTR(m_strText); }
    virtual int         GetImage() const            { return m_nImage;  }
    virtual LPARAM      GetData() const             { return m_lParam;  }
    virtual CGridCtrl*  GetGrid() const             { return m_pGrid;   }
    // virtual DWORD    GetState() const - use base class
    virtual DWORD       GetFormat() const;
    virtual COLORREF    GetTextClr() const          { return m_crFgClr; } // TODO: change to use default cell
    virtual COLORREF    GetBackClr() const          { return m_crBkClr; }
    virtual LOGFONT*    GetFont() const;
    virtual CFont*      GetFontObject() const;
    virtual UINT        GetMargin() const;

    virtual BOOL        IsEditing() const           { return m_bEditing; }
    virtual BOOL        IsDefaultFont() const       { return (m_plfFont == NULL); }
    virtual void        Reset();

// editing cells
public:
    virtual BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar);
    virtual void EndEdit();
protected:
    virtual void OnEndEdit();

protected:
    CString    m_strText;      // Cell text (or binary data if you wish...)
    LPARAM     m_lParam;       // 32-bit value to associate with item
    int        m_nImage;       // Index of the list view item�s icon
    DWORD      m_nFormat;
    COLORREF   m_crFgClr;
    COLORREF   m_crBkClr;
    LOGFONT*   m_plfFont;
    UINT       m_nMargin;

    BOOL       m_bEditing;     // Cell being edited?

    CGridCtrl* m_pGrid;        // Parent grid control
    CInPlaceEdit*   m_pEditWnd;
};

// This class is for storing grid default values. It's a little heavy weight, so
// don't use it in bulk 
class CGridDefaultCell : public CGridCell
{
// Construction/Destruction
public:
    CGridDefaultCell();
    virtual ~CGridDefaultCell();

public:
    virtual DWORD GetStyle() const                      { return m_dwStyle;      }
    virtual void  SetStyle(DWORD dwStyle)               { m_dwStyle = dwStyle;   }
    virtual int   GetWidth() const                      { return m_Size.cx;      }
    virtual int   GetHeight() const                     { return m_Size.cy;      }
    virtual void  SetWidth(int nWidth)                  { m_Size.cx = nWidth;    }
    virtual void  SetHeight(int nHeight)                { m_Size.cy = nHeight;   }

    // Disable these properties
    virtual void     SetData(LPARAM /*lParam*/)             { ATLASSERT(FALSE);         }      
    virtual void     SetState(DWORD /*nState*/)             { ATLASSERT(FALSE);         }
    virtual DWORD    GetState() const                       { return CGridCell::GetState()|GVIS_READONLY; }
    virtual void     SetCoords( int /*row*/, int /*col*/)   { ATLASSERT(FALSE);         }
    virtual void     SetFont(const LOGFONT* /*plf*/);
    virtual LOGFONT* GetFont() const;   
    virtual CFont*   GetFontObject() const;

protected:
    CSize m_Size;       // Default Size
    CFont m_Font;       // Cached font
    DWORD m_dwStyle;    // Cell Style - unused
};


#endif //__GRIDCELL_H__