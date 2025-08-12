// Begin vmWidget.cpp

#include "vmWidget.h"

bool VmWidget::m_isLoading = false;

VmWidget::VmWidget(const VMachine& inVm, QWidget* parent){
    m_vmIcon   = new QFrame(this);

    m_vmIcon->setStyleSheet("background-color: red;");
    m_vmIcon->setFixedSize(QSize(m_vmIconSize, m_vmIconSize));

    m_vmVTextLayout = new QVBoxLayout();
    m_vmVTextLayout->setSpacing(0);

    // Virtual machine name
    m_vmName  = new QLabel(this);
    setName(inVm.name);

    // m_vmName->setAlignment(Qt::AlignBottom);
    m_vmVTextLayout->addWidget(m_vmName);

    // Virtual machine status
    m_vmState = new QLabel(this);
    // m_vmState->setStyleSheet("background-color: orange;");
    setState(inVm.state);

    m_vmVTextLayout->addWidget(m_vmState);

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

// End vmWidget.cpp
