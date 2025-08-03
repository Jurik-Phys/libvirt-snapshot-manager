// Begin snapManager.h
#ifndef SNAPMANAGER_H
#define SNAPMANAGER_H

#include <QVector>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
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
#include "vmDataStructs.h"

class SnapManager : public QObject {

    Q_OBJECT

    public:
        SnapManager(QWidget* parent = nullptr);
        ~SnapManager();

        QStringList doSnapshot(const QString& vmName, const QStringList& mntStorages,
                                                bool silenceFlag = false);
        void gotoSnapshot(const QString& vmName, const ChainNode& node);
        bool deleteSnapshot(const QString& vmName, const ChainNode& node);

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
};

#endif
// End snapManager.h
