#if !defined(AFX_VALIDATINGEDIT_H__20010604_43EF_5E9B_8634_0080AD509054__INCLUDED_)
#define AFX_VALIDATINGEDIT_H__20010604_43EF_5E9B_8634_0080AD509054__INCLUDED_

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CValidateEdit - Validating Edit control
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2001 Bjarke Viksoe.
//
// Add the following macro to the parent's message map:
//   REFLECT_NOTIFICATIONS()
// If you handle the notifications:
//   EN_CHANGE
//   EN_UPDATE
//   EN_SETFOCUS
//   EN_KILLFOCUS
// make sure the set "bHandled" to FALSE to
// allow proper reflection.
//
// The pattern matching function understands:
//  *      = match all
//  ?      = match single
//  [abc]  = match set
//  [a-z]  = match range
//  [^a-z] = exclude range
//  \x     = escape
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#ifndef __cplusplus
  #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLCTRLS_H__
  #error ValidateEdit.h requires atlctrls.h to be included first
#endif

#ifdef _MBCS
  #pragma message("Warning: The Pattern Matching algorithm is not fully MBCS compatible")
#endif


/////////////////////////////////////////////////////////////////////////////
// CValidateEdit

#define ES_EX_ERRORMARKS      0x00000001
#define ES_EX_ERRORSTRIPES    0x00000002
#define ES_EX_ERRORBEEP       0x00000004
#define ES_EX_ERRORTIP        0x00000008
#define ES_EX_ERRORMASK       0x0000000F

#define ES_EX_BULKYSTRIPES    0x00000100
#define ES_EX_BALLOONTIP      0x00000200


typedef enum 
{
   ES_COMPLETE, 
   ES_INCOMPLETE, 
   ES_NOMATCH, 
   ES_OUTOFBOUNDS,
   ES_SYNTAX 
} ES_MATCHSTATUS;

typedef struct
{
   LPCTSTR pstrStart;      // [in]
   LPCTSTR pstrPattern;    // [in]
   ES_MATCHSTATUS status;  // [out]
   int iErrPos;            // [out]
   int iPattPos;           // [out]
} ES_MATCHINFO;


// ToolTip flags from Platform SDK (Windows ver 5.0)
#ifndef TTS_NOFADE
   #define TTS_NOANIMATE 0x00000010
   #define TTS_NOFADE    0x00000020
   #define TTS_BALLOON   0x00000040
#endif // TTS_NOFADE


