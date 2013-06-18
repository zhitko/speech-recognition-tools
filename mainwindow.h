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

enum class RecMode {
    RecordTemplate,
    SingleRecognition,
    CycledRecognition
};

struct Pattern {
    QString path, name;
};

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
    void recognize();
    void newFileStorred(QString);
    void play();
    void setCurrentItem();
    void showSettings();
    
private:
    RecMode mode;
    Recorder *recorder;
    AddText *addText;
    QList<WordItem*> wordItems;
    QList<Pattern> patterns;
    WordItem* currentItem;
    Settings* settings;
    Player* player;
    QAudioFormat format;

    Ui::MainWindow *ui;
    void setupUI();
};

#endif // MAINWINDOW_H
