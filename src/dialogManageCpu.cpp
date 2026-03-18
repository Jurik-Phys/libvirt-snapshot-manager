// Begin dialogManageCpu.cpp

#include "dialogManageCpu.h"
#include "vmInfoWidget.h"
#include <QVBoxLayout>
#include <QSpinBox>
#include <QFrame>

DialogManageCpu::DialogManageCpu(const uint& hostCpuCount,
                                    const QString& vmCpuModel,
                                        const QString& vmMaxCpuCount,
                                            const QString& vmCpuTopology,
                                             QWidget* parent) : QDialog(parent){
    cpuModelDescriptionInit();
    m_hostCpuCount = hostCpuCount;
    m_maxVmCpuCount = vmMaxCpuCount.toInt();

    // *** Fill m_sockets, m_cores, m_threads, m_vmCpuCount variable *** //
    parseVmCpuTopology(vmCpuTopology);
    uint sockets = getVmSockets();
    uint cores = getVmCores();
    uint threads = getVmThreads();
    uint cpuCount = getVmCpuCount();

    double frameWidthRatio = 1.51;
    double leftWidthRatio  = 0.40;
    double rightWidthRatio = 0.50;

    int    frameWidth = qRound(frameWidthRatio * parent->width() - 22);
    int    leftWidth  = qRound(frameWidth * leftWidthRatio);
    int    rightWidth = qRound(frameWidth * rightWidthRatio);

    // *** parent is VmInfoWidget *** //
    this->setFixedWidth(frameWidth + 22);
    this->setWindowTitle("VM CPU topology setup");

    // *** Dialog main vertical layout *** //
    QVBoxLayout* dlgVBoxLayout = new QVBoxLayout(this);

    // *** [1] Title box *** //
    QFrame* dlgTitleFrame = new QFrame(this);
    dlgTitleFrame->setFrameShape(QFrame::StyledPanel);
    dlgTitleFrame->setFrameShadow(QFrame::Plain);
    dlgTitleFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgTitleFrame->setFixedWidth(frameWidth);

    QHBoxLayout* dlgTitleFrameHLayout = new QHBoxLayout(dlgTitleFrame);
    dlgTitleFrameHLayout->setAlignment(Qt::AlignHCenter);
    QLabel* dlgText = new QLabel("Manage virtual CPU model & topology");
    dlgTitleFrameHLayout->addWidget(dlgText);

    // *** [2] Main box *** //
    QFrame* dlgInfoFrame = new QFrame(this);
    dlgInfoFrame->setFrameShape(QFrame::StyledPanel);
    dlgInfoFrame->setFrameShadow(QFrame::Plain);
    dlgInfoFrame->setFixedWidth(frameWidth);

    m_infoFrameLayout = new QFormLayout(dlgInfoFrame);
    m_infoFrameLayout->setLabelAlignment(Qt::AlignRight);
    m_infoFrameLayout->setFormAlignment(Qt::AlignLeft);
    m_infoFrameLayout->setVerticalSpacing(15);

    // *** [2.1] Main box. Host logical CPU *** //
    // Title
    QLabel* hostLogicalCPUTitle = new QLabel("Logical host CPUs:");
    hostLogicalCPUTitle->setFixedWidth(leftWidth);
    hostLogicalCPUTitle->setAlignment(Qt::AlignRight);
    // Value
    QLabel* hostLogicalCPUValue = new QLabel(QString::number(m_hostCpuCount));
    hostLogicalCPUValue->setFixedWidth(rightWidth);
    hostLogicalCPUValue->setAlignment(Qt::AlignRight);
    // Title + Value
    QHBoxLayout* hostLogicalCPUHLayout = new QHBoxLayout();
    hostLogicalCPUHLayout->addWidget(hostLogicalCPUTitle, 0, Qt::AlignVCenter);
    hostLogicalCPUHLayout->addWidget(hostLogicalCPUValue, 0, Qt::AlignVCenter);
    // Title + Value to frame layout
    m_infoFrameLayout->addRow(hostLogicalCPUHLayout );

    // *** [2.2] Main box. VM virtual CPU *** //
    // Title
    QLabel* vmAllocateCPUTitle = new QLabel("VM allocated CPUs:");
    vmAllocateCPUTitle->setFixedWidth(leftWidth);
    vmAllocateCPUTitle->setAlignment(Qt::AlignRight);
    // Value
    m_vmAllocateCPUValue = new QLabel();
    m_vmAllocateCPUValue->setFixedWidth(rightWidth);
    m_vmAllocateCPUValue->setAlignment(Qt::AlignRight);
    this->updVmCpuCount();
    // Title + Value
    QHBoxLayout* vmAllocateCPUHLayout = new QHBoxLayout();
    vmAllocateCPUHLayout->addWidget(vmAllocateCPUTitle, 0, Qt::AlignVCenter);
    vmAllocateCPUHLayout->addWidget(m_vmAllocateCPUValue, 0, Qt::AlignVCenter);
    // Title + Value to frame layout
    m_infoFrameLayout->addRow(vmAllocateCPUHLayout );

    // *** [2.3] Main box. - sockets *** //
    // Title
    QLabel* vCpuSocketsTitle = new QLabel("- sockets:");
    vCpuSocketsTitle->setFixedWidth(leftWidth);
    vCpuSocketsTitle->setAlignment(Qt::AlignRight);
    // Value
    QSpinBox* vCpuSocketsValue = new QSpinBox();
    vCpuSocketsValue->setMinimum(1);
    vCpuSocketsValue->setMaximum(256);
    vCpuSocketsValue->setValue(getVmSockets());
    vCpuSocketsValue->setFixedWidth(0.20 * rightWidth);
    vCpuSocketsValue->setAlignment(Qt::AlignRight);
    // Title + Value
    QHBoxLayout* vCpuSocketsHLayout = new QHBoxLayout();
    vCpuSocketsHLayout->addWidget(vCpuSocketsTitle, 0, Qt::AlignVCenter);
    vCpuSocketsHLayout->addSpacing(rightWidth * 0.88);
    vCpuSocketsHLayout->addWidget(vCpuSocketsValue, 0, Qt::AlignVCenter);
    // Title + Value to frame layout
    m_infoFrameLayout->addRow(vCpuSocketsHLayout );
    // Update count of sockets
    QObject::connect(vCpuSocketsValue, &QSpinBox::valueChanged, this,
            [&](int newSockets){
                m_sockets = newSockets;
                updVmCpuCount();
            });

    // *** [2.4] Main box. - cores *** //
    // Title
    QLabel* vCpuCoresTitle = new QLabel("- cores:");
    vCpuCoresTitle->setFixedWidth(leftWidth);
    vCpuCoresTitle->setAlignment(Qt::AlignRight);
    // Value
    QSpinBox* vCpuCoresValue = new QSpinBox();
    vCpuCoresValue->setMinimum(1);
    vCpuCoresValue->setMaximum(256);
    vCpuCoresValue->setValue(getVmCores());
    vCpuCoresValue->setFixedWidth(0.20 * rightWidth);
    vCpuCoresValue->setAlignment(Qt::AlignRight);
    // Title + Value
    QHBoxLayout* vCpuCoresHLayout = new QHBoxLayout();
    vCpuCoresHLayout->addWidget(vCpuCoresTitle, 0, Qt::AlignVCenter);
    vCpuCoresHLayout->addSpacing(rightWidth * 0.88);
    vCpuCoresHLayout->addWidget(vCpuCoresValue, 0, Qt::AlignVCenter);
    // Title + Value to frame layout
    m_infoFrameLayout->addRow(vCpuCoresHLayout );
    // Update count of cores
    QObject::connect(vCpuCoresValue, &QSpinBox::valueChanged, this,
            [&](int newCores){
                m_cores = newCores;
                updVmCpuCount();
            });

    // *** [2.5] Main box. - threads *** //
    // Title
    QLabel* vCpuThreadsTitle = new QLabel("- threads:");
    vCpuThreadsTitle->setFixedWidth(leftWidth);
    vCpuThreadsTitle->setAlignment(Qt::AlignRight);
    // Value
    QSpinBox* vCpuThreadsValue = new QSpinBox();
    vCpuThreadsValue->setMinimum(1);
    vCpuThreadsValue->setMaximum(256);
    vCpuThreadsValue->setValue(getVmThreads());
    vCpuThreadsValue->setFixedWidth(0.20 * rightWidth);
    vCpuThreadsValue->setAlignment(Qt::AlignRight);
    // Title + Value
    QHBoxLayout* vCpuThreadsHLayout = new QHBoxLayout();
    vCpuThreadsHLayout->addWidget(vCpuThreadsTitle, 0, Qt::AlignVCenter);
    vCpuThreadsHLayout->addSpacing(rightWidth * 0.88);
    vCpuThreadsHLayout->addWidget(vCpuThreadsValue, 0, Qt::AlignVCenter);
    // Title + Value to frame layout
    m_infoFrameLayout->addRow(vCpuThreadsHLayout );
    // Update count of threads
    QObject::connect(vCpuThreadsValue, &QSpinBox::valueChanged, this,
            [&](int newThreads){
                m_threads = newThreads;
                updVmCpuCount();
            });

    // *** [2.6] Main box. - CPU mode/model *** //
    // Title
    QLabel* vCpuModelTitle = new      QLabel("Virtual CPUs model:");
    vCpuModelTitle->setFixedWidth(leftWidth);
    vCpuModelTitle->setAlignment(Qt::AlignRight);
    // Value
    m_vCpuModelValue = new QComboBox();
    m_vCpuModelValue->setFixedWidth(1.04 * rightWidth);
    this->cpuModelValuesFill(m_vCpuModelValue, vmCpuModel);

    // Title + Value
    QHBoxLayout* vCpuModelHLayout = new QHBoxLayout();
    vCpuModelHLayout->addWidget(vCpuModelTitle, 0, Qt::AlignVCenter);
    vCpuModelHLayout->addSpacing(rightWidth * 0.04);
    vCpuModelHLayout->addWidget(m_vCpuModelValue, 0, Qt::AlignVCenter);
    // Title + Value to frame layout
    m_infoFrameLayout->addRow(vCpuModelHLayout );

    // *** Button box *** //
    QFrame* dlgBtnFrame = new QFrame(this);
    dlgBtnFrame->setFrameShape(QFrame::StyledPanel);
    dlgBtnFrame->setFrameShadow(QFrame::Plain);
    dlgBtnFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgBtnFrame->setFixedWidth(frameWidth);

    QHBoxLayout* dlgBtnFrameHLayout = new QHBoxLayout(dlgBtnFrame);
    m_dlgBtnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                      QDialogButtonBox::Cancel);
    dlgBtnFrameHLayout->addWidget(m_dlgBtnBox);

    QObject::connect(m_dlgBtnBox, &QDialogButtonBox::accepted,
                                          this, &DialogManageCpu::setVmCpuInfo);
    QObject::connect(m_dlgBtnBox, &QDialogButtonBox::rejected, this,
                                                      &DialogManageCpu::reject);

    // *** Final grouping widget *** //
    dlgVBoxLayout->addWidget(dlgTitleFrame);
    dlgVBoxLayout->addWidget(dlgInfoFrame);
    dlgVBoxLayout->addWidget(dlgBtnFrame);
}

