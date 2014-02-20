#ifndef DISCOUNTCONVERTER_H
#define DISCOUNTCONVERTER_H

#include "markdownconverter.h"

class DiscountConverter : public MarkDownConverter
{
public:
    QString convert(const QString &source);
};

#endif // DISCOUNTCONVERTER_H
