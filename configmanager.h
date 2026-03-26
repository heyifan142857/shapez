#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>

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

    QString getLanguage() const;

    void setLanguage(const QString &languageCode);

private:
    QSettings *settings;
};

#endif // CONFIGMANAGER_H
