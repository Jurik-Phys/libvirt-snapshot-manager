// Begin treeLinesStyle.cpp

#include "treeLinesStyle.h"
#include <QStyleFactory>

TreeLinesStyle::TreeLinesStyle() : QProxyStyle(QStyleFactory::create("fusion")),
                                treeLineStyle(QStyleFactory::create("windows")){
}

TreeLinesStyle::~TreeLinesStyle(){
    delete treeLineStyle;
}

void TreeLinesStyle::drawPrimitive(PrimitiveElement element,
                                const QStyleOption* option,
                                QPainter* painter,
                                const QWidget* widget) const {
    // Рисуем линии для ветвей дерева из windows-стиля
    if (element == PE_IndicatorBranch && treeLineStyle) {
        treeLineStyle->drawPrimitive(element, option, painter, widget);
    } else {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}
// End treeLinesStyle.cpp
