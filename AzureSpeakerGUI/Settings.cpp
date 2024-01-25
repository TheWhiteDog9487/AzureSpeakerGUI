#pragma once
#include "Settings.hpp"

#include <cstdlib>
#include <string>
#include <QMessageBox>
#include "AzureSpeakerGUI.h"
using namespace std;

namespace TTS_Var {
    extern string speechKey;
    extern string speechRegion;
    extern AzureSpeakerGUI* w;}

// void Settings::closeEvent(QCloseEvent* event){ exit(0); }

Settings::Settings(QWidget* parent){
    settings.setupUi(this);

    connect(settings.Cancel, &QPushButton::clicked, [&]() { exit(0); });

    connect(settings.Save, &QPushButton::clicked, [&]() {
        using namespace TTS_Var;
        TTS_Var::speechKey = string(settings.KeyInput->text().toLocal8Bit());
        TTS_Var::speechRegion = string(settings.RegionInput->text().toUtf8());
        if (TTS_Var::speechKey.empty()){
            QMessageBox::warning(this, "警告", "API密钥为空！");
		} else if (TTS_Var::speechRegion.empty()){
			QMessageBox::warning(this, "警告", "终结点区域为空！");
		} else {
			QMessageBox::information(this, "提示", "配置已保存。");
            emit (*w).Ready();
			this->close();}});
}

Settings::~Settings(){}