DialogManageCpu::~DialogManageCpu(){
}

void DialogManageCpu::parseVmCpuTopology(const QString& vmCpuTopology){

    QStringList vmCpuTopologyList = vmCpuTopology.split("·");

    for (QStringList::iterator it = vmCpuTopologyList.begin();
                                          it != vmCpuTopologyList.end(); ++it) {
        QString str = (*it).simplified();
        QString name = str.split(" ").first();
        uint value = str.split(" ").last().toInt();

        if ( name == "sockets" ){
            m_sockets = value;
        }
        else {
            if (name == "cores"){
                m_cores = value;
            }
            else {
                if ( name == "threads" ){
                    m_threads = value;
                }
            }
        }
    }
    m_vmCpuCount = m_sockets * m_cores * m_threads;
}

void DialogManageCpu::cpuModelDescriptionInit(){

    // *** host-passthrough *** //
    m_vmCpuModel["host-passthrough"] = {
        "Host CPU (HW host-passthrough)",
        "<p style='white-space:nowrap;'>"
            "Exposes the host CPU to the guest<br>"
            "as-is for maximum performance"
        "</p>"
    };

    // *** host-model *** //
    m_vmCpuModel["host-model"] = {
        "Host CPU (HW compatible model)",
        "<p style='white-space:nowrap;'>"
            "Uses a host-compatible CPU model<br>"
            "suitable for migration between similar hosts"
        "</p>"
    };

    // *** maximum *** //
    m_vmCpuModel["maximum"] = {
        "HW host-passthrough or soft mode",
        "<p style='white-space:nowrap;'>"
            "Identical to host-passthrough<br>"
            "with hardware virtualization,<br>"
            "and enables maximum features<br>"
            "supported by the virt engine<br>"
            "under CPU software emulation</p>"
    };

    // *** kvm64-v1 *** //
    m_vmCpuModel["kvm64-v1"] = {
        "Generic x86_64 CPU (kvm64-v1, hw)",
        "<p style='white-space:nowrap;'>"
            "Stable generic x86_64 CPU optimized for KVM.<br>"
            "Requires hardware virtualization"
        "</p>"
    };

    // *** qemu64-v1 *** //
    m_vmCpuModel["qemu64-v1"] = {
        "Legacy x86_64 CPU (qemu64-v1, sw)",
        "<p style='white-space:nowrap;'>"
            "Suitable for maximum compatibility. Can run with or without KVM<br>"
            "(software emulated if hardware virtualization is unavailable)"
        "</p>"
    };

    // *** custom cpu model *** //
    m_vmCpuModel["custom"] = {
        "- - -",
        "<p style='white-space:nowrap;'>"
            "A guest will see the selected CPU<br>"
            "no matter which host it is booted on"
        "</p>"
    };
}

