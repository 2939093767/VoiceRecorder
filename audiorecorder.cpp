#include "audiorecorder.h"
#include <QAudioFormat>
#include <QMediaDevices>
#include <QDateTime>

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject(parent)
    , m_audioSource(nullptr)
    , m_audioDevice(nullptr)
    , m_isRecording(false)
    , m_startTime(0)
    , m_duration(0)
    , m_durationTimer(new QTimer(this))
{
    m_durationTimer->setInterval(100);
    connect(m_durationTimer, &QTimer::timeout, this, &AudioRecorder::updateDuration);

    m_buffer.setBuffer(&m_audioData);
}

AudioRecorder::~AudioRecorder()
{
    if (m_isRecording) {
        stopRecording();
    }
}

bool AudioRecorder::isRecording() const
{
    return m_isRecording;
}

qint64 AudioRecorder::recordDuration() const
{
    return m_duration;
}

void AudioRecorder::startRecording()
{
    if (m_isRecording) {
        return;
    }

    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (inputDevice.isNull()) {
        emit errorOccurred(QStringLiteral("未找到可用的麦克风设备"));
        return;
    }

    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!inputDevice.isFormatSupported(format)) {
        emit errorOccurred(QStringLiteral("音频格式不支持"));
        return;
    }

    m_audioSource = new QAudioSource(inputDevice, format, this);

    m_audioData.clear();
    m_buffer.open(QIODevice::WriteOnly | QIODevice::Truncate);

    m_audioDevice = m_audioSource->start();
    if (!m_audioDevice) {
        emit errorOccurred(QStringLiteral("无法启动音频输入"));
        delete m_audioSource;
        m_audioSource = nullptr;
        return;
    }

    connect(m_audioDevice, &QIODevice::readyRead, this, &AudioRecorder::onReadyRead);

    m_isRecording = true;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_duration = 0;
    m_durationTimer->start();

    emit recordingStarted();
}

void AudioRecorder::stopRecording()
{
    if (!m_isRecording) {
        return;
    }

    m_durationTimer->stop();

    if (m_audioSource) {
        m_audioSource->stop();
        m_audioDevice = nullptr;
        delete m_audioSource;
        m_audioSource = nullptr;
    }

    m_buffer.close();

    m_isRecording = false;

    emit recordingStopped();
}

void AudioRecorder::onReadyRead()
{
    if (!m_audioDevice || !m_isRecording) {
        return;
    }

    QByteArray data = m_audioDevice->readAll();
    if (data.isEmpty()) {
        return;
    }

    m_buffer.write(data);

    calculateLevel(data);

    emit audioDataAvailable(data);
}

void AudioRecorder::updateDuration()
{
    if (m_isRecording) {
        m_duration = QDateTime::currentMSecsSinceEpoch() - m_startTime;
    }
}

void AudioRecorder::calculateLevel(const QByteArray &data)
{
    if (data.isEmpty()) {
        return;
    }

    const qint16 *samples = reinterpret_cast<const qint16 *>(data.constData());
    int sampleCount = data.size() / sizeof(qint16);

    if (sampleCount <= 0) {
        return;
    }

    qint32 sum = 0;
    for (int i = 0; i < sampleCount; ++i) {
        sum += qAbs(samples[i]);
    }

    qreal avg = static_cast<qreal>(sum) / sampleCount;
    qreal level = avg / 32768.0;
    level = qBound(0.0, level, 1.0);

    emit levelChanged(level);
}
