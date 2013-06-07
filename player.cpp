#include "player.h"
#include "ui_player.h"

#include <QDir>
#include <QDebug>

#include <QVector>

#include "speechproc.h"

#include "plotter.h"
#include "spectrumplotter.h"
#include "consts.h"

#include "aquila/global.h"
#include "aquila/source/WaveFile.h"
#include "aquila/source/SignalSource.h"
#include "aquila/source/window/HammingWindow.h"
#include "aquila/transform/FftFactory.h"
#include "aquila/source/Frame.h"
#include "aquila/source/FramesCollection.h"
#include "aquila/transform/Spectrogram.h"
#include "aquila/functions.h"

Player::Player(QAudioFormat* format, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Player)
{
    ui->setupUi(this);

    this->format = format;

    connect(this->ui->items, &QListWidget::itemSelectionChanged, this, &Player::changed);

    connect(this->ui->deleteFile, &QPushButton::clicked, this, &Player::remove);
    connect(this->ui->play, &QPushButton::clicked, this, &Player::play);
    connect(this->ui->waveForm, &QPushButton::clicked, this, &Player::showWaveForm);
    connect(this->ui->highpassFilter, &QPushButton::clicked, this, &Player::showHighpassFilter);
    connect(this->ui->Hamming, &QPushButton::clicked, this, &Player::showHamming);
    connect(this->ui->FFT, &QPushButton::clicked, this, &Player::showFFT);
    connect(this->ui->chanels, &QPushButton::clicked, this, &Player::showChanels);
    connect(this->ui->formantProcessor, &QPushButton::clicked, this, &Player::showFormantProcessor);
    connect(this->ui->compare, &QPushButton::clicked, this, &Player::compare);

    this->outputAudio = new QAudioOutput(*format);
    QObject::connect(this->outputAudio,SIGNAL(stateChanged(QAudio::State)),this,SLOT(stop()));
}

Player::~Player()
{
    delete ui;
}

Aquila::SignalSource * Player::getParamsData(int i)
{
    if(i >= this->ui->items->count()) return NULL;
    QListWidgetItem * item = this->ui->items->item(i);
    if(item == NULL) return NULL;
    QString path = this->path + QString(QDir::separator()) + item->text();
    qDebug() << path;
    Aquila::SignalSource * data = new Aquila::WaveFile(path.toStdString());

    Aquila::SignalSource * _data = procHighpassFilter(data);
    delete data;
    data = procHamming(_data);
    delete _data;
    Aquila::Spectrogram * spectrogram = procSpectrum(data);
    delete data;
    data = procChannels(spectrogram);
    Aquila::FramesCollection channels(*data, bandNumb);
    delete spectrogram;
    _data = procParams(&channels);
    delete data;
    return _data;
}

void Player::compare()
{
    Aquila::SignalSource * data = getRawData();

    if(data != NULL){
        Aquila::SignalSource * _data = procHighpassFilter(data);
        delete data;
        data = procHamming(_data);
        delete _data;
        Aquila::Spectrogram * spectrogram = procSpectrum(data);
        delete data;
        data = procChannels(spectrogram);
        Aquila::FramesCollection channels(*data, bandNumb);
        delete spectrogram;
        _data = procParams(&channels);
        delete data;

        int count = this->ui->items->count();
        CONTRES res;
        res.R = -1;
        res.key = -1;
        res.pos[0] = -1;
        res.pos[1] = -1;

        for(int i=0; i<count; i++){
            Aquila::SignalSource * ptrn = getParamsData(i);
            DynTimeWarping(((*_data)*255).toArray(), _data->getSamplesCount()/FrmVectSize
                         , ((* ptrn)*255).toArray(),  ptrn->getSamplesCount()/FrmVectSize
                         , &res );
            delete ptrn;
            qDebug() << this->ui->items->item(i)->text();
//            qDebug() << res.key;
            qDebug() << "Dist: " << res.R;
//            qDebug() << res.pos[0];
//            qDebug() << res.pos[1];
            qDebug() << "----------";
        }

        delete _data;
    }
}

//void Player::showFormantProcessor()
//{
//    Aquila::SignalSource * data = getRawData();

//    if(data != NULL){
//        Aquila::SignalSource * _data = procHighpassFilter(data);
//        delete data;
//        data = procHamming(_data);
//        delete _data;
//        Aquila::Spectrogram * spectrogram = procSpectrum(data);
//        delete data;

//        int * bandIndex = new int[bandNumb+1];
//        MakeLinIndexTab(startFreq,endFreq,smplFreq, fftOrder, bandIndex, bandNumb+1);

//        Plotter* plotter2 = new Plotter();
//        plotter2->setTitle("Spectrum (F)");
//        plotter2->show();
//        Plotter* plotter5 = new Plotter();
//        plotter5->setTitle("Spectrum (Fn)");
//        plotter5->show();
//        Plotter* plotter3 = new Plotter();
//        plotter3->setTitle("Spectrum (A)");
//        plotter3->show();

