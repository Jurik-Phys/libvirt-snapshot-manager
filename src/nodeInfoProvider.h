// Begin nodeInfoProvider.h

#ifndef NODEINFOPROVIDER_H
#define NODEINFOPROVIDER_H

#include <QObject>
#include <QDebug>

class NodeInfoProvider : public QObject {

    Q_OBJECT

    public:
        NodeInfoProvider(QObject* parent = nullptr);
        ~NodeInfoProvider();

        long int logicalCpuCount();
        long int memorySizeBytes();

    private:
        const QString m_libVirtConnectURI = "qemu:///system";
        void getNodeInfo();
        long int getRamBytes(const long int& value, const QString& units);
        long int m_memSizeBytes;
        int m_cpuCount;
};

#endif
// End nodeInfoProvider.h
