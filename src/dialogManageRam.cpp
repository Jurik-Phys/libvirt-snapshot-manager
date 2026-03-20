// Begin dialogManageRam.cpp

#include <QFrame>
#include <QLabel>
#include <QFormLayout>
#include "dialogManageRam.h"

DialogManageRam::DialogManageRam(const QString& osNameIn, const QString& vmRam,
              const QString& vmMinRam, const QString& hostRam, QWidget* parent)
                                                              : QDialog(parent){
    QString osName = checkOsName(osNameIn);
    QString minRam = checkVmMinRam(osName, vmMinRam);

    double frameWidthRatio = 1.7;
    double leftWidthRatio  = 0.42;
    double rightWidthRatio = 0.51;

    int    frameWidth = qRound(frameWidthRatio * parent->width() - 22);
    int    leftWidth  = qRound(frameWidth * leftWidthRatio);
    int    rightWidth = qRound(frameWidth * rightWidthRatio);

    long int vmRamBytes = this->vmRamToBytes(vmRam);

    // *** parent is VmInfoWidget *** //
    this->setFixedWidth(frameWidth + 22);
    this->setWindowTitle("VM RAM setup");

    // *** Dialog main vertical layout *** //
    QVBoxLayout* dlgVBoxLayout = new QVBoxLayout(this);

    // *** Title box *** //
    QFrame* dlgTitleFrame = new QFrame(this);
    dlgTitleFrame->setFrameShape(QFrame::StyledPanel);
    dlgTitleFrame->setFrameShadow(QFrame::Plain);
    dlgTitleFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgTitleFrame->setFixedWidth(frameWidth);

    QHBoxLayout* dlgTitleFrameHLayout = new QHBoxLayout(dlgTitleFrame);
    dlgTitleFrameHLayout->setAlignment(Qt::AlignHCenter);
    QLabel* dlgText = new QLabel("Manage virtual machine memory settings");
    dlgTitleFrameHLayout->addWidget(dlgText);

    // *** Main box *** //
    QFrame* dlgInfoFrame = new QFrame(this);
    dlgInfoFrame->setFrameShape(QFrame::StyledPanel);
    dlgInfoFrame->setFrameShadow(QFrame::Plain);
    dlgInfoFrame->setFixedWidth(frameWidth);

    QFormLayout* infoFrameLayout = new QFormLayout(dlgInfoFrame);
    infoFrameLayout->setLabelAlignment(Qt::AlignRight);
    infoFrameLayout->setFormAlignment(Qt::AlignLeft);
    infoFrameLayout->setVerticalSpacing(15);

    // *** [2.1] Main box. Opetration system info line *** //
    // Title
    QLabel* osTitle = new QLabel("Guest operating system:");
    osTitle->setFixedWidth(leftWidth);
    osTitle->setAlignment(Qt::AlignRight);
    // Value
    QLabel* osNameValue = new QLabel(osName);
    osNameValue->setFixedWidth(rightWidth);
    osNameValue->setAlignment(Qt::AlignRight);
    // Title + Value
    QHBoxLayout* osInfoHLayout = new QHBoxLayout();
    osInfoHLayout->addWidget(osTitle, 0, Qt::AlignVCenter);
    osInfoHLayout->addWidget(osNameValue, 0, Qt::AlignVCenter);
    // Title + Value to frame layout
    infoFrameLayout->addRow(osInfoHLayout);

    // *** [2.2] Minimum memory for the guest OS line *** //
    // >>>
    QLabel* minRamTitle = new QLabel("Recommended minimum memory:");
    minRamTitle->setFixedWidth(leftWidth);
    minRamTitle->setAlignment(Qt::AlignRight);
    // >>>
    QLabel* minRamValue = new QLabel(minRam);
    minRamValue->setFixedWidth(rightWidth);
    minRamValue->setAlignment(Qt::AlignRight);
    // >>>
    QHBoxLayout* minRamHLayout = new QHBoxLayout();
    minRamHLayout->addWidget(minRamTitle, 0, Qt::AlignVCenter);
    minRamHLayout->addWidget(minRamValue, 0, Qt::AlignVCenter);
    // Title + Value to frame layout
    infoFrameLayout->addRow(minRamHLayout);

    // *** [2.3] Host Minimum memory for the guest OS line *** //
    // >>>
    QLabel* hostRamTitle = new QLabel("Memory available on the host:");
    hostRamTitle->setFixedWidth(leftWidth);
    hostRamTitle->setAlignment(Qt::AlignRight);
    // >>>
    double hostRamDouble = hostRam.split(" ").first().toDouble();
    QString roundHostRam = QString::number(hostRamDouble, 'f', 1);
    QString ruHostRam = (roundHostRam
                            + " " + hostRam.split(" ").last()).replace(".",",");
    QLabel* hostRamValue = new QLabel(ruHostRam);
    hostRamValue->setFixedWidth(rightWidth);
    hostRamValue->setAlignment(Qt::AlignRight);
    // >>>
    QHBoxLayout* hostRamHLayout = new QHBoxLayout();
    hostRamHLayout->addWidget(hostRamTitle, 0, Qt::AlignVCenter);
    hostRamHLayout->addWidget(hostRamValue, 0, Qt::AlignVCenter);
    // Title + Value to frame layout
    infoFrameLayout->addRow(hostRamHLayout);

    // *** [2.4] Virtual Machine RAM *** //
    // >>>
    QLabel* vmRamTitle = new QLabel("Virtual Machine memory:");
    vmRamTitle->setFixedWidth(leftWidth);
    vmRamTitle->setAlignment(Qt::AlignRight);
    // >>>
    m_vmRamValue = new QDoubleSpinBox();

    float vmMinRamNumber = minRam.split(" ").first().toFloat();

    float vmRamUpLimit   = getVmRamUpLim(minRam, hostRam);
    float vmRamDownLimit = getVmRamDownLim(minRam);
    float vmRamCurrent   = getVmRamCurrent(minRam, vmRamBytes);

    m_vmRamValue->setRange(vmRamDownLimit , vmRamUpLimit);
    m_vmRamValue->setValue(vmRamCurrent );
    m_vmRamValue->setSingleStep(0.1);
    m_vmRamValue->setDecimals(1);
    m_vmRamValue->setSuffix(" " + minRam.split(" ").last());
    m_vmRamValue->setFixedWidth(qRound(rightWidth * 0.35));
    m_vmRamValue->setAlignment(Qt::AlignRight);
    // >>>
    QHBoxLayout* vmRamHLayout = new QHBoxLayout();
    vmRamHLayout->addWidget(vmRamTitle, 0, Qt::AlignVCenter);
    vmRamHLayout->addSpacing(rightWidth * 0.72);
    vmRamHLayout->addWidget(m_vmRamValue, 0, Qt::AlignVCenter);
    // >>>
    infoFrameLayout->addRow(vmRamHLayout);

    // *** [2.5] Virtual Machine RAM (slider) *** //
    QVBoxLayout* sliderVLayout = new QVBoxLayout();
    sliderVLayout->setSpacing(4);
    m_vmRamSlider = new QSlider(Qt::Horizontal, this);
    if ( minRam.split(" ").last() == "GiB" ){
        m_sliderScale = 10;
    }
    else {
        m_sliderScale = 1;
    }
    m_vmRamSlider->setRange(vmRamDownLimit*m_sliderScale,
                                                    vmRamUpLimit*m_sliderScale);
    m_vmRamSlider->setValue(vmRamCurrent*m_sliderScale);
    sliderVLayout->addWidget(m_vmRamSlider);

    // *** [2.6] Slider details *** //
    QString vmRamDownLimitStr = (QString::number(vmRamDownLimit, 'f', 1)
                           + " " + minRam.split(" ").last()).replace(".",",");
    QLabel* sliderMinValue = new QLabel(vmRamDownLimitStr);
    float avgRamBytes = hostRamToBytes(hostRam)/2.0;
    float avgVal = getVmAvgRam(minRam, avgRamBytes);
    QString avgRam = QString::number(avgVal, 'f', 1)
                                               + " " + minRam.split(" ").last();
    QString ruAvgRam = avgRam.replace(".", ",");
    QLabel* sliderAvgValue = new QLabel(ruAvgRam);
    QString vmRamUpLimitStr = (QString::number(vmRamUpLimit, 'f', 1)
                             + " " + minRam.split(" ").last()).replace(".",",");
    QLabel* sliderMaxValue = new QLabel(vmRamUpLimitStr);
    // >>
    QHBoxLayout* sliderDetailsHLayout = new QHBoxLayout();
    sliderDetailsHLayout->setSpacing(0);
    sliderDetailsHLayout->addWidget(sliderMinValue, 0, Qt::AlignVCenter);
    sliderDetailsHLayout->addStretch();
    sliderDetailsHLayout->addWidget(sliderAvgValue, 0, Qt::AlignVCenter);
    sliderDetailsHLayout->addStretch();
    sliderDetailsHLayout->addWidget(sliderMaxValue, 0, Qt::AlignVCenter);
    // >>>
    sliderVLayout->addLayout(sliderDetailsHLayout);
    infoFrameLayout->addRow(sliderVLayout);

    // *** QSlider => QDoubleSpinBox *** //
    QObject::connect(m_vmRamSlider, &QSlider::valueChanged, this,
            [=](int val){
                double realValue = val/ static_cast<double>(m_sliderScale);
                m_vmRamValue->setValue(realValue);
            });
    // *** QDoubleSpinBox => QSlider *** //
    QObject::connect(m_vmRamValue, &QDoubleSpinBox::valueChanged, this,
            [=](double val){
                int sliderValue = val * m_sliderScale;
                m_vmRamSlider->setValue(sliderValue);
            });

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
                                                this, &DialogManageRam::setRam);
    QObject::connect(m_dlgBtnBox, &QDialogButtonBox::rejected,
                                                this, &DialogManageRam::reject);

    // *** Final grouping widget *** //
    dlgVBoxLayout->addWidget(dlgTitleFrame);
    dlgVBoxLayout->addWidget(dlgInfoFrame);
    dlgVBoxLayout->addWidget(dlgBtnFrame);
}

