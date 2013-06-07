#include "worditem.h"

WordItem::WordItem(const QString & text, QListWidget * parent, int type) :
    QListWidgetItem(text, parent, type)
{
}
