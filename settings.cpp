#include "settings.h"
#include "ui_settings.h"
#include "recorder.h"

Settings::Settings(Recorder * recorder, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    this->recorder = recorder;

    connect(this->ui->factor, SIGNAL(valueChanged(double)), this->recorder, SLOT(setFactor(double)));
    connect(this->ui->frame, SIGNAL(valueChanged(int)), this->recorder, SLOT(setFrameSize(int)));
    connect(this->ui->step, SIGNAL(valueChanged(int)), this->recorder, SLOT(setStepSize(int)));
    connect(this->ui->buffer, SIGNAL(valueChanged(int)), this->recorder, SLOT(setBufferSize(int)));
}

Settings::~Settings()
{
    delete ui;
}