DialogManageRam::~DialogManageRam(){
}

void DialogManageRam::setRam(){
    long int memoryInKiB = getManualMemoryInKiB();
    emit requestVmRamXmlUpdate(memoryInKiB);
    this->accept();
}

void DialogManageRam::setHostRamInBytes(const long int& hostRamInBytes){
    m_hostRamInBytes = hostRamInBytes;
}

long int DialogManageRam::getManualMemoryInKiB(){
    long int res;
    double memValue = m_vmRamValue->value();
    QString suffix = m_vmRamValue->suffix().simplified();

    if ( suffix == "GiB" ){
        res = qRound(memValue * 1024 * 1024);
    }

    if ( suffix == "MiB" ){
        res = qRound(memValue * 1024);
    }

    // *** В результате округления, устанавливаемое значение RAM *** //
    //     может быть больше, чем есть в хостовой системе.           //
    //     Для libvirt это не критично, но для логики, заложеной     //
    //     в интерфейсе пусть будет ограничение по размеру памяти    //
    //     хостовой системы                                          //
    long int hostRamInKiB = m_hostRamInBytes / 1024;
    if ( res > hostRamInKiB ){
        res = hostRamInKiB;
    }

    return res;
}

long int DialogManageRam::vmRamToBytes(const QString& vmRam){
    return strRamToBytes(QString(vmRam).replace(",", "."));
}

