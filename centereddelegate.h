#ifndef CENTEREDDELEGATE_H
#define CENTEREDDELEGATE_H


#include <QStyledItemDelegate>

class CenteredDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CenteredDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem centeredOption(option);
        centeredOption.displayAlignment = Qt::AlignCenter;

        // 根据内容设置颜色
        QVariant value = index.data();
        if (value.isValid() && value.type() == QVariant::String)
        {
            QString text = value.toString();
            if (text == "对") {
                       centeredOption.palette.setColor(QPalette::Text, Qt::green);
                   } else if (text == "错") {
                       centeredOption.palette.setColor(QPalette::Text, Qt::red);
                   }
               }
        QStyledItemDelegate::paint(painter, centeredOption, index);
    }
};


#endif // CENTEREDDELEGATE_H
