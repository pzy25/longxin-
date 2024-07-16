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
        QStyledItemDelegate::paint(painter, centeredOption, index);
    }
};


#endif // CENTEREDDELEGATE_H
