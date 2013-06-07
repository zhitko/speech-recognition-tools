#include "addtext.h"
#include "ui_addtext.h"

#include <QDebug>

AddText::AddText(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddText)
{
    ui->setupUi(this);
    connect(this->ui->buttonBox, &QDialogButtonBox::accepted, this, &AddText::processText);
}

AddText::~AddText()
{
    delete ui;
}

void AddText::processText()
{
    QString text = this->ui->addedText->toPlainText();
    emit acceptText(text.split(" "));
}
