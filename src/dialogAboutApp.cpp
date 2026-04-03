// Begin dialogAboutApp.cpp

#include "dialogAboutApp.h"
#include "appInfo.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QSvgWidget>
#include <QLabel>
#include <QFrame>

DialogAboutApp::DialogAboutApp(QWidget* parent) : QDialog(parent){
    // *** Dialog main vertical layout *** //
    QVBoxLayout* dlgVBoxLayout = new QVBoxLayout(this);

    // *** [1] Title box *** //
    QFrame* dlgTitleFrame = new QFrame(this);
    dlgTitleFrame->setFrameShape(QFrame::StyledPanel);
    dlgTitleFrame->setFrameShadow(QFrame::Plain);
    dlgTitleFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgTitleFrame->setFixedWidth(m_frameWidth);

    QHBoxLayout* dlgTitleFrameHLayout = new QHBoxLayout(dlgTitleFrame);
    dlgTitleFrameHLayout->setAlignment(Qt::AlignHCenter);
    QLabel* dlgText = new QLabel(AppInfo::appFullName());
    dlgTitleFrameHLayout->addWidget(dlgText);

    // *** [2] Info box *** //
    QFrame* dlgInfoFrame = new QFrame(this);
    dlgInfoFrame->setFrameShape(QFrame::StyledPanel);
    dlgInfoFrame->setFrameShadow(QFrame::Plain);
    dlgInfoFrame->setFixedWidth(m_frameWidth);

    QHBoxLayout* dlgInfoFrameHLayout = new QHBoxLayout(dlgInfoFrame);
    dlgInfoFrameHLayout->setContentsMargins(25, 25, 25, 25);

    QVBoxLayout* logoVLayout = new QVBoxLayout();
    dlgInfoFrameHLayout->addLayout(logoVLayout);

    QSvgWidget* logo = new QSvgWidget(":/app-logo.svg");
    logo->setFixedHeight(75);
    logo->setFixedWidth(75);
    logoVLayout->addWidget(logo);
    logoVLayout->addStretch();
    dlgInfoFrameHLayout->addSpacing(25);

    QVBoxLayout* dlgInfoFrameVLayout = new QVBoxLayout();
    dlgInfoFrameHLayout->addLayout(dlgInfoFrameVLayout);

    QLabel* description = new QLabel();
    description->setWordWrap(true);
    description->setTextInteractionFlags(Qt::TextSelectableByMouse
                                                  | Qt::LinksAccessibleByMouse);
    description->setOpenExternalLinks(true);
    dlgInfoFrameVLayout->addWidget(description);
    QString descriptionText(R"(
    <p>
        <b>%1</b> is a lightweight tool for managing
        external snapshots of virtual machines using libvirt and QEMU.
    </p>
    <p>
        The application provides a clear tree-based view of snapshot chains,
        allowing you to inspect, create, and manage snapshots safely
        and efficiently. It also offers basic configuration capabilities for
        virtual machines, including setting the guest operating system type,
        allocated memory size, and CPU topology and model.
    </p>
    <p>
        Designed for advanced users who need precise control over virtual disk
        states and core VM parameters without unnecessary complexity.
    </p>
    <p>
        Built with Qt and powered by virsh and qemu-img.<br>
    </p>
    <p>
        <b>Version: </b>%2<br>
    </p>
    <p>
        <b>Author:</b><br>
            &nbsp;&nbsp;name: %3<br>
            &nbsp;&nbsp;mail: <a href="mailto:%4"
                                       style="text-decoration: none;">%4</a><br>
            &nbsp;&nbsp;site: <a href="%5"
                                       style="text-decoration: none;">%5</a><br>
    </p>
    <p>
       <b>License:</b> <a href="https://www.gnu.org/licenses/gpl-3.0.html"
            style="text-decoration: none;"
                             > GNU General Public License v3.0 (GPL-3.0)</a></p>
    )");
    descriptionText = descriptionText.arg(AppInfo::appBaseName());
    descriptionText = descriptionText.arg(AppInfo::appFullVersion());
    descriptionText = descriptionText.arg(AppInfo::appAuthorName());
    descriptionText = descriptionText.arg(AppInfo::appAuthorMail());
    descriptionText = descriptionText.arg(AppInfo::appAuthorSite());
    description->setText(descriptionText);

    // *** Button box *** //
    QFrame* dlgBtnFrame = new QFrame(this);
    dlgBtnFrame->setFrameShape(QFrame::StyledPanel);
    dlgBtnFrame->setFrameShadow(QFrame::Plain);
    dlgBtnFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgBtnFrame->setFixedWidth(m_frameWidth);

    QHBoxLayout* dlgBtnFrameHLayout = new QHBoxLayout(dlgBtnFrame);
    QDialogButtonBox* dlgBtnBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    dlgBtnFrameHLayout->addWidget(dlgBtnBox);

    QObject::connect(dlgBtnBox, &QDialogButtonBox::accepted,
                                                 this, &DialogAboutApp::accept);

    // *** Final grouping widget *** //
    dlgVBoxLayout->addWidget(dlgTitleFrame);
    dlgVBoxLayout->addWidget(dlgInfoFrame);
    dlgVBoxLayout->addWidget(dlgBtnFrame);

    // *** Фиксация ширины диалога после применения всех иных размеров *** //
    this->adjustSize();
    this->setFixedWidth(this->width());
    this->setFixedHeight(this->height());
}

DialogAboutApp::~DialogAboutApp(){

}


// End dialogAboutApp.cpp
