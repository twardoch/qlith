// this_file: qlith-pro/src/floatsize.cpp
#include "qlith/floatsize.h"

#include "qlith/common.h"

#include "qlith/intsize.h"
#include <cmath>

FloatSize::FloatSize(const IntSize& size) : m_width(size.width()), m_height(size.height())
{
}

float FloatSize::diagonalLength() const
{
    return sqrtf(diagonalLengthSquared());
}

FloatSize FloatSize::narrowPrecision(double width, double height)
{
    return FloatSize(narrowPrecisionToFloat(width), narrowPrecisionToFloat(height));
}
