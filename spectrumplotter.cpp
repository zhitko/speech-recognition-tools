#include "spectrumplotter.h"

#include <QMap>
#include <QDebug>

#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>

class MyZoomer: public QwtPlotZoomer
{
public:
    QwtRasterData * data;
    MyZoomer( QWidget *canvas, QwtRasterData * data ):
        QwtPlotZoomer( canvas )
    {
        setTrackerMode( AlwaysOn );
        this->data = data;
    }

    virtual QwtText trackerTextF( const QPointF &pos ) const
    {
        QColor bg( Qt::white );
        bg.setAlpha( 200 );

        QwtText text = QwtText(QString::number(pos.x()) + ", "
                               + QString::number(pos.y()) + ", "
                               + QString::number(data->value(pos.x(), pos.y())));
        text.setBackgroundBrush( QBrush( bg ) );
        return text;
    }
};

class SpectrogramData: public QwtRasterData
{
public:
    SpectrogramData()
    {
        X = Y = Z = 0;
    }

    virtual double value( double x, double y ) const
    {
        double sh = 0.5;
        if(spec.contains(x))
            for(double sy: spec.value(x)->keys())
                if( sy-y < sh && sy-y > -sh )
                    return spec.value(x)->value(sy);
        double res = 0.0;
        QMap<int, double>* d = NULL;
        for(int i=0; i<500; i++){
            if(map.contains(x+i))
            {
                d = map.value(x+i);
                break;
            }
            if(map.contains(x-i))
            {
                d = map.value(x-i);
                break;
            }
        }
        for(int i=0; i<500; i++){
            if(d!=NULL && d->contains(y+i))
            {
                res = d->value(y+i);
                break;
            }
            if(d!=NULL && d->contains(y-i))
            {
                res = d->value(y-i);
                break;
            }
        }
        return res;
    }

    void addPoint(int x, int y, double z)
    {
        if(X<x) X = x;
        if(Y<y) Y = y;
        if(Z<z) Z = z;
        if(!this->map.contains(x)) map.insert(x, new QMap<int, double>() );
        this->map.value(x)->insert(y,z);
    }

    void addSpecPoint(int x, double y, double z)
    {
        if(!this->spec.contains(x)) spec.insert(x, new QMap<double, double>() );
        this->spec.value(x)->insert(y,z);
    }

    void apply(){
        setInterval( Qt::XAxis, QwtInterval( 0, X ) );
        setInterval( Qt::YAxis, QwtInterval( 0, Y ) );
        setInterval( Qt::ZAxis, QwtInterval( 0.0, Z ) );
        qDebug() << X << "-" << Y << "-" << Z;
    }
    int X,Y,Z;

private:
    QMap<int, QMap<int, double>* > map;
    QMap<int, QMap<double, double>* > spec;
};

class ColorMap: public QwtLinearColorMap
{
public:
    ColorMap():
        QwtLinearColorMap( Qt::red, Qt::black )
    {
        addColorStop( -5,   Qt::red );
        addColorStop( 0.0,  Qt::white );
        addColorStop( 0.4,  Qt::white );
        addColorStop( 0.8,  Qt::gray );
        addColorStop( 0.95, Qt::black );
    }
};

SpectrumPlotter::SpectrumPlotter(QWidget *parent):
    QwtPlot( parent )
{
    d_spectrogram = new QwtPlotSpectrogram();
    d_spectrogram->setRenderThreadCount( 0 ); // use system specific thread count

    d_spectrogram->setColorMap( new ColorMap() );
    d_spectrogram->setCachePolicy( QwtPlotRasterItem::PaintCache );

    this->data = new SpectrogramData();
    d_spectrogram->setData( this->data );
    d_spectrogram->attach( this );
    resize(600, 350);
}

void SpectrumPlotter::addPoint  (double x, double y, double z)
{
    ((SpectrogramData*)this->data)->addPoint(x,y,z);
}

void SpectrumPlotter::addSpecPoint  (double x, double y, double z)
{
    ((SpectrogramData*)this->data)->addSpecPoint(x,y,z);
}

void SpectrumPlotter::applyPlot()
{
    ((SpectrogramData*)this->data)->apply();
    d_spectrogram->setData( this->data );

    int z = ((SpectrogramData*)this->data)->Z;
    QList<double> contourLevels;
    for ( double level = 0.5; level < z; level += z/100 )
        contourLevels += level;
    d_spectrogram->setContourLevels( contourLevels );

    const QwtInterval zInterval = d_spectrogram->data()->interval( Qt::ZAxis );
    // A color bar on the right axis
    QwtScaleWidget *rightAxis = axisWidget( QwtPlot::yRight );
    rightAxis->setTitle( "Intensity" );
    rightAxis->setColorBarEnabled( true );
    rightAxis->setColorMap( zInterval, new ColorMap() );

    setAxisScale( QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );
    enableAxis( QwtPlot::yRight );

    plotLayout()->setAlignCanvasToScales( true );
    replot();

    QwtPlotZoomer* zoomer = new MyZoomer( canvas(), data );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
        Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
        Qt::RightButton );

    QwtPlotPanner *panner = new QwtPlotPanner( canvas() );
    panner->setAxisEnabled( QwtPlot::yRight, false );
    panner->setMouseButton( Qt::MidButton );

    const QFontMetrics fm( axisWidget( QwtPlot::yLeft )->font() );
    QwtScaleDraw *sd = axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( fm.width( "100.00" ) );

    const QColor c( Qt::darkBlue );
    zoomer->setRubberBandPen( c );
    zoomer->setTrackerPen( c );
}

void SpectrumPlotter::showContour( bool on )
{
    d_spectrogram->setDisplayMode( QwtPlotSpectrogram::ContourMode, on );
    replot();
}

void SpectrumPlotter::showSpectrogram( bool on )
{
    d_spectrogram->setDisplayMode( QwtPlotSpectrogram::ImageMode, on );
    d_spectrogram->setDefaultContourPen(
        on ? QPen( Qt::black, 0 ) : QPen( Qt::NoPen ) );

    replot();
}

void SpectrumPlotter::setAlpha( int alpha )
{
    d_spectrogram->setAlpha( alpha );
    replot();
}
