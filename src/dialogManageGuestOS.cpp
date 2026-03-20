// Begin dialogManageGuestOS.cpp

#include <QToolTip>
#include <QLineEdit>
#include <QCompleter>

#include "dialogManageGuestOS.h"

DialogManageGuestOS::DialogManageGuestOS(const QString& guestOS,
                                             QWidget* parent) : QDialog(parent){
    m_inGuestOS = guestOS;
    // *** parent - VmInfoWidget *** //
    this->setFixedWidth(1.7 *  parent->width());
    this->setWindowTitle("VM guest OS info");

    // *** Dialog main vertical layout *** //
    QVBoxLayout* dlgVBoxLayout = new QVBoxLayout(this);

    // *** Title box *** //
    QFrame* dlgTitleFrame = new QFrame(this);
    dlgTitleFrame->setFrameShape(QFrame::StyledPanel);
    dlgTitleFrame->setFrameShadow(QFrame::Plain);
    dlgTitleFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgTitleFrame->setFixedWidth(1.7 * parent->width() - 22);

    QHBoxLayout* dlgTitleFrameHLayout = new QHBoxLayout(dlgTitleFrame);
    dlgTitleFrameHLayout->setAlignment(Qt::AlignHCenter);
    QLabel* dlgText = new QLabel("Manage guest operation system information");
    dlgTitleFrameHLayout->addWidget(dlgText);

    m_osInfoProvider = new OsInfoProvider(this);
    connect(m_osInfoProvider, &OsInfoProvider::localOsInfoUpdateFinished,
                                 this, &DialogManageGuestOS::doUpdateLibOsInfo);

    // *** Main box *** //
    QFrame* dlgInfoFrame = new QFrame(this);
    dlgInfoFrame->setFrameShape(QFrame::StyledPanel);
    dlgInfoFrame->setFrameShadow(QFrame::Plain);
    dlgInfoFrame->setFixedWidth(1.7 * parent->width() - 22);

    m_infoFrameLayout = new QFormLayout(dlgInfoFrame);
    m_infoFrameLayout->setLabelAlignment(Qt::AlignRight);
    m_infoFrameLayout->setFormAlignment(Qt::AlignLeft);
    m_infoFrameLayout->setVerticalSpacing(15);
    // *** Main box. Line #1 *** //
    QStringList osNameList = m_osInfoProvider->getOsNameList();
    m_osNameListModel = new QStringListModel(osNameList, this);
    QLabel* osLabel = new QLabel("Operating system:");
    osLabel->setFixedWidth(m_leftWidth);
    osLabel->setIndent(m_lblIndent);
    osLabel->setAlignment(Qt::AlignRight);
    m_osEditor = new QComboBox();
    m_osEditor->setEditable(true);
    m_osEditor->lineEdit()->setClearButtonEnabled(true);
    m_osEditor->addItems(osNameList);
    m_osEditor->lineEdit()->setText(guestOS);
    QObject::connect(m_osEditor, &QComboBox::currentTextChanged,
        [&](const QString&){
            QPalette pal = m_osEditor->lineEdit()->palette();
            QColor textColor;
            textColor = m_osEditor->lineEdit()->palette().color(QPalette::Text);
            if (textColor == Qt::red){
                pal.setColor(QPalette::Text, Qt::black);
                m_osEditor->lineEdit()->setPalette(pal);
            }
        });
    QCompleter* completer = new QCompleter(m_osNameListModel, this);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_osEditor->setCompleter(completer);
    // Layout необходим длля корректного выравнивания по высоте внтутри строки
    QHBoxLayout* osLabelHLayout = new QHBoxLayout();
    osLabelHLayout->addWidget(osLabel, 0, Qt::AlignVCenter);
    osLabelHLayout->addWidget(m_osEditor, 0, Qt::AlignVCenter);
    m_infoFrameLayout->addRow(osLabelHLayout);
    // *** Main box. Line #2 *** //
    QLabel* libOsInfoLabel = new QLabel("App libosinfo db:");
    libOsInfoLabel->setFixedWidth(m_leftWidth);
    libOsInfoLabel->setIndent(m_lblIndent);
    libOsInfoLabel->setAlignment(Qt::AlignRight);
    unsigned int libOsInfoVer = m_osInfoProvider->getLocalLibOsInfoVersion();

    m_libOsInfoVersion = new QLabel();
    m_libOsInfoVersion->setText(formatVersionString(libOsInfoVer));
    QHBoxLayout* libOsInfoHLayout = new QHBoxLayout();
    libOsInfoHLayout->addWidget(libOsInfoLabel, 0, Qt::AlignVCenter);
    libOsInfoHLayout->addWidget(m_libOsInfoVersion, 0, Qt::AlignVCenter);
    m_infoFrameLayout->addRow(libOsInfoHLayout);
    // *** Main box. Line #3 *** //
    QString remoteLibOsInfoLabelText = "Available online:";
    QLabel* remoteLibOsInfoLabel = new QLabel(remoteLibOsInfoLabelText);
    remoteLibOsInfoLabel->setFixedWidth(m_leftWidth);
    remoteLibOsInfoLabel->setIndent(m_lblIndent);
    remoteLibOsInfoLabel->setAlignment(Qt::AlignRight);

    QToolButton* checkLibOsInfoUpdates = new QToolButton(this);
    checkLibOsInfoUpdates->setAutoRaise(true);
    checkLibOsInfoUpdates->setText("Check Update");
    QObject::connect(checkLibOsInfoUpdates, &QToolButton::clicked,
                        this, &DialogManageGuestOS::printLatesLibOsInfoVersion);

    m_updateInfoError = new QLabel("Failed to retrieve information!!!");
    m_updateInfoError->setStyleSheet("font-weight: bold; color: red;");
    m_updateInfoError->setAlignment(Qt::AlignCenter);
    m_updateInfoError->setVisible(false);
    m_remoteLibOsInfoHLayout = new QHBoxLayout();
    m_remoteLibOsInfoHLayout
                         ->addWidget(remoteLibOsInfoLabel, 0, Qt::AlignVCenter);
    m_remoteLibOsInfoHLayout
                        ->addWidget(checkLibOsInfoUpdates, 0, Qt::AlignVCenter);
    m_remoteLibOsInfoHLayout->addWidget(m_updateInfoError, 0, Qt::AlignVCenter);
    m_infoFrameLayout->addRow(m_remoteLibOsInfoHLayout);

    // Row c информацией о доступной версии libosinfo
    QLabel* remoteLibOsInfoLabelTwo = new QLabel(remoteLibOsInfoLabelText);
    remoteLibOsInfoLabelTwo->setFixedWidth(m_leftWidth);
    remoteLibOsInfoLabelTwo->setIndent(m_lblIndent);
    remoteLibOsInfoLabelTwo->setAlignment(Qt::AlignRight);

    m_lastLibOsInfoVersion = new QLabel("ver. YYYY-MM-DD");
    m_updateLocalLibOsInfo = new QToolButton();
    m_updateLocalLibOsInfo->setAutoRaise(true);
    m_updateLocalLibOsInfo->setText("Update Application Database");
    QObject::connect(m_updateLocalLibOsInfo, &QToolButton::clicked,
                              this, &DialogManageGuestOS::updateLocalLibOsInfo);

    m_lastLibOsInfoHLayout = new QHBoxLayout();
    m_lastLibOsInfoHLayout
                      ->addWidget(remoteLibOsInfoLabelTwo, 0, Qt::AlignVCenter);
    m_lastLibOsInfoHLayout
                       ->addWidget(m_lastLibOsInfoVersion, 0, Qt::AlignVCenter);
    m_lastLibOsInfoHLayout->addSpacing(m_lblIndent);
    m_lastLibOsInfoHLayout
                       ->addWidget(m_updateLocalLibOsInfo, 0, Qt::AlignVCenter);
    m_lastLibOsInfoHLayout->addStretch();

    // *** Button box *** //
    QFrame* dlgBtnFrame = new QFrame(this);
    dlgBtnFrame->setFrameShape(QFrame::StyledPanel);
    dlgBtnFrame->setFrameShadow(QFrame::Plain);
    dlgBtnFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgBtnFrame->setFixedWidth(1.7 * parent->width() - 22);

    QHBoxLayout* dlgBtnFrameHLayout = new QHBoxLayout(dlgBtnFrame);
    m_dlgBtnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                      QDialogButtonBox::Cancel);
    dlgBtnFrameHLayout->addWidget(m_dlgBtnBox);

    QObject::connect(m_dlgBtnBox, &QDialogButtonBox::accepted,
                                           this, &DialogManageGuestOS::setInfo);
    QObject::connect(m_dlgBtnBox, &QDialogButtonBox::rejected,
                                            this, &DialogManageGuestOS::reject);

    dlgVBoxLayout->addWidget(dlgTitleFrame);
    dlgVBoxLayout->addWidget(dlgInfoFrame);
    dlgVBoxLayout->addWidget(dlgBtnFrame);
}

