#include "AzureSpeakerGUI.h"
#include "QFileDialog"
#include "QMessageBox"
#include <fstream>
#include <deque>
#include <string>
#include <locale>
#include <algorithm>
#include <thread>
#include <iostream>
#include <filesystem>
#include <vector>
#include <locale>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <toml++/toml.hpp>
#include <speechapi_cxx.h>

#include "Settings.hpp"
// #include <httplib.h>

//using std::wstring,std::string,std::deque,std::fstream,std::wfstream;
using namespace std;
using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

namespace TTS_Var {
    const auto ConfigTemplates = toml::parse(
        u8R"(
    [Azure]
    Key = ""
    Region = ""
    )");

    deque<wstring> FileContent;
    auto LanguageList = nlohmann::json();
    vector<string> LanguageNameVector;
    string VoiceShortName = "";
    Ui::AzureSpeakerGUIClass* UI = nullptr;
    string speechKey = "";
    string speechRegion = "eastasia";
    shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig> speechConfig = nullptr;
    shared_ptr<Microsoft::CognitiveServices::Speech::SpeechSynthesizer> speechSynthesizer = nullptr;}

auto TTS(wstring Text = L"") { return TTS_Var::speechSynthesizer->SpeakTextAsync(Text).get(); }

auto InitAzure() -> bool {
    using namespace TTS_Var;
    if (speechConfig != nullptr && speechSynthesizer != nullptr) { return false; }
    if (!filesystem::exists(u8"配置文件.toml")) {
        fstream fs(L"配置文件.toml", ios::out);
        fs.imbue(::std::locale("zh_CN.UTF-8"));
        fs << ConfigTemplates;
        fs.close();}
    const auto ConfigFile = toml::parse_file(u8"配置文件.toml");
    speechKey = ConfigFile[L"Azure"][L"Key"].value_or("");
    speechRegion = ConfigFile[L"Azure"][L"Region"].value_or("");
    if (ConfigFile[L"Azure"][L"Key"].value<std::string_view>() == "" || ConfigFile[L"Azure"][L"Region"].value<std::string_view>() == "") {
        return true;}
    return false;}

auto FillVoiceName() {
    using namespace TTS_Var;
    for (auto& Language : LanguageList) {
        /*
        {
        "DisplayName": "Adri",
        "Gender": "Female",
        "LocalName": "Adri",
        "Locale": "af-ZA",
        "LocaleName": "Afrikaans (South Africa)",
        "Name": "Microsoft Server Speech Text to Speech Voice (af-ZA, AdriNeural)",
        "SampleRateHertz": "48000",
        "ShortName": "af-ZA-AdriNeural",
        "Status": "GA",
        "VoiceType": "Neural",
        "WordsPerMinute": "147"
}       }
        */
        string FullName = "";
        if (Language["Gender"].get<string>() == "Female") { FullName = string(Language["LocalName"].get<string>()) + "(女)"; }
        else if(Language["Gender"].get<string>() == "Male") { FullName = string(Language["LocalName"].get<string>()) + "(男)"; }
        else if (Language["Gender"].get<string>() == "Neutral") { FullName = string(Language["LocalName"].get<string>()) + "(中性)"; }
        else{throw ::std::exception("什么玩意"); }
        auto LanguageLocaleName = QString(string(Language["LocaleName"].get<string>()).c_str());
        auto LanguageLocalName = QString(FullName.c_str());
        if (LanguageLocaleName == UI->VoiceLanguage->currentText()) {
            UI->VoiceName->addItem(LanguageLocalName);
            VoiceShortName = Language["ShortName"].get<string>();}}};

