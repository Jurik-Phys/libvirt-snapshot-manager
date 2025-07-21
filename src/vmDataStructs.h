// Begin vmDataStructs.h

#ifndef VMDATASTRUCT_H
#define VMDATASTRUCT_H


#include <QString>
#include <QStringList>

// Узел цепочки состояний
// атомарный элемент QTreeView
struct ChainNode {
    int     id;
    int     parentId;
    QString name;
    // QString imageLeftId;    // Временной идентификатор состояния
    QString imagesType;    // Тип текущего состояния [snap|work|root]
    //   bool isFree;         // Флаг "висящего" (не имеющего потомков состояния)
    // В одном состоянии (снимке) может быть несолько жёстких дисков,
    // информацией по всем дискам, соответственно, необходимо управлять
    QStringList imagesFullNames; // Полное имя образа жёсткого диска
    QStringList imagesFileNames; // Только имя образа жёсткого диска
    QStringList backFullNames;   // Полное имя backing (родительского) файла
};

struct VMachine {
    QString     name;          // Название виртуальной машины
    QString     uuid;          // Уникальный идентификатор
    QString     osId;          // Идентификатор операционной системы
    QString     state;         // Состояние виртуальной машины (вкл./выкл.)
    QString     cpu;           // Информация о процессоре
    QString     ram;           // Оперативная память
    QStringList mountStorages; // Примонтированные хранилища данных
    QStringList rootFullName;  // Полные именa корневый файлов цепочек состояний
    QStringList snapshotsDirs; // Каталоги хранения цепочек сохранения состояний
    QVector<ChainNode> vmStateChain; // Узлоы цепочки сохранения состояний ВМ
};

struct VmImageRawInfo {
    QString imageBasePath;     // Каталог хранения образа жёсткого диска
    QString imageFullName;     // Полное имя образа жёсткого диска
    QString backFullName;      // Полное имя backing (родительского) файла
       bool inChain;           // Флаг отнесения к цепочке сохранения состояний
};

const QStringList nodeNameList = {
    "White", "Black", "Red", "Green", "Blue", "Yellow", "Cyan", "Magenta",
    "Orange", "Purple", "Pink", "Brown", "Gray", "Light Gray", "Dark Gray",
    "Sky Blue", "Light Blue", "Dark Blue", "Lime Green", "Forest Green",
    "Olive Green", "Sea Green", "Spring Green", "Mint Green", "Pale Green",
    "Chartreuse", "Lawn Green", "Turquoise", "Teal", "Aquamarine", "Light Cyan",
    "Dark Cyan", "Steel Blue", "Dodger Blue", "Royal Blue", "Midnight Blue",
    "Powder Blue", "Cornflower Blue", "Navy Blue", "Slate Blue", "Medium Blue",
    "Deep Sky Blue", "Alice Blue", "Baby Blue", "Light Steel Blue", "Indigo",
    "Violet", "Lavender", "Plum", "Orchid", "Thistle", "Medium Purple",
    "Medium Orchid", "Dark Violet", "Blue Violet", "Dark Orchid", "Amethyst",
    "Fuchsia", "Hot Pink", "Deep Pink", "Light Pink", "Pale Violet Red",
    "Rosy Brown", "Indian Red", "Firebrick", "Maroon", "Dark Red", "Tomato",
    "Coral", "Light Coral", "Salmon", "Dark Salmon", "Light Salmon",
    "Peach Puff", "Misty Rose", "Blanched Almond", "Papaya Whip", "Moccasin",
    "Navajo White", "Lemon Chiffon", "Light Goldenrod", "Gold", "Goldenrod",
    "Dark Goldenrod","Khaki", "Dark Khaki", "Beige", "Wheat", "Tan",
    "Burlywood", "Peru", "Sandy Brown", "Chocolate", "Sienna", "Saddle Brown",
    "Dark Brown", "Antique White", "Snow", "Ivory", "Linen", "Old Lace",
    "Floral White", "Ghost White", "Honeydew", "Mint Cream", "Azure",
    "Seashell", "Cornsilk", "Lavender Blush", "Gainsboro", "Light Slate Gray",
    "Slate Gray", "Dim Gray", "Dark Slate Gray", "Black Olive", "Jet",
    "Charcoal", "Ash Gray", "Cool Gray", "Warm Gray", "Silver", "Platinum",
    "Bronze", "Copper", "Brass", "Rose Gold", "Gunmetal", "Champagne", "Ruby",
    "Emerald", "Sapphire", "Jade", "Topaz", "Amber", "Onyx", "Ivory Black",
    "Payne's Gray", "Zaffre", "Verdigris", "Carmine", "Cerulean",
    "Persian Blue", "Persian Green", "Persian Red", "Persian Pink",
    "Persian Indigo", "Mahogany", "Chestnut", "Raspberry", "Mulberry",
    "Burgundy", "Eggplant", "Smoky Black", "Dark Olive", "Dark Moss Green",
    "Myrtle Green", "Pine Green", "Bottle Green", "Kelly Green"
};

#endif
// End vmDataStructs.h
