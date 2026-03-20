// Begin dialogAddNewImage.h

#ifndef DIALOGADDNEWIMAGE_H
#define DIALOGADDNEWIMAGE_H

#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>

class DialogAddNewImage : public QDialog {

    Q_OBJECT

    public:
        DialogAddNewImage(QWidget* parent = nullptr);
        ~DialogAddNewImage();

        QString getImageFullName();
        QString getImageSize();
        QString getImageBusType();
        long int capacity;
        void setVmName(const QString& vmName);

    private:
        void checkSelectedPath();
        void selectDir();
        bool isValidFileName(const QString& fName);
        bool isValidDirPath(const QString& dir);
        QLineEdit*      m_fileNameEdit;
        QLineEdit*      m_dirPathEdit;
        QDoubleSpinBox* m_capacitySpinBox;
        QLabel*         m_errorOut;
        QComboBox*      m_controllerBox;
        int m_leftWidth = 100;
};


#endif
// End dialogAddNewImage.h
