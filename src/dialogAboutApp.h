// Begin dialogAboutApp.h

#ifndef DIALOGABOUTAPP_H
#define DIALOGABOUTAPP_H

#include <QDialog>

class DialogAboutApp : public QDialog {

    Q_OBJECT

    public:
        DialogAboutApp(QWidget* parent = nullptr);
        ~DialogAboutApp();

    private:
        const int m_frameWidth = 595;
        const int m_dlgTitleFrameHeight = 55;
};

#endif
// End dialogAboutApp.h
