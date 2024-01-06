#include "AzureSpeakerGUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    AzureSpeakerGUI w;
    w.show();
    return a.exec();
}