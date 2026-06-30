#ifndef CLOUDLLM_H
#define CLOUDLLM_H

#include <QObject>
#include <QString>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace skill_Ability {

/**
 * @brief 大模型消息结构
 * @note 纯数据结构体，表示一条对话消息
 */
struct LLMMessage {
    QString role;       ///< 角色（user/assistant/system）
    QString content;    ///< 消息内容
};

/**
 * @brief 云端大模型调用器
 * @note 业务能力层，基于HTTP协议调用云端大模型API
 *       支持流式输出（SSE），纯业务逻辑，无UI依赖
 */
class CloudLLM : public QObject
{
    Q_OBJECT

public:
    explicit CloudLLM(QObject *parent = nullptr);
    ~CloudLLM() override;

    /**
     * @brief 设置API密钥
     * @param apiKey API密钥
     */
    void setApiKey(const QString &apiKey);

    /**
     * @brief 设置API基础URL
     * @param baseUrl 基础URL
     */
    void setBaseUrl(const QString &baseUrl);

    /**
     * @brief 设置模型名称
     * @param model 模型名称
     */
    void setModel(const QString &model);

    /**
     * @brief 发送单轮对话请求
     * @param prompt 用户提示词
     */
    void chat(const QString &prompt);

    /**
     * @brief 发送多轮对话请求
     * @param messages 消息列表
     */
    void chat(const QList<LLMMessage> &messages);

    /**
     * @brief 中止当前请求
     */
    void abort();

signals:
    /**
     * @brief 收到流式数据
     * @param delta 增量文本
     */
    void streamReceived(const QString &delta);

    /**
     * @brief 请求完成
     * @param fullContent 完整内容
     */
    void finished(const QString &fullContent);

    /**
     * @brief 发生错误
     * @param error 错误信息
     */
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

} // namespace skill_Ability

#endif // CLOUDLLM_H
