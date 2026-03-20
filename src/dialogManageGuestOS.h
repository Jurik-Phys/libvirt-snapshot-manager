// Begin dialogManageGuestOS.h

#ifndef DIALOGMANAGEGUESTOS_H
#define DIALOGMANAGEGUESTOS_H

#include <QLabel>
#include <QDialog>
#include <QComboBox>
#include <QToolButton>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QStringListModel>
#include "osInfoProvider.h"

class DialogManageGuestOS : public QDialog {

    Q_OBJECT

    public:
        DialogManageGuestOS(const QString& guestOS, QWidget* parent = nullptr);
        ~DialogManageGuestOS();

        void setInfo();
        void printLatesLibOsInfoVersion();

    signals:
        void requestVmOsInfoUpdate();
        void requestVmOsXmlInfoClear();
        void requestVmOsXmlInfoUpdate(const OsInfo&);

    public slots:
        void doUpdateLibOsInfo();

    private:
        QComboBox* m_osEditor;
        QString m_inGuestOS;
        QString formatVersionString(const unsigned int&);
        void updateLocalLibOsInfo();
        OsInfoProvider* m_osInfoProvider;
        const int m_dlgTitleFrameHeight = 55;
        QLabel* m_updateInfoError;
        QLabel* m_lastLibOsInfoVersion;
        QLabel* m_libOsInfoVersion;
        QToolButton* m_updateLocalLibOsInfo;
        QDialogButtonBox* m_dlgBtnBox;
        int m_leftWidth = 175;
        int m_lblIndent = 25;

        QFormLayout* m_infoFrameLayout;
        QHBoxLayout* m_remoteLibOsInfoHLayout;
        QHBoxLayout* m_lastLibOsInfoHLayout;

        QString m_upToDate = "(no update required)";

        QStringListModel* m_osNameListModel;
};

#endif
// End dialogManageGuestOS.h
