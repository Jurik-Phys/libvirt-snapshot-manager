// Begin vmDataCollector.h

#ifndef VMDATACOLLECTOR_H
#define VMDATACOLLECTOR_H

#include <QDir>
#include <QSet>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>
#include <QDomDocument>
#include <QHash>
#include <QRegularExpression>
#include "vmDataStructs.h"

class VmDataCollector {

    public:
        VmDataCollector(QWidget *parent = nullptr);
        ~VmDataCollector();

        VMachine getVmInfo(const VMachine& vm);
        QVector<VMachine> getVmList();

    private:
        QDomDocument getVmXml(const QString& vmName);
        void setSnapChainData(VMachine& vm);
        bool isVMachineImage(const QString& imageFullName);
        void loadVmImagesRawInfoOverQEMU(const QString& dir);
        QString getBackFullNameQEMU(const VmImageRawInfo&);
        QString getBackFullNameFast(const QString& fullFileName);
        QString getRootFullName(const QString& fileFullName);
        QVector<VmImageRawInfo> m_vmImagesRawInfo;
        QString getNodeName(const QString& imgFileName, const QString& imgType);

        QWidget *parentWindow;
};

#endif
// End vmDataCollector.h
