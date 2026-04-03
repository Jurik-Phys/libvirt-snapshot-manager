// Begin dialogAboutThirdParty.cpp

#include <QBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QDialogButtonBox>
#include "dialogAboutThirdParty.h"

DialogAboutThirdParty::DialogAboutThirdParty(QWidget* parent) : QDialog(parent){
    // *** Dialog main vertical layout *** //
    QVBoxLayout* dlgVBoxLayout = new QVBoxLayout(this);

    // *** [1] Title framebox *** //
    QFrame* dlgTitleFrame = new QFrame();
    dlgTitleFrame->setFrameShape(QFrame::StyledPanel);
    dlgTitleFrame->setFrameShadow(QFrame::Plain);
    dlgTitleFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgTitleFrame->setFixedWidth(m_frameWidth);

    QHBoxLayout* dlgTitleFrameHLayout = new QHBoxLayout(dlgTitleFrame);
    dlgTitleFrameHLayout->setAlignment(Qt::AlignCenter);
    QLabel* dlgTitle = new QLabel("Third-party components & licenses");
    dlgTitleFrameHLayout->addWidget(dlgTitle);

    // *** [2] Info frame box *** //
    QFrame* dlgInfoFrame = new QFrame();
    dlgInfoFrame->setFrameShape(QFrame::StyledPanel);
    dlgInfoFrame->setFrameShadow(QFrame::Plain);
    dlgInfoFrame->setFixedWidth(m_frameWidth);

    // *** Горизонтальный layout для двух столбцов *** //
    QHBoxLayout* dlgInfoFrameHLayout = new QHBoxLayout(dlgInfoFrame);
    dlgInfoFrameHLayout->setContentsMargins(25, 25, 25, 25);

    // *** Верикальный layout для логотипа third-party *** //
    QVBoxLayout* logoVLayout = new QVBoxLayout();
    dlgInfoFrameHLayout->addLayout(logoVLayout);

    QLabel* logoWidget = new QLabel();
    logoWidget->setFixedWidth(75);
    logoWidget->setFixedHeight(75);
    QPixmap logoPixmap(":/third-party-logo-75x75");
    logoWidget->setPixmap(logoPixmap);
    logoVLayout->addWidget(logoWidget);
    logoVLayout->addStretch();
    dlgInfoFrameHLayout->addSpacing(25);

    QLabel* aboutThirdPartyText = new QLabel();
    aboutThirdPartyText->setWordWrap(true);
    aboutThirdPartyText->setTextInteractionFlags(Qt::TextSelectableByMouse
                                                  | Qt::LinksAccessibleByMouse);
    aboutThirdPartyText->setOpenExternalLinks(true);
    dlgInfoFrameHLayout->addWidget(aboutThirdPartyText);
    aboutThirdPartyText->setText(R"(
        <h4>CLI and GUI virtualization tools</h4>
        <p>
            <a href=https://www.libvirt.org/manpages/virsh.html
            style="text-decoration: none;">virsh</a>,
            <a href=https://www.libvirt.org/manpages/virsh.html#license
                                   style="text-decoration: none;">LGPL v2.1</a>.
            Used for querying virtual machine state and managing
                                                     snapshots via system calls.
        </p>
        <p>
            <a href=https://qemu-project.gitlab.io/qemu/tools/qemu-img.html
            style="text-decoration: none;">qemu-img</a>,
            <a href=https://qemu-project.gitlab.io/qemu/about/license.html
                                      style="text-decoration: none;">GPL v2</a>.
            Used for inspecting and managing disk images and external snapshot
            chains.
        </p>
        <p>
            <a href=https://virt-manager.org
            style="text-decoration: none;">virt-manager</a>,
                        <a href=https://virt-manager.org/faq.html
                                      style="text-decoration: none;">GPL v2</a>.
            Provides a graphical interface for accessing virtual machines,
                     as well as creating and independently configuring them.
        </p>
        <h4>Frameworks and libraries</h4>
        <p>
            <a href=https://qt.io
                       style="text-decoration: none;">Qt Framework</a>,
            <a href=https://www.qt.io/development/qt-framework/qt-licensing
                                     style="text-decoration: none;">LGPL v3</a>.
            Provides the cross-platform application framework, including UI,
            model/view architecture, JSON handling,  and execution of
                                                             external utilities.
        </p>
        <p>
            <a href=https://gitlab.com/libosinfo/osinfo-db
            style="text-decoration: none;">osinfo-db</a>,
            <a href=https://gitlab.com/libosinfo/osinfo-db/-/blob/main/COPYING
                                      style="text-decoration: none;">GPL v2</a>.
            Provides a database of guest operating system metadata.
        </p>
        <p>
            <a href=https://tukaani.org/xz/
            style="text-decoration: none;">liblzma</a>,
            <a href=https://tukaani.org/xz/#_licensing
                                        style="text-decoration: none;">0BSD</a>.
            Used to decompress osinfo-db data.
        </p>
        <h4>Additional components</h4>
        <p>
            Any other dependencies included in the build process are used
            in accordance with their respective licenses.
        </p>
        <h4>Disclaimer</h4>
        <p>
            This application is an independent project and is not affiliated
            with or endorsed by the developers of the above-mentioned software.
        </p>
    )");

    // *** Dialog button box frame *** //
    QFrame* dlgBtnFrame = new QFrame();
    dlgBtnFrame->setFrameShape(QFrame::StyledPanel);
    dlgBtnFrame->setFrameShadow(QFrame::Plain);
    dlgBtnFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgBtnFrame->setFixedWidth(m_frameWidth);

    QHBoxLayout* dlgBtnFrameHLayout = new QHBoxLayout(dlgBtnFrame);
    QDialogButtonBox* dlgBtnBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    dlgBtnFrameHLayout->addWidget(dlgBtnBox);

    QObject::connect(dlgBtnBox, &QDialogButtonBox::accepted,
                                          this, &DialogAboutThirdParty::accept);

    // *** Add all widet to main layout *** //
    dlgVBoxLayout->addWidget(dlgTitleFrame);
    dlgVBoxLayout->addWidget(dlgInfoFrame);
    dlgVBoxLayout->addWidget(dlgBtnFrame);

    // *** Fix dialog width *** //
    this->adjustSize();
    this->setFixedWidth(this->width());
    this->setFixedHeight(this->height());
}

DialogAboutThirdParty::~DialogAboutThirdParty(){

}

// End dialogAboutThirdParty.cpp
