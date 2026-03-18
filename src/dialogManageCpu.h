// Begin dialogManageCpu.h
#ifndef DIALOGMANAGECPU_H
#define DIALOGMANAGECPU_H

#include <QMap>
#include <QObject>
#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFormLayout>

struct CpuModelInfo {
    QString title;
    QString description;
};

class DialogManageCpu : public QDialog {

    Q_OBJECT

    public:
        DialogManageCpu(const uint& hostCpuCount,
                                        const QString& vmCpuModel,
                                            const QString& vmMaxCpuCount,
                                                const QString& vmCpuTopology,
                                                     QWidget* parent = nullptr);
        ~DialogManageCpu();

        void setVmCpuInfo();

    signals:
        void requestVmCpuInfoXmlUpdate(const uint& sockets,
                                       const uint& cores,
                                       const uint& threads,
                                       const QString& model);

    private:
        const int m_dlgTitleFrameHeight = 55;
        QDialogButtonBox* m_dlgBtnBox = nullptr;
        QFormLayout* m_infoFrameLayout;
        QLabel* m_vmAllocateCPUValue;
        QComboBox* m_vCpuModelValue;

        void parseVmCpuTopology(const QString& vmCpuTopology);

        void updVmCpuCount();
        uint getVmCpuCount();
        uint getVmSockets();
        uint getVmCores();
        uint getVmThreads();
        uint getVmMaxCpuCount();
        QString getVmCpuModel();

        uint m_vmCpuCount = 1;
        uint m_sockets = 1;
        uint m_cores = 1;
        uint m_threads = 1;
        uint m_maxVmCpuCount = 4;
        uint m_hostCpuCount = 1;

        QMap<QString, CpuModelInfo> m_vmCpuModel;
        void cpuModelDescriptionInit();
        void cpuModelValuesFill(QComboBox* vCpuModelValue, QString vmCpuModel);
        QString getVmCpuValueSpacer(const int& vmCpuCount);
};


#endif
// End dialogManageCpu.h
