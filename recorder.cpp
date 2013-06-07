#include "recorder.h"

#include <QDebug>

#include <QDate>
#include <QTime>
#include <QDir>

#include <QTimer>

#include <QBuffer>

#include "SpeechDetector.h"

#include "plotter.h"

#include "aquila/global.h"
#include "aquila/source/WaveFile.h"
#include "aquila/source/SignalSource.h"

Recorder::Recorder(QAudioFormat * format, QObject *parent) :
    QObject(parent)
{
    this->format = format;

    device = QAudioDeviceInfo::defaultInputDevice();

    inputAudio = new QAudioInput(device, *format);
    detector  = new SpeechDetector(*format);

    QObject::connect(detector, &SpeechDetector::update, this, &Recorder::update);
    QObject::connect(detector, &SpeechDetector::needToStop, this, &Recorder::needToStop);
}

void Recorder::record(QString word)
{
    emit active(true);
    this->word = word;
    inputAudio->start(detector->start());
}

void Recorder::needToStop()
{
    QTimer::singleShot(100,this, SLOT(stopRecord()));
}

void Recorder::stopRecord()
{
    inputAudio->stop();
    detector->stop();
    storeBuffer();
    emit active(false);
}

void Recorder::storeBuffer()
{
    QDir(QDir::currentPath()).mkdir("base");
    QDir(QDir::currentPath()).mkdir("base"+QString(QDir::separator())+word);

    QString fname = "base"+QString(QDir::separator())+word+QString(QDir::separator())
                + QDate::currentDate().toString("dd-MM-yy")
                + " "
                + QTime::currentTime().toString("hh:mm:ss:zzz")
                + ".wav";
    QVector<double> array = this->detector->getBuffer();
    qDebug() << array.size();
    qDebug() << format->sampleRate();
    Aquila::SignalSource source(array.data(),array.size(),format->sampleRate());
    Aquila::WaveFile::save(source, fname.toStdString());
}

void Recorder::play()
{

}

int Recorder::getMaxVolume()
{
    qDebug() << "getMaxVolume " << this->detector->getMaxAmplitude();
    return this->detector->getMaxAmplitude();
}

void Recorder::update()
{
    qreal value = detector->level();
    qDebug() << "volume: " << value;
    emit volume(value);
}

void Recorder::startTest()
{
    qDebug() << "Start test";
    inputAudio->start(detector->start());
    QTimer::singleShot(1000,this, SLOT(stopTest()));
}

void Recorder::stopTest()
{
    qDebug() << "Stop test";
    inputAudio->stop();
    detector->stop();
    detector->calculateNoiseLevel();
}

void Recorder::setFactor(double f)
{
    this->detector->setOverK(f);
}

void Recorder::setFrameSize(int fs)
{
    this->detector->setIT2(fs);
}

void Recorder::setStepSize(int ss)
{
    this->detector->setITstep2(ss);
}

void Recorder::setBufferSize(int bs)
{
    this->detector->setDelay(bs);
}
