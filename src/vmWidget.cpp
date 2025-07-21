// Begin vmWidget.cpp

#include "vmWidget.h"

VmWidget::VmWidget(int idx, const QVector<VMachine>& vmList, QWidget* parent){
    m_idx = idx;
    m_vmList = vmList;

    m_vmIcon   = new QFrame(this);

    m_vmIcon->setStyleSheet("background-color: red;");
    m_vmIcon->setFixedSize(QSize(m_vmIconSize, m_vmIconSize));

    m_vmVTextLayout = new QVBoxLayout();
    m_vmVTextLayout->setSpacing(0);

    // Virtual machine name
    m_vmLabel  = new QLabel(this);
    m_vmLabel->setText("<b>"+m_vmList[m_idx].name+"</b>");

    // m_vmLabel->setAlignment(Qt::AlignBottom);
    m_vmVTextLayout->addWidget(m_vmLabel);

    // Virtual machine status
    m_vmState = new QLabel(this);
    // m_vmState->setStyleSheet("background-color: orange;");
    QString state = m_vmList[m_idx].state;
    state[0] = state[0].toUpper();
    m_vmState->setText(state);
    // m_vmState->setAlignment(Qt::AlignTop);
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
    m_selected = true;
    updateStyle();
    emit clicked(this);
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

// End vmWidget.cpp
