#ifndef CLOUDLLM_H
#define CLOUDLLM_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct LLMMessage {
    QString role;
    QString content;
};

class CloudLLM : public QObject
{
    Q_OBJECT

public:
    explicit CloudLLM(QObject *parent = nullptr);
    ~CloudLLM();

    void setApiKey(const QString &apiKey);
    void setBaseUrl(const QString &baseUrl);
    void setModel(const QString &model);

    void chat(const QString &prompt);
    void chat(const QList<LLMMessage> &messages);

    void abort();

signals:
    void streamReceived(const QString &delta);
    void finished(const QString &fullContent);
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();
    void onFinished();
    void onErrorOccurred(QNetworkReply::NetworkError code);

private:
    QByteArray buildRequestBody(const QList<LLMMessage> &messages) const;
    QString parseSseLine(const QByteArray &line) const;
    bool handleSseData(const QByteArray &data) const;

private:
    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_currentReply = nullptr;

    QString m_apiKey;
    QString m_baseUrl;
    QString m_model;
    QString m_fullContent;
    QByteArray m_buffer;
    bool m_isStreaming = false;
};

#endif // CLOUDLLM_H
