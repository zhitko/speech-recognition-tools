#ifndef RESULTS_H
#define RESULTS_H

#include <QDialog>

namespace Ui {
class Results;
}

class Results : public QDialog
{
    Q_OBJECT
    
public:
    explicit Results(QString text, QWidget *parent = 0);
    ~Results();
    
private:
    Ui::Results *ui;
};

#endif // RESULTS_H
