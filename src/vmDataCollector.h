// Begin vmDataCollector.h

#ifndef VMDATACOLLECTOR_H
#define VMDATACOLLECTOR_H

#include <QDir>
#include <QSet>
#include <QThread>
#include <QProcess>
#include <QtEndian>
#include <QFileInfo>
#include <QMessageBox>
#include <QDomDocument>
#include <QCryptographicHash>
#include <QRegularExpression>
#include "vmDataStructs.h"

class VmDataCollector : public QObject {

    Q_OBJECT

    public:
        VmDataCollector(QWidget *parent = nullptr);
        VmDataCollector(const VMachine& vm, QWidget* parent = nullptr);
        ~VmDataCollector();

        VMachine getVmInfo();
        QVector<VMachine> getVmList();

    public slots:
        void process();

    signals:
        void finished(const VMachine&);
        void errorMsg(const QString& title, const QString& message);

    private:
        VMachine m_vm;
        VMachine getVmInfo(const VMachine& vm);
        QDomDocument getVmXml(const QString& vmName);
        void setSnapChainData(VMachine& vm);
        void setChildrenData(VMachine& vm);
        bool isVMachineImage(const QString& imageFullName);
        void loadVmImagesRawInfoOverQEMU(const QString& dir);
        QString getBackFullNameQEMU(const VmImageRawInfo&);
        QString getBackFullNameFast(const QString& fullFileName);
        QString getRootFullName(const QString& fileFullName);
        QVector<VmImageRawInfo> m_vmImagesRawInfo;
        QString getNodeName(const QString& imgFileName, const QString& imgType);

        QWidget* parentWindow;
};

#endif
// End vmDataCollector.h
