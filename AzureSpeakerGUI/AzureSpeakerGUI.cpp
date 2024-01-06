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
// #include <httplib.h>
#include <speechapi_cxx.h>

//using std::wstring,std::string,std::deque,std::fstream,std::wfstream;
using namespace std;
using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;

string GetEnvironmentVar(const char* name);

namespace TTS_Var {
    deque<wstring> FileContent;
    auto LanguageList = nlohmann::json();
    vector<string> LanguageNameVector;
    string VoiceShortName = "";
    Ui::AzureSpeakerGUIClass* UI = nullptr;
    auto speechKey = GetEnvironmentVar("Voice_API_Key");
    auto speechRegion = "eastasia";
    auto speechConfig = SpeechConfig::FromSubscription(speechKey, speechRegion);
    auto speechSynthesizer = SpeechSynthesizer::FromConfig(speechConfig);}

auto TTS(wstring Text = L"") { return TTS_Var::speechSynthesizer->SpeakTextAsync(Text).get(); }

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

    UI = &ui;
    auto LanguageListURL = "https://" + string(TTS_Var::speechRegion) + '.' + string("tts.speech.microsoft.com");
    cpr::Response res = cpr::Get(cpr::Url{LanguageListURL + "/cognitiveservices/voices/list"}, cpr::Header{{"Ocp-Apim-Subscription-Key", TTS_Var::speechKey}});
    LanguageList = nlohmann::json::parse(res.text);
    for (auto& Language : LanguageList) {
        // https://blog.csdn.net/xiangxianghehe/article/details/90637998
        LanguageNameVector.push_back(Language["LocaleName"].get<string>());
        sort(LanguageNameVector.begin(), LanguageNameVector.end());
        LanguageNameVector.erase(unique(LanguageNameVector.begin(),LanguageNameVector.end()), LanguageNameVector.end());}
    for (auto& Language : LanguageNameVector) {
		ui.VoiceLanguage->addItem(Language.c_str());}
    FillVoiceName();

    // auto (QComboBox::*cTC)(const QString&) = &QComboBox::currentTextChanged;
    //connect(ui.VoiceLanguage,cTC, [&](const QString& text) {
    connect(ui.VoiceLanguage, &QComboBox::currentTextChanged, [&]() {
    ui.VoiceName->clear();
    FillVoiceName();});

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
            ::std::thread(&TTS, Line).detach();
        }});

    connect(ui.Stop, &QPushButton::clicked, [&]() {
        ui.Start->setEnabled(true);
        ui.Stop->setEnabled(false);
        TTS_Var::speechSynthesizer->StopSpeakingAsync(); });}
    
    // connect(ui.Stop, &QPushButton::clicked, this, &AzureSpeakerGUI::close);}

AzureSpeakerGUI::~AzureSpeakerGUI() {}

string GetEnvironmentVar(const char* name) {
#if defined(_MSC_VER)
    size_t requiredSize = 0;
    (void)getenv_s(&requiredSize, nullptr, 0, name);
    if (requiredSize == 0) {
        return "";
    }
    auto buffer = std::make_unique<char[]>(requiredSize);
    (void)getenv_s(&requiredSize, buffer.get(), requiredSize, name);
    return buffer.get();
#else
    auto value = getenv(name);
    return value ? value : "";
#endif
}