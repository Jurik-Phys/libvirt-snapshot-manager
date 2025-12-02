// Begin osinfoloader.h
#ifndef OSINFOLOADER_H
#define OSINFOLOADER_H

#include <QObject>
#include <QDebug>

class OsInfoLoader : public QObject {

    Q_OBJECT

    public:
        OsInfoLoader(QObject* parent = nullptr);
        ~OsInfoLoader();

    void run();
};

#endif
// End osinfoloader.h
