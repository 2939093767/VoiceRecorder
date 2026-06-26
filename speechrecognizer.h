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

class SpeechRecognizer : public QObject
{
    Q_OBJECT
public:
    explicit SpeechRecognizer(QObject *parent = nullptr);
    ~SpeechRecognizer() override;

    bool isAvailable() const;
    bool isListening() const;
    QString lastError() const;

public slots:
    bool startListening();
    void stopListening();

signals:
    void recognitionResult(const QString &text, bool isFinal);
    void hypothesisResult(const QString &text);
    void listeningStarted();
    void listeningStopped();
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

#endif