void DialogManageCpu::cpuModelValuesFill(QComboBox* vCpuModelValue,
                                                            QString vmCpuModel){
    QStringList orderKeys = {
        "host-passthrough",
        "host-model",
        "maximum",
        "kvm64-v1",
        "qemu64-v1",
        "custom"
    };

    // *** Создание пунктов выпадающего меню и подсказок для них *** //
    //     (последний пункт оставлен для custom CPU)
    for (int idx = 0; idx < orderKeys.count() - 1; ++idx){
        QString key = orderKeys[idx];
        vCpuModelValue->addItem(m_vmCpuModel[key].title, key);

        // *** ToolTip для всплывающих пунктов *** //
        vCpuModelValue->setItemData(vCpuModelValue->count() - 1,
                                m_vmCpuModel[key].description, Qt::ToolTipRole);
    }

    // *** Активация актуального пункта в QComboBox *** //
    int actualIdx = -1;;
    for (int idx = 0; idx < orderKeys.size() - 1; ++idx ){
        if (vmCpuModel == orderKeys[idx]){
            actualIdx = idx;
        }
    }
    // *** Если vmCpuModel не найден, значит надо использовать custom, *** //
    //     который в списке QComboBox будет последним                     //
    if (actualIdx == -1){
        actualIdx = orderKeys.count() - 1;
        // *** Добавление последнего "custom" пункта *** //
        QString key = orderKeys[actualIdx];
        QString customCpuItemTitle;
        if (!vmCpuModel.isEmpty()){
            customCpuItemTitle = vmCpuModel;
            customCpuItemTitle[0] = customCpuItemTitle[0].toUpper();
        }
        else {
            customCpuItemTitle = "Custom CPU";
        }
        vCpuModelValue->addItem(customCpuItemTitle, vmCpuModel);
        vCpuModelValue->setItemData(vCpuModelValue->count() - 1,
                                m_vmCpuModel[key].description, Qt::ToolTipRole);
    }

    vCpuModelValue->setCurrentIndex(actualIdx);
    vCpuModelValue->setToolTip(m_vmCpuModel[orderKeys[actualIdx]].description);

    // *** Смена ToolTip'а QComboBox'а при смене активного пункта *** //
    QObject::connect(vCpuModelValue, &QComboBox::currentIndexChanged, this,
            [=](int index){
                vCpuModelValue->setToolTip(m_vmCpuModel[orderKeys[index]]
                                                                  .description);
            });
}

