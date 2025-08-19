// Begin infoWidget.cpp

#include "infoWidget.h"

InfoWidget::InfoWidget(QWidget* parent) : QFrame(parent){

    QVBoxLayout* vFrameLayout = new QVBoxLayout(this);
    vFrameLayout->setAlignment(Qt::AlignTop);
    vFrameLayout->setContentsMargins(7, 5, 0, 0);

    QLabel* headline = new QLabel(this);
    headline->setText("<b>Virtual machine overview</b>");
    vFrameLayout->addWidget(headline);

    QScrollArea* scrollForm = new QScrollArea(parent);
    scrollForm->setFrameShape(QFrame::NoFrame);
    scrollForm->setFrameShadow(QFrame::Plain);
    this->fixScrollBar(scrollForm);

    // *** Обязательно, чтобы контейнер не "схлопнулся" *** //
    scrollForm->setWidgetResizable(true);
    vFrameLayout->addWidget(scrollForm);

    // *** Виджет контейнер для правильного скролла *** //
    //       (контейнер должен быть без родителя)
    QWidget* container = new QWidget();

    m_vScrollLayout = new QVBoxLayout();
    m_vScrollLayout->setAlignment(Qt::AlignTop);
    container->setLayout(m_vScrollLayout);
    scrollForm->setWidget(container);

    QStringList colAList = {"Title:",
                            "Description:",
                            "UUID:",
                            "Canonical name:",
                            "Virtual CPUs:",
                            "RAM:",
                            "Libosinfo ID:",
                            "Mounted drives:"};

    int colAWidth = 90;
    QVector<QHBoxLayout*> hLayoutArray;

    // *** Создание горизонтальных layout'ов и установка отсупов *** //
    for (int i = 0; i < colAList.size(); ++i){
        hLayoutArray.push_back(new QHBoxLayout);
        hLayoutArray[i]->setContentsMargins(5, 0, 0, 0);
    }

    // *** Генерация виджетов первого столбца *** //
    for (int i = 0; i < colAList.size(); ++i){
        m_colAWidgets.push_back(new QLabel(colAList[i]));
        m_colAWidgets[i]->setFixedWidth(colAWidth);
    }

    // *** Генерация виджетов второго столбца *** //
    for (int i = 0; i < colAList.size(); ++i){
        if (i == 0){ // Title
            m_colBWidgets.push_back(new QTextEdit(this));
            QWidget* widget = m_colBWidgets.last();
            QTextEdit* line = qobject_cast<QTextEdit*>(widget);
            QFontMetrics fm(line->font());
            int rowHeight = fm.lineSpacing();
            line->setFixedHeight(rowHeight + 2 * line->frameWidth() + 10);
            line->viewport()->setStyleSheet("background-color: white;");
            line->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            line->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            line->setLineWrapMode(QTextEdit::NoWrap);
            line->setStyleSheet("background-color: white;");
            line->setPlaceholderText("Virtual machine title…");
            continue;
        }

        if (i == 1){ // Description
            // *** Виджет для Description должен быть многострочный *** //
            m_colBWidgets.push_back(new QTextEdit(this));
            QWidget* widget = m_colBWidgets.last();
            QTextEdit* edit = qobject_cast<QTextEdit*>(widget);
            // *** Увеличение числа отображемых строк в поле ввода до 4 *** //
            QFontMetrics fm(edit->font());
            int rowHeight = fm.lineSpacing();
            edit->setFixedHeight(rowHeight * 4 + 2 * edit->frameWidth() + 10);
            edit->viewport()->setStyleSheet("background-color: white;");
            fixScrollBar(edit);
            edit->setPlaceholderText("Virtual machine description…");
            continue;
        }

        m_colBWidgets.push_back(new QLabel(this));
        QWidget* widget = m_colBWidgets.last();
        QLabel* lbl = qobject_cast<QLabel*>(widget);
        lbl->setTextInteractionFlags(
                      Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

        // *** Присвоение значений виджетам второго столбца *** //
        switch (i){
            case 0: // Title
                lbl->setText("—");
                break;
            case 1: // Description
                lbl->setText("—");
                break;
            case 2: // UUID
                lbl->setText("—");
                break;
            case 3: // Canonical name
                lbl->setText("—");
                break;
            case 4: // CPU
                lbl->setText("—");
                break;
            case 5: // RAM
                lbl->setText("—");
                break;
            case 6: // OsID
                lbl->setText("—");
                break;
            case 7: // Mounted drives
                lbl->setText("—");
                break;
        }
    }

    // *** Добавление виджетов первого и второго столбцов в hLayout *** //
    for (int i = 0; i < m_colAWidgets.size(); ++i){
        hLayoutArray[i]->addWidget(m_colAWidgets[i]);
        hLayoutArray[i]->addWidget(m_colBWidgets[i]);
    }

    // *** Добавление hLayout'ов в основной вертикальный layout *** //
    for (int i = 0; i < hLayoutArray.size(); ++i){
        m_vScrollLayout->addLayout(hLayoutArray[i]);
    }

    // *** Добавление и настройка виджета для отображения списка дисков *** //
    QTextEdit* storageList = new QTextEdit();
    storageList->setLineWrapMode(QTextEdit::NoWrap);
    storageList->setReadOnly(true);
    storageList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    storageList->setLineWrapMode(QTextEdit::NoWrap);
    storageList->viewport()->setStyleSheet("background-color: white;");
    storageList->setStyleSheet("QTextEdit {" "border: none;" "}");
    fixScrollBar(storageList);
    m_vScrollLayout->addWidget(storageList);

    m_vScrollLayout->addStretch();
}

InfoWidget::~InfoWidget(){
}

void InfoWidget::fixScrollBar(QScrollArea* scrollArea){
    // *** Костыль для скрытия квадратика при обоих скроллах *** //
    QWidget* corner = new QWidget();
    corner->setFixedSize(0, 0);
    scrollArea->setCornerWidget(corner);

    // *** Костыль для вертикального скролла *** //
    scrollArea->verticalScrollBar()->setStyleSheet(R"(
                                    QScrollBar:vertical {
                                        background: transparent;
                                        width: 6px;
                                        margin: 0px 0px 0px 0px;
                                    }
                                    QScrollBar::handle:vertical {
                                        background: #badcef;
                                        min-height: 15px;
                                        border-radius: 1px;
                                    }
                                    QScrollBar::add-line, QScrollBar::sub-line {
                                        height: 0px;
                                    }
                                    )");

    // *** Костыль для горизонтального скролла *** //
    scrollArea->horizontalScrollBar()->setStyleSheet(R"(
                                    /* Горизонтальный скроллбар */
                                     QScrollBar:horizontal {
                                        background: transparent;
                                        height: 6px;
                                        margin: 0px 0px 0px 0px;
                                     }
                                     QScrollBar::handle:horizontal {
                                        background: #badcef;
                                        min-width: 15px;
                                        border-radius: 1px;
                                     }
                                    QScrollBar::add-line, QScrollBar::sub-line {
                                        height: 0px;
                                    }
                                    )");
}

void InfoWidget::fixScrollBar(QTextEdit* edit){

    // *** Костыль для вертикального скролла *** //
    edit->verticalScrollBar()->setStyleSheet(R"(
                                    QScrollBar:vertical {
                                        background: transparent;
                                        width: 5px;
                                        margin: 0px 0px 0px 0px;
                                    }
                                    QScrollBar::handle:vertical {
                                        background: #badcef;
                                        min-height: 15px;
                                        border-radius: 1px;
                                    }
                                    QScrollBar::add-line, QScrollBar::sub-line {
                                        height: 0px;
                                    }
                                    )");

    // *** Костыль для горизонтального скролла *** //
    edit->horizontalScrollBar()->setStyleSheet(R"(
                                    /* Горизонтальный скроллбар */
                                     QScrollBar:horizontal {
                                        background: transparent;
                                        height: 5px;
                                        margin: 0px 0px 0px 0px;
                                     }
                                     QScrollBar::handle:horizontal {
                                        background: #badcef;
                                        min-width: 15px;
                                        border-radius: 1x;
                                     }
                                    QScrollBar::add-line, QScrollBar::sub-line {
                                        height: 0px;
                                    }
                                    )");
}

void InfoWidget::setData(const VMachine& vm){
    qobject_cast<QTextEdit*>(m_colBWidgets[0])->setText(vm.title);
    qobject_cast<QTextEdit*>(m_colBWidgets[1])->setText(vm.description);
    qobject_cast<QLabel*>(m_colBWidgets[2])->setText(vm.uuid);
    qobject_cast<QLabel*>(m_colBWidgets[3])->setText(vm.name);
    qobject_cast<QLabel*>(m_colBWidgets[4])->setText(vm.cpu);
    qobject_cast<QLabel*>(m_colBWidgets[5])->setText(humanMemory(vm.ram));
    qobject_cast<QLabel*>(m_colBWidgets[6])->setText(vm.osId);
    QString mountStoragesCount = QString::number(vm.mountStorages.size());
    qobject_cast<QLabel*>(m_colBWidgets[7])->setText(mountStoragesCount);
    setStorageList(vm.mountStorages);
}

QString InfoWidget::humanMemory(const QString& rawRam){
    QString res = rawRam;

    float humanMemValue;
    QString humanMemUnits;

    QStringList parts = rawRam.split(" ", Qt::SkipEmptyParts);

    long int     memValue = parts[0].toLong();
    QString memUnits = parts[1];

    QStringList s1024BaseUnits = {   "k",   "M",   "G",   "T" };
    QStringList l1024BaseUnits = { "KiB", "MiB", "GiB", "TiB" };
    QStringList v1000BaseUnits = {  "KB",  "MB",  "GB",  "TB" };

    long int bytesRam;
    if (memUnits == "b" || memUnits == "bytes"){
        bytesRam = memValue;
    }

    if (memUnits == "KB" ){
        bytesRam = memValue * 1000;
    }

    if (memUnits == "MB"){
        bytesRam = memValue * 1000 * 1000;
    }

    if (memUnits == "GB"){
        bytesRam = memValue * 1000 * 1000 * 1000;
    }

    if (memUnits == "TB"){
        bytesRam = memValue * 1000 * 1000 * 1000 * 1000;
    }

    if (memUnits == "k" || memUnits == "KiB"){
        bytesRam = memValue * 1024;
    }

    if (memUnits == "M" || memUnits == "MiB"){
        bytesRam == memValue * 1024 * 1024;
    }

    if (memUnits == "G" || memUnits == "GiB"){
        bytesRam == memValue * 1024 * 1024 * 1024;
    }

    if (memUnits == "T" || memUnits == "TiB"){
        bytesRam == memValue * 1024 * 1024 * 1024 * 1024;
    }
    qDebug() << "bytesRam" << bytesRam;

    humanMemValue = bytesRam;
    for (int i = 0; i < l1024BaseUnits.size(); ++i){
        humanMemValue = humanMemValue / 1024.0;
        if (humanMemValue < 1024){
            humanMemUnits = l1024BaseUnits[i];
            break;
        }
    }

    if (abs(humanMemValue - (int)humanMemValue) < 0.01){
        res = QString::number(humanMemValue, 'f', 0) + " " + humanMemUnits;
    } else {
        res = QString::number(humanMemValue, 'f', 2) + " " + humanMemUnits;
    }

    return res;
}

void InfoWidget::setStorageList(const QStringList& mountStorages){
    QStringList listData;

    for (int i = 0; i < mountStorages.size(); ++i){
        listData.push_back(QString::number(i+1) + ". " + mountStorages[i]);
    }

    // *** Доcтуп к виджету списка примонтированных дисков *** //
    QWidget* editWidget =  m_vScrollLayout
                               ->itemAt(m_vScrollLayout->count() - 2)->widget();
    QTextEdit* storageList = qobject_cast<QTextEdit*>(editWidget);

    // *** Изменение его вертикального размера для вмещения всех дисков *** //
    QFontMetrics fm(storageList->font());
    int rowHeight = fm.lineSpacing();

    storageList->setFixedHeight(rowHeight * listData.size()
                                          + 2 * storageList->frameWidth() + 14);
    storageList->setText(listData.join("\n"));
}

// End infoWidget.cpp
