#include "tcpclient.h"
#include <QDebug>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent)
    , socket_(new QTcpSocket(this))
    , host_("127.0.0.1")
    , port_(8080)  // 서버 문서에 명시된 포트
{
    // 시그널 연결
    connect(socket_, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(socket_, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(socket_, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &TcpClient::onErrorOccurred);
    connect(socket_, &QTcpSocket::connected, this, &TcpClient::onConnected);
}

TcpClient::~TcpClient()
{
    if (socket_->state() == QTcpSocket::ConnectedState) {
        socket_->disconnectFromHost();
        if (socket_->state() != QTcpSocket::UnconnectedState) {
            socket_->waitForDisconnected(3000);
        }
    }
}

void TcpClient::connectToServer(const QString& host, quint16 port)
{
    host_ = host;
    port_ = port;

    if (socket_->state() == QTcpSocket::ConnectedState) {
        qWarning() << "Already connected to server";
        return;
    }

    if (socket_->state() == QTcpSocket::ConnectingState) {
        qWarning() << "Connection attempt in progress";
        return;
    }

    qInfo() << "Connecting to" << host_ << ":" << port_;
    socket_->connectToHost(host_, port_);
}

void TcpClient::disconnectFromHost()
{
    if (socket_->state() == QTcpSocket::ConnectedState) {
        qInfo() << "Disconnecting from server";
        socket_->disconnectFromHost();
    } else {
        qWarning() << "Not connected to server";
    }
}

void TcpClient::sendMessage(const QString& message)
{
    if (socket_->state() != QTcpSocket::ConnectedState) {
        qWarning() << "Not connected to server";
        emit errorOccurred("Not connected to server");
        return;
    }

    QByteArray data = message.toUtf8();
    // 서버 문서에 따라 메시지 끝에 개행 추가하지 않음
    qint64 bytesWritten = socket_->write(data);
    socket_->flush(); // 즉시 전송 보장

    if (bytesWritten == -1) {
        qWarning() << "Failed to send message:" << socket_->errorString();
        emit errorOccurred("Failed to send message: " + socket_->errorString());
    } else {
        qInfo() << "Sent message:" << message;
    }
}

// 창문 제어 명령 메서드들 (서버 문서의 TCP 명령어 사용)
void TcpClient::sendWindowOpen()
{
    sendMessage("window_open");
}

void TcpClient::sendWindowClose()
{
    sendMessage("window_close");
}

void TcpClient::sendWindowStatus()
{
    sendMessage("window_status");
}

void TcpClient::setWindowAngle(int angle)
{
    QString command = QString("set_open_angle=%1").arg(angle);
    sendMessage(command);
}

bool TcpClient::isConnected() const
{
    return socket_->state() == QTcpSocket::ConnectedState;
}

void TcpClient::onReadyRead()
{
    // 서버 응답 처리 - 라인 단위와 일반 데이터 모두 처리
    while (socket_->canReadLine()) {
        QByteArray data = socket_->readLine();
        QString message = QString::fromUtf8(data).trimmed();

        if (!message.isEmpty()) {
            qInfo() << "Received message:" << message;
            emit messageReceived(message);
        }
    }

    // 라인 단위로 읽을 수 없는 데이터도 처리
    if (socket_->bytesAvailable() > 0) {
        QByteArray data = socket_->readAll();
        QString message = QString::fromUtf8(data).trimmed();

        if (!message.isEmpty()) {
            qInfo() << "Received message:" << message;
            emit messageReceived(message);
        }
    }
}

void TcpClient::onDisconnected()
{
    qInfo() << "Disconnected from server";
    emit disconnected();
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError error)
{
    QString errorString = socket_->errorString();
    qWarning() << "Socket error:" << error << "-" << errorString;
    emit errorOccurred(errorString);
}

void TcpClient::onConnected()
{
    qInfo() << "Connected to server" << host_ << ":" << port_;
    emit connected();
}
