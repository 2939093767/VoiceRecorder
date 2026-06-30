#ifndef SPEECHRECOGNIZER_H
#define SPEECHRECOGNIZER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#include <sapi.h>
#endif

namespace skill_Ability {

/**
 * @brief 语音识别器
 * @note 业务能力层，基于Windows SAPI的语音识别
 *       纯业务逻辑，无UI依赖
 */
class SpeechRecognizer : public QObject
{
    Q_OBJECT
public:
    explicit SpeechRecognizer(QObject *parent = nullptr);
    ~SpeechRecognizer() override;

    /**
     * @brief 语音识别是否可用
     * @return true表示可用
     */
    bool isAvailable() const;

    /**
     * @brief 是否正在监听
     * @return true表示正在监听
     */
    bool isListening() const;

    /**
     * @brief 获取最后错误信息
     * @return 错误信息
     */
    QString lastError() const;

public slots:
    /**
     * @brief 开始监听
     * @return 是否启动成功
     */
    bool startListening();

    /**
     * @brief 停止监听
     */
    void stopListening();

signals:
    /**
     * @brief 识别结果
     * @param text 识别文本
     * @param isFinal 是否为最终结果
     */
    void recognitionResult(const QString &text, bool isFinal);

    /**
     * @brief 临时识别结果（假设中）
     * @param text 临时文本
     */
    void hypothesisResult(const QString &text);

    /**
     * @brief 已开始监听
     */
    void listeningStarted();

    /**
     * @brief 已停止监听
     */
    void listeningStopped();

    /**
     * @brief 发生错误
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

private slots:
    void checkEvents();

private:
#ifdef Q_OS_WIN
    ISpRecognizer *m_recognizer;
    ISpRecoContext *m_recoContext;
    ISpRecoGrammar *m_recoGrammar;
    HANDLE m_eventHandle;
#endif
    bool m_available;
    bool m_listening;
    QString m_lastError;
    QTimer *m_eventTimer;

    bool initialize();
    void cleanup();
    void processEvents();
};

} // namespace skill_Ability

#endif // SPEECHRECOGNIZER_H
