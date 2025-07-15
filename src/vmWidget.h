// Begin vmWidget.h
#ifndef VMWIDGET_H
#define VMWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QVector>
#include <QStringList>
#include <QBoxLayout>

class VmWidget : public QFrame {

    Q_OBJECT

    public:
        VmWidget(int, QVector<QStringList>, QWidget* parent = nullptr);
        ~VmWidget();
        void setSelected(bool);

    signals:
        void clicked(VmWidget*);

    protected:
        void enterEvent(QEnterEvent*) override;
        void leaveEvent(QEvent*) override;
        void mousePressEvent(QMouseEvent*) override;

    private:
        QFrame* m_vmIcon;
        QLabel* m_vmLabel;
        QLabel* m_vmState;
        QHBoxLayout* m_vmHFrameLayout;
        QVBoxLayout* m_vmVTextLayout;
        int m_vmIconSize = 32;
        int m_idx = -1;
        QVector<QStringList> m_vmList;
        bool m_selected = false;

        void setDefaultStyle();
        void setHoverStyle();
        void setSelectedStyle();
        void updateStyle();
};

#endif
// End vmWidget.h
