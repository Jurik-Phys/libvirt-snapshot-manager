// Begin vmInfoWidget.h
#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QProcess>
#include <QEventLoop>
#include <QKeyEvent>
#include <QEvent>
#include <QDomDocument>
#include <QFile>
#include <QLabel>
#include <QTimer>
#include "vmDataStructs.h"

class VmInfoWidget : public QFrame {

    Q_OBJECT

    public:
        VmInfoWidget(QWidget* parent = nullptr);
        ~VmInfoWidget();

        void setData(const VMachine&);
        VMachine getData();
        void setStorageList(const QStringList&);

        void setName(const QString&);
        void setTitle(const QString&);
        void setDescription(const QString&);
        void setCpu(const QString&);
        void setRam(const QString&);
        void setOsId(const QString&);
        void setReadOnly(bool);
        void clearData();

    signals:
        void textChangedBegin();
        void textChangedEnd();
        void writeVmTitleRequested(const QString& vm_uuid,
                                                        const QString& vmTitle);
        void writeVmDescriptionRequested(const QString& vm_uuid,
                                                  const QString& vmDescription);

    private:
        QVBoxLayout* m_vScrollLayout;
        void fixScrollBar(QScrollArea*);
        void fixScrollBar(QTextEdit*);

        QVector<QWidget*> m_colAWidgets;
        QVector<QWidget*> m_colBWidgets;
        QString humanMemory(const QString&);

        QString m_uuid;
        void writeVmTitle();
        void writeVmDescription();
        void restartSaveTitleTimer();
        void restartSaveDescriptionTimer();
        bool eventFilter(QObject* obj, QEvent* event) override;

        QTimer* m_saveTitleTimer;
        QTimer* m_saveDescriptionTimer;

        int m_titleChangedCounter = 0;
        int m_descriptionChangedCounter = 0;
        int calcOptimalFontSize(const QStringList&);
};

#endif
// End vmInfoWidget.h
