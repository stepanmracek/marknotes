#include "discountconverter.h"

extern "C" {
#include <mkdio.h>
}
#include <cstdio>

QString DiscountConverter::convert(const QString &source)
{
    mkd_flag_t flags = 0;
    QString sourceText = source;
    if (!sourceText.endsWith('\n')) sourceText.append('\n');
    QByteArray utf8Data = sourceText.toUtf8();
    MMIOT *doc = mkd_string(utf8Data, utf8Data.length(), flags);
    mkd_compile(doc, flags);
    char *out;
    mkd_document(doc, &out);
    QString html = QString::fromUtf8(out);
    mkd_cleanup(doc);

    return html;
}
