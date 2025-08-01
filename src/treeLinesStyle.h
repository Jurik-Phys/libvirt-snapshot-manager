// Begin treeLinesStyle.h
#ifndef TREELINESSTYLE_H
#define TREELINESSTYLE_H

#include <QProxyStyle>
#include <QStyle>

class TreeLinesStyle : public QProxyStyle {

    public:
        TreeLinesStyle();
        ~TreeLinesStyle() override;

        void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
            QPainter* painter, const QWidget* widget = nullptr) const override;

    private:
        QStyle* treeLineStyle;
};

#endif
// End treeLinesStyle.h