long int DialogManageRam::hostRamToBytes(const QString& hostRam){
    long int res;
    QString enHostRam = QString(hostRam).replace(",", ".");
    res = strRamToBytes(enHostRam);
    return res;
}

long int DialogManageRam::strRamToBytes(const QString& memString){
    long int res;
    QString ram   = memString.split(" ").first().simplified();
    QString memSuffix = memString.split(" ").last().simplified();
    double vmRamValue = ram.toDouble();

    if (memSuffix == "GiB"){
        res = vmRamValue*1024*1024*1024;
    }
    else {
        if (memSuffix == "MiB"){
            res = vmRamValue*1024*1024;
        }
        else {
            if (memSuffix == "KiB"){
                res = vmRamValue*1024;
            }
        }
    }

    return res;
}

float DialogManageRam::getVmRamUpLim(const QString& vmMinRam,
                                                        const QString& hostRam){
    float vmRamUpLimit;
    QString vmMinRamSuffix = vmMinRam.split(" ").last();
    float hostRamNumber = hostRam.split(" ").first().toFloat();

    if (vmMinRamSuffix == "GiB") {
        vmRamUpLimit = hostRamNumber;
    }
    else {
        if (vmMinRamSuffix == "MiB") {
            vmRamUpLimit = hostRamNumber * 1024;
        }
        else {
            if (vmMinRamSuffix == "KiB") {
                vmRamUpLimit = hostRamNumber * 1024 * 1024;
            }
        }
    }

    return qRound(vmRamUpLimit * 10.0) / 10.0;
}