uint DialogManageCpu::getVmCpuCount(){
    return m_vmCpuCount;
}

void DialogManageCpu::updVmCpuCount(){
    m_vmCpuCount = m_sockets * m_cores * m_threads;
    if (m_vmCpuCount > m_hostCpuCount ){
        QString space = this->getVmCpuValueSpacer(m_vmCpuCount);
        QString text;
        if (m_vmCpuCount <= m_maxVmCpuCount){
            text = "<span style='color:brown;'>vCPUs > host CPUs"
                         + space + QString::number(getVmCpuCount()) + "</span>";
            m_vmAllocateCPUValue->setToolTip("Warning: CPU overcommitting "
                                   "can have a negative impact on performance");

                        // *** Включение кнопки "Ok" *** //
            // Если в виртуальной машине число вычислительных модулей меньше  //
            // максимального лимита для данной вирт. машины, то активируется  //
            // кнопка "Ok", позволяющая применить изменения новых данных.     //
            // Но при первом создании диалога, виджет "Ok" ещё не существует, //
            // поэтому необходимо проводить проверку на его существование,    //
            // в противном случае обращение к несуществующему виджету вызовет //
            // segmentation fault. Здесь достаточно проверки на nullptr при   //
            // условии, что m_dlgBtnBox инициализирован через nullptr         //
            if (m_dlgBtnBox != nullptr){
                if (!m_dlgBtnBox->button(QDialogButtonBox::Ok)->isEnabled()){
                    m_dlgBtnBox->button(QDialogButtonBox::Ok)->setEnabled(true);
                }
            }
        }
        else {
            QString limitValue = QString::number(m_maxVmCpuCount);
            text = "<span style='color:red;'>"
                "vCPUs > limit (" + limitValue + ")" + space
                                 + QString::number(getVmCpuCount()) + "</span>";
            m_vmAllocateCPUValue->setToolTip("Error: vCPU count exceeds "
                             "virtual machine type limit (" + limitValue + ")");

            // *** Отключение кнопки "Ok" *** //
            m_dlgBtnBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        }
        m_vmAllocateCPUValue->setText(text);
    }
    else {
        m_vmAllocateCPUValue->setText(QString::number(getVmCpuCount()));
        m_vmAllocateCPUValue->setToolTip(QString());
    }
}

