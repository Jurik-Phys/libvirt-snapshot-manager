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

class SnapManager : public QObject {

    Q_OBJECT

    public:
        SnapManager(QObject* parent = nullptr);
        SnapManager(const QString& vmName, QObject* parent = nullptr);
        ~SnapManager();

        QVector<QStringList> getVmList();
        void takeSnapshot(const QString& vmName);
        void setVmName(const QString&);
        QVector<SnapNode> getSnapTreeModelData(const QString& vmName);
    public slots:
        void process();
    signals:
        void finished(QVector<SnapNode> result);

    private:
        QString m_vmName;
        QDomElement getVmXml(const QString& vmName);
        QStringList getFullPathDisks(const QString& vmName);
        QStringList getFileNameDisks(const QStringList& disksList);
        QStringList getBaseDirsList(const QStringList& disksList);
        QString     getRootDisksChain(const QString& fullPathDisks);
        QString     getCoreFileNames(const QStringList& filesList);
        QString     getBackingFile(const QString& qemuImgOut);
};

#endif
// End snapManager.h