AzureSpeakerGUI::AzureSpeakerGUI(QWidget* parent) : QMainWindow(parent) {
    using namespace TTS_Var;
    ui.setupUi(this);

    connect(ui.SelectButton, &QPushButton::clicked, [&]() {
        auto FilePath = QFileDialog().getOpenFileName();
        ui.FilePath->setText(FilePath); });

    // auto (QComboBox::*cTC)(const QString&) = &QComboBox::currentTextChanged;
    //connect(ui.VoiceLanguage,cTC, [&](const QString& text) {
    connect(ui.VoiceLanguage, &QComboBox::currentTextChanged, [&]() {
        ui.VoiceName->clear();
        FillVoiceName(); });

    connect(ui.FilePath, &QLineEdit::textChanged, [&]() {
        ui.Start->setEnabled(true);
        FileContent.clear();
        wfstream fs(string(ui.FilePath->text().toLocal8Bit()));
        fs.imbue(::std::locale("zh_CN.UTF-8"));     // 匿名变量
        wstring Line = L"";
        if (!fs.is_open()) {
            QMessageBox::critical(this, "错误", "文件打开失败");
            // throw
            return;}
        else {
            wstring Line = L"";
            while (getline(fs, Line)) {
                FileContent.push_back(Line);}}
        fs.close(); });

    connect(ui.Start, &QPushButton::clicked, [&]() {
        using namespace TTS_Var;
        ui.Start->setEnabled(false);
        ui.Stop->setEnabled(true);
        if (FileContent.empty()) {
            QMessageBox::critical(this, "错误", "文件为空");
            return;}
        speechConfig->SetSpeechSynthesisVoiceName(VoiceShortName);
        speechSynthesizer = SpeechSynthesizer::FromConfig(speechConfig);
        for (auto& Line : FileContent) {
            if (Line == L"") continue;
            ::std::thread(&TTS, Line).detach();}});

    connect(ui.Stop, &QPushButton::clicked, [&]() {
        ui.Start->setEnabled(true);
        ui.Stop->setEnabled(false);
        TTS_Var::speechSynthesizer->StopSpeakingAsync(); });

    connect(this, &AzureSpeakerGUI::Ready, [&]() {
        using namespace TTS_Var;
        assert(speechKey != "");
        assert(speechRegion != "");
        speechConfig = SpeechConfig::FromSubscription(speechKey, speechRegion);
        speechSynthesizer = SpeechSynthesizer::FromConfig(speechConfig);
        UI = &ui;
        ui.VoiceLanguage->setEnabled(true);
        ui.VoiceName->setEnabled(true);
        std::thread([&]() {
            auto LanguageListURL = "https://" + string(TTS_Var::speechRegion) + '.' + string("tts.speech.microsoft.com");
            cpr::Response response = cpr::Get(cpr::Url{ LanguageListURL + "/cognitiveservices/voices/list" }, cpr::Header{ {"Ocp-Apim-Subscription-Key", TTS_Var::speechKey} });
            LanguageList = nlohmann::json::parse(response.text);
            for (auto& Language : LanguageList) {
                // https://blog.csdn.net/xiangxianghehe/article/details/90637998
                LanguageNameVector.push_back(Language["LocaleName"].get<string>());}
            sort(LanguageNameVector.begin(), LanguageNameVector.end());
            LanguageNameVector.erase(unique(LanguageNameVector.begin(), LanguageNameVector.end()), LanguageNameVector.end());
            for (auto& Language : LanguageNameVector) {
                ui.VoiceLanguage->addItem(Language.c_str());}
            fstream fs(L"配置文件.toml", ios::out | ios::trunc);
            fs.imbue(::std::locale("zh_CN.UTF-8"));
            auto ConfigFile = ConfigTemplates;
            ConfigFile[L"Azure"][L"Key"].as_string()->get() = speechKey;
            ConfigFile[L"Azure"][L"Region"].as_string()->get() = speechRegion;
            fs << ConfigFile;
            fs.close();
            FillVoiceName();
            }).detach();});}
    // connect(ui.Stop, &QPushButton::clicked, this, &AzureSpeakerGUI::close);}

AzureSpeakerGUI::~AzureSpeakerGUI() {}