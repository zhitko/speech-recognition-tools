#include "results.h"
#include "ui_results.h"

Results::Results(QString text, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Results)
{
    ui->setupUi(this);
    ui->text->appendPlainText(text);
}

Results::~Results()
{
    delete ui;
}
