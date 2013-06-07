#ifndef WORDITEM_H
#define WORDITEM_H

#include <QListWidgetItem>

class WordItem : public QListWidgetItem
{
public:
    WordItem(const QString & text, QListWidget * parent = 0, int type = Type);
};

#endif // WORDITEM_H
