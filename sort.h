#ifndef SORT_H
#define SORT_H

#include <QString>

#include <utility>

inline bool word_frequency_less(
    const std::pair<QString, int>& left, const std::pair<QString, int>& right)
{
    if (left.second != right.second)
        return left.second > right.second;

    return left.first < right.first;
}

#endif
