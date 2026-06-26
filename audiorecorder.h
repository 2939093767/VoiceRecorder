#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QObject>
#include <QAudioSource>
#include <QBuffer>
#include <QByteArray>
#include <QTimer>

class AudioRecorder : public QObject
{
    Q_OBJECT
public:
    explicit AudioRecorder(QObject *parent = nullptr);
    ~AudioRecorder() override;

    bool isRecording() const;
    qint64 recordDuration() const;

public slots:
    void startRecording();
    void stopRecording();

signals:
    void recordingStarted();
    void recordingStopped();
    void audioDataAvailable(const QByteArray &data);
    void levelChanged(qreal level);
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

#endif
