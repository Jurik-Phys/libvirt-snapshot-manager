// Begin snapInfoWidget.cpp

#include "snapInfoWidget.h"

SnapInfoWidget::SnapInfoWidget(QWidget* parent) : QFrame (parent){

    m_saveTitleTimer = new QTimer();
    m_saveTitleTimer->setInterval(1000);
    m_saveTitleTimer->setSingleShot(true);

    m_saveDescriptionTimer = new QTimer();
    m_saveDescriptionTimer->setInterval(1000);
    m_saveDescriptionTimer->setSingleShot(true);

    m_updateImageFilesTimer = new QTimer();
    m_updateImageFilesTimer->setInterval(10);
    m_updateImageFilesTimer->setSingleShot(true);
    QObject::connect(m_updateImageFilesTimer, &QTimer::timeout,
                                    this, &SnapInfoWidget::doResizeImageFiles);

    QVBoxLayout* vFrameLayout = new QVBoxLayout(this);
    vFrameLayout->setAlignment(Qt::AlignTop);
    vFrameLayout->setContentsMargins(7, 5, 0, 0);

    QLabel* headline = new QLabel(this);
    QFont headlineFont = headline->font();
    int headlineOriginalFontSize = headlineFont.pointSize();
    headlineFont.setPointSize(headlineOriginalFontSize);
    headline->setFont(headlineFont);
    headline->setText("<b>Snapshot overview</b>");
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
                            "Type & children:",
                            "Image files:"};

    int colAWidth = 107;
    QVector<QHBoxLayout*> hLayoutArray;
    // *** Создание горизонтальных layout'ов и установка отсупов *** //
    for (int i = 0; i < colAList.size(); ++i){
        hLayoutArray.push_back(new QHBoxLayout);
        hLayoutArray[i]->setContentsMargins(0, 0, 0, 0);
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
            // *** a) корректировка шрифта *** //
            QFont font = this->font();
            int newFontSize = calcOptimalFontSize(colAList);
            font.setPointSize(newFontSize);
            this->setFont(font);
            line->setFont(font);
            // *** б) установка высоты виджета *** //
            QFontMetrics fm(line->font());
            int rowHeight = fm.lineSpacing();
            line->setFixedHeight(rowHeight * 1.8);
            line->viewport()->setStyleSheet("background-color: white;");
            line->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            line->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            line->setLineWrapMode(QTextEdit::NoWrap);
            line->setStyleSheet("background-color: white;");
            line->setPlaceholderText("Snapshot title…");
            line->setReadOnly(true);
            line->installEventFilter(this);
            QObject::connect(line, &QTextEdit::textChanged,
                                  this, &SnapInfoWidget::restartSaveTitleTimer);
            QObject::connect(m_saveTitleTimer, &QTimer::timeout,
                                         this, &SnapInfoWidget::writeSnapTitle);
            continue;
        }

        if (i == 1){ // Description
            // *** Виджет для Description должен быть многострочный *** //
            m_colBWidgets.push_back(new QTextEdit(this));
            QWidget* widget = m_colBWidgets.last();
            QTextEdit* edit = qobject_cast<QTextEdit*>(widget);
            // *** Увеличение числа отображемых строк в поле ввода до 7+ *** //
            // *** a) корректировка шрифта *** //
            QFont font = this->font();
            int newFontSize = calcOptimalFontSize(colAList);
            font.setPointSize(newFontSize);
            this->setFont(font);
            edit->setFont(font);
            // *** б) Изменение числа строк *** //
            QFontMetrics fm(edit->font());
            int rowHeight = fm.lineSpacing();
            edit->setFixedHeight(rowHeight * 8.2);
            edit->viewport()->setStyleSheet("background-color: white;");
            fixScrollBar(edit);
            edit->setReadOnly(true);
            edit->setPlaceholderText("Snapshot description…");
            QObject::connect(edit, &QTextEdit::textChanged,
                            this, &SnapInfoWidget::restartSaveDescriptionTimer);
            QObject::connect(m_saveDescriptionTimer, &QTimer::timeout,
                                   this, &SnapInfoWidget::writeSnapDescription);
            continue;
        }

        m_colBWidgets.push_back(new QLabel(this));
        QWidget* widget = m_colBWidgets.last();
        QLabel* lbl = qobject_cast<QLabel*>(widget);
        lbl->setTextInteractionFlags(
                Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        // *** Присвоение значений виджетам второго столбца *** //
        switch (i){
            case 2: // UUID
                lbl->setText("—");
                break;
            case 3: // Canonical name
                lbl->setText("—");
                break;
            case 4: // Type
                lbl->setText("—");
                break;
            case 5: // Children
                lbl->setText("—");
                break;
            case 6: // Image files
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
    //     Также дополнительный статичный контейнер                 //
    //     для верхних виджетов, необходимый для предотвращения     //
    //     "прыжков" виджетов при добавлении нового диска           //
    //     П.С. впрочем, проблема прыжков может быть решена через   //
    //     изменение размера виджета по таймеру, но менее надёжно.  //
    // ************************************************************ //
    QWidget*     staticContainerWidget = new QWidget();
    QVBoxLayout* staticContainerLayout = new QVBoxLayout();
    staticContainerWidget->setContentsMargins(0, 0, 0, 0);
    staticContainerLayout->setContentsMargins(0, 0, 0, 0);
    staticContainerWidget->setLayout(staticContainerLayout);
    for (int i = 0; i < hLayoutArray.size(); ++i){
        staticContainerLayout->addLayout(hLayoutArray[i]);
    }
    staticContainerWidget->setSizePolicy(QSizePolicy::Expanding,
                                                            QSizePolicy::Fixed);
    QFontMetrics fm(this->font());
    int rowHeight = fm.lineSpacing();
    staticContainerWidget->setFixedHeight(17*rowHeight);

    m_vScrollLayout->setSpacing(0.4*rowHeight);
    m_vScrollLayout->addWidget(staticContainerWidget);

    // *** Добавление и настройка виджета списка файлов снапшота *** //
    m_imageFiles = new QTextEdit();

    m_imageFiles->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_imageFiles->setMaximumHeight(32);
    m_imageFiles->setLineWrapMode(QTextEdit::NoWrap);
    m_imageFiles->setReadOnly(true);
    m_imageFiles->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_imageFiles->setLineWrapMode(QTextEdit::NoWrap);
    m_imageFiles->viewport()->setStyleSheet("background-color: white;");
    m_imageFiles->setStyleSheet("QTextEdit {" "border: none;" "}");
    fixScrollBar(m_imageFiles);

    // *** Dynamic font size *** //
    QFont font = this->font();
    int newFontSize = calcOptimalFontSize(colAList);
    font.setPointSize(newFontSize);
    this->setFont(font);
    qobject_cast<QTextEdit*>(m_colBWidgets[0])->setFont(font);
    m_imageFiles->setFont(font);

    m_vScrollLayout->addWidget(m_imageFiles);
    m_vScrollLayout->addStretch();
}

SnapInfoWidget::~SnapInfoWidget(){
}

void SnapInfoWidget::fixScrollBar(QScrollArea* scrollArea){
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

void SnapInfoWidget::fixScrollBar(QTextEdit* edit){

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

bool SnapInfoWidget::eventFilter(QObject *obj, QEvent* event){
    QWidget*   widget = m_colBWidgets[0];
    QTextEdit* line = qobject_cast<QTextEdit*>(widget);
    if (obj == line && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return
                                          || keyEvent->key() == Qt::Key_Enter) {
                return true;
            }
            else {
                if (keyEvent->key() == Qt::Key_Tab){
                    QTextEdit* edt = qobject_cast<QTextEdit*>(m_colBWidgets[1]);
                    edt->setFocus();
                    edt->moveCursor(QTextCursor::End);
                    return true;
                }
            }
        }
        return QWidget::eventFilter(obj, event);
}

void SnapInfoWidget::setData(const VMachine& vm){
    m_vm = vm;
}

void SnapInfoWidget::setData(const ChainNode& node){
    m_node = node;

    // *** Setup counter to prevent raise write event in this step *** //
    m_titleChangedCounter = 1;
    qobject_cast<QTextEdit*>(m_colBWidgets[0])->setText(node.title);
    if (m_vm.state == "shut off"){
        qobject_cast<QTextEdit*>(m_colBWidgets[0])->setReadOnly(false);
    }
    m_descriptionChangedCounter = 1;
    QString desc = node.description;
    qobject_cast<QTextEdit*>(m_colBWidgets[1])
                                      ->setPlainText(desc.replace("\\n", "\n"));
    if (m_vm.state == "shut off"){
        qobject_cast<QTextEdit*>(m_colBWidgets[1])->setReadOnly(false);
    }
    qobject_cast<QLabel*>(m_colBWidgets[2])->setText(node.uuid);
    qobject_cast<QLabel*>(m_colBWidgets[3])->setText(node.name);
    // *** Type & children *** //
    QString typeOut;
    if (node.imagesType == "work"){
        typeOut = "Inactive workpoint. It always has no children";
    }
    else {
        if (node.imagesType == "snap"){
            switch (node.childrenImagesFullNames.size()){
                case 1:
                    typeOut = "Fixed state with 1 child";
                    break;
                default:
                    typeOut = "Fixed state with %1 children";
                    typeOut = typeOut.arg(node.childrenImagesFullNames.size());
            }
        }
        else {
            typeOut = "Active workpoint. It always has no children";
        }
    }
    qobject_cast<QLabel*>(m_colBWidgets[4])->setText(typeOut);
    setStorageList(node.imagesFullNames);
}

void SnapInfoWidget::setStorageList(const QStringList& imagesFullNames){
    // *** Сначала установка текста (c добавлением номеров в списке) *** //
    QStringList listData;
    for (int i = 0; i < imagesFullNames.size(); ++i){
        listData.push_back(QString::number(i+1) + ". " + imagesFullNames[i]);
    }

    m_imageFiles->setText(listData.join("\n"));
    // *** Изменение размера виджета через задержку таймера т.к. *** //
    //      в противном случае текст мигает/скачет над виджетом      //
    m_updateImageFilesTimer->start();

    // *** Обновление числа дисков снапшота (Image files) *** //
    qobject_cast<QLabel*>(m_colBWidgets[5])
                             ->setText(QString::number(imagesFullNames.size()));
}

void SnapInfoWidget::doResizeImageFiles(){
    m_imageFiles->setUpdatesEnabled(false);

    // *** Изменение вертикального размера, исключение прокрутки *** //
    QFontMetrics fm(m_imageFiles->font());
    int rowHeight = fm.lineSpacing();
    int imagesCount = m_imageFiles->document()->blockCount();
    int docMargin = m_imageFiles->document()->documentMargin() + 4;
    int newHeight = rowHeight * imagesCount + docMargin;

    m_imageFiles->setFixedHeight(newHeight);

    m_imageFiles->setUpdatesEnabled(true);
}

void SnapInfoWidget::restartSaveTitleTimer(){
    // *** Первые события - создание QTextEdit и присвоение через setData *** //
    if (m_titleChangedCounter < 10){
        m_titleChangedCounter++;
        if (m_titleChangedCounter <= 2){
            return;
        }
    }
    m_saveTitleTimer->start();
}

void SnapInfoWidget::restartSaveDescriptionTimer(){
    // *** Первые события - создание QTextEdit и присвоение через setData *** //
    if (m_descriptionChangedCounter < 10){
        m_descriptionChangedCounter++;
        if (m_descriptionChangedCounter <= 2){
            return;
        }
    }
    m_saveDescriptionTimer->start();
}

void SnapInfoWidget::writeSnapTitle(){

    QString vm_uuid    = m_vm.uuid;
    QString snap_uuid  = m_node.uuid;
    QString snap_name  = m_node.name;
    QString snap_title;
    snap_title = qobject_cast<QTextEdit*>(m_colBWidgets[0])->toPlainText();
    QString snap_desc;
    snap_desc = qobject_cast<QTextEdit*>(m_colBWidgets[1])->toPlainText();

    QStringList uuid = {vm_uuid, snap_uuid};
    QStringList snapInfo = {snap_name, snap_title};

    if (snap_title.size() == 0 && snap_desc.size() == 0){
        emit removeEmptySnapshotInfoRequested(uuid);
    }
    else {
        emit writeSnapTitleRequested(uuid, snapInfo);
    }
}

void SnapInfoWidget::writeSnapDescription(){
    QString vm_uuid          = m_vm.uuid;
    QString snap_uuid        = m_node.uuid;
    QString snap_name        = m_node.name;
    QString snap_title;
    snap_title = qobject_cast<QTextEdit*>(m_colBWidgets[0])->toPlainText();
    QString snap_desc;
    snap_desc = qobject_cast<QTextEdit*>(m_colBWidgets[1])->toPlainText();

    QStringList uuid = {vm_uuid, snap_uuid};
    QStringList snapInfo = {snap_name, snap_desc};

    if (snap_title.size() == 0 && snap_desc.size() == 0){
        emit removeEmptySnapshotInfoRequested(uuid);
    }
    else {
        emit writeSnapDescriptionRequested(uuid, snapInfo);
    }
}

void SnapInfoWidget::setReadOnly(bool ro){
    QTextEdit* title = qobject_cast<QTextEdit*>(m_colBWidgets[0]);
    QTextEdit* description = qobject_cast<QTextEdit*>(m_colBWidgets[1]);

    title->setReadOnly(ro);
    description->setReadOnly(ro);

    QString roToolTip = "Can edit only when VM is shut off "
                                                  "and a snapshot is selected.";
    QString blankToolTip = "";

    if (ro){
        title->setToolTip(roToolTip);
        description->setToolTip(roToolTip);
    }
    else {
        title->setToolTip(blankToolTip);
        description->setToolTip(blankToolTip);
    }
}

void SnapInfoWidget::clearData(){
    // *** Clear uuid's *** //
    ChainNode zeroNode;
    VMachine  zeroVm;
    m_vm = zeroVm;
    m_node = zeroNode;

    // *** Setup counter to prevent raise write event in this step *** //
    m_titleChangedCounter = 1;
    m_descriptionChangedCounter = 1;

    qobject_cast<QTextEdit*>(m_colBWidgets[0])->setText("");
    qobject_cast<QTextEdit*>(m_colBWidgets[0])->setReadOnly(true);
    qobject_cast<QTextEdit*>(m_colBWidgets[1])->setText("");
    qobject_cast<QTextEdit*>(m_colBWidgets[1])->setReadOnly(true);
    qobject_cast<QLabel*>(m_colBWidgets[2])->setText("—");
    qobject_cast<QLabel*>(m_colBWidgets[3])->setText("—");
    qobject_cast<QLabel*>(m_colBWidgets[4])->setText("—");
    qobject_cast<QLabel*>(m_colBWidgets[5])->setText("—");
    m_imageFiles->clear();
    m_imageFiles->setFixedHeight(32);
}

int SnapInfoWidget::calcOptimalFontSize(const QStringList& text){
    int res;
    int maxColAWidgetWidth = 0;
    int maxTextSize = 0;
    QString maxLengthText;

    QFont f = this->font();
    res = f.pointSize();
    QFontMetrics fm(f);

    for (int i = 2; i < m_colAWidgets.size(); ++i){
        int widgetWidth = qobject_cast<QLabel*>(m_colAWidgets[i])->width();
        int textSize = qobject_cast<QLabel*>(m_colAWidgets[i])->text().size();

        if (maxColAWidgetWidth < widgetWidth){
            maxColAWidgetWidth = widgetWidth;
        }

        if (maxTextSize < textSize){
           maxTextSize = textSize;
           maxLengthText = qobject_cast<QLabel*>(m_colAWidgets[i])->text();
        }
    }

    while (fm.horizontalAdvance(maxLengthText) > maxColAWidgetWidth
                                                          && f.pointSize() > 6){
        --res;
        f.setPointSize(res);
        fm = QFontMetrics(f);
    }

    return res;
}

// End snapInfoWidget.cpp
