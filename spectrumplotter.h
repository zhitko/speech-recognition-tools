#ifndef SPECTRUMPLOTTER_H
#define SPECTRUMPLOTTER_H

#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>

class QwtRasterData;

class SpectrumPlotter : public QwtPlot
{
    Q_OBJECT
public:
    explicit SpectrumPlotter(QWidget *parent = 0);
    
signals:

public slots:
    void showContour( bool on );
    void showSpectrogram( bool on );
    void setAlpha( int );
    void addPoint(double x, double y, double z);
    void addSpecPoint(double x, double y, double z);
    void applyPlot();
private:
    QwtPlotSpectrogram *d_spectrogram;
    QwtRasterData * data;
    
};

#endif // SPECTRUMPLOTTER_H
