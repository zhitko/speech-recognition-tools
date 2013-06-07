#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QAudioDeviceInfo>
#include <QAudioInput>

class SpeechDetector;
class QAudioFormat;

class Recorder : public QObject
{
    Q_OBJECT
public:
    explicit Recorder(QAudioFormat * format, QObject *parent = 0);

    void record(QString word);
    void play();
    int getMaxVolume();
    
signals:
    void volume(int volume);
    void active(bool);

public slots:
    void update();
    void stopRecord();
    void needToStop();
    void stopTest();
    void startTest();

    void setFactor(double);
    void setFrameSize(int);
    void setStepSize(int);
    void setBufferSize(int);

private:
    QAudioFormat * format;
    QAudioDeviceInfo device;
    QAudioInput * inputAudio;
    SpeechDetector * detector;
    QString word;
    void storeBuffer();
    
};

#endif // RECORDER_H
