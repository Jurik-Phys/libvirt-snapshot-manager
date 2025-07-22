// Begin snapManager.h
#ifndef SNAPMANAGER_H
#define SNAPMANAGER_H

#include <QVector>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QDomDocument>
#include <QFileInfo>
#include <QDir>
#include "vmDataStructs.h"

class SnapManager : public QObject {

    Q_OBJECT

    public:
        SnapManager(QWidget* parent = nullptr);
        ~SnapManager();

        void doSnapshot(const QString& vmName, const QStringList& mntStorages);

    private:
        VMachine m_vm;
        QWidget* parentWindow;

        QString getStorageId(const QString& mntStorage);
        QString getSnapName(const QString& imgName, const QString& id,
                                                       const QString& parentId);
        void switchVmMountStorages(const QString& vmName,
                                               const QStringList& snapStorages);
};

#endif
// End snapManager.h
