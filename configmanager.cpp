#include "configmanager.h"

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

bool ConfigManager::getUpgradeStatus(const QString &upgradeName) const
{
    return settings->value("Upgrades/" + upgradeName, false).toBool();
}

void ConfigManager::setUpgradeStatus(const QString &upgradeName, bool status)
{
    settings->setValue("Upgrades/" + upgradeName, status);
}
