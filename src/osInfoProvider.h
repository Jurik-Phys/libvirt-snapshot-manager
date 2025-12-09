// Begin osInfoProvider.h
#ifndef OSINFOPROVIDER_H
#define OSINFOPROVIDER_H

#include <QObject>
#include "osinfoloader.h"


class OsInfoProvider : public QObject {

    Q_OBJECT

    public:
        OsInfoProvider(QObject* parent = nullptr);
        ~OsInfoProvider();

        OsInfo getOsInfo(const QString& osId);

    private:
        QVector<OsInfo> m_osInfoData;
        unsigned int    m_libOsInfoVersion;

        void dataInit();

        void loadFromResources();
        void loadFromAppFile();
        void loadFromNetwork(const QString& url);
};

#endif
// End osInfoProvider.h