QString DialogManageCpu::getVmCpuValueSpacer(const int& vmCpuCount){
    QString space;
    // *** Warning |-Space-| Value *** //
    //     Min value 1;                //
    //     Max value 256*256*256       //
    if (vmCpuCount < 10){
        space = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    }
    else {
        if (m_vmCpuCount < 100){
            space = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
        }
        else {
            if (m_vmCpuCount < 1000){
                space = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
            }
            else {
                if (m_vmCpuCount < 10000){
                    space = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
                }
                else {
                    if (m_vmCpuCount < 100000){
                        space = "&nbsp;&nbsp;&nbsp;&nbsp;";
                    }
                    else {
                        if (m_vmCpuCount < 1000000){
                            space = "&nbsp;&nbsp;&nbsp;";
                        }
                        else {
                            if (m_vmCpuCount < 10000000){
                                space = "&nbsp;&nbsp;";
                            }
                            else {
                                space = "&nbsp;";
                            }
                        }
                    }
                }
            }
        }
    }
    return space;
}

uint DialogManageCpu::getVmMaxCpuCount(){
    return m_maxVmCpuCount;
}

uint DialogManageCpu::getVmSockets(){
    return m_sockets;
}

uint DialogManageCpu::getVmCores(){
    return m_cores;
}

uint DialogManageCpu::getVmThreads(){
    return m_threads;
}

QString DialogManageCpu::getVmCpuModel(){
    int idx = m_vCpuModelValue->currentIndex();
    return m_vCpuModelValue->itemData(idx).toString();
}

void DialogManageCpu::setVmCpuInfo(){

    uint sockets = getVmSockets();
    uint cores = getVmCores();
    uint threads = getVmThreads();
    QString vmCpuModel = getVmCpuModel();

    emit requestVmCpuInfoXmlUpdate(sockets, cores, threads, vmCpuModel);
    this->accept();
}



// End dialogManageCpu.cpp
