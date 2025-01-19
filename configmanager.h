#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>

class ConfigManager: public QObject
{
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();

    int getGold() const;

    void addGold(int gold);

    void setGold(int gold);

    bool getUpgradeStatus(const QString &upgradeName) const;

    void setUpgradeStatus(const QString &upgradeName, bool status);

private:
    QSettings *settings;
};

#endif // CONFIGMANAGER_H
