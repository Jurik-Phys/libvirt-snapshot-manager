// Begin vmWidget.h
#ifndef VMWIDGET_H
#define VMWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QVector>
#include <QStringList>
#include <QBoxLayout>
#include "vmDataStructs.h"

class VmWidget : public QFrame {

    Q_OBJECT

    public:
        VmWidget(const VMachine&, QWidget* parent = nullptr);
        ~VmWidget();
        void setSelected(bool);

        static void setLoadingFlag(bool);
        void setProperties(const VMachine&);

    signals:
        void clicked(VmWidget*);

    protected:
        void enterEvent(QEnterEvent*) override;
        void leaveEvent(QEvent*) override;
        void mousePressEvent(QMouseEvent*) override;

    private:
        QFrame* m_vmIcon;
        QLabel* m_vmName;
        QLabel* m_vmState;
        QHBoxLayout* m_vmHFrameLayout;
        QVBoxLayout* m_vmVTextLayout;
        int m_vmIconSize = 32;
        bool m_selected = false;

        void setDefaultStyle();
        void setHoverStyle();
        void setSelectedStyle();
        void updateStyle();

        static bool m_isLoading;

        void setName(const QString&  vmName);
        void setState(const QString& vmState);
};

#endif
// End vmWidget.h
