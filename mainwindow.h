#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioFormat>

class AddText;
class WordItem;
class QListWidgetItem;
class Recorder;
class Settings;
class Player;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void addItems(QStringList data);
    void reloadItems();
    void record();
    void play();
    void setCurrentItem();
    void showSettings();
    
private:
    Recorder *recorder;
    AddText *addText;
    QList<WordItem*> wordItems;
    WordItem* currentItem;
    Settings* settings;
    Player* player;
    QAudioFormat format;

    Ui::MainWindow *ui;
    void setupUI();
};

#endif // MAINWINDOW_H
