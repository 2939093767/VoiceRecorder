#include "speechrecognizer.h"
#include <QDebug>

namespace skill_Ability {

#ifdef Q_OS_WIN

SpeechRecognizer::SpeechRecognizer(QObject *parent)
    : QObject(parent)
    , m_recognizer(nullptr)
    , m_recoContext(nullptr)
    , m_recoGrammar(nullptr)
    , m_eventHandle(nullptr)
    , m_available(false)
    , m_listening(false)
    , m_eventTimer(new QTimer(this))
{
    m_eventTimer->setInterval(100);
    connect(m_eventTimer, &QTimer::timeout, this, &SpeechRecognizer::checkEvents);
    m_available = initialize();
}

SpeechRecognizer::~SpeechRecognizer()
{
    stopListening();
    cleanup();
}

bool SpeechRecognizer::isAvailable() const
{
    return m_available;
}

bool SpeechRecognizer::isListening() const
{
    return m_listening;
}

QString SpeechRecognizer::lastError() const
{
    return m_lastError;
}

bool SpeechRecognizer::initialize()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        m_lastError = QStringLiteral("COM初始化失败");
        return false;
    }

    hr = CoCreateInstance(CLSID_SpSharedRecognizer,
                          nullptr,
                          CLSCTX_ALL,
                          IID_ISpRecognizer,
                          reinterpret_cast<void **>(&m_recognizer));
    if (FAILED(hr)) {
        m_lastError = QStringLiteral("创建语音识别器失败，请确保系统已安装语音识别引擎");
        return false;
    }

    hr = m_recognizer->CreateRecoContext(&m_recoContext);
    if (FAILED(hr)) {
        m_lastError = QStringLiteral("创建识别上下文失败");
        return false;
    }

    m_eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_eventHandle) {
        m_lastError = QStringLiteral("创建事件句柄失败");
        return false;
    }

    hr = m_recoContext->SetNotifyWin32Event();
    if (FAILED(hr)) {
        m_lastError = QStringLiteral("设置通知事件失败");
        return false;
    }

    hr = m_recoContext->SetInterest(
        SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_HYPOTHESIS) | SPFEI(SPEI_RECO_STATE_CHANGE) | SPFEI(SPEI_FALSE_RECOGNITION),
        SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_HYPOTHESIS) | SPFEI(SPEI_RECO_STATE_CHANGE) | SPFEI(SPEI_FALSE_RECOGNITION));
    if (FAILED(hr)) {
        m_lastError = QStringLiteral("设置事件兴趣失败");
    }

    return true;
}

void SpeechRecognizer::cleanup()
{
    if (m_eventHandle) {
        CloseHandle(m_eventHandle);
        m_eventHandle = nullptr;
    }
    if (m_recoGrammar) {
        m_recoGrammar->Release();
        m_recoGrammar = nullptr;
    }
    if (m_recoContext) {
        m_recoContext->Release();
        m_recoContext = nullptr;
    }
    if (m_recognizer) {
        m_recognizer->Release();
        m_recognizer = nullptr;
    }
    CoUninitialize();
}

bool SpeechRecognizer::startListening()
{
    if (!m_available || m_listening) {
        return false;
    }

    if (!m_recoContext) {
        m_lastError = QStringLiteral("识别上下文未初始化");
        return false;
    }

    HRESULT hr = m_recoContext->CreateGrammar(0, &m_recoGrammar);
    if (FAILED(hr)) {
        m_lastError = QStringLiteral("创建语法对象失败");
        return false;
    }

    hr = m_recoGrammar->LoadDictation(L"", SPLO_STATIC);
    if (FAILED(hr)) {
        m_lastError = QStringLiteral("加载听写语法失败，请确保系统已安装中文语音包");
        m_recoGrammar->Release();
        m_recoGrammar = nullptr;
        return false;
    }

    hr = m_recoGrammar->SetDictationState(SPRS_ACTIVE);
    if (FAILED(hr)) {
        m_lastError = QStringLiteral("激活听写失败");
        return false;
    }

    m_listening = true;
    m_eventTimer->start();
    emit listeningStarted();

    return true;
}

void SpeechRecognizer::stopListening()
{
    if (!m_listening) {
        return;
    }

    m_eventTimer->stop();

    if (m_recoGrammar) {
        m_recoGrammar->SetDictationState(SPRS_INACTIVE);
        m_recoGrammar->Release();
        m_recoGrammar = nullptr;
    }

    m_listening = false;
    emit listeningStopped();
}

void SpeechRecognizer::checkEvents()
{
    processEvents();
}

void SpeechRecognizer::processEvents()
{
    if (!m_recoContext) {
        return;
    }

    ISpEventSource *pEventSource = nullptr;
    HRESULT hr = m_recoContext->QueryInterface(IID_ISpEventSource,
        reinterpret_cast<void **>(&pEventSource));
    if (FAILED(hr) || !pEventSource) {
        return;
    }

    SPEVENT event;
    ULONG fetched = 0;
    while (SUCCEEDED(pEventSource->GetEvents(1, &event, &fetched)) && fetched > 0) {
        switch (event.eEventId) {
        case SPEI_RECOGNITION: {
            ISpRecoResult *recoResult = reinterpret_cast<ISpRecoResult *>(event.lParam);
            if (recoResult) {
                WCHAR *text = nullptr;
                HRESULT hrText = recoResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &text, nullptr);
                if (SUCCEEDED(hrText) && text) {
                    QString result = QString::fromWCharArray(text);
                    CoTaskMemFree(text);
                    emit recognitionResult(result, true);
                }
                recoResult->Release();
            }
            break;
        }
        case SPEI_HYPOTHESIS: {
            ISpRecoResult *recoResult = reinterpret_cast<ISpRecoResult *>(event.lParam);
            if (recoResult) {
                WCHAR *text = nullptr;
                HRESULT hrText = recoResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &text, nullptr);
                if (SUCCEEDED(hrText) && text) {
                    QString result = QString::fromWCharArray(text);
                    CoTaskMemFree(text);
                    emit hypothesisResult(result);
                }
                recoResult->Release();
            }
            break;
        }
        case SPEI_FALSE_RECOGNITION:
            break;
        case SPEI_RECO_STATE_CHANGE:
            break;
        default:
            break;
        }
    }

    pEventSource->Release();
}

#else

SpeechRecognizer::SpeechRecognizer(QObject *parent)
    : QObject(parent)
    , m_available(false)
    , m_listening(false)
    , m_eventTimer(new QTimer(this))
{
    m_lastError = QStringLiteral("当前平台不支持语音识别");
}

SpeechRecognizer::~SpeechRecognizer()
{
}

bool SpeechRecognizer::isAvailable() const
{
    return false;
}

bool SpeechRecognizer::isListening() const
{
    return false;
}

QString SpeechRecognizer::lastError() const
{
    return m_lastError;
}

bool SpeechRecognizer::initialize()
{
    return false;
}

void SpeechRecognizer::cleanup()
{
}

bool SpeechRecognizer::startListening()
{
    emit errorOccurred(m_lastError);
    return false;
}

void SpeechRecognizer::stopListening()
{
}

void SpeechRecognizer::checkEvents()
{
}

void SpeechRecognizer::processEvents()
{
}

#endif

} // namespace skill_Ability
