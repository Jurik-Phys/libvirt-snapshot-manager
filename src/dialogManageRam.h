// Begin dialogManageRam.h

#ifndef DIALOGMANAGERAM_H
#define DIALOGMANAGERAM_H

#include <QDialog>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>

class DialogManageRam : public QDialog {

    Q_OBJECT

    public:
        DialogManageRam(const QString& osName, const QString& vmRam,
                        const QString& vmMinRam, const QString& hostRam,
                                                     QWidget* parent = nullptr);
        ~DialogManageRam();

        void setHostRamInBytes(const long int& hostRamInBytes);
        void setRam();

    signals:
        void requestVmRamXmlUpdate(const long int& memoryInKiB);

    private:
        const int m_dlgTitleFrameHeight = 55;
        int m_leftWidth = 255;
        QDoubleSpinBox* m_vmRamValue;
        QSlider*        m_vmRamSlider;
        QDialogButtonBox* m_dlgBtnBox;
        int m_sliderScale;

        long int strRamToBytes(const QString&);
        long int vmRamToBytes(const QString&);
        long int hostRamToBytes(const QString&);
        long int getManualMemoryInKiB();
        float getVmRamUpLim(const QString& vmMinRam, const QString& hostRam);
        float getVmAvgRam(const QString& vmMinRam, const float& avgRam);
        float getVmRamDownLim(const QString& vmMinRam);
        float getVmRamCurrent(const QString& vmMinRam, const float& vmRamBytes);
        QString checkVmMinRam(const QString& osName, const QString& vmMinRam);
        QString checkOsName(const QString& osName);
        long int m_hostRamInBytes;
};

#endif
// End dialogManageRam.h
