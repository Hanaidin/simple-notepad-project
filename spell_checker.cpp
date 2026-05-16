#include "spell_checker.h"

#include "notepad_exception.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>

void SpellChecker::load_words(const std::string& path)
{
    std::ifstream input(path);
    if (!input)
        throw file_read_exception(path);

    words.clear();
    wordsByFirstLetter.clear();
    wordsByLength.clear();

    std::string word;
    while (std::getline(input, word)) {
        word = normalize_word(word);
        if (!word.empty()) {
            words.insert(word);
            wordsByFirstLetter[word.front()].push_back(word);
            wordsByLength[static_cast<int>(word.size())].push_back(word);
        }
    }
}

bool SpellChecker::is_loaded() const { return !words.empty(); }

bool SpellChecker::is_correct(const std::string& word) const
{
    std::string normalized = normalize_word(word);
    if (normalized.empty())
        return true;

    return words.find(normalized) != words.end();
}

std::vector<std::string> SpellChecker::suggestions(const std::string& word, int limit) const
{
    std::string normalized = normalize_word(word);
    if (normalized.empty() || limit <= 0)
        return {};

    struct Candidate {
        int distance;
        int lengthDifference;
        std::string word;
    };

    std::vector<Candidate> candidates;
    std::set<std::string> visited;

    auto consider_word = [&](const std::string& dictionaryWord) {
        if (!visited.insert(dictionaryWord).second)
            return;

        int lengthDifference
            = std::abs(static_cast<int>(dictionaryWord.size() - normalized.size()));
        if (lengthDifference > 2)
            return;

        int distance = edit_distance_limited(normalized, dictionaryWord, 2);
        if (distance <= 2)
            candidates.push_back({ distance, lengthDifference, dictionaryWord });
    };

    auto firstLetterBucket = wordsByFirstLetter.find(normalized.front());
    if (firstLetterBucket != wordsByFirstLetter.end()) {
        for (const std::string& dictionaryWord : firstLetterBucket->second)
            consider_word(dictionaryWord);
    }

    int normalizedLength = static_cast<int>(normalized.size());
    for (int length = normalizedLength - 1; length <= normalizedLength + 1; ++length) {
        auto lengthBucket = wordsByLength.find(length);
        if (lengthBucket == wordsByLength.end())
            continue;

        for (const std::string& dictionaryWord : lengthBucket->second)
            consider_word(dictionaryWord);
    }

    std::sort(
        candidates.begin(), candidates.end(), [](const Candidate& left, const Candidate& right) {
            if (left.distance != right.distance)
                return left.distance < right.distance;
            if (left.lengthDifference != right.lengthDifference)
                return left.lengthDifference < right.lengthDifference;
            return left.word < right.word;
        });

    std::vector<std::string> result;
    for (const Candidate& candidate : candidates) {
        if (static_cast<int>(result.size()) == limit)
            break;
        result.push_back(candidate.word);
    }

    return result;
}

std::string SpellChecker::normalize_word(const std::string& word)
{
    std::string normalized;

    for (unsigned char ch : word) {
        if (std::isalpha(ch))
            normalized.push_back(static_cast<char>(std::tolower(ch)));
    }

    return normalized;
}

int SpellChecker::edit_distance_limited(
    const std::string& left, const std::string& right, int limit)
{
    if (std::abs(static_cast<int>(left.size() - right.size())) > limit)
        return limit + 1;

    std::vector<int> previous(right.size() + 1);
    std::vector<int> current(right.size() + 1);

    for (size_t j = 0; j <= right.size(); ++j)
        previous[j] = static_cast<int>(j);

    for (size_t i = 1; i <= left.size(); ++i) {
        current[0] = static_cast<int>(i);
        int rowMinimum = current[0];

        for (size_t j = 1; j <= right.size(); ++j) {
            int substitutionCost = left[i - 1] == right[j - 1] ? 0 : 1;
            current[j] = std::min({
                previous[j] + 1,
                current[j - 1] + 1,
                previous[j - 1] + substitutionCost,
            });

            rowMinimum = std::min(rowMinimum, current[j]);
        }

        if (rowMinimum > limit)
            return limit + 1;

        std::swap(previous, current);
    }

    return previous[right.size()];
}
