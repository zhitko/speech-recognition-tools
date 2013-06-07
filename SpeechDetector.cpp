#include <stdlib.h>
#include <math.h>

#include <QDebug>

#include <QAudioDeviceInfo>
#include <QAudioInput>

#include <QFile>
#include <QDir>

#include <QBuffer>

#include <QtCore/qendian.h>
#include <QDataStream>

#include "SpeechDetector.h"

#include "QTextStream"

SpeechDetector::SpeechDetector(const QAudioFormat &format, QObject *parent)
    :   QIODevice(parent)
    ,   m_format(format)
    ,   m_maxAmplitude(0)
    ,   m_level(0)
    ,   criticalValue(5000)
    ,   isCriticalOver(false)
    ,   kOverValue(1.5)
    ,   m_isAuto(true)

{
    setArgs();
    switch (m_format.sampleSize()) {
    case 8:
        switch (m_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (m_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 32767;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

SpeechDetector::~SpeechDetector()
{
    delete firstStepValues;
    delete secondStepValues;
    delete bufferedValues;
    delete firstStepValuesBuf;

}

QIODevice * SpeechDetector::start()
{
    open(QIODevice::WriteOnly);
    this->clearBuffers();
    this->dBuffer.clear();
    return this;
}

void SpeechDetector::stop()
{
    close();
}

qint64 SpeechDetector::readData(char *data, qint64 maxlen)
{
    return 0;
}

qint64 SpeechDetector::writeData(const char *data, qint64 len)
{
    qDebug() << "isCriticalOver " << isCriticalOver;
    qDebug() << "m_isAuto " << m_isAuto;

    if (m_maxAmplitude) {
        Q_ASSERT(m_format.sampleSize() % 8 == 0);
        const int channelBytes = m_format.sampleSize() / 8;
        const int sampleBytes = m_format.channelCount() * channelBytes;
        Q_ASSERT(len % sampleBytes == 0);
        const int numSamples = len / sampleBytes;

        quint16 maxValue = 0;
        const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);

        for (int i = 0; i < numSamples; ++i) {
            for(int j = 0; j < m_format.channelCount(); ++j) {
                quint16 value = 0;
                double dValue = 0.0;

                if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
                    value = *reinterpret_cast<const quint8*>(ptr);
                    dValue = *reinterpret_cast<const quint8*>(ptr);
                } else if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::SignedInt) {
                    value = qAbs(*reinterpret_cast<const qint8*>(ptr));
                    dValue = *reinterpret_cast<const qint8*>(ptr);
                } else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian){
                        value = qFromLittleEndian<quint16>(ptr);
                        dValue = qFromLittleEndian<quint16>(ptr);
                    }else{
                        value = qFromBigEndian<quint16>(ptr);
                        dValue = qFromBigEndian<quint16>(ptr);
                    }
                } else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::SignedInt) {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian){
                        value = qAbs(qFromLittleEndian<qint16>(ptr));
                        dValue = qFromLittleEndian<qint16>(ptr);
                    }else{
                        value = qAbs(qFromBigEndian<qint16>(ptr));
                        dValue = qFromBigEndian<qint16>(ptr);
                    }
                }

                dBuffer.append(dValue);
                if(!isCriticalOver && m_isAuto && dBuffer.size()==this->delayBufferSize){
                    dBuffer.remove(0);
                    qDebug() << "remove first" << this->delayBufferSize;
                }

                maxValue = qMax(value, maxValue);
                calculate(qAbs((int) value));
                ptr += channelBytes;
            }
        }
        maxValue = qMin(maxValue, m_maxAmplitude);
        m_level = maxValue;
    }

    emit update();
    return len;
}

