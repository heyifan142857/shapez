#include "GlobalUpgradeDialog.h"
#include <QMessageBox>

GlobalUpgradeDialog::GlobalUpgradeDialog(ConfigManager &config, QWidget *parent)
    : QDialog(parent), config(config)  // 初始化 config
{
    setWindowTitle("购买升级");

    QVBoxLayout *layout = new QVBoxLayout(this);

    goldLabel = new QLabel(this);
    int gold = this->config.getGold();
    goldLabel->setText("当前金币: " + QString::number(gold));
    layout->addWidget(goldLabel);

    // 升级矿物
    QPushButton *mineButton = new QPushButton("升级矿物(100)", this);
    connect(mineButton, &QPushButton::clicked, this, [&config,this]() {
        if (config.getUpgradeStatus("mine")) {
            QMessageBox::information(this, "提示", "矿物已升级，无需重复购买！");
        } else {
            int cost = 100;
            if (config.getGold() >= cost) {
                config.setGold(config.getGold() - cost);
                config.setUpgradeStatus("mine", true);
                selectedOption = 1;
                QMessageBox::information(this, "成功", "矿物升级成功！");
                close();
            } else {
                QMessageBox::warning(this, "失败", "金币不足，无法升级！");
            }
        }
    });
    layout->addWidget(mineButton);

    // 升级切割机
    QPushButton *cutButton = new QPushButton("升级切割机(150)", this);
    connect(cutButton, &QPushButton::clicked, this, [&config,this]() {
        if (config.getUpgradeStatus("cut")) {
            QMessageBox::information(this, "提示", "切割机已升级，无需重复购买！");
        } else {
            int cost = 150;
            if (config.getGold() >= cost) {
                config.setGold(config.getGold() - cost);
                config.setUpgradeStatus("cut", true);
                selectedOption = 2;
                QMessageBox::information(this, "成功", "切割机升级成功！");
                close();
            } else {
                QMessageBox::warning(this, "失败", "金币不足，无法升级！");
            }
        }
    });
    layout->addWidget(cutButton);

    // 解锁矿物
    QPushButton *moremineButton = new QPushButton("解锁矿物(200)", this);
    connect(moremineButton, &QPushButton::clicked, this, [&config,this]() {
        if (config.getUpgradeStatus("moremine")) {
            QMessageBox::information(this, "提示", "矿物已解锁，无需重复购买！");
        } else {
            int cost = 200;
            if (config.getGold() >= cost) {
                config.setGold(config.getGold() - cost);
                config.setUpgradeStatus("moremine", true);
                selectedOption = 3;
                QMessageBox::information(this, "成功", "矿物解锁成功！");
                close();
            } else {
                QMessageBox::warning(this, "失败", "金币不足，无法解锁！");
            }
        }
    });
    layout->addWidget(moremineButton);

    setLayout(layout);
}

int GlobalUpgradeDialog::getSelectedOption() const
{
    return selectedOption;
}
