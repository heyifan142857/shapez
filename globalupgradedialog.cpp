#include "globalupgradedialog.h"
#include <QMessageBox>
#include "localization.h"

GlobalUpgradeDialog::GlobalUpgradeDialog(ConfigManager &config, QWidget *parent)
    : QDialog(parent), config(config)  // 初始化 config
{
    const QString languageCode = config.getLanguage();
    const auto text = [&](const QString &zhText, const QString &enText) {
        return Localization::text(languageCode, zhText, enText);
    };

    setWindowTitle(text("购买升级", "Buy Upgrades"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    goldLabel = new QLabel(this);
    int gold = this->config.getGold();
    goldLabel->setText(text("当前金币: ", "Current Gold: ") + QString::number(gold));
    layout->addWidget(goldLabel);

    // 升级矿物
    QPushButton *mineButton = new QPushButton(text("升级矿物(100)", "Upgrade Mining Output (100)"), this);
    connect(mineButton, &QPushButton::clicked, this, [&config,this,text]() {
        if (config.getUpgradeStatus("mine")) {
            QMessageBox::information(this, text("提示", "Notice"), text("矿物已升级，无需重复购买！", "Mining output has already been upgraded."));
        } else {
            int cost = 100;
            if (config.getGold() >= cost) {
                config.setGold(config.getGold() - cost);
                config.setUpgradeStatus("mine", true);
                selectedOption = 1;
                QMessageBox::information(this, text("成功", "Success"), text("矿物升级成功！", "Mining output upgraded successfully."));
                close();
            } else {
                QMessageBox::warning(this, text("失败", "Failed"), text("金币不足，无法升级！", "Not enough gold for this upgrade."));
            }
        }
    });
    layout->addWidget(mineButton);

    // 升级切割机
    QPushButton *cutButton = new QPushButton(text("升级切割机(150)", "Upgrade Cutters (150)"), this);
    connect(cutButton, &QPushButton::clicked, this, [&config,this,text]() {
        if (config.getUpgradeStatus("cut")) {
            QMessageBox::information(this, text("提示", "Notice"), text("切割机已升级，无需重复购买！", "Cutters have already been upgraded."));
        } else {
            int cost = 150;
            if (config.getGold() >= cost) {
                config.setGold(config.getGold() - cost);
                config.setUpgradeStatus("cut", true);
                selectedOption = 2;
                QMessageBox::information(this, text("成功", "Success"), text("切割机升级成功！", "Cutters upgraded successfully."));
                close();
            } else {
                QMessageBox::warning(this, text("失败", "Failed"), text("金币不足，无法升级！", "Not enough gold for this upgrade."));
            }
        }
    });
    layout->addWidget(cutButton);

    setLayout(layout);
}

int GlobalUpgradeDialog::getSelectedOption() const
{
    return selectedOption;
}
