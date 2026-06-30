#include "cloudllm.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>

CloudLLM::CloudLLM(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_apiKey()
    , m_baseUrl(QStringLiteral("https://ark.cn-beijing.volces.com/api/v3"))
    , m_model(QStringLiteral("doubao-lite-32k"))
    , m_fullContent()
    , m_buffer()
    , m_isStreaming(false)
{
}

CloudLLM::~CloudLLM()
{
    abort();
}

void CloudLLM::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

void CloudLLM::setBaseUrl(const QString &baseUrl)
{
    m_baseUrl = baseUrl;
}

void CloudLLM::setModel(const QString &model)
{
    m_model = model;
}

void CloudLLM::chat(const QString &prompt)
{
    QList<LLMMessage> messages;
    LLMMessage msg;
    msg.role = QStringLiteral("user");
    msg.content = prompt;
    messages.append(msg);
    chat(messages);
}

void CloudLLM::chat(const QList<LLMMessage> &messages)
{
    if (m_currentReply) {
        abort();
    }

    if (m_apiKey.isEmpty()) {
        emit errorOccurred(QStringLiteral("API Key 未设置"));
        return;
    }

    m_fullContent.clear();
    m_buffer.clear();
    m_isStreaming = true;

    QUrl url(m_baseUrl + QStringLiteral("/chat/completions"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(m_apiKey).toUtf8());

    QByteArray body = buildRequestBody(messages);
    m_currentReply = m_networkManager->post(request, body);

    connect(m_currentReply, &QNetworkReply::readyRead, this, &CloudLLM::onReadyRead);
    connect(m_currentReply, &QNetworkReply::finished, this, &CloudLLM::onFinished);
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
        this, &CloudLLM::onErrorOccurred);
}

void CloudLLM::abort()
{
    if (m_currentReply) {
        if (m_currentReply->isRunning()) {
            m_currentReply->abort();
        }
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    m_isStreaming = false;
}

QByteArray CloudLLM::buildRequestBody(const QList<LLMMessage> &messages) const
{
    QJsonObject root;
    root[QStringLiteral("model")] = m_model;
    root[QStringLiteral("stream")] = true;

    QJsonArray messagesArray;
    for (const LLMMessage &msg : messages) {
        QJsonObject msgObj;
        msgObj[QStringLiteral("role")] = msg.role;
        msgObj[QStringLiteral("content")] = msg.content;
        messagesArray.append(msgObj);
    }
    root[QStringLiteral("messages")] = messagesArray;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Compact);
}

void CloudLLM::onReadyRead()
{
    if (!m_currentReply) return;

    QByteArray data = m_currentReply->readAll();
    m_buffer.append(data);

    while (m_buffer.contains("\n\n")) {
        int idx = m_buffer.indexOf("\n\n");
        QByteArray chunk = m_buffer.left(idx);
        m_buffer = m_buffer.mid(idx + 2);

        QString delta = parseSseLine(chunk);
        if (!delta.isEmpty()) {
            m_fullContent += delta;
            emit streamReceived(delta);
        }
    }
}

QString CloudLLM::parseSseLine(const QByteArray &line) const
{
    QByteArray trimmed = line.trimmed();
    if (!trimmed.startsWith("data:")) {
        return QString();
    }

    QByteArray dataPart = trimmed.mid(5).trimmed();
    if (dataPart == "[DONE]") {
        return QString();
    }

    if (!handleSseData(dataPart)) {
        return QString();
    }

    QString result;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(dataPart, &error);
    if (error.error != QJsonParseError::NoError) {
        return QString();
    }

    QJsonObject obj = doc.object();
    QJsonArray choices = obj.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        return QString();
    }

    QJsonObject choice = choices.first().toObject();
    QJsonObject delta = choice.value(QStringLiteral("delta")).toObject();
    if (delta.isEmpty()) {
        return QString();
    }

    result = delta.value(QStringLiteral("content")).toString();
    return result;
}

bool CloudLLM::handleSseData(const QByteArray &data) const
{
    Q_UNUSED(data);
    return true;
}

void CloudLLM::onFinished()
{
    if (!m_currentReply) return;

    if (m_currentReply->error() == QNetworkReply::NoError) {
        if (!m_buffer.isEmpty()) {
            QByteArray trimmed = m_buffer.trimmed();
            if (!trimmed.isEmpty()) {
                QString delta = parseSseLine(trimmed);
                if (!delta.isEmpty()) {
                    m_fullContent += delta;
                    emit streamReceived(delta);
                }
            }
        }
        emit finished(m_fullContent);
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    m_isStreaming = false;
}

void CloudLLM::onErrorOccurred(QNetworkReply::NetworkError code)
{
    Q_UNUSED(code);
    if (!m_currentReply) return;
    QString errorString = m_currentReply->errorString();
    emit errorOccurred(errorString);
}
