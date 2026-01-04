// Begin osInfoProvider.h
#ifndef OSINFOPROVIDER_H
#define OSINFOPROVIDER_H

#include <QObject>
#include <QStandardPaths>
#include "osinfoloader.h"


class OsInfoProvider : public QObject {

    Q_OBJECT

    public:
        OsInfoProvider(QObject* parent = nullptr);
        ~OsInfoProvider();

        OsInfo getOsInfoByOsId(const QString& osId);
        OsInfo getOsInfoByOsName(const QString&);
        unsigned int getLocalLibOsInfoVersion();
        unsigned int getLatestLibOsInfoVersion();
        void updateLocalLibOsInfo();
        void reloadData();
        QStringList getOsNameList();

    signals:
        void localOsInfoUpdateFinished();

    private:
        QVector<OsInfo> m_osInfoData;
        unsigned int    m_libOsInfoVersion;
        OsInfoLoader*   m_osInfoLoader;

        void dataInit();

        void loadFromJson(const QString&, QVector<OsInfo>&, unsigned int&);
        void loadFromResources(QVector<OsInfo>&, unsigned int&);
        void loadFromAppConfigDir(QVector<OsInfo>&, unsigned int&);
        void loadFromNetwork(const QString& url);

        QString m_appConfigDirName = "libvirt-snapshot-manager";
        QString m_localLibOsInfo   = "libOsInfo.json";
        QString getLocalLibOsInfoFullFileName();

        void backupLocalLibOsInfo(const QString&);
};

#endif
// End osInfoProvider.h
