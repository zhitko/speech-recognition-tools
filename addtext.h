#ifndef ADDTEXT_H
#define ADDTEXT_H

#include <QDialog>

namespace Ui {
class AddText;
}

class AddText : public QDialog
{
    Q_OBJECT
    
public:
    explicit AddText(QWidget *parent = 0);
    ~AddText();

private slots:
    void processText();

signals:
    void acceptText(QStringList data);
    
private:
    Ui::AddText *ui;
};

#endif // ADDTEXT_H
