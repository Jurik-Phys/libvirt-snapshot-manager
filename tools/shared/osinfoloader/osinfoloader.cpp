// Begin osinfoloader.cpp

#include "osinfoloader.h"

OsInfoLoader::OsInfoLoader(QObject* parent){
}

OsInfoLoader::~OsInfoLoader(){
}

QByteArray OsInfoLoader::decompressXZ(const QByteArray& xzData){
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret    ret;
    const uint32_t flags = LZMA_CONCATENATED;
    const uint64_t memory_limit = UINT64_MAX;

    // *** Инициализация lzma декодера *** //
    ret = lzma_auto_decoder(&strm, memory_limit, flags);
    if (ret != LZMA_OK) {
        qDebug() << "[EE] LZMA decoder initialize error:" << ret;
        return QByteArray();
    }

    QByteArray decompressed;
    const size_t CHUNK_SIZE = 64 * 1024;
    uint8_t outbuf[CHUNK_SIZE];

    strm.next_in = reinterpret_cast<const uint8_t*>(xzData.constData());
    strm.avail_in = xzData.size();
    strm.next_out = outbuf;
    strm.avail_out = CHUNK_SIZE;

    // Основной цикл декодирования
    do {
        if (strm.avail_in == 0) {
            // *** Данных для декодирования (больше) нет *** //
            ret = lzma_code(&strm, LZMA_FINISH);
        } else {
            ret = lzma_code(&strm, LZMA_RUN);
        }

        if (strm.avail_out == 0 || ret == LZMA_STREAM_END) {
            // *** Буфер вывода заполнен или поток закончился *** //
            size_t decompressed_size = CHUNK_SIZE - strm.avail_out;
            decompressed.append(reinterpret_cast<const char*>(outbuf),
                              decompressed_size);

            strm.next_out = outbuf;
            strm.avail_out = CHUNK_SIZE;
        }

        if (ret != LZMA_OK && ret != LZMA_STREAM_END) {
            qDebug() << "Ошибка декодирования:" << ret;
            lzma_end(&strm);
            return QByteArray();
        }

    } while (ret != LZMA_STREAM_END);

    lzma_end(&strm);
    return decompressed;
}

QString OsInfoLoader::getLatestReleaseUrl(){
    QString res;

    QString releaseUrlSring = "https://releases.pagure.org/libosinfo/";

    QUrl releaseUrl(releaseUrlSring);

    QNetworkAccessManager* netManager = new QNetworkAccessManager();
    QNetworkRequest       req(releaseUrl);
    // *** Попытка решить проблему ошибки подключения к серверу *** //
    //     qt.network.http2: stream 1 finished with error: "Server  //
    //     stopped accepting new streams before this stream         //
    //     was established"                                         //
    //     через отключение HTTP/2 с переходом на HTTP/1.1          //
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    QNetworkReply*        reply = netManager->get(req);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // *** Проверка на возврат ошибки *** //
    if (reply->error() != QNetworkReply::NoError){
        qDebug() << "[EE] [getLatestReleaseUrl] Network error:"
                                                        << reply->errorString();
        reply->deleteLater();
        return QString();
    }

    // *** Ошибок нет, получен корректный ответ, можно парсить *** //
    QString html = reply->readAll();
    // qDebug() << html;
    reply->deleteLater();
    netManager->deleteLater();

    // <a href=\"osinfo-db-20240523.tar.xz\">
    QRegularExpression re(R"(<a\s+href=\"(osinfo-db-(\d{8})\.tar\.xz)\")");

    QRegularExpressionMatchIterator it = re.globalMatch(html);

    int prevDateNumber = -1;
    while (it.hasNext()){
        QRegularExpressionMatch m = it.next();

        QString href = m.captured(1);
        int currentDateNumber = m.captured(2).toInt();

        if (currentDateNumber > prevDateNumber){
            res = href;
            prevDateNumber = currentDateNumber;
        }
    }

    return releaseUrlSring + res;
}

unsigned int OsInfoLoader::getLatestLibOsInfoVersion(){
    unsigned int res;

    QString latestLibOsInfoUrl = getLatestReleaseUrl();
    QRegularExpression re(R"(osinfo-db-(\d{8})\.tar\.xz)");
    QRegularExpressionMatch m = re.match(latestLibOsInfoUrl);

    if (m.hasMatch()){
        res = m.captured(1).toInt();
    }
    else {
        res = 0;
    }

    return res;
}

