// Begin vmDataStructs.h

#ifndef VMDATASTRUCT_H
#define VMDATASTRUCT_H

#include <QString>
#include <QStringList>

// Узел цепочки состояний
// "атомарный" элемент QTreeView
struct ChainNode {
    int     id;
    int     parentId;
    QString name;
    QString imagesType;          // Тип текущего состояния [snap|work|root]
    QStringList imagesFullNames; // Полные имена образов жёстких дисков узла
    QStringList imagesFileNames; // Короткие имена образов жёстких дисков
    QStringList backFullNames;   // Полные имена backing (родительских) файлов
};

struct VMachine {
    QString     name;          // Название виртуальной машины
    QString     uuid;          // Уникальный идентификатор
    QString     osId;          // Идентификатор операционной системы
    QString     state;         // Состояние виртуальной машины (вкл./выкл.)
    QString     cpu;           // Информация о процессоре
    QString     ram;           // Оперативная память
    QStringList mountStorages; // Примонтированные хранилища данных
    QStringList rootFullName;  // Полные имена корневых файлов цепочек состояний
    QStringList snapshotsDirs; // Каталоги хранения цепочек сохранения состояний
    QVector<ChainNode> vmStateChain; // Узлы цепочки сохранения состояний ВМ
};

struct VmImageRawInfo {
    QString imageBasePath;     // Каталог хранения образа жёсткого диска
    QString imageFullName;     // Полное имя образа жёсткого диска
    QString backFullName;      // Полное имя backing (родительского) файла
       bool inChain;           // Флаг отнесения к цепочке сохранения состояний
};

