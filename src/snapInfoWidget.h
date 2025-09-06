// Begin snapInfoWidget.h

#ifndef SNAPINFOWIDGET_H
#define SNAPINFOWIDGET_H

#include <QFrame>
#include <QTimer>
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
        void setData(const VMachine& vm);
        void setReadOnly(bool);
        void clearData();

    signals:
        void writeSnapTitleRequested(const QStringList& uuid,
                                                   const QStringList& snapInfo);
        void writeSnapDescriptionRequested(const QStringList& uuid,
                                                   const QStringList& snapInfo);
    private:
        QVBoxLayout*      m_vScrollLayout;
        void fixScrollBar(QTextEdit*);
        void fixScrollBar(QScrollArea*);
        QVector<QWidget*> m_colAWidgets;
        QVector<QWidget*> m_colBWidgets;

        bool eventFilter(QObject* obj, QEvent* event) override;

        VMachine m_vm;
        ChainNode m_node;
        QTimer* m_saveTitleTimer;
        QTimer* m_saveDescriptionTimer;

        void restartSaveTitleTimer();
        void restartSaveDescriptionTimer();
        void writeSnapTitle();
        void writeSnapDescription();

        int m_titleChangedCounter = 0;
        int m_descriptionChangedCounter = 0;

        QString m_vmUUID;
        QString m_snapUUID;

};

#endif
// End snapInfoWidget.h
