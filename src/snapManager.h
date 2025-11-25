// Begin snapManager.h
#ifndef SNAPMANAGER_H
#define SNAPMANAGER_H

#include <QVector>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QProgressDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QDomDocument>
#include <QFileInfo>
#include <QDir>
#include <QEventLoop>
#include <QTimer>
#include <QTemporaryFile>
#include "vmDataStructs.h"

class SnapManager : public QObject {

    Q_OBJECT

    public:
        SnapManager(QWidget* parent = nullptr);
        ~SnapManager();

        QStringList doSnapshot(const QString& vmName, const QString& snapName,
                      const QStringList& mntStorages, bool silenceFlag = false);
        QStringList gotoSnapshot(const QString& vmName, const ChainNode& node);
        bool deleteSnapshot(const QString& vmName, const QString& snapName,
                                                         const ChainNode& node);
        bool deleteImageFiles(const QStringList&);

        void detachBlockDevice(const QString& vmName, const QString&);
        void mountBlockDevice(const QString& vmName,
                              const QString& imageFullName,
                              const QString& blockDevice,
                              const QString& busType);
        void createQcow2Image(const QString& imageFile,
                              const QString& backingFile, const QString& iSize);
        void createQcow2Images(const QVector<ChainNode>& vmNodes,
                               const QString& newImageSize,
                               const int& insertDriveIndex);
    private:
        VMachine m_vm;
        QWidget* parentWindow;

        QString getSnapName(const QString& imgName, const QString& id);
        void switchVmMountStorages(const QString& vmName,
                                               const QStringList& snapStorages);
        bool rebaseImages(const QStringList& parentImages,
                                const QStringList& idImages,
                                    const QVector<QStringList>& childrenImages);
        void doNewRoot(const QStringList& idImgs, const QStringList& childImgs);


        bool checkWriteAccessToDirs(const QStringList& dirsForWriteCheck,
                                            QStringList* noWriteDirs = nullptr);
        bool checkAllImagesReadable(const QStringList&);
        QProgressDialog* createNewQProgressDialog(const int& maxValue,
                                                     QWidget* parent = nullptr);
        long int getRebaseDataValue(const QString& backFullName,
                                                    const QString& rebaseImage);
        const QString m_libVirtConnectURI = "qemu:///system";
        void sleep(const int&);

        QStringList doSnapshotOverQemuImg(const QString& vmName,
                                        const QStringList& workDisks,
                                        const QStringList& snapshotsFullNames);
        QStringList doSnapshotOverVirsh(const QString& vmName,
                                        const QStringList& workDisk,
                                        const QStringList& snapshotsFullNames);
        void turnOnOffVirtualMachine(const QString& vmName);
        QString removeVmXmlNetwork(const QString& vmXml);
};

#endif
// End snapManager.h
