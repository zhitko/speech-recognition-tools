#ifndef SPEECH_DETECTOR_H
#define SPEECH_DETECTOR_H

#include <QIODevice>
#include <QAudioFormat>
#include <QVector>
#include <QQueue>

class SpeechDetector : public QIODevice
{
    Q_OBJECT
public:
    SpeechDetector(const QAudioFormat &format, QObject *parent = 0);
    ~SpeechDetector();

    QIODevice * start();
    void stop();

    qreal level() const { return m_level; }

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    quint16 getMaxAmplitude(){return m_maxAmplitude;}

    void switchAutoMod(bool val){ m_isAuto = val; }
    void setOverK(double val){ kOverValue = val;}
    void recalc();
    void setIT1(int val){ kIT1 = val; recalc();}
    void setITstep1(int val){ kITstep1 = val; recalc();}
    void setIT2(int val){ kIT2 = val; recalc();}
    void setITstep2(int val){ kITstep2 = val; recalc();}
    void setDelay(int val){ kDelay = val; recalc();}

    int calculateNoiseLevel();

    QVector<double> getBuffer(){return dBuffer;}


private:  
    void setArgs();
    void calculate(int);
    bool isCriticalOver;
    int criticalValue;
    double kOverValue;
    int counter1, counter2;
    int kIT1, kITstep1, kIT2, kITstep2, kDelay;
    int firstPeriod, secondPeriod;
    int firstPeriodStep, secondPeriodStep;
    QVector<int> * firstStepValues;
    QVector<int> * secondStepValues;
    QVector<int> * bufferedValues;
    QVector<int> * firstStepValuesBuf;
    void clearBuffers();
    int delayBufferSize;

    bool m_isAuto;

    QVector<double> dBuffer;

    const QAudioFormat m_format;
    quint16 m_maxAmplitude;
    quint16 m_level;

signals:
    void update();
    void needToStop();
};

#endif // SPEECH_DETECTOR_H
