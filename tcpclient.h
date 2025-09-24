#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    // 기본 연결 메서드 (서버 문서에 맞춰 8080 포트)
    void connectToServer(const QString& host = "127.0.0.1", quint16 port = 8080);
    void disconnectFromHost();
    void sendMessage(const QString& message);
    bool isConnected() const;

    // 창문 제어 명령 메서드만 (문서의 TCP 명령어 기반)
    void sendWindowOpen();   // "window_open" 명령 전송
    void sendWindowClose();  // "window_close" 명령 전송

    // 창문 상태 조회 및 각도 설정 (문서에서 언급된 기능)
    void sendWindowStatus(); // 창문 상태 조회
    void setWindowAngle(int angle); // 창문 각도 설정

signals:
    void messageReceived(const QString& message);
    void connected();
    void disconnected();
    void errorOccurred(const QString& errorString);

private slots:
    void onReadyRead();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onConnected();

private:
    QTcpSocket* socket_;
    QString host_;
    quint16 port_;
};

#endif // TCPCLIENT_H
