#ifndef MARKDOWNCONVERTER_H
#define MARKDOWNCONVERTER_H

#include <QString>

class MarkDownConverter
{
public:
    MarkDownConverter() { }
    virtual ~MarkDownConverter() { }

    virtual QString convert(const QString &source) = 0;
};

#endif // MARKDOWNCONVERTER_H
