#include "plotter.h"

#include <QDebug>

#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>

const int colorCount = 13;
const QColor colors[] = {
    Qt::red,    //A1
    Qt::green,  //A2
    Qt::blue,   //A3

    Qt::black,  //Spectrum Normalize
    Qt::cyan,   //Frequency parameters
    Qt::magenta,
    Qt::yellow,
    Qt::darkRed,
    Qt::darkGreen,
    Qt::darkBlue,
    Qt::darkCyan,
    Qt::darkMagenta,
    Qt::darkYellow
};

Plotter::Plotter( QWidget *parent ):
    QwtPlot( parent )
{
    // Assign a title
    setTitle( "WAVE DATA" );

    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setFrameStyle( QFrame::Box | QFrame::Plain );
    canvas->setLineWidth( 1 );
    canvas->setPalette( Qt::white );

    setCanvas( canvas );

    alignScales();

    // Insert grid
    d_grid = new QwtPlotGrid();
    d_grid->attach( this );

    // Axis
    setAxisTitle( QwtPlot::xBottom, "Time" );
//    setAxisScale( QwtPlot::xBottom, -d_interval, 0.0 );

    setAxisTitle( QwtPlot::yLeft, "Amplitude" );
//    setAxisScale( QwtPlot::yLeft, -1.0, 1.0 );

    resize(600, 350);
}

//
//  Set a plain canvas frame and align the scales to it
//
void Plotter::alignScales()
{
    // The code below shows how to align the scales to
    // the canvas frame, but is also a good example demonstrating
    // why the spreaded API needs polishing.

    for ( int i = 0; i < QwtPlot::axisCnt; i++ )
    {
        QwtScaleWidget *scaleWidget = axisWidget( i );
        if ( scaleWidget )
            scaleWidget->setMargin( 0 );

        QwtScaleDraw *scaleDraw = axisScaleDraw( i );
        if ( scaleDraw )
            scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
    }

    plotLayout()->setAlignCanvasToScales( true );
}

void Plotter::appendPoint(qreal y)
{
    appendPoint(y, "default");
}

void Plotter::appendPoint(qreal y, QString name)
{
    qreal lastX = 0;
    QVector<QPointF> * points;
    if(d_points.contains(name)){
        points = d_points.value(name);
    }else{
        points = new QVector<QPointF>();
        d_points.insert(name, points);
    }
    if(points->size() != 0)
        lastX = points->last().rx();
    lastX += 1.0;
    addPoint(lastX, y, name);
}

void Plotter::addPoint(qreal x, qreal y)
{
    addPoint(x, y, "default");
}

void Plotter::addPoint(qreal x, qreal y, QString name)
{
    QwtPlotCurve * d_curve;
    if(!d_curves.contains(name))
    {
        // Insert curve
        d_curve = new QwtPlotCurve(name);
        QColor color = colors[d_curves.count()];
        d_curve->setPen( color, 2.0, Qt::SolidLine );
        d_curve->attach( this );
        d_curves.insert(name, d_curve);
    }else{
        d_curve = d_curves.value(name);
    }

    QVector<QPointF> * points;
    if(d_points.contains(name)){
        points = d_points.value(name);
    }else{
        points = new QVector<QPointF>();
        d_points.insert(name, points);
    }

    points->append(QPointF(x,y));
    d_curve->setSamples(*points);
}

void Plotter::applyPlot()
{
    for(QString name: d_curves.keys())
    {
        QwtPlotCurve * d_curve = d_curves.value(name);
        QVector<QPointF> * points = d_points.value(name);
        d_curve->setSamples(*points);
    }
    Plotter::replot();
}
