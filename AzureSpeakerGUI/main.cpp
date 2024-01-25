#include "AzureSpeakerGUI.h"
#include "Settings.hpp"
#include <QtWidgets/QApplication>

namespace TTS_Var {
    AzureSpeakerGUI* w = nullptr;}

int main(int argc, char *argv[]){
    extern auto InitAzure() -> bool;
    QApplication a(argc, argv);
    AzureSpeakerGUI w;
    TTS_Var::w = &w;
    w.show();
    Settings s(&w);
    if (InitAzure()) {
        s.show();}
    else {
        emit w.Ready();}
    return a.exec();
}