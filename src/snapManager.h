// Begin snapManager.h
#ifndef SNAPMANAGER_H
#define SNAPMANAGER_H

#include <QVector>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QProcess>
#include <QDebug>
#include <QDomDocument>
#include <QFileInfo>
#include <QDir>
#include <QThread>
#include "snapTreeModel.h"
#include "vmDataCollector.h"

class SnapManager : public QObject {

    Q_OBJECT

    public:
        SnapManager(QObject* parent = nullptr);
        SnapManager(const VMachine& vm, QObject* parent = nullptr);
        ~SnapManager();

        QVector<QStringList> getVmList();
        void takeSnapshot(const VMachine& vm);
        void setVmName(const VMachine&);
        QVector<ChainNode> getSnapTreeModelData(const VMachine& vm);
    public slots:
        void process();
    signals:
        void finished(QVector<ChainNode> result);

    private:
        VMachine m_vm;
        QDomElement getVmXml(const QString& vmName);
        QStringList getFullPathDisks(const VMachine& vm);
        QStringList getFileNameDisks(const QStringList& disksList);
        QStringList getBaseDirsList(const QStringList& disksList);
        QString     getRootDisksChain(const QString& fullPathDisks);
        QString     getCoreFileNames(const QStringList& filesList);
        QString     getBackingFile(const QString& qemuImgOut);
};

#endif
// End snapManager.h