void SpeechDetector::recalc()
{
        double freq = m_format.sampleRate();        // Hz
        this->delayBufferSize = (double)kDelay*freq/1000;
        double firstPeriodSec = (double)kIT1/1000;
        double firstPeriodStepSec = (double)kITstep1/1000;
        double secondPeriodSec = (double)kIT2/1000;
        double secondPeriodStepSec = (double)kITstep2/1000;
        this->firstPeriod = firstPeriodSec * freq;
        this->firstPeriodStep = firstPeriodStepSec * freq;
        this->secondPeriod = secondPeriodSec / firstPeriodStepSec;
        this->secondPeriodStep = secondPeriodStepSec / firstPeriodStepSec;
        qDebug() << "kDelay " << this->kDelay;
        qDebug() << "delayBufferSize " << this->delayBufferSize;
        qDebug() << "kIT2 " << this->kIT2;
        qDebug() << "kITstep2 " << this->kITstep2;
        qDebug() << "firstPeriod " << this->firstPeriod;
        qDebug() << "firstPeriodStep " << this->firstPeriodStep;
        qDebug() << "secondPeriod " << this->secondPeriod;
        qDebug() << "secondPeriodStep " << this->secondPeriodStep;
}

void SpeechDetector::setArgs()
{
    kIT1 = 8;
    kITstep1 = 4;
    kIT2 = 400;
    kITstep2 = 160;
    kDelay = 400;
    recalc();
    this->firstStepValues = new QVector<int>();
    this->firstStepValuesBuf = new QVector<int>();
    this->secondStepValues = new QVector<int>();
    this->bufferedValues = new QVector<int>();
}

void SpeechDetector::calculate(int value)
{
    this->counter1 ++;
    this->bufferedValues->append(value);
    if (this->counter1 >= this->firstPeriodStep)
    {
        this->counter1 = 0;
        if (this->bufferedValues->count() >= this->firstPeriod)
        {
            // calculate first value
            long value1 = 0;
            for( int i=0 ; i<this->firstPeriod; i++ )
                value1 += this->bufferedValues->at(i);
            // remove first elements
            this->bufferedValues->remove(0,this->firstPeriodStep);
            // add calculated value to vector <1>
            value1 = value1 * 3 / this->firstPeriod;
            if(value1 > m_maxAmplitude){
                this->firstStepValues->append( m_maxAmplitude );
                this->firstStepValuesBuf->append( m_maxAmplitude );
            }else{
                this->firstStepValues->append( value1 );
                this->firstStepValuesBuf->append( value1 );
            }
            this->counter2 ++;
            if(this->counter2 >= this->secondPeriodStep)
            {
                this->counter2 = 0;
                if(this->firstStepValues->count() >= this->secondPeriod)
                {
                    long value2 = 0;
                    // calculate second value
                    for(int i=0; i<this->secondPeriod; i++)
                        value2 += this->firstStepValues->at(i);
                    this->firstStepValues->remove(0,this->secondPeriodStep);
                    value2 = value2/this->secondPeriod;

                    if (m_isAuto){
                        if (value2 > criticalValue){
                            isCriticalOver = true;
                        }else
                            if(isCriticalOver)
                            {
                                emit needToStop();
                                isCriticalOver = false;
                            }
                    }

                    if(value2 > m_maxAmplitude)
                        this->secondStepValues->append( m_maxAmplitude );
                    else
                        this->secondStepValues->append( value2 );
                }
            }
        }
    }
}

void SpeechDetector::clearBuffers()
{
    this->firstStepValues->clear();
    this->firstStepValuesBuf->clear();
    this->secondStepValues->clear();
    this->bufferedValues->clear();
    this->counter1 = 0;
    this->counter2 = 0;
    this->isCriticalOver = false;
}

int SpeechDetector::calculateNoiseLevel()
{
    int noiseLevel = 0;
    for(int i=0; i<this->secondStepValues->count(); i++)
        noiseLevel += this->secondStepValues->at(i);
    criticalValue = (double)noiseLevel * kOverValue / this->secondStepValues->count();
    noiseLevel = noiseLevel/this->secondStepValues->count();
    qDebug() << "noiseLevel " << noiseLevel;
    qDebug() << "criticalValue " << criticalValue;
    return noiseLevel;
}
