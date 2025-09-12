// Begin vmWidget.cpp

#include "vmWidget.h"

bool VmWidget::m_isLoading = false;

VmWidget::VmWidget(const VMachine& inVm, QWidget* parent){
    m_vmIcon   = new QLabel(this);

    m_vmIcon->setScaledContents(true);
    m_vmIcon->setFixedSize(QSize(m_vmIconSize, m_vmIconSize));

    m_vmVTextLayout = new QVBoxLayout();
    m_vmVTextLayout->setSpacing(0);

    // Virtual machine name
    m_vmName  = new QLabel(this);
    setName(inVm.name);

    m_vmVTextLayout->addStretch();
    m_vmVTextLayout->addWidget(m_vmName);
    m_vmVTextLayout->addSpacing(4);

    // Virtual machine status
    m_vmState = new QLabel(this);
    setState(inVm.state);
    setStateIcon(inVm.state);

    m_vmVTextLayout->addWidget(m_vmState);
    m_vmVTextLayout->addStretch();

    m_vmHFrameLayout = new QHBoxLayout(this);
    m_vmHFrameLayout->addWidget(m_vmIcon);
    m_vmHFrameLayout->addLayout(m_vmVTextLayout);
}

VmWidget::~VmWidget(){
}

void VmWidget::enterEvent(QEnterEvent* e) {
    if (!m_selected){
        setHoverStyle();
    }
}

void VmWidget::leaveEvent(QEvent* e) {
    if (!m_selected){
        setDefaultStyle();
    }
}

void VmWidget::mousePressEvent(QMouseEvent* e) {
    if (!m_isLoading){
        m_selected = true;
        updateStyle();
        emit clicked(this);
    }
}

void VmWidget::setDefaultStyle() {
    setStyleSheet("background-color: none;");
}

void VmWidget::setHoverStyle() {
    setStyleSheet("background-color: #badcef;");
}

void VmWidget::setSelectedStyle() {
    setStyleSheet("background-color: #3daee9;");
}

void VmWidget::setSelected(bool val){
    m_selected = val;
    updateStyle();
}

void VmWidget::updateStyle(){
    if (m_selected){
        setSelectedStyle();
    }
    else {
        setDefaultStyle();
    }
}

void VmWidget::setLoadingFlag(bool isLoad){
    m_isLoading = isLoad;
}

void VmWidget::setProperties(const VMachine& inVm){
    setName(inVm.name);
    setState(inVm.state);
    setStateIcon(inVm.state);
}

void VmWidget::setName(const QString& vmName){
    m_vmName->setText("<b>" + vmName + "</b>");
}

void VmWidget::setState(const QString& inVmState){
    QString vmState = inVmState;
    vmState[0] = vmState[0].toUpper();

    if (vmState == "Running" || vmState == "Paused"){
        m_vmState->setText(vmState + " | view-only snapshots");
    }
    else {
        m_vmState->setText(vmState);
    }
}

void VmWidget::setStateIcon(const QString& inVmState){

    if (inVmState == "shut off"){
        m_vmIcon->setPixmap(QPixmap(":/vm-status-stop.png"));
    }

    if (inVmState == "running"){
        m_vmIcon->setPixmap(QPixmap(":/vm-status-play.png"));
    }

    if (inVmState == "paused"){
        m_vmIcon->setPixmap(QPixmap(":/vm-status-pause.png"));
    }
}

// End vmWidget.cpp
