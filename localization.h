#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <QString>

namespace Localization {

inline QString normalizeLanguageCode(const QString &languageCode)
{
    return languageCode == "en" ? "en" : "zh-CN";
}

inline bool isEnglish(const QString &languageCode)
{
    return normalizeLanguageCode(languageCode) == "en";
}

inline QString text(const QString &languageCode, const QString &zhText, const QString &enText)
{
    return isEnglish(languageCode) ? enText : zhText;
}

} // namespace Localization

#endif // LOCALIZATION_H
