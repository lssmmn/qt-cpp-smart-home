#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QSizePolicy>
#include <QFont>
#include <QStackedWidget>
#include <QDir>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <QKeyEvent>
#include <QScreen>
#include <QApplication>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTime>
#include <QPainter>
#include "safety.h"
#include "certified.h"
#include "search.h"
#include "tcpclient.h"

class CustomToggleSwitch;

// 센서 데이터 상태 열거형
enum class SensorStatus {
    Normal,
    Warning,
    Danger,
    Optimal
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public slots:
    // 센서 데이터 업데이트 슬롯들
    void updatePlantHumidityStatus(int value);
    void updatePetStatus(bool poopDetected);
    void updateGasStatus(int value);
    void updateFireStatus(int value);

private slots:
    void toggleWindow();
    void onCameraClicked();
    void onHomeClicked();
    void onSecurityClicked();
    void onSearchClicked();
    void toggleFireAlert();
    void toggleGasAlert();
    void updateClock();  // 시계 업데이트 슬롯 추가
    void updateDbData();  // DB 데이터 업데이트 슬롯 추가

    // TCP 클라이언트 관련 슬롯
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpMessageReceived(const QString& message);
    void onTcpErrorOccurred(const QString& errorString);

private:
    void setupUI();
    void setupStyles();
    void setupFonts();
    QTimer *dbUpdateTimer;  // DB 업데이트 타이머 추가

    // Layout setup methods
    void setupDashboardLayout(QWidget *dashboardPage);
    void createHeader(QVBoxLayout *canvasLayout);
    void createDashboardContent(QVBoxLayout *canvasLayout);
    void createCardsGrid(QHBoxLayout *mainLayout);
    void createSideButtons(QHBoxLayout *mainLayout);
    void createRightSideCards(QHBoxLayout *mainLayout); // 오른쪽 사이드 카드 생성 메서드 추가

    // Card creation methods
    QWidget* createSensorCard(const QString& title, const QString& value, const QString& iconPath);
    QWidget* createStatusCard(const QString& title);
    QWidget* createPetCard();
    QWidget* createLockCard();
    QWidget* createClockCard();  // 시계 카드 생성 메서드 추가
    QWidget* createWindowCard(); // Door Lock 카드 생성 메서드 추가
    QPushButton* createCircleButton(const QString& iconPath);
    QPixmap loadResourceImage(const QString& imagePath, const QSize& size = QSize());

    // Helper methods
    void applyShadowEffect(QWidget* widget);
    void animateCardAlert(QWidget* card, bool enable);

    // 센서 상태 업데이트 헬퍼 메서드들
    void updateCardColor(QWidget* card, SensorStatus status);
    void updateCardStatusText(QWidget* card, const QString& statusText, SensorStatus status);
    QString getStatusColor(SensorStatus status);

    // 창문 제어 메서드 추가
    void requestWindowStatus();
    void setWindowAngle(int angle);

    // UI Components
    QStackedWidget *stackedWidget;
    QWidget *centralWidget;
    QWidget *headerWidget;
    QWidget *dashboardWidget;
    QWidget *mainCanvas;
    Safety *safetyWidget;
    Certified *certifiedWidget;
    Search *searchWidget;

    // Header
    QLabel *helloLabel;

    // Cards
    QWidget *tempCard;
    QWidget *humCard;
    QWidget *fireCard;
    QWidget *gasCard;
    QWidget *petCard;
    QWidget *plantCard;
    QWidget *clockCard;      // 시계 카드 추가
    QWidget *windowCard;

    // Side controls
    QPushButton *cameraButton;
    QPushButton *homeButton;
    QPushButton *securityButton;
    QPushButton *searchButton;

    // window toggle 추가
    CustomToggleSwitch *windowToggle;
    QLabel *windowLabel;
    bool isWindowOpen;

    // Clock components 추가
    QLabel *clockLabel;
    QTimer *clockTimer;

    // Alert states
    bool fireAlert;
    bool gasAlert;

    // TCP 클라이언트 관련 멤버 (백그라운드 처리용)
    TcpClient *tcpClient;

    // 현재 센서 상태들
    SensorStatus currentPlantStatus;
    SensorStatus currentGasStatus;
    SensorStatus currentFireStatus;
    bool currentPetPoopDetected;
};

// Custom Toggle Switch Widget
class CustomToggleSwitch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int position READ position WRITE setPosition)

public:
    explicit CustomToggleSwitch(QWidget *parent = nullptr);

    bool isChecked() const { return checked; }
    void setChecked(bool checked);

    int position() const { return handlePosition; }
    void setPosition(int pos);

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    QSize sizeHint() const override;

private:
    void toggle();
    void updateAnimation();

    bool checked;
    int handlePosition;
    QPropertyAnimation *animation;

    static constexpr int Width = 80;
    static constexpr int Height = 40;
    static constexpr int HandleSize = 32;
    static constexpr int Margin = 4;
};

#endif // MAINWINDOW_H