template< class T, class TBase = CEdit, class TWinTraits = CControlWinTraits >
class ATL_NO_VTABLE CValidateEditImpl : 
   public CWindowImpl< T, TBase, TWinTraits >
{
public:
   DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

   BEGIN_MSG_MAP(CValidateEditImpl)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(OCM_CTLCOLOREDIT, OnCtlColorEdit)
      MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
      MESSAGE_HANDLER(WM_PAINT, OnPaint)
      MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
      MESSAGE_HANDLER(WM_CHAR, OnChar)
      MESSAGE_HANDLER(WM_SETTEXT, OnSetText)
      MESSAGE_HANDLER(WM_PASTE, OnPaste)
      MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
      REFLECTED_COMMAND_CODE_HANDLER(EN_CHANGE, OnChanged)
      REFLECTED_COMMAND_CODE_HANDLER(EN_UPDATE, OnUpdate)
      REFLECTED_COMMAND_CODE_HANDLER(EN_SETFOCUS, OnSetFocus)
      REFLECTED_COMMAND_CODE_HANDLER(EN_KILLFOCUS, OnKillFocus)
      DEFAULT_REFLECTION_HANDLER()
   END_MSG_MAP()

   enum { MAX_PATTERN_LEN = 128 };

   DWORD         m_dwExtStyle;
   TCHAR         m_szPattern[MAX_PATTERN_LEN+1];
   ES_MATCHINFO  m_info;
   CBrush        m_brStripes;
   CBrush        m_brBackground;
   COLORREF      m_clrText;
   COLORREF      m_clrComplete;
   COLORREF      m_clrError;
   COLORREF      m_clrBack;
   HBRUSH        m_brBack;
   CToolTipCtrl  m_tip;
   TOOLINFO      m_ti;

   // Operations

   BOOL SubclassWindow(HWND hWnd)
   {
      ATLASSERT(m_hWnd==NULL);
      ATLASSERT(::IsWindow(hWnd));
#ifdef _DEBUG
      // Check class
      TCHAR szBuffer[16] = { 0 };
      if( ::GetClassName(hWnd, szBuffer, (sizeof(szBuffer)/sizeof(TCHAR))-1) ) {
         ATLASSERT(::lstrcmpi(szBuffer, CEdit::GetWndClassName()) == 0);
      }
#endif // _DEBUG
      BOOL bRet = CWindowImpl< T, TBase, TWinTraits >::SubclassWindow(hWnd);
      if( bRet ) _Init();
      return bRet;
   }

   DWORD GetExtendedEditStyle() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return m_dwExtStyle;
   }

   void SetExtendedEditStyle(DWORD dwStyle)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT( !((dwStyle & ES_EX_ERRORBEEP) && (dwStyle & (ES_EX_ERRORSTRIPES|ES_EX_ERRORMARKS))) );
      m_dwExtStyle = dwStyle;
      if( m_tip.IsWindow() ) {
         if( dwStyle & ES_EX_BALLOONTIP ) m_tip.ModifyStyle(0, TTS_BALLOON); else m_tip.ModifyStyle(TTS_BALLOON, 0);
      }
      SendMessage(WM_SETTINGCHANGE);
   }

   void SetPattern(LPCTSTR pstrPattern)
   {
      ATLASSERT(!::IsBadStringPtr(pstrPattern, MAX_PATTERN_LEN));
      ::lstrcpyn(m_szPattern, pstrPattern, MAX_PATTERN_LEN);
   }

   void SetTextColor(COLORREF clr)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      m_clrText = clr;
      Invalidate();
   }

   void SetBkColor(COLORREF clr)
   {
      if( !m_brBackground.IsNull() ) m_brBackground.DeleteObject();
      m_brBackground.CreateSolidBrush(clr);
      m_brBack = m_brBackground;
      m_clrBack = clr;
      Invalidate();
   }

   void SetCompleteColor(COLORREF clr)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      m_clrComplete = clr;
      Invalidate();
   }

   void SetTipBkColor(COLORREF clr)
   {
      if( m_tip.IsWindow() ) m_tip.SetTipBkColor(clr);
   }

   void SetTipTextColor(COLORREF clr)
   {
      if( m_tip.IsWindow() ) m_tip.SetTipTextColor(clr);
   }

   void SetParseState(ES_MATCHINFO* info)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(!::IsBadReadPtr(info, sizeof(ES_MATCHINFO)));
      m_info = *info;
      T* pT = static_cast<T*>(this);
      pT->_DoErrorCheck(info);
      Invalidate();
   }

   void GetParseState(ES_MATCHINFO* info) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(!::IsBadWritePtr(info, sizeof(ES_MATCHINFO)));
      *info = m_info;
      info->pstrStart = NULL;           // don't reveal internal text buffer
      info->pstrPattern = m_szPattern;  // let user know the pattern
   }

   // Implementation

   void _Init()
   {
      ATLASSERT(::IsWindow(m_hWnd));

	  DWORD style = GetStyle();
      ATLASSERT((style & (ES_AUTOHSCROLL|ES_UPPERCASE|ES_LOWERCASE|ES_PASSWORD|ES_READONLY|ES_MULTILINE))==0);
      ModifyStyle(ES_AUTOHSCROLL,0);

      m_tip.Create(m_hWnd);
      ATLASSERT(m_tip.IsWindow());
      if( m_tip.IsWindow() ) {
         m_tip.ModifyStyle(0, TTS_NOPREFIX | TTS_NOFADE);
         CToolInfo ti(TTF_IDISHWND | TTF_ABSOLUTE | TTF_TRACK, GetParent(), (UINT)m_hWnd, NULL, LPSTR_TEXTCALLBACK);
         m_tip.AddTool(ti);
         m_ti = ti;
         m_tip.TrackActivate(&m_ti, FALSE);
      }

      ::ZeroMemory(m_szPattern, sizeof(m_szPattern)/sizeof(TCHAR));
      ::lstrcpy(m_szPattern, _T("*"));     
      ::ZeroMemory(&m_info, sizeof(ES_MATCHINFO));
      m_info.status = ES_INCOMPLETE;
      m_dwExtStyle = 0;

      SendMessage(WM_SETTINGCHANGE);
   }

   void DoPaint(CDCHandle dc)
   {
      // It's a bit risky to overload the WM_PAINT of an Edit control.
      // The reason is that it is a very "old" control and doesn't comply strictly
      // to any custom/ownerdrawn/subclassing methology.
      // Basically the control internally redraws itself during keypresses, focus,
      // selection changes - not using the WM_PAINT message!
      // We override this behaviour by invalidating the control intuitively.
      // Caret management and selection highlighting is still left to the Edit control.

      HideCaret();

      DWORD dwStyle = GetStyle();
      DWORD dwMargins = GetMargins();

      RECT rc;
      GetClientRect(&rc); 
      dc.IntersectClipRect(&rc);

      // Repaint background
      HFONT hOldFont = dc.SelectFont(GetFont());
      dc.SetBkMode(TRANSPARENT);
      dc.FillRect(&rc, m_brBack);

      // Get edit text
      int len = GetWindowTextLength() + 1;
      LPTSTR pstr = (LPTSTR) _alloca(len * sizeof(TCHAR));
      if( pstr == NULL ) return; // out of memory
      GetWindowText(pstr, len);

      // Find correct rectangle
      ::InflateRect(&rc, -1, -1);
      rc.left += LOWORD(dwMargins);
      rc.right -= HIWORD(dwMargins);

      // Get colors
      COLORREF clrText = m_info.status == ES_COMPLETE ? m_clrComplete : m_clrText;
      COLORREF clrError = m_clrError;
      if( dwStyle & WS_DISABLED ) clrText = clrError = ::GetSysColor(COLOR_GRAYTEXT);

      // Get text style
      UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_EDITCONTROL;
      if( dwStyle & ES_LEFT ) uFormat |= DT_LEFT;
      if( dwStyle & ES_RIGHT ) uFormat |= DT_RIGHT;
      if( dwStyle & ES_CENTER ) uFormat |= DT_CENTER;

      switch( m_info.status ) {
      case ES_SYNTAX:
      case ES_NOMATCH:
      case ES_OUTOFBOUNDS:
         {
            // We need to split the text in two halfs: the correct
            // text and the incorrect text part.
            // Both parts must have their size correctly calculated.
            LPTSTR pstrCorrect = (LPTSTR) _alloca(len * sizeof(TCHAR));
            LPTSTR pstrWrong = (LPTSTR) _alloca(len * sizeof(TCHAR));
            if( pstrCorrect == NULL || pstrWrong == NULL ) return; // out of memory
            if( m_info.iErrPos>len ) m_info.iErrPos = len;
            ::lstrcpyn(pstrCorrect, pstr, m_info.iErrPos + 1);
            ::lstrcpy(pstrWrong, pstr + m_info.iErrPos);
            // Calculate the text width for both parts
            RECT rcLeftWidth = { 0 };
            RECT rcRightWidth = { 0 };
            dc.DrawText(pstrCorrect, -1, &rcLeftWidth, DT_SINGLELINE | DT_EDITCONTROL | DT_CALCRECT);
            dc.DrawText(pstrWrong, -1, &rcRightWidth, DT_SINGLELINE | DT_EDITCONTROL | DT_CALCRECT);
            // Calculate the paint rectangles
            RECT rcLeft = { rc.left, rc.top, rc.left + (rcLeftWidth.right-rcLeftWidth.left), rc.bottom };
            RECT rcRight = { rcLeft.right, rc.top, rcLeft.right + (rcRightWidth.right-rcRightWidth.left), rc.bottom };
            if( uFormat & (DT_RIGHT|DT_CENTER) ) {
               int offset = (rc.right-rc.left)-((rcRight.right-rcRight.left)+(rcLeft.right-rcLeft.left));
               offset--;
               if( uFormat & DT_CENTER ) offset >>= 1;
               ::OffsetRect(&rcLeft, offset,0);
               ::OffsetRect(&rcRight, offset,0);
            }
            // Paint the stripes
            if( m_dwExtStyle & ES_EX_ERRORSTRIPES ) {
               TEXTMETRIC tm;
               dc.GetTextMetrics(&tm);
               int y = rcRight.top + tm.tmHeight + 1;
               RECT rc = { rcRight.left, y, rcRight.right, y+4 };
               dc.SetBkColor(m_clrBack);
               dc.SetTextColor(clrError);
               dc.FillRect(&rc, m_brStripes);
            }
            // Paint the two text segments
            if( m_dwExtStyle & ES_EX_ERRORMARKS ) {
               dc.SetTextColor(clrText);
               dc.DrawText(pstrCorrect, -1, &rcLeft, uFormat);
               dc.SetTextColor(clrError);
               dc.DrawText(pstrWrong, -1, &rcRight, uFormat);
            }
            else {
               dc.SetTextColor(clrText);
               dc.DrawText(pstr, -1, &rc, uFormat);
            }
         }
         break;
      default:
         dc.SetTextColor(clrText);
         dc.DrawText(pstr, -1, &rc, uFormat);
      }

      dc.SelectFont(hOldFont);

      ShowCaret();
   }

   BOOL _DoErrorCheck(ES_MATCHINFO* info)
   {
      ATLASSERT(!::IsBadReadPtr(info,sizeof(ES_MATCHINFO)));
      switch( info->status ) {
      case ES_COMPLETE:
      case ES_INCOMPLETE:
         if( m_tip.IsWindow() ) m_tip.TrackActivate(&m_ti, FALSE);
         return FALSE; // No error; accept char
      }
      if( m_dwExtStyle & ES_EX_ERRORBEEP ) {
         ::MessageBeep((UINT)-1);
         return TRUE; // Don't accept char
      }
      if( m_dwExtStyle & ES_EX_ERRORTIP ) {
         if( m_tip.IsWindow() ) {
            RECT rcTip;
            m_tip.GetClientRect(&rcTip);
            RECT rcWin;
            GetWindowRect(&rcWin);
            POINT pt = PosFromChar(info->iErrPos);
            pt.x += rcWin.left;
            if( m_dwExtStyle & ES_EX_BALLOONTIP ) {
               pt.x += 2;
               pt.y = rcWin.bottom - 8;
            }
            else {
               pt.y += rcWin.top - (rcTip.bottom-rcTip.top) - 3;
            }
            m_tip.TrackPosition(pt.x,pt.y);
            m_tip.TrackActivate(&m_ti, TRUE);
         }
      }
      return FALSE; // Accept char
   }

   BOOL _MatchPattern(LPCTSTR String, LPCTSTR Pattern, ES_MATCHINFO* info) const
   {
      TCHAR c, p, l;
      bool fTruth;
      for( ; ; ) {
         switch( p = *Pattern++ ) {
         case _T('\0'):                      // end of pattern
            if( *String == 0 ) {
               info->status = ES_COMPLETE;
               info->iErrPos = 0;
               info->iPattPos = Pattern - info->pstrPattern - 1;
               return TRUE;                  // SUCCESS
            }
            else {
               info->status = ES_OUTOFBOUNDS;
               info->iErrPos = String - info->pstrStart;
               info->iPattPos = Pattern - info->pstrPattern - 1;
               return FALSE;                 // too many character
            }

         case _T('*'):
            while (*String) {                // match zero or more char
               if( _MatchPattern(String++, Pattern, info) ) return TRUE;
            }
            return _MatchPattern(String, Pattern, info);

         case _T('?'):
            if( *String++ == 0 ) {           // match any one char
               info->status = ES_INCOMPLETE;
               info->iErrPos = String - info->pstrStart - 1;
               info->iPattPos = Pattern - info->pstrPattern - 1;
               return FALSE;                 // not end of string
            }
            break;

         case _T('['):
            if( (c = *String++) == 0 ) {     // match char set
               info->status = ES_INCOMPLETE;
               info->iErrPos = String - info->pstrStart - 1;
               info->iPattPos = Pattern - info->pstrPattern - 1;
               return FALSE;                 // not end of string
            }
            l = _T('0');
            fTruth = true;
            while( (p = *Pattern++) != 0 ) {
                if( p == _T(']') ) {         // if end of char set, then
                   if( fTruth ) {
                      info->status = ES_NOMATCH;
                      info->iErrPos = String - info->pstrStart - 1;
                      info->iPattPos = Pattern - info->pstrPattern - 1;
                      return FALSE;          // no match found
                   }
                   break;                    // not in exclude, move on
                }
                if( p == _T('^') ) {
                  fTruth = false;
                  p = *Pattern++;
                }
                if( p == _T('-') ) {         // check a range of chars?
                    p = *Pattern;            // get high limit of range
                    if( p == _T('\0') || p == _T(']') || p == _T('^') ) {
                       info->status = ES_SYNTAX;
                       info->iErrPos = String - info->pstrStart - 1;
                       info->iPattPos = Pattern - info->pstrPattern;
                       return FALSE;        // syntax
                    }
                    if( ( c >= l  &&  c <= p ) ) {
                       if( !fTruth ) {
                          info->status = ES_NOMATCH;
                          info->iErrPos = String - info->pstrStart - 1;
                          info->iPattPos = Pattern - info->pstrPattern;
                          return FALSE;     // matched exclude
                       }
                       break;               // if in range, move on
                    }
                }
                l = p;
                if( (c == p) ) {            // if char matches this element
                   if( !fTruth ) {
                      info->status = ES_NOMATCH;
                      info->iErrPos = String - info->pstrStart - 1;
                      info->iPattPos = Pattern - info->pstrPattern - 1;
                      return FALSE;         // matched exclude
                   }
                   break;                   // move on
                }
            }
            while( p  &&  p != _T(']') )    // got a match in char set
               p = *Pattern++;              // skip to end of set
            break;

         default:
            if( (c = *String++) == 0 ) {
               info->status = ES_INCOMPLETE;
               info->iErrPos = String - info->pstrStart - 1;
               info->iPattPos = Pattern - info->pstrPattern - 1;
               return FALSE;                 // not end of string
            }
            if( c == _T('\\') )              // check escape
               c = *String++;                
            if( c != p ) {                   // check for exact char
               info->status = ES_NOMATCH;
               info->iErrPos = String - info->pstrStart - 1;
               info->iPattPos = Pattern - info->pstrPattern - 1;
               return FALSE;                 // not a match
            }
            break;
         }
      }
   }

   // Message Handlers

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
      _Init();
      return lRes;
   }

   LRESULT OnCtlColorEdit(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      CDCHandle dc( (HDC) wParam );
      dc.SetBkColor( m_clrBack );
      return (LRESULT) (HBRUSH)m_brBack;
   }

   LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      WORD StripeBits1[8] = { 0x77, 0xAA, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
      WORD StripeBits2[8] = { 0x11, 0x22, 0x44, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
      LPVOID pBits = StripeBits1;
      if( m_dwExtStyle & ES_EX_BULKYSTRIPES ) pBits = StripeBits2;

      CBitmap bmStripes;
      bmStripes.CreateBitmap(8,8,1,1, pBits);
      if( !m_brStripes.IsNull() ) m_brStripes.DeleteObject();
      m_brStripes.CreatePatternBrush(bmStripes);

      m_clrError = RGB(255,0,0);
      m_clrText = m_clrComplete = ::GetSysColor(COLOR_WINDOWTEXT);
      SetTipBkColor(RGB(255,100,100));

      // If no background color was set, then use the default system color
      if( m_brBackground.IsNull() ) {
         m_brBack = ::GetSysColorBrush(COLOR_WINDOW);
         m_clrBack = ::GetSysColor(COLOR_WINDOW);
      }

      Invalidate();
      return 0;
   }

   LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      T* pT = static_cast<T*>(this);
      if( wParam != NULL ) {
         pT->DoPaint((HDC)wParam);
      }
      else {
         CPaintDC dc(m_hWnd);
         pT->DoPaint(dc.m_hDC);
      }
      return 0;
   }

   LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      if( ::GetFocus() != m_hWnd ) return DefWindowProc(uMsg, wParam, lParam);
      
      bool bCombi = (::GetKeyState(VK_CONTROL) & 0x8000) ||
                    (::GetKeyState(VK_MENU) & 0x8000);
      if( !bCombi && wParam>=VK_SPACE ) {
         int posStart, posEnd;
         GetSel(posStart, posEnd);
         int len = GetWindowTextLength();
         ATLASSERT(posStart>=0 && posStart<=posEnd && posEnd<=len); // sanity
         // Need to construct the text as how it will look
         // after the character insert...
         LPTSTR pstrOld = (LPTSTR) _alloca((len + 1 ) * sizeof(TCHAR));
         LPTSTR pstrNew = (LPTSTR) _alloca((len + 3) * sizeof(TCHAR));
         if( pstrOld == NULL || pstrNew == NULL ) return 0; // out of memory (stack space)
         GetWindowText(pstrOld,len + 1);
#ifdef _UNICODE
         const int cchSize = 1;
#else
         const int cchSize = ::IsDBCSLeadByte((BYTE)wParam) ? 2 : 1;
#endif
         memcpy( pstrNew, pstrOld, posStart * sizeof(TCHAR) );
         memcpy( pstrNew + posStart, &wParam, cchSize * sizeof(TCHAR) );
         memcpy( pstrNew + posStart + cchSize, pstrOld + posEnd, (len - posEnd + 1) * sizeof(TCHAR) );

         T* pT = static_cast<T*>(this);
         ES_MATCHINFO info = { 0 };
         info.pstrStart = pstrNew;
         info.pstrPattern = m_szPattern;
         pT->_MatchPattern(pstrNew, m_szPattern, &info);
         if( pT->_DoErrorCheck(&info) ) return 0;
      }

      bHandled = FALSE;
      return 0;
   }

   LRESULT OnChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
   {
      // We use the EN_CHANGE to update the internal status
      // information. Eventually the control will send
      // an EN_UPDATE notification, where we repaint the control.

      int len = GetWindowTextLength() + 1;
      LPTSTR pstr = (LPTSTR) _alloca(len * sizeof(TCHAR));
      if( pstr == NULL ) return 0; // out of memory
      GetWindowText(pstr, len);

      T* pT = static_cast<T*>(this);
      ES_MATCHINFO info = { 0 };
      info.pstrStart = pstr;
      info.pstrPattern = m_szPattern;
      pT->_MatchPattern(pstr, m_szPattern, &info);
      pT->_DoErrorCheck(&info);
      m_info = info;

      bHandled = FALSE;
      return 0;
   }

   LRESULT OnUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
   {
      Invalidate();
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnSetFocus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
   {
      T* pT = static_cast<T*>(this);
      pT->_DoErrorCheck(&m_info); // To display the tooltip again if nessecary
      SetSel(0,0);               // Prevent full text selection which is not displayed correctly
      Invalidate();
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnKillFocus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
   {
      if( m_tip.IsWindow() ) m_tip.TrackActivate(&m_ti, FALSE);
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      // HACK: To avoid validating the WM_SETTEXT we internally check
      //       the wParam for other values then 0.
      LPCTSTR pstr = (LPCTSTR)lParam;
      if( pstr != NULL && wParam == 0 ) {
         ATLASSERT(!::IsBadStringPtr(pstr, -1));
         T* pT = static_cast<T*>(this);
         ES_MATCHINFO info = { 0 };
         info.pstrStart = pstr;
         info.pstrPattern = m_szPattern;
         pT->_MatchPattern(pstr, m_szPattern, &info);
         switch( info.status ) {
         case ES_COMPLETE:
         case ES_INCOMPLETE:
            break;
         default:
            ::MessageBeep((UINT)-1);
            return 0; // Don't accept SetWindowText()
         }
      }
      return DefWindowProc(uMsg, 0, lParam);
   }

   LRESULT OnPaste(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      // We accept the change and check if the new text is
      // valid. If not, we revert to the old text.
      // This strategy is a quick way to do a safe check - even when 
      // we're pasting into a selection in the edit field.
      T* pT = static_cast<T*>(this);

      int len = GetWindowTextLength() + 1;
      LPTSTR pstrOld = (LPTSTR) _alloca(len * sizeof(TCHAR));
      if( pstrOld == NULL ) return 0;
      GetWindowText(pstrOld, len);
      
      LRESULT lResult = DefWindowProc(uMsg, wParam, lParam);

      len = GetWindowTextLength() + 1;
      LPTSTR pstrNew = (LPTSTR) _alloca(len * sizeof(TCHAR));
      if( pstrNew == NULL ) return 0;
      GetWindowText(pstrNew, len);
            
      ES_MATCHINFO info = { 0 };
      info.pstrStart = pstrNew;
      info.pstrPattern = m_szPattern;
      pT->_MatchPattern(pstrNew, m_szPattern, &info);
      switch( info.status ) {
      case ES_COMPLETE:
      case ES_INCOMPLETE:
         break;
      default:
         SendMessage(WM_SETTEXT, 0x6666, (LPARAM)pstrOld);
         SetSel(0,0);
         ::MessageBeep((UINT)-1);
         return 0; // Don't accept paste
      }
      return lResult;
   }

   LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      if( m_tip.IsWindow() ) {
         MSG msg = { m_hWnd, uMsg, wParam, lParam };
         m_tip.RelayEvent(&msg);
      }
      bHandled = FALSE;
      return 0;
   }
};

class CValidateEdit : public CValidateEditImpl<CValidateEdit>
{
public:
   DECLARE_WND_SUPERCLASS(_T("WTL_ValidateEdit"), GetWndClassName())  
};


#endif // !defined(AFX_VALIDATINGEDIT_H__20010604_43EF_5E9B_8634_0080AD509054__INCLUDED_)

