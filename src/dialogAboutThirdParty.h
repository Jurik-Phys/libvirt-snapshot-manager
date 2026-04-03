// Begin dialogAboutThirdParty.h

#ifndef DIALOGABOUTTHIRDPARTY_H
#define DIALOGABOUTTHIRDPARTY_H

#include <QDialog>

class DialogAboutThirdParty : public QDialog {

    Q_OBJECT

    public:
        DialogAboutThirdParty(QWidget* parent = nullptr);
        ~DialogAboutThirdParty();

    private:
        const int m_frameWidth = 595;
        const int m_dlgTitleFrameHeight = 55;
};





#endif
// End dialogAboutThirdParty.h