const QStringList nodeNameList = {
    "Happy birthday", "Good morning", "Best friend", "High school", "Big city",
    "Small town", "Hot coffee", "Cold water", "Fast car", "Long time",
    "Young girl", "Yellow sun", "Classic music","Next week", "Old man",
    "Black cat", "White dog", "Red apple", "Green grass", "Blue sky",
    "Brown bear", "Happy family", "Strong man", "Beautiful woman", "Smart kid",
    "Hard work", "Easy job", "New book", "Old house", "Modern art",
    "Popular song", "Favorite movie", "Interesting story", "Funny joke",
    "Sad news", "Bad dream", "Good news","Found money", "Brave soldier",
    "Sweet tea", "Sour lemon", "Spicy food", "Fresh air", "Clean water",
    "Dirty hands", "Wet clothes", "Dry towel", "Soft pillow", "Hard rock",
    "Loud noise", "Quiet place", "Bright light", "Dark room", "Heavy rain",
    "Light snow", "Warm sun", "Cool breeze", "Open door", "Closed window",
    "Empty box", "Full cup", "Broken glass", "Fixed car", "Lost key",
    "Dead phone", "Alive plant", "Real story", "Fake news",
    "Happy smile", "Angry face", "Sad eyes", "Excited child", "Bored student",
    "Tired worker", "Hungry dog", "Thirsty cat", "Scared rabbit",
    "Rich man", "Poor woman", "Strong wind", "Weak signal", "High price",
    "Low cost", "Private garden", "Right turn", "Busy street", "Quiet library",
    "Big problem", "Small issue", "Long hair", "Short dress",
    "Fast train", "Slow bus", "Early bird", "Late night", "Young tree",
    "Old tradition", "Modern technology", "Ancient history", "Public park",
    "Main road", "Side street", "Front door", "Back yard", "Left hand",
    "North pole", "South coast", "East side", "West end",
    "Happy couple", "Loving parents", "Strict teacher", "Kind stranger",
    "Honest person", "Loyal friend", "Smart student", "Creative artist",
    "Healthy food", "Sick child", "Tasty meal", "Bitter medicine",
    "Sweet fruit", "Rough paper", "Hungry lion", "Poor village", "Big ocean",
    "Sour candy", "Salty chips", "Fresh bread", "Cold milk", "Hot soup",
    "Tall building", "Short man", "Wide river", "Narrow path", "Deep lake",
    "Shallow pool", "Heavy bag", "Light feather", "Strong coffee", "Weak tea",
    "Happy life", "Sad song", "Fun game", "Boring lecture", "Exciting trip",
    "Tiring journey", "Relaxing bath", "Stressful job", "Peaceful place",
    "Noisy crowd", "Sharp knife", "Dull pencil", "Smooth surface",
    "Hard stone","Soft fabric", "Wet paint", "Dry leaves", "Clean floor",
    "Dirty shoes", "Open book", "Closed shop", "Empty room", "Full bag",
    "Broken chair", "Fixed bike", "Lost ticket", "Found wallet", "Dead battery",
    "Alive fish", "Real diamond", "Fake smile", "Happy child", "Angry boss",
    "Sad movie", "Excited fan", "Bored audience", "Tired athlete",
    "Thirsty traveler", "Scared mouse", "Brave hero", "Rich country",
    "Strong current", "Weak password", "High mountain", "Low valley",
    "Small pond", "Ancient ruin", "Sour grapes", "Shallow water","Happy ending",
    "Long journey", "Short visit", "Fast runner", "Slow turtle", "Early start",
    "Late arrival", "Young plant", "Old castle", "Modern design",
    "Public transport", "Private jet", "Main course", "Side dish", "Front seat",
    "Back row", "Left lane", "Right answer", "North wind", "South beach",
    "East gate", "West wing", "Happy holiday", "Loving family", "Strict rule",
    "Kind heart", "Honest opinion", "Loyal customer", "Smart choice",
    "Creative solution", "Busy schedule", "Quiet moment", "Healthy lifestyle",
    "Sick patient", "Tasty dessert", "Bitter truth", "Sweet memory",
    "Salty popcorn", "Fresh juice", "Cold winter", "Hot summer", "Tall tree",
    "Short story", "Wide screen", "Narrow bridge", "Deep thought",
    "Heavy traffic", "Light rain", "Strong argument", "Weak excuse",
    "Sad story", "Fun party", "Boring meeting", "Exciting adventure",
    "Tiring workout", "Relaxing music", "Stressful exam", "Peaceful protest",
    "Noisy party", "Sharp mind", "Dull color", "Smooth skin", "Rough road",
    "Hard decision", "Soft voice", "Wet grass", "Dry desert", "Open mind",
    "Closed case", "Empty stomach", "Full moon", "Broken heart",
    "Fixed mistake", "Low quality","Slow dance","Modern world",
    "Lost opportunity", "Found treasure", "Dead end", "Alive culture",
    "Real love", "Fake ID", "Happy memory", "Angry mob", "Sad ending",
    "Excited crowd", "Bored expression", "Tired eyes", "Hungry baby",
    "Thirsty earth", "Scared look", "Brave decision", "Rich culture",
    "Poor condition", "Strong bond", "Weak spot", "High standard",
    "Big heart", "Small step", "Long list", "Short break", "Fast food",
    "Early morning", "Late afternoon", "Young adult", "Old friend",
    "Ancient artifact", "Public opinion", "Private matter", "Main attraction",
    "Side effect", "Front page", "Back door", "Left corner", "Right direction",
    "North star", "South wind", "East wind", "West coast", "Happy moment",
    "Loving gesture", "Strict diet", "Kind word", "Honest work",
    "Smart investment", "Creative mind", "Busy life", "Quiet night",
    "Healthy diet","Shallow mind","Noisy neighbor", "Loyal companion",
    "Sick leave", "Tasty snack", "Bitter cold", "Sweet voice", "Sour mood",
    "Salty sea", "Fresh start", "Cold shoulder", "Hot topic", "Tall order",
    "Short temper", "Wide range", "Narrow escape", "Deep breath",
    "Heavy load", "Light breeze", "Strong will", "Weak point", "Happy place",
    "Sad face", "Fun experience", "Boring day", "Exciting news", "Tiring task",
    "Relaxing holiday", "Stressful situation", "Peaceful sleep",
    "Sharp tongue", "Dull sound", "Smooth talker", "Rough sea", "Hard worker",
    "Soft touch", "Wet weather", "Dry humor", "Open heart", "Closed mind",
    "Empty promise", "Full house", "Broken trust", "Fixed price", "Lost time",
    "Found happiness", "Dead silence", "Alive and well", "Great idea",
    "First time", "Last night"
};

#endif
// End vmDataStructs.h