DialogManageGuestOS::~DialogManageGuestOS(){
}

void DialogManageGuestOS::setInfo(){
    QString osName = m_osEditor->lineEdit()->text();
    OsInfo osInfo = m_osInfoProvider->getOsInfoByOsName(osName);
    if (osInfo.id.isEmpty()){
        if (m_inGuestOS != osName && !osName.isEmpty()){
            QPalette pal = m_osEditor->lineEdit()->palette();
            pal.setColor(QPalette::Text, Qt::red);
            m_osEditor->lineEdit()->setPalette(pal);

            QToolTip::showText(
                m_osEditor->mapToGlobal(QPoint(-10, m_osEditor->height()/1.3)),
                "Unknown OS. Clear the field to select an unspecified OS"
            );
        }
        else{
            // *** Пропуск удаления записи только, когда были изменения *** //
            if ((m_inGuestOS != osName) && (m_inGuestOS != "Not specified")){
                emit requestVmOsXmlInfoClear();
            }
            this->accept();
        }
    }
    else {
        // *** Пропуск для случая без измеения вида гостевой ОС *** //
        //     Создание временного xml-файла виртуальной машины,    //
        //     его включение через virsh, удаление из ~/tmp,        //
        //     всё это нецелесообразно делать, если изменений нет.  //
        if (m_inGuestOS != osName){
            emit requestVmOsXmlInfoUpdate(osInfo);
        }
        this->accept();
    }
}

