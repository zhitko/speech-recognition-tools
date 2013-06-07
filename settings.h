#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

class Recorder;

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT
    
public:
    explicit Settings(Recorder * recorder, QWidget *parent = 0);
    ~Settings();
    
private:
    Ui::Settings *ui;
    Recorder * recorder;
};

#endif // SETTINGS_H
