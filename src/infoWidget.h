// Begin infoWidget.h
#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include "vmDataStructs.h"

class InfoWidget : public QFrame {

    Q_OBJECT

    public:
        InfoWidget(QWidget* parent = nullptr);
        ~InfoWidget();

        void setData(const VMachine&);
        void setStorageList(const QStringList&);

    private:
        QVBoxLayout* m_vScrollLayout;
        void fixScrollBar(QScrollArea*);
        void fixScrollBar(QTextEdit*);

        QVector<QWidget*> m_colAWidgets;
        QVector<QWidget*> m_colBWidgets;
        QString humanMemory(const QString&);
};

#endif
// End infoWidget.h
