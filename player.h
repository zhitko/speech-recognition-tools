#ifndef PLAYER_H
#define PLAYER_H

#include <QDialog>
#include <QFile>
#include <QAudioFormat>
#include <QAudioOutput>

namespace Ui {
    class Player;
}

namespace Aquila {
    class SignalSource;
    class Spectrogram;
}

class Player : public QDialog
{
    Q_OBJECT
    
public:
    explicit Player(QAudioFormat* format, QWidget *parent = 0);
    ~Player();
    void showPlayer(QString path);

public slots:
    void play();
    void stop();
    void changed();
    void remove();

    void showWaveForm();
    void showHighpassFilter();
    void showHamming();
    void showFFT();
    void showChanels();
    void showFormantProcessor();
    void compare();
    
private:
    Ui::Player *ui;
    QString path;
    QAudioFormat* format;
    QAudioOutput* outputAudio;
    QFile * wavFile;

    void makeItemsList();

    Aquila::SignalSource * getRawData();
    Aquila::SignalSource * getParamsData(int i);
};

#endif // PLAYER_H
