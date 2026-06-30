#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QObject>
#include <QAudioSource>
#include <QBuffer>
#include <QByteArray>
#include <QTimer>

namespace skill_Ability {

/**
 * @brief 音频录制器
 * @note 业务能力层，负责麦克风音频采集、音量计算
 *       纯业务逻辑，无UI依赖
 */
class AudioRecorder : public QObject
{
    Q_OBJECT
public:
    explicit AudioRecorder(QObject *parent = nullptr);
    ~AudioRecorder() override;

    /**
     * @brief 是否正在录制
     * @return true表示正在录制
     */
    bool isRecording() const;

    /**
     * @brief 获取录制时长
     * @return 录制时长（毫秒）
     */
    qint64 recordDuration() const;

public slots:
    /**
     * @brief 开始录制
     */
    void startRecording();

    /**
     * @brief 停止录制
     */
    void stopRecording();

signals:
    /**
     * @brief 录制已开始
     */
    void recordingStarted();

    /**
     * @brief 录制已停止
     */
    void recordingStopped();

    /**
     * @brief 音频数据可用
     * @param data 音频数据
     */
    void audioDataAvailable(const QByteArray &data);

    /**
     * @brief 音量变化
     * @param level 音量等级（0.0~1.0）
     */
    void levelChanged(qreal level);

    /**
     * @brief 发生错误
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();
    void updateDuration();

private:
    QAudioSource *m_audioSource;
    QIODevice *m_audioDevice;
    QBuffer m_buffer;
    QByteArray m_audioData;
    bool m_isRecording;
    qint64 m_startTime;
    qint64 m_duration;
    QTimer *m_durationTimer;

    void calculateLevel(const QByteArray &data);
};

} // namespace skill_Ability

#endif // AUDIORECORDER_H
