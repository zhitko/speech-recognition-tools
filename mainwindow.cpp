#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "addtext.h"
#include "worditem.h"
#include "settings.h"
#include "consts.h"

#include "recorder.h"
#include "player.h"
#include "results.h"

#include "speechproc.h"

#include <QDir>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    this->addText = new AddText(this);
    connect(addText, &AddText::acceptText, this, &MainWindow::addItems);

    connect(ui->recognize, &QPushButton::clicked, this, &MainWindow::recognize);
    connect(ui->record, &QPushButton::clicked, this, &MainWindow::record);
    connect(ui->play, &QPushButton::clicked, this, &MainWindow::play);

    this->currentItem = NULL;
    connect(ui->wordItems, &QListWidget::itemSelectionChanged, this, &MainWindow::setCurrentItem);

    this->recorder = new Recorder(&this->format);
    connect(this->recorder, &Recorder::volume, ui->volume, &QProgressBar::setValue);
    this->ui->volume->setMaximum(this->recorder->getMaxVolume());
    connect(this->recorder, &Recorder::active, ui->record, &QPushButton::setDisabled);
    connect(this->recorder, &Recorder::active, ui->recognize, &QPushButton::setDisabled);
    connect(this->recorder, &Recorder::stored, this, &MainWindow::newFileStorred);

    connect(this->ui->stop, &QPushButton::clicked, this->recorder, &Recorder::needToStop);

    this->settings = new Settings(this->recorder);

    this->player = new Player(&this->format);

    setupUI();
    reloadItems();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    ui->menuBar->addAction("Add text", this->addText, SLOT(show()));
    ui->menuBar->addAction("Run test", this->recorder, SLOT(startTest()));
    ui->menuBar->addAction("Settings", this, SLOT(showSettings()));
}

void MainWindow::addItems(QStringList data)
{
    QDir(QDir::currentPath()).mkdir("base");
    foreach (QString word, data) {
        QDir(QDir::currentPath()).mkdir("base"+QString(QDir::separator())+word);
    }
    reloadItems();
}

void MainWindow::reloadItems()
{
    this->wordItems.clear();
    this->patterns.clear();
    this->ui->wordItems->clear();
    QDir dir(QDir::currentPath()+QString(QDir::separator())+"base"+QString(QDir::separator()));
    foreach (QFileInfo file, dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot)) {
        QString name = file.baseName();
        WordItem * item = new WordItem(name);
        this->wordItems.append(item);
        this->ui->wordItems->addItem(item);
        QStringList nameFilter;
        nameFilter << "*.wav";
        if(!name.startsWith("_"))
            foreach (QFileInfo ptrn, QDir(file.absoluteFilePath()).entryInfoList(nameFilter, QDir::Files)) {
                Pattern pattern = {ptrn.absoluteFilePath(), name};
                this->patterns.append(pattern);
            }
    }
}

void MainWindow::record()
{
    qDebug() << "Record start";
    this->mode = RecMode::RecordTemplate;
    if(this->currentItem != NULL){
        qDebug() << "Record word: " << this->currentItem->text();
        this->recorder->record(this->currentItem->text());
    }
}

void MainWindow::recognize()
{
    qDebug() << "Recognition start";
    this->mode = RecMode::SingleRecognition;
    this->recorder->record("_rec");
}

void MainWindow::newFileStorred(QString name)
{
    if(mode == RecMode::SingleRecognition){
        Aquila::SignalSource * source = proc(name.toStdString());
        QMap<QString,int> dist;
        foreach(Pattern p, this->patterns) {
            Aquila::SignalSource * pattern = proc(p.path.toStdString());
            int R = compare(source, pattern);
            qDebug() << p.name << " : " << R;
//            dist.insert(p.name, (dist.value(p.name, R)+R)/2 );
            dist.insert(p.name, (dist.value(p.name, R) > R ? R : dist.value(p.name, R)) );
            delete pattern;
        }
        delete source;
        QString result = "";
        foreach(QString key, dist.keys())
            result += "" + key + " : " + QString::number(dist.value(key)) + "\n";
        Results * results = new Results(result, this);
        results->show();
    }
}

void MainWindow::play()
{
    if(this->currentItem != NULL){
        this->player->showPlayer(QDir::currentPath()+QString(QDir::separator())
                                 +"base"+QString(QDir::separator())+this->currentItem->text());
    }
}

void MainWindow::setCurrentItem()
{
    qDebug() << this->ui->wordItems->currentItem()->text();
    currentItem = (WordItem*) this->ui->wordItems->currentItem();
}

void MainWindow::showSettings()
{
    this->recorder->needToStop();
    this->settings->show();
}
