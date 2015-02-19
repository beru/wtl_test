// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainFrm.h"

#include "sqlite3/sqlite3.h"
#include <stdint.h>

void CMainFrame::loadDB()
{
	const char* ver = sqlite3_libversion();
	int hoge = 0;

	sqlite3* db;
	int ret = sqlite3_open16(L"test.db", &db);
	if (ret == SQLITE_OK) {
		
		sqlite3_stmt* stmt;
		ret = sqlite3_prepare16_v2(db, L"SELECT * FROM t_test;", -1, &stmt, NULL);
		if (ret == SQLITE_OK) {
#if 0
			LVCOLUMN lvc = {0};
			CHeaderCtrl& hdr = m_view.GetHeader();
			LONG_PTR wlp = hdr.GetWindowLongPtrW(GWL_STYLE);
			wlp |= HDS_CHECKBOXES;
			hdr.SetWindowLongPtrW(GWL_STYLE, wlp);

			HDITEM hdi = {0};
			hdi.mask = HDI_FORMAT;
			hdr.GetItem(0, &hdi);
			hdi.fmt |= HDF_CHECKBOX | HDF_FIXEDWIDTH;
			hdr.SetItem(0, &hdi);
#endif
			size_t rowIdx = 1;
			while (1) {
				ret = sqlite3_step(stmt);
				if (ret == SQLITE_ROW) {
					m_gridCtrl.SetRowCount(m_gridCtrl.GetRowCount() + 1);
					const wchar_t* str = (const wchar_t*) sqlite3_column_text16(stmt, 1);
					int64_t num = sqlite3_column_int64(stmt, 2);
					m_gridCtrl.SetItemText(rowIdx, 1, str);
					TCHAR buff[64];
					swprintf(buff, L"%lld", num);
					ret = m_gridCtrl.SetItemText(rowIdx, 2, buff);
					++rowIdx;
				}else {
					break;
				}
				
			}
		}
		sqlite3_finalize(stmt);
		sqlite3_close(db);
	}

}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg)) {
		return TRUE;
	}

	if (m_criteriaView.PreTranslateMessage(pMsg)) {
		return TRUE;
	}

	return m_gridCtrl.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();

	m_splitter.SetSplitterExtendedStyle(SPLIT_FIXEDBARSIZE);
	m_splitter.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	m_criteriaView.Create(m_splitter);
	m_splitter.SetSplitterPane(SPLIT_PANE_TOP, m_criteriaView);

#if 0
	m_listView.Create(
		m_splitter,
		rcDefault,
		NULL,
		WS_CHILD
		| WS_VISIBLE
		| WS_CLIPSIBLINGS
		| WS_CLIPCHILDREN
		| LVS_REPORT
		| LVS_SHOWSELALWAYS
		,
		WS_EX_CLIENTEDGE
	);

//	m_view.ModifyStyle(0, LVS_REPORT | LVS_SHOWSELALWAYS);
	m_listView.SetExtendedListViewStyle(0
		| LVS_EX_DOUBLEBUFFER
		| LVS_EX_CHECKBOXES
		| LVS_EX_FULLROWSELECT
		| LVS_EX_AUTOSIZECOLUMNS
	);
	m_splitter.SetSplitterPane(SPLIT_PANE_BOTTOM, m_listView);
#endif

	m_gridCtrl.Create(
		m_splitter,
		rcDefault,
		NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT | LVS_OWNERDATA,
		WS_EX_CLIENTEDGE
		);
	m_gridCtrl.SetDoubleBuffering(TRUE);
	//m_gridCtrl.SetVirtualMode(TRUE);
	m_gridCtrl.SetFixedRowCount(1);
	m_gridCtrl.SetFixedColumnCount(1);
	m_gridCtrl.SetColumnCount(3);
	m_gridCtrl.SetColumnWidth(0, 100);
	m_gridCtrl.SetColumnWidth(1, 200);
	m_gridCtrl.SetItemText(0, 1, L"filename");
	m_gridCtrl.SetItemText(0, 2, L"timestamp");
	m_gridCtrl.AutoSizeColumns(GVS_BOTH);

	m_splitter.SetSplitterPane(SPLIT_PANE_BOTTOM, m_gridCtrl);

	m_hWndClient = m_splitter;
	UpdateLayout();

	m_splitter.SetSplitterPos(200);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	loadDB();

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: add code to initialize document

	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}
