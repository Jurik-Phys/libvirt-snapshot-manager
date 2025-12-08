// Begin osinfoloader.h
#ifndef OSINFOLOADER_H
#define OSINFOLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QEventLoop>
#include <QObject>
#include <QDebug>
#include <QFile>
#include <QDir>

extern "C" {
    #include "lzma_decoder.h"
}

// *** *** ***  Описание формата *.tar (USTAR) файла  *** *** *** //
//     Каждый файл хранится блоками по 512 байт:                  //
//     - Заголовок (512 байт) содержит:                           //
//       - имя файла (100 байт)                                   //
//       - режим доступа (8 байт)                                 //
//       - uid (8 байт)                                           //
//       - gid (8 байт)                                           //
//       - размер файла (12 байт, восьмиричная строка)            //
//       - другие поля (не требуется в данном случае)             //
//     - Данне блоками по 512 байт с выравниванием:               //
//       - если файл 1000 байт → 2 блока (1024 байта),            //
//         последние 24 байта заполняются нулями (padding)        //
//     -  Архив заканчивается двумя блоками из нулей (1024 байта) //
// ************************************************************** //

struct TarFile {
    QString    fullName;
    QByteArray data;
};

struct OsInfo {
    QString vendor;
    QString name;
    QString id;
    QString ram;
};

class OsInfoLoader : public QObject {

    Q_OBJECT

    public:
        OsInfoLoader(QObject* parent = nullptr);
        ~OsInfoLoader();

        QJsonDocument getLibOsInfoJson(const QString& url = "None");
        void writeLibOsInfoJsonToFile(const QJsonDocument&,
                                    const QString& fileName = "libOsInfo.json");

    private:
        QByteArray xzLibInfoDownload(const QString&);
        QByteArray decompressXZ(const QByteArray& xzData);
        QVector<TarFile> unTar(const QByteArray& tarData);
        QVector<TarFile> getShortOsInfo(const QVector<TarFile>& tarData);

        QString  getLatestReleaseUrl();
        long int getRemoteFileSize(const QUrl&);
        long int octToDec(const QByteArray &octalStr);
        QString  getLibOsInfoVersion(const QVector<TarFile>&);
        OsInfo   getOsInfo(const QByteArray&);
};

#endif
// End osinfoloader.h
