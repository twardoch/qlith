// this_file: qlith/src/container_qpainter_p.h
#ifndef CONTAINER_QPAINTER_P_H
#define CONTAINER_QPAINTER_P_H

#include <QFont>
#include <QFontMetrics>
#include <QMap>
#include <QString>

// Font information cache
struct font_metrics_t
{
    QFont font;
    QFontMetrics metrics;
    
    font_metrics_t() : metrics(font) {}
    font_metrics_t(const QFont& f) : font(f), metrics(font) {}
};

#endif // CONTAINER_QPAINTER_P_H 