//        SpectrumPlotter* plotter = new SpectrumPlotter();
//        plotter->setTitle("Spectrum");
//        plotter->show();

//        int size = spectrogram->getFrameCount();
//        float * f1 = new float[size];
//        float * f2 = new float[size];
//        float * f3 = new float[size];

//        for (std::size_t x = 0; x < size; x++)
//        {
//            int fftSize = spectrogram->getSpectrumSize() / 2;
//            float * evalVect = new float[bandNumb];
//            float * pFft = new float[fftSize];
//            for (std::size_t y = 0; y < fftSize; y++){
//                pFft[y] = Aquila::dB(spectrogram->getPoint(x, y));
//            }

//            DivideIntoZones(pFft,fftSize,evalVect,bandNumb,bandIndex,overFact);

//            for (std::size_t y = 0; y < bandNumb; y++)
//                plotter->addPoint(x, y, evalVect[y]);

//            float byteVector[FrmVectSize*2];
//            TransformVector2(evalVect,byteVector);

//            plotter2->appendPoint(byteVector[0], QString::number(0));
//            plotter2->appendPoint(byteVector[1], QString::number(1));
//            plotter2->appendPoint(byteVector[2], QString::number(2));

//            plotter3->appendPoint(byteVector[3], QString::number(3));
//            plotter3->appendPoint(byteVector[4], QString::number(4));
//            plotter3->appendPoint(byteVector[5], QString::number(5));
//            f1[x] = byteVector[0];
//            f2[x] = byteVector[1];
//            f3[x] = byteVector[2];

//            plotter->addSpecPoint(x, byteVector[0], -5.0);
//            plotter->addSpecPoint(x, byteVector[1], -5.0);
//            plotter->addSpecPoint(x, byteVector[2], -5.0);

//            delete evalVect;
//            delete pFft;
//        }
//        for(int i=0; i<size; i++){
//            float fv1 = (f2[i]-1);
//                if(fv1 < 0.0) fv1 = 0.0;
//                else if(fv1 > (7-1)) fv1 = 1.0;
//                else fv1 /= (7-1);
//            plotter5->appendPoint(fv1, QString::number(0));
//            float fv2 = (f3[i]-4);
//                if(fv2 < 0.0) fv2 = 0.0;
//                else if(fv2 > (29-4)) fv2 = 1.0;
//                else fv2 /= (29-4);
//            plotter5->appendPoint(fv2, QString::number(1));
//            float fv3 = (f1[i]-16);
//                if(fv3 < 0.0) fv3 = 0.0;
//                else if(fv3 > (31-16)) fv3 = 1.0;
//                else fv3 /= (31-16);
//            plotter5->appendPoint(fv3, QString::number(2));
//        }

//        plotter->applyPlot();
//        plotter2->applyPlot();
//        plotter3->applyPlot();
//        plotter5->applyPlot();

//        delete f1,f2,f3;
//        delete spectrogram;
//    }
//}

void Player::showFormantProcessor()
{
    Aquila::SignalSource * data = getRawData();

    if(data != NULL){
        Aquila::SignalSource * _data = procHighpassFilter(data);
        delete data;
        data = procHamming(_data);
        delete _data;
        Aquila::Spectrogram * spectrogram = procSpectrum(data);
        delete data;
        data = procChannels(spectrogram);
        Aquila::FramesCollection channels(*data, bandNumb);
        delete spectrogram;
        _data = procParams(&channels);
        Aquila::FramesCollection params(*_data, FrmVectSize);
        delete data;

        Plotter* plotterF = new Plotter();
        plotterF->setTitle("Spectrum (Fn)");
        plotterF->show();
        Plotter* plotterA = new Plotter();
        plotterA->setTitle("Spectrum (A)");
        plotterA->show();

        for(int i=0; i<params.count(); i++){
                plotterF->appendPoint(params.frame(i).sample(0), QString::number(0));
                plotterF->appendPoint(params.frame(i).sample(1), QString::number(1));
                plotterF->appendPoint(params.frame(i).sample(2), QString::number(2));

                plotterA->appendPoint(params.frame(i).sample(3), QString::number(3));
                plotterA->appendPoint(params.frame(i).sample(4), QString::number(4));
                plotterA->appendPoint(params.frame(i).sample(5), QString::number(5));
        }

        plotterF->applyPlot();
        plotterA->applyPlot();

        delete _data;
    }
}