void DialogManageGuestOS::printLatesLibOsInfoVersion(){
    unsigned int latestLibOsInfoVersion;
    unsigned int libOsInfoVer;
    latestLibOsInfoVersion = m_osInfoProvider->getLatestLibOsInfoVersion();
    libOsInfoVer = m_osInfoProvider->getLocalLibOsInfoVersion();
    if (latestLibOsInfoVersion > 0){
        if (latestLibOsInfoVersion > libOsInfoVer){
            // *** Удалить строку с кнопкой запроса обновления *** //
            //     и (скрытым) текстом ошибки. Вставка столбца     //
            //     с полученной информацией и кнопкой сохранения   //
            //     обновлённой версии библиотеки.                  //
            m_infoFrameLayout->removeRow(m_remoteLibOsInfoHLayout);
            m_lastLibOsInfoVersion
                         ->setText(formatVersionString(latestLibOsInfoVersion));
            m_infoFrameLayout->addRow(m_lastLibOsInfoHLayout);
        }
        else {
            // ***        Версия в сети не выше локальной         *** //
            //     Кнопка обновления локалаьных данных выключена,     //
            //     отобржается полученная версия и комментарий        //
            //     о том, что обновлять не надо.                      //
            m_infoFrameLayout->removeRow(m_remoteLibOsInfoHLayout);
            QString info = formatVersionString(latestLibOsInfoVersion) + " "
                                                                   + m_upToDate;
            m_lastLibOsInfoVersion->setText(info);
            // Hide m_updateLocalLibOsInfo without vertical line jump
            m_updateLocalLibOsInfo->setEnabled(false);
            m_updateLocalLibOsInfo->setText(QString());
            m_infoFrameLayout->addRow(m_lastLibOsInfoHLayout);
        }
    }
    else {
        m_updateInfoError->setVisible(true);
    }
}


QString DialogManageGuestOS::formatVersionString(const unsigned int& intVer){
    QString res;
    int year = intVer/10000;
    int month = (intVer - year*10000)/100;
    int day = intVer - year*10000 - month * 100;

    QString strMonth = QString("%1").arg(month, 2, 10, QChar('0'));
    QString strDay   = QString("%1").arg(day,   2, 10, QChar('0'));
    res = QString("ver. ") + QString::number(year)  + "-"
                           + strMonth + "-"
                           + strDay;

    return res;
}

void DialogManageGuestOS::updateLocalLibOsInfo(){
    m_updateLocalLibOsInfo->setEnabled(false);
    m_osInfoProvider->updateLocalLibOsInfo();
}

void DialogManageGuestOS::doUpdateLibOsInfo(){
    // *** Обновление строки с текущей версией базы данных libosinfo *** //
    QString version;
    version = formatVersionString(m_osInfoProvider->getLocalLibOsInfoVersion());
    m_libOsInfoVersion->setText(version);

    // *** "Скрытие" кнопки скачивания и обновления базы данных libosinfo *** //
    //     Способ скрытия определяется тем, что при setVisible(false)         //
    //     происходит вертикальное смещение данной строки, что есть плохо.    //
    m_updateLocalLibOsInfo->setText(QString());

    // *** После обновления версии совпадают *** //
    m_lastLibOsInfoVersion->setText(version + " " + m_upToDate);

    // *** Обновление названия операционной системы, если ранее оно было *** //
    //     неизвестным т.е., содержало N/A.                                  //
    QString guestOS = m_osEditor->lineEdit()->text();
    if (m_inGuestOS.contains("N/A")){
        QString osId = guestOS.split(" ").last();
        OsInfo osInfo = m_osInfoProvider->getOsInfoByOsId(osId);
        if (!osInfo.name.isEmpty()){
            // *** Изменения действительно есть, их надо отобразить *** //
            //     в диалоге и послать сигнал об этом событии.          //
            m_osEditor->lineEdit()->setText(osInfo.name);
        }

        // *** Обновление m_inGuestOS для предотвращения лишней записи *** //
        //     xml-файла виртуальной машины с неизменившимися данными      //
        QString inOsId = m_inGuestOS.split(" ").last();
        OsInfo inOsInfo = m_osInfoProvider->getOsInfoByOsId(inOsId);
        if (!inOsInfo.name.isEmpty()){
            m_inGuestOS = inOsInfo.name;
        }

        emit requestVmOsInfoUpdate();
    }

    // *** Обновление списка операционных систем в комплитере *** //
    m_osNameListModel->setStringList(m_osInfoProvider->getOsNameList());
}

// End dialogManageGuestOS.cpp
