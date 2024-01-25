#pragma once

#include <QtWidgets/qwidget.h>
#include "ui_Settings.h"

class Settings : public QWidget
{
    Q_OBJECT

public:
    Settings(QWidget* parent = nullptr);
    ~Settings();

protected:
    // void closeEvent(QCloseEvent* event) override;

private:
    Ui::Form settings;
};