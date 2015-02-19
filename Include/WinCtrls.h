// WinCtrls.h
// Implementation Helpers for Windows Controls
// Copyright 2000 - Maxime Labelle

#ifndef __WINCTRLS_H__
#define __WINCTRLS_H__

#ifndef __cplusplus
	#error winctrls.h requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLWIN_H__
	#error winctrls.h requires atlwin.h to be included first
#endif

#define DECLARE_CTRL_WND_CLASS(WndClassName, style, bkgnd) \
static CWndClassInfo& GetWndClassInfo() \
{ \
	static CWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), style | CS_GLOBALCLASS, SWndProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(bkgnd + 1), NULL, WndClassName, NULL }, \
		NULL, NULL, IDC_ARROW, TRUE, 0, _T("") \
	}; \
	return wc; \
}

#define DECLARE_CTRL_WND_SUPERCLASS(WndClassName, OrigClassName) \
static CWndClassInfo& GetWndClassInfo() \
{ \
	static CWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), CS_GLOBALCLASS, SWndProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH) NULL, NULL, WndClassName, NULL }, \
		OrigClassName, NULL, IDC_ARROW, TRUE, 0, _T("") \
	}; \
	return wc; \
}


template <class T, class TBase = CWindow>
class ATL_NO_VTABLE CWindowsControlImpl : public TBase, public CMessageMap
{
// Construction / destruction
public:
	CWindowsControlImpl(HWND hWnd) : TBase(hWnd)
	{}

// Registration Helpers
public:
	static ATOM RegisterClass(HINSTANCE hInstance)
	{
		CWndClassInfo& ci = T::GetWndClassInfo();

		if (ci.m_atom == 0) {
			::EnterCriticalSection(&_Module.m_csWindowCreate);
			if (ci.m_atom == 0) {
				if (ci.m_lpszOrigName != 0) { // Windows SuperClassing
					LPCTSTR lpsz = ci.m_wc.lpszClassName;
					WNDPROC proc = ci.m_wc.lpfnWndProc;

					WNDCLASSEX wndClass;
					wndClass.cbSize = sizeof(WNDCLASSEX);

					// Try global class
					if (!::GetClassInfoEx(NULL, ci.m_lpszOrigName, &wndClass)) {
						// Try local class
						if (!::GetClassInfoEx(_Module.GetModuleInstance(), ci.m_lpszOrigName, &wndClass)) {
							::LeaveCriticalSection(&_Module.m_csWindowCreate);
							return 0;
						}
					}

					memcpy(&ci.m_wc, &wndClass, sizeof(WNDCLASSEX));
					ci.pWndProc = ci.m_wc.lpfnWndProc;
					ci.m_wc.lpszClassName = lpsz;
					ci.m_wc.lpfnWndProc = proc;

				} else {	// Traditionnal registration

					ci.m_wc.hCursor = ::LoadCursor(ci.m_bSystemCursor ? NULL :
										_Module.GetModuleInstance(), ci.m_lpszCursorID);
					ci.pWndProc = ::DefWindowProc;

				}

				ci.m_wc.hInstance = _Module.GetModuleInstance();
				ci.m_wc.style |= CS_GLOBALCLASS;

				// Synthetize custom class name

				if (ci.m_wc.lpszClassName == 0) { 
					wsprintf(ci.m_szAutoName, _T("WTL:%8.8X"), (DWORD) &ci.m_wc);
					ci.m_wc.lpszClassName = ci.m_szAutoName;
				}

				// Check previous registration

				WNDCLASSEX wndClassTemp;
				memcpy(&wndClassTemp, &ci.m_wc, sizeof(WNDCLASSEX));
				ci.m_atom = (ATOM) ::GetClassInfoEx(ci.m_wc.hInstance, ci.m_wc.lpszClassName, &wndClassTemp);
				if (ci.m_atom == 0) {
					ci.m_atom = ::RegisterClassEx(&ci.m_wc);
				}
			}
			::LeaveCriticalSection(&_Module.m_csWindowCreate);
		}

		return ci.m_atom;
	}

	static LRESULT __stdcall SWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Find C++ object associated with the window
		T* pT = reinterpret_cast<T*>(::GetWindowLong(hWnd, GWL_USERDATA));

		if (pT == 0) {
			// The class hasn't been created yet

			if (uMsg == WM_CREATE) {
				pT = new T(hWnd);
				::SetWindowLong(hWnd, GWL_USERDATA, reinterpret_cast<LONG>(pT));
				return pT->WndProc(hWnd, uMsg, wParam, lParam);

			} else return ::CallWindowProc(T::GetWndClassInfo().pWndProc, hWnd, uMsg, wParam, lParam);

		} else {

			if (uMsg == WM_NCDESTROY) {
				::SetWindowLong(hWnd, GWL_USERDATA, 0L);
				delete pT;
				return ::CallWindowProc(T::GetWndClassInfo().pWndProc, hWnd, uMsg, wParam, lParam);

			} else return pT->WndProc(hWnd, uMsg, wParam, lParam);
		}

		return 0L;
	}

// Window Procedure
protected:
	LRESULT __stdcall WndProc(HWND /*hWnd*/, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LONG nResult = 0L;

		T* pT = static_cast<T*>(this);
		BOOL bRet = pT->ProcessWindowMessage(m_hWnd, uMsg, wParam, lParam, nResult);
		if (!bRet) nResult = ::CallWindowProc(pT->GetWndClassInfo().pWndProc, m_hWnd, uMsg, wParam, lParam);

		return nResult;
	}

// Helpers
protected:
	void NotifyParent(UINT code)
	{
		NMHDR nmh;
		nmh.hwndFrom = m_hWnd;
		nmh.idFrom = GetWindowLong(GWL_ID);
		nmh.code = code;

		::SendMessage(GetParent(), WM_NOTIFY, (WPARAM) nmh.idFrom, (LPARAM) &nmh);
	}

	BOOL InvalidateRect(BOOL bErase = TRUE)
	{
		CRect rect;
		GetClientRect(&rect);

		return CWindow::InvalidateRect(rect, bErase);
	}
};

#endif // __WINCTRLS_H__
