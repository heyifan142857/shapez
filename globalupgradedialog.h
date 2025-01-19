#ifndef GLOBALUPGRADEDIALOG_H
#define GLOBALUPGRADEDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include "configmanager.h"
#include <QLabel>

class GlobalUpgradeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalUpgradeDialog(ConfigManager &config, QWidget *parent = nullptr);
    int getSelectedOption() const;

private:
    QLabel *goldLabel;
    int selectedOption = 0;
    ConfigManager &config;  // 引用 ConfigManager
};

#endif // GLOBALUPGRADEDIALOG_H