float DialogManageRam::getVmRamDownLim(const QString& vmMinRam){
    float vmRamDownLimit;
    QString vmMinRamSuffix = vmMinRam.split(" ").last();

    if (vmMinRamSuffix == "GiB") {
        vmRamDownLimit = 0.1;
    }
    else {
        if (vmMinRamSuffix == "MiB") {
            vmRamDownLimit = 1.0;
        }
        else {
            if (vmMinRamSuffix == "KiB") {
                vmRamDownLimit = 640.0;
            }
        }
    }

    return vmRamDownLimit;
}

float DialogManageRam::getVmAvgRam(const QString& vmMinRam,
                                                    const float& vmAvgRamBytes){
    float vmAvgRam;
    QString vmMinRamSuffix = vmMinRam.split(" ").last();

    if (vmMinRamSuffix == "GiB") {
        vmAvgRam = vmAvgRamBytes / (1024. * 1024. * 1024.);
    }
    else {
        if (vmMinRamSuffix == "MiB") {
            vmAvgRam = vmAvgRamBytes / (1024. * 1024.);
        }
        else {
            if (vmMinRamSuffix == "KiB") {
                vmAvgRam = vmAvgRamBytes / 1024.;
            }
        }
    }

    return vmAvgRam;
}

float DialogManageRam::getVmRamCurrent(const QString& vmMinRam,
                                                       const float& vmRamBytes){
    double vmRamCurrent;

    QString vmMinRamSuffix = vmMinRam.split(" ").last();

    if (vmMinRamSuffix == "GiB") {
        vmRamCurrent = vmRamBytes / (1024. * 1024. * 1024.);
    }
    else {
        if (vmMinRamSuffix == "MiB") {
            vmRamCurrent = vmRamBytes / (1024. * 1024.);
        }
        else {
            if (vmMinRamSuffix == "KiB") {
                vmRamCurrent = vmRamBytes / 1024.;
            }
        }
    }

    // *** Округление до десятых долей требуется для QSliner'а *** //
    return qRound(vmRamCurrent * 10.)/10.;
}

QString DialogManageRam::checkOsName(const QString& osNameIn){

    QString osName = osNameIn;

    if (osNameIn.contains("N/A:")){
        osName = "Unknown OS type. Update libOsInfo db";
    }

    return osName;
}

QString DialogManageRam::checkVmMinRam(const QString& osName,
                                                       const QString& vmMinRam){
    QString minRam = vmMinRam;

    if (osName == "Not specified" || osName.contains("Unknown OS type.")){
        minRam = "-- GiB";
    }
    else {
        // *** В libOsInfo не для всех осей есть рекомендованая память *** //
        if (vmMinRam.split(" ").first() == "0"){
            minRam = "N/A GiB";
        }
    }

    return minRam;
}

// End dialogManageRam.cpp
