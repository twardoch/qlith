// this_file: qlith-pro/include/qlith/floatrect.h
#pragma once

#include "qlith/floatpoint.h"
#include "qlith/floatsize.h"
#include "qlith/common.h"
#include "qlith/intrect.h"
//#include "vector.h"

QT_BEGIN_NAMESPACE
class QRectF;
QT_END_NAMESPACE

//class IntRect;

class FloatRect {
public:
    FloatRect() { }
    FloatRect(const FloatPoint& location, const FloatSize& size)
        : m_location(location), m_size(size) { }
    FloatRect(float x, float y, float width, float height)
        : m_location(FloatPoint(x, y)), m_size(FloatSize(width, height)) { }
    FloatRect(const IntRect&);

    static FloatRect narrowPrecision(double x, double y, double width, double height);

    FloatPoint location() const { return m_location; }
    FloatSize size() const { return m_size; }

    void setLocation(const FloatPoint& location) { m_location = location; }
    void setSize(const FloatSize& size) { m_size = size; }

    float x() const { return m_location.x(); }
    float y() const { return m_location.y(); }
    float width() const { return m_size.width(); }
    float height() const { return m_size.height(); }

    void setX(float x) { m_location.setX(x); }
    void setY(float y) { m_location.setY(y); }
    void setWidth(float width) { m_size.setWidth(width); }
    void setHeight(float height) { m_size.setHeight(height); }

    bool isEmpty() const { return m_size.isEmpty(); }

    float left() const { return x(); }
    float right() const { return x() + width(); }
    float top() const { return y(); }
    float bottom() const { return y() + height(); }

    FloatPoint center() const { return FloatPoint(x() + width() / 2, y() + height() / 2); }

    void move(const FloatSize& delta) { m_location += delta; }
    void move(float dx, float dy) { m_location.move(dx, dy); }

    bool intersects(const FloatRect&) const;
    bool contains(const FloatRect&) const;

    void intersect(const FloatRect&);
    void unite(const FloatRect&);

    // Note, this doesn't match what IntRect::contains(IntPoint&) does; the int version
    // is really checking for containment of 1x1 rect, but that doesn't make sense with floats.
    bool contains(float px, float py) const
        { return px >= x() && px <= right() && py >= y() && py <= bottom(); }
    bool contains(const FloatPoint& point) const { return contains(point.x(), point.y()); }


    void inflateX(float dx) {
        m_location.setX(m_location.x() - dx);
        m_size.setWidth(m_size.width() + dx + dx);
    }
    void inflateY(float dy) {
        m_location.setY(m_location.y() - dy);
        m_size.setHeight(m_size.height() + dy + dy);
    }
    void inflate(float d) { inflateX(d); inflateY(d); }
    void scale(float s) { scale(s, s); }
    void scale(float sx, float sy);

    // Re-initializes this rectangle to fit the sets of passed points.
    void fitToPoints(const FloatPoint& p0, const FloatPoint& p1);
    void fitToPoints(const FloatPoint& p0, const FloatPoint& p1, const FloatPoint& p2);
    void fitToPoints(const FloatPoint& p0, const FloatPoint& p1, const FloatPoint& p2, const FloatPoint& p3);



    FloatRect(const QRectF&);
    operator QRectF() const;
    FloatRect normalized() const;

private:
    FloatPoint m_location;
    FloatSize m_size;

    void setLocationAndSizeFromEdges(float left, float top, float right, float bottom)
    {
        m_location.set(left, top);
        m_size.setWidth(right - left);
        m_size.setHeight(bottom - top);
    }
};

inline FloatRect intersection(const FloatRect& a, const FloatRect& b)
{
    FloatRect c = a;
    c.intersect(b);
    return c;
}

inline FloatRect unionRect(const FloatRect& a, const FloatRect& b)
{
    FloatRect c = a;
    c.unite(b);
    return c;
}


inline bool operator==(const FloatRect& a, const FloatRect& b)
{
    return a.location() == b.location() && a.size() == b.size();
}

inline bool operator!=(const FloatRect& a, const FloatRect& b)
{
    return a.location() != b.location() || a.size() != b.size();
}

IntRect enclosingIntRect(const FloatRect&);

// Map rect r from srcRect to an equivalent rect in destRect.
FloatRect mapRect(const FloatRect& r, const FloatRect& srcRect, const FloatRect& destRect);


