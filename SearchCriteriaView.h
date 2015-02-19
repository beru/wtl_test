#pragma once

class CSearchCriteriaView : public CDialogImpl<CSearchCriteriaView>
{
public:
    enum { IDD = IDD_FORMVIEW };

    BOOL PreTranslateMessage(MSG* pMsg){
        return CWindow::IsDialogMessage(pMsg);
    }

    BEGIN_MSG_MAP(CSearchCriteriaView)
    END_MSG_MAP()

};