QByteArray OsInfoLoader::xzLibInfoDownload(const QString& urlString){
    QByteArray osInfoData;
    int osInfoDataSize;
    const int osInfoDataMaxSize = 5242880;
    QUrl url;

    // *** Установка полного url'а для скачивания архива с данными *** //
    if (urlString == "None"){
        url = QUrl(this->getLatestReleaseUrl());
        if (url.isEmpty()){
            // *** Ошибка получения ссылки на архив с данными *** //
            return QByteArray();
        }
    }
    else {
        url = QUrl(urlString);
    }

    // *** Определение размера скачиваемого файла и проверка его размера *** //
    osInfoDataSize = getRemoteFileSize(url);
    osInfoData.reserve(osInfoDataSize);

    if (osInfoDataSize > osInfoDataMaxSize){
        qDebug() << "[EE] [xzLibInfoDownload] [File more than 5MB]. Break!";
        return QByteArray();
    }
    else{
        if (osInfoDataSize < 0){
            qDebug() << "[EE] [xzLibInfoDownload] [Unknown file size]. Break!";
            return QByteArray();
        }
    }

    QNetworkAccessManager* netManager = new QNetworkAccessManager();
    QNetworkRequest        req(url);
    // *** Попытка решить проблему ошибки подключения к серверу *** //
    //     qt.network.http2: stream 1 finished with error: "Server  //
    //     stopped accepting new streams before this stream         //
    //     was established"                                         //
    //     через отключение HTTP/2 с переходом на HTTP/1.1          //
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    QNetworkReply* reply = netManager->get(req);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError){
        osInfoData = reply->readAll();
    }
    else {
        qDebug() << "[EE] Network error:" << reply->errorString();
        reply->deleteLater();
    }

    reply->deleteLater();
    netManager->deleteLater();
    return osInfoData;
}

long int OsInfoLoader::getRemoteFileSize(const QUrl& url){
    long int res = -1;

    QNetworkAccessManager* netManager = new QNetworkAccessManager();
    QNetworkRequest        req(url);
    // *** Попытка решить проблему ошибки подключения к серверу *** //
    //     qt.network.http2: stream 1 finished with error: "Server  //
    //     stopped accepting new streams before this stream         //
    //     was established"                                         //
    //     через отключение HTTP/2 с переходом на HTTP/1.1          //
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    QNetworkReply* reply = netManager->head(req);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError){
        QVariant length = reply->header(QNetworkRequest::ContentLengthHeader);
        if (length.isValid())
            res = length.toInt();
    }
    else {
        qDebug() << "[EE] Network error:" << reply->errorString();
        reply->deleteLater();
    }

    netManager->deleteLater();
    return res;
}

QVector<TarFile> OsInfoLoader::unTar(const QByteArray& tarData){
    QVector<TarFile> res;

    int pos = 0;
    const int blockSize = 512;

    while (pos + blockSize < tarData.size()){
        // *** 1. Получение заголовка длиной blockSize из позиции pos *** //
        QByteArray header = tarData.mid(pos, blockSize);

        // *** 2. После заголовка идут данные, переход к началу данных *** //
        pos += blockSize;

        // *** 3. Проверка заголовка на критерий окончания файла *** //
        if (header.trimmed().isEmpty()){
            break;
        }

        // *** 4. Получение имени файла из заголовка данных *** //
        QByteArray nameBytes = header.mid(0, 100);
        QString name = QString::fromUtf8(nameBytes).split('\0').first();

        // *** 5. Пропуск действий, если получено пустое имя *** //
        if (name.isEmpty()){
            continue;
        }

        // *** 6. Пропуск действий, если имеем дело со служебным файлом, *** //
        //        содержащим длинное имя файла. В теории, PAX header может   //
        //        занимать несколько блоков, что необходимо учитывать.       //
        if (name.contains("PaxHeader/")){
            QByteArray sizeBytes = header.mid(124, 12);
            long int paxSize = octToDec(sizeBytes);
            int padding = (blockSize - (paxSize % blockSize)) % blockSize;
            pos += paxSize + padding;
            continue;
        }

        // *** 7. Получение размера файла (12 байт со смещения 124 байта *** //
        QByteArray sizeBytes = header.mid(124, 12);
        long int fileSize = octToDec(sizeBytes);

        // *** 8. Чтение данных из tarData *** //
        QByteArray data;
        if (fileSize > 0){
            data = tarData.mid(pos, fileSize);

            // *** 9. Переход к следующему header'у  (с выравниванием *** //
            //     Двойное вычисление % blockSize необходимo для случая,  //
            //     когда размер файла кратен 512.                         //
            //       fileSize = 1024                                      //
            //       blockSize = 512                                      //
            //       fileSize % blockSize = 0                             //
            //       => padding = blockSize - 0 = 512, что не верно т.к., //
            //          выравнивание в данном случае не требуется.        //
            int padding = (blockSize - (fileSize % blockSize)) % blockSize;
            pos += fileSize + padding;

            // *** 10. На выход только файлы, каталоги пропускаются *** //
            res.push_back(TarFile{name, data});
        }
    }

    return res;
}

long int OsInfoLoader::octToDec(const QByteArray& octalStr){
    bool isOk = false;
    long int value = octalStr.trimmed().toLongLong(&isOk, 8);
    if (!isOk){
        return 0;
    }
    return value;
}

