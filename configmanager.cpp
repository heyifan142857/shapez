#include "configmanager.h"
#include "localization.h"

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    settings = new QSettings("config.ini", QSettings::IniFormat, this);
}

ConfigManager::~ConfigManager()
{
    delete settings;
}

int ConfigManager::getGold() const
{
    return settings->value("Gold", 0).toInt();
}

void ConfigManager::addGold(int gold){
    int initialGold = getGold();
    int finalGold = initialGold + gold;
    setGold(finalGold);
}

void ConfigManager::setGold(int gold)
{
    settings->setValue("Gold", gold);
}

QString ConfigManager::getLanguage() const
{
    return Localization::normalizeLanguageCode(settings->value("Language", "zh-CN").toString());
}

void ConfigManager::setLanguage(const QString &languageCode)
{
    settings->setValue("Language", Localization::normalizeLanguageCode(languageCode));
}
