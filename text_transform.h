#ifndef TEXT_TRANSFORM_H
#define TEXT_TRANSFORM_H

#include <QString>

namespace text_transform {

inline QString uppercase(QString text) { return text.toUpper(); }

inline QString lowercase(QString text) { return text.toLower(); }

inline QString capitalize(QString text)
{
    bool insideWord = false;

    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i];

        if (ch.isLetter()) {
            text[i] = insideWord ? ch.toLower() : ch.toUpper();
            insideWord = true;
        } else {
            insideWord = false;
        }
    }

    return text;
}

inline QString sentence_case(QString text)
{
    bool shouldCapitalize = true;

    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i];

        if (ch == '.' || ch == '!' || ch == '?') {
            shouldCapitalize = true;
            continue;
        }

        if (!ch.isLetter())
            continue;

        if (shouldCapitalize) {
            text[i] = ch.toUpper();
            shouldCapitalize = false;
        } else {
            text[i] = ch.toLower();
        }
    }

    return text;
}

inline QString swap_case(QString text)
{
    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i];

        if (ch.isUpper())
            text[i] = ch.toLower();
        else if (ch.isLower())
            text[i] = ch.toUpper();
    }

    return text;
}

}

#endif
