// Begin vmDataCollector.h

#ifndef VMDATACOLLECTOR_H
#define VMDATACOLLECTOR_H

#include <QDir>
#include <QSet>
#include <QTimer>
#include <QThread>
#include <QProcess>
#include <QtEndian>
#include <QUuid>
#include <QEventLoop>
#include <QFileInfo>
#include <QMessageBox>
#include <QDomDocument>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QTextDocumentFragment>
#include "vmDataStructs.h"

class VmDataCollector : public QObject {

    Q_OBJECT

    public:
        VmDataCollector(QWidget* parent = nullptr);
        VmDataCollector(const VMachine& vm, QWidget* parent = nullptr);
        ~VmDataCollector();

        VMachine getVmFullInfo();
        VMachine getVmShortInfo(const QString& uuid, bool* isOk = nullptr);
        QString getNodeName(const QString& imageFullName);
        QString getNodeUuid(const QString& imageFullName);

        void vmListStartTimer();
        void vmListStopTimer();
        void vmGeneralInfoStartTimer(const VMachine&);
        void vmGeneralInfoStartTimer();

        void vmGeneralInfoStopTimer();
        QVector<VMachine> getVmList(bool* isOk);

        void getSnapshotXmlInfo(ChainNode&);

        // *** Write VM & Snapshot info *** //
        void writeVmTitle(const QString& vm_uuid, const QString& vmTitle);
        void writeVmDescription(const QString& vm_uuid, const QString& vmDesc);
        void writeSnapTitle(const QStringList& uuid,
                                                   const QStringList& snapInfo);
        void writeSnapDescription(const QStringList& uuid,
                                                   const QStringList& snapInfo);
        void rmSnapshotXmlElement(const QStringList& uuid);

    public slots:
        void process();

    signals:
        void finished(const VMachine&);
        void errorMsg(const QString& title, const QString& message);
        void vmListReady(const QVector<VMachine> newVmList);
        void newVmInfoReady(const VMachine&);

    private:
        VMachine m_vm;
        const QString m_libVirtConnectURI = "qemu:///system";
        VMachine getVmFullInfo(const VMachine& vm);
        QDomDocument getVmXml(const QString& vmUuid);
        void pushVmXml(const QDomDocument& vmXmlDoc);
        void setSnapChainData(VMachine& vm);
        void setChildrenData(VMachine& vm);
        bool isVMachineImage(const QString& imageFullName);
        QVector<VmImageRawInfo> loadVmImagesRawInfoOverQEMU(const QString& dir,
                                                         bool* isOkLoadRawInfo);
        QVector<VmImageRawInfo> rmExtBackingInfo(const QVector<VmImageRawInfo>&,
                                              const QStringList& mountStorages);
        QString getBackFullNameQEMU(const VmImageRawInfo&);
        QString getBackFullNameFast(const QString& fullFileName);
        QString getRootFullName(const QString& fileFullName);
        QVector<VmImageRawInfo> m_vmImagesRawInfo;
        int getFileNameId(const QString& imgFileName);
        bool checkExtBackChainFiles(const QVector<VmImageRawInfo>& imgsRawInfo,
                                                    const QString& backingFile);
        bool checkBackingFile(const QVector<VmImageRawInfo>& imgsRawInfo,
                                                    const QString& backingFile);
        bool checkNodeFilesCount(const QVector<VmImageRawInfo>& imgsRawInfo,
                                const QSet<int>& idSet,
                                const QStringList& mountStorages);
        QWidget* parentWindow;
        QTimer*  m_getListTimer;
        QTimer*  m_getActualVmGeneralInfoTimer;
        void vmListSender();
        void selectedVmActualInfoSender();

        QDomElement findOrCreateElement(QDomDocument& doc, QDomElement& parent,
                                                            const QString& tag);
        QDomElement findOrCreateSnapshotElement(QDomDocument& doc,
                               QDomElement& snapInfo, const QString& snap_uuid);

        void writeQDomElementText(QDomDocument& doc, QDomElement& el,
                                                          const QString& value);
        QStringList getExtMountStorages(const QVector<VmImageRawInfo>&,
                                              const QStringList& mountStorages);
};

#endif
// End vmDataCollector.h
