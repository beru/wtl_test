// DropTargetImpl.h: interface for the CDropTargetImpl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __DROPTARGETIMPL_H__
#define __DROPTARGETIMPL_H__


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <ShlGuid.h>


struct __declspec(uuid("4657278B-411B-11d2-839A-00C04FD918D0")) IDropTargetHelper;

template <class T>
class CDropTargetImpl : public CComObjectRootEx<CComSingleThreadModel>,
                        public CComCoClass<T>,
                        public IDropTarget
{
public:
    BEGIN_COM_MAP(CDropTargetImpl<T>)
        COM_INTERFACE_ENTRY(IDropTarget)
    END_COM_MAP()

    void InitializeDT(T* pT)
    {
		m_pDropTargetWindow = pT;
        RegisterDragDrop (m_pDropTargetWindow->m_hWnd, this);

        m_pDropHelper.CoCreateInstance ( CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER );
    }
    
    void RevokeDT()
    {
        RevokeDragDrop ( m_pDropTargetWindow->m_hWnd );
		if (m_pDropHelper != NULL)
		{
			m_pDropHelper.Release();
		}
    }

    // IDropTarget
    STDMETHOD(DragEnter)( IDataObject* pDataObj, DWORD grfKeyState,
                          POINTL pt, DWORD* pdwEffect )
    {
		SetDataAvailable(true);
		DWORD dwEffect = DROPEFFECT_COPY | DROPEFFECT_LINK;

        if (m_pDropHelper)
		{
            m_pDropHelper->DragEnter ( m_pDropTargetWindow->m_hWnd, 
                                       pDataObj, &(POINT&) pt, dwEffect );
		}

		CPoint point(pt.x, pt.y);
		m_pDropTargetWindow->ScreenToClient(&point);
        *pdwEffect = m_pDropTargetWindow->OnDragEnter(pDataObj, grfKeyState, point);

		//ATLTRACE(_T("DragEnter - dpe=%08x\n"), *pdwEffect);

        return S_OK;
    }

	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect )
    {
		DWORD dwEffect = DROPEFFECT_COPY | DROPEFFECT_LINK;

        if ( m_pDropHelper )
		{
            m_pDropHelper->DragOver ( &(POINT&) pt, dwEffect );
		}

		CPoint point(pt.x, pt.y);
		m_pDropTargetWindow->ScreenToClient(&point);
        *pdwEffect = m_pDropTargetWindow->OnDragOver(grfKeyState, point);

		//ATLTRACE(_T("DragOver - dpe=%08x\n"), *pdwEffect);

        return S_OK;
    }
        
    STDMETHOD(DragLeave)()
    {
        if ( m_pDropHelper )
		{
            m_pDropHelper->DragLeave();
			m_pDropTargetWindow->OnDragLeave();
			m_bDataAvailable = true;
		}

        return S_OK;
    }

    STDMETHOD(Drop)( IDataObject* pDataObj, DWORD grfKeyState,
                     POINTL pt, DWORD* pdwEffect )
    {
		DWORD dwEffect = DROPEFFECT_COPY;

        if ( m_pDropHelper )
            m_pDropHelper->Drop ( pDataObj, &(POINT&) pt, dwEffect );

		CPoint point(pt.x, pt.y);
		m_pDropTargetWindow->ScreenToClient(&point);

		//ATLTRACE(_T("Drop - dpe=%08x\n"), *pdwEffect);

		HRESULT hr = S_OK;

		if (m_pDropTargetWindow->OnDrop(pDataObj, grfKeyState, pdwEffect, point))
			hr = E_FAIL;

		m_bDataAvailable = true;
		return hr;
    }

	bool IsDataAvailable() { return m_bDataAvailable; }
	void SetDataAvailable(const bool & b) { m_bDataAvailable = b; }

protected:
    CDropTargetImpl()
    {
        // only creatable thru CreateMe()
		m_pDropTargetWindow = NULL;
		m_pDropHelper = NULL;
		m_bDataAvailable = true;
    }

    CComPtr<IDropTargetHelper> m_pDropHelper;
	T *m_pDropTargetWindow;
	bool m_bDataAvailable;
};

/////////////////////////////////////////////////////////////////////////////
// CDropTarget class

template <class T>
class CDropTarget
{
public:
	CDropTarget()
	{
		m_bRegistered = FALSE;
		m_pDropTarget = NULL;
	}

	~CDropTarget()
	{
		if (m_bRegistered)
		{
			Revoke();
		}

		if (m_pDropTarget)
			m_pDropTarget->Release();
	}

	// Attributes
public:
	BOOL m_bRegistered;
	CDropTargetImpl<T>* m_pDropTarget;

	// Operations
public:
	BOOL Register(T * pT)
	{
		if (m_bRegistered)
			return TRUE;

		// Init our drop target object
		CComObject< CDropTargetImpl<T> >* pObj;

		if ( FAILED(CComObject< CDropTargetImpl<T> >::CreateInstance(&pObj)) )
			return FALSE;
		else
			m_pDropTarget = pObj;

		if ( m_pDropTarget )
		{
			m_pDropTarget->InitializeDT(pT);
			m_bRegistered = TRUE;
		}

		return m_bRegistered;
	}

	void Revoke()
	{
		m_bRegistered = FALSE;
		m_pDropTarget->RevokeDT();
	}

	bool IsDataAvailable()
	{
		return m_pDropTarget->IsDataAvailable();
	}

	void SetDataAvailable(const bool & b)
	{
		m_pDropTarget->SetDataAvailable(b);
	}
};

#endif //__DROPTARGETIMPL_H__