void Player::showChanels()
{
    Aquila::SignalSource * data = getRawData();

    if(data != NULL){

        SpectrumPlotter* plotter = new SpectrumPlotter();
        plotter->setTitle("Chanels");
        plotter->show();

        Aquila::SignalSource * _data = procHighpassFilter(data);
        delete data;
        data = procHamming(_data);
        delete _data;
        Aquila::Spectrogram * spectrogram = procSpectrum(data);
        delete data;
        data = procChannels(spectrogram);
        Aquila::FramesCollection channels(*data, bandNumb);
        delete spectrogram;

        for (std::size_t x = 0; x < channels.count(); x++)
        {
            for (std::size_t y = 0; y < channels.getSamplesPerFrame(); ++y)
            {
                double z = channels.frame(x).sample(y);
                plotter->addPoint(x,y,z);
            }
        }
        plotter->applyPlot();
        delete data;
    }
}

void Player::showFFT()
{
    Aquila::SignalSource * data = getRawData();

    if(data != NULL){
        Aquila::SignalSource * _data = procHighpassFilter(data);
        delete data;
        data = procHamming(_data);
        delete _data;
        Aquila::Spectrogram * spectrogram = procSpectrum(data);
        double scaleY = data->getSampleFrequency() / CSpectrogramFrameSize;
        delete data;

        SpectrumPlotter* plotter = new SpectrumPlotter();
        plotter->setTitle("Spectrum");
        plotter->show();

        for (std::size_t x = 0; x < spectrogram->getFrameCount(); ++x)
        {
            for (std::size_t y = 0; y < spectrogram->getSpectrumSize() / 2; ++y)
            {
                Aquila::ComplexType point = spectrogram->getPoint(x, y);
                double Y = y*scaleY;
                plotter->addPoint(x,Y,Aquila::dB(point));
            }
        }
        plotter->applyPlot();
        delete spectrogram;
    }
}

void Player::showHamming()
{
    Aquila::SignalSource * data = getRawData();
    if(data != NULL){
        Aquila::SignalSource * _data = procHighpassFilter(data);
        delete data;
        data = procHamming(_data);
        delete _data;

        Plotter* plotter = new Plotter();
        plotter->setTitle("Hamming");
        plotter->show();
        for (std::size_t i = 0; i < data->getSamplesCount(); i+=10)
        {
            plotter->appendPoint(data->sample(i));
        }
        plotter->applyPlot();
        delete data;
    }
}

void Player::showHighpassFilter()
{
    Aquila::SignalSource * data = getRawData();
    if(data != NULL){
        Aquila::SignalSource * _data = procHighpassFilter(data);
        delete data;
        data = _data;

        Plotter* plotter = new Plotter();
        plotter->setTitle("Wave form");
        plotter->show();
        for (std::size_t i = 0; i < data->getSamplesCount(); i+=10)
        {
            plotter->appendPoint(data->sample(i));
        }
        plotter->applyPlot();
        delete data;
    }
}

void Player::showWaveForm()
{
    Aquila::SignalSource * data = getRawData();
    if(data != NULL){
        Plotter* plotter = new Plotter();
        plotter->setTitle("Wave form");
        plotter->show();
        for (std::size_t i = 0; i < data->getSamplesCount(); i+=10)
        {
            plotter->appendPoint(data->sample(i));
        }
        plotter->applyPlot();
        delete data;
    }
}

void Player::showPlayer(QString path)
{
    this->path = path;
    makeItemsList();
    show();
}

void Player::makeItemsList()
{
    this->ui->items->clear();
    QDir dir(path);
    QStringList filters;
        filters << "*.wav";
    foreach (QFileInfo file, dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot)) {
        this->ui->items->addItem(file.fileName());
    }
}

void Player::play()
{
    QListWidgetItem * item = this->ui->items->currentItem();
    if(item != NULL){
        QString path = this->path + QString(QDir::separator()) + item->text();
        qDebug() << path;
        wavFile = new QFile(path);
        this->ui->fileSize->setText(QString::number(wavFile->size()));
        wavFile->open(QFile::ReadWrite);
        outputAudio->start(wavFile);
    }
}

void Player::changed()
{
    QListWidgetItem * item = this->ui->items->currentItem();
    if(item != NULL){
        QString path = this->path + QString(QDir::separator()) + item->text();
        qDebug() << path;
        wavFile = new QFile(path);
        this->ui->fileSize->setText(QString::number(wavFile->size()));
        delete wavFile;
    }
}

void Player::stop()
{
    if(this->outputAudio->state() == QAudio::StoppedState){
        outputAudio->stop();
        if(wavFile != NULL)
        {
            wavFile->close();
            delete wavFile;
        }
    }
}

void Player::remove()
{
    QListWidgetItem * item = this->ui->items->currentItem();
    if(item != NULL){
        QString path = item->text();
        QDir(this->path).remove(path);
        makeItemsList();
    }
}

Aquila::SignalSource * Player::getRawData()
{
    QListWidgetItem * item = this->ui->items->currentItem();
    if(item == NULL) return NULL;
    QString path = this->path + QString(QDir::separator()) + item->text();
    qDebug() << path;
    Aquila::WaveFile * wav = new Aquila::WaveFile(path.toStdString());
    return wav;
}