QVector<TarFile> OsInfoLoader::getShortOsInfo(const QVector<TarFile>& tarData){
    QVector<TarFile> tarShortOsInfo;

    // *** Регулярное выражения для выделения *.xml файлов с инофрмацией *** //
    //     об ОС. Файлы вида osinfo-db-20250606/os/suse.com/sles-10.1.xml.   //
    static const QRegularExpression re(
        R"(^osinfo-db-(\d{8})/os/[a-zA-Z0-9-]+\.[a-zA-Z]+/[a-zA-Z0-9.-]+\.xml$)"
    );

    // *** Выделение только информации об ОС *** //
    for (int i = 0; i < tarData.size(); ++i){
        QString path = tarData[i].fullName;
        QRegularExpressionMatch m = re.match(path);

        if (m.hasMatch()){
            tarShortOsInfo.push_back(tarData[i]);
        }
    }

    return tarShortOsInfo;
}

QJsonDocument OsInfoLoader::getLibOsInfoJson(const QString& url){
    // *** Скачивание архива osinfo-db-YYYYMMDD.tar.xz *** //
    QByteArray xzData = this->xzLibInfoDownload(url);
    if (xzData.isEmpty()){
        // *** В случае ошибок получения архива из сети, *** //
        //     возврат пустого результата.                   //
        return QJsonDocument();
    }

    // *** Разархивирование полученного ранее архива *** //
    QByteArray unPackData = this->decompressXZ(xzData);

    // *** Извлечение данных из *.tar архива *** //
    QVector<TarFile> fullOsInfoData = this->unTar(unPackData);

    // *** Фильтрация содержимого *.tar архива (только данне по OS) *** //
    QVector<TarFile> tarShortOsInfoData;
    tarShortOsInfoData = this->getShortOsInfo(fullOsInfoData);

    // *** Фомирование выходного JSON'а *** //
    QJsonObject   jsonRootObj;
    jsonRootObj["libOsInfoVersion"] = getLibOsInfoVersion(tarShortOsInfoData);
    QJsonArray    osInfoArray;

    for (int i = 0; i < tarShortOsInfoData.size(); ++i){
        OsInfo osInfo = getOsInfo(tarShortOsInfoData[i].data);

        QJsonObject osInfoJson;

        osInfoJson["id"]     = osInfo.id;
        osInfoJson["name"]   = osInfo.name;
        osInfoJson["ram"]    = osInfo.ram;
        osInfoJson["vendor"] = osInfo.vendor;

        osInfoArray.append(osInfoJson);
    }

    jsonRootObj["osInfo"] = osInfoArray;

    QJsonDocument jsonDoc(jsonRootObj);
    return jsonDoc;
}

QString OsInfoLoader::getLibOsInfoVersion(const QVector<TarFile>& tarData){
    QString res;

    static const QRegularExpression re(
        R"(^osinfo-db-(\d{8})/os/[a-zA-Z0-9-]+\.[a-zA-Z]+/[a-zA-Z0-9.-]+\.xml$)"
    );
    QRegularExpressionMatch m = re.match(tarData.first().fullName);

    if (m.hasMatch()){
        res = m.captured(1);
    }

    return res;
}

OsInfo OsInfoLoader::getOsInfo(const QByteArray& osXmlInfo){
    OsInfo res;
    QDomDocument doc;

    if (!doc.setContent(osXmlInfo)) {
        qDebug() << "[EE] XML parse error!";
        return OsInfo();
    }

    // *** Корневой элемент *** //
    QDomElement rootElement = doc.firstChildElement("libosinfo");

    // *** Элемент os *** //
    QDomElement osElement = rootElement.firstChildElement("os");

    // id
    res.id  = osElement.attribute("id");

    // name, первый в списке - name без атрибута xml:lang
    QDomElement nameElement = osElement.firstChildElement("name");
    res.name = nameElement.text();

    // vendor, первый в списке - vendor у которого нет атрибута xml:lang
    QDomElement vendorElement = osElement.firstChildElement("vendor");
    res.vendor = vendorElement.text();

    // ram
    QDomElement resourcesEl = osElement.firstChildElement("resources");
    if (!resourcesEl.isNull()){
        QDomElement minElem = resourcesEl.firstChildElement("minimum");
        if (!minElem.isNull()){
            res.ram = minElem.firstChildElement("ram").text();
        }
    }

    return res;
}

void OsInfoLoader::writeLibOsInfoJsonToFile(
                                        const QJsonDocument& libOsInfoJsonDoc,
                                                   const QString& fullFileName){
    // *** QFile не создаст файл, если катлоги не существуют *** //
    QFileInfo fileInfo(fullFileName);
    QDir      dir(fileInfo.absolutePath());

    // *** Если каталога нет, то создадим его со всей иерархией *** //
    if (!dir.exists()){
        if (!dir.mkpath(fileInfo.absolutePath())){
            qDebug() << "[II] Failed to create dir:" << fileInfo.absolutePath();
            return;
        }
    }

    QFile file(fullFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "[EE] Error write file " + fullFileName;
    } else {
        file.write(libOsInfoJsonDoc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

// End osinfoloader.cpp
