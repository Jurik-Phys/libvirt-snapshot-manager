// Begin dialogs.h

#ifndef DIALOGS_H
#define DIALOGS_H

#include <QBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include <QFileDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QTemporaryFile>
#include <QToolButton>

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
// End dialogs.h
