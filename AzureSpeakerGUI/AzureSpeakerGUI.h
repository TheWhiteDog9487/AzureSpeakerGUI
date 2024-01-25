#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AzureSpeakerGUI.h"

class AzureSpeakerGUI : public QMainWindow{
    Q_OBJECT

signals:
    void Ready();

public:
    AzureSpeakerGUI(QWidget *parent = nullptr);
    ~AzureSpeakerGUI();

private:
    Ui::AzureSpeakerGUIClass ui;};