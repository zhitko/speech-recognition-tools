#ifndef PLOTTER_H
#define PLOTTER_H

#include <qwt_plot.h>

#include <QMap>

class QwtPlotGrid;
class QwtPlotCurve;

class Plotter : public QwtPlot
{
    Q_OBJECT

public:
    Plotter( QWidget* = NULL );

public slots:
    void appendPoint(qreal y);
    void appendPoint(qreal y, QString name);
    void addPoint(qreal x, qreal y);
    void addPoint(qreal x, qreal y, QString name);
    void applyPlot();

private:
    QMap<QString,QVector<QPointF>* > d_points;
    QwtPlotGrid *d_grid;
    QMap<QString,QwtPlotCurve *> d_curves;

    void alignScales();
};

#endif // PLOTTER_H
