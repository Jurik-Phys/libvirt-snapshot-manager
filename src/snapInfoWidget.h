// Begin snapInfoWidget.h

#ifndef SNAPINFOWIDGET_H
#define SNAPINFOWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QScrollArea>
#include <QScrollBar>
#include "vmDataStructs.h"

class SnapInfoWidget : public QFrame {

    Q_OBJECT

    public:
        SnapInfoWidget(QWidget* parent = nullptr);
        ~SnapInfoWidget();

        void setData(const ChainNode& node);

    private:
        QVBoxLayout*      m_vScrollLayout;
        void fixScrollBar(QTextEdit*);
        void fixScrollBar(QScrollArea*);
        QVector<QWidget*> m_colAWidgets;
        QVector<QWidget*> m_colBWidgets;

        bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif
// End snapInfoWidget.h
