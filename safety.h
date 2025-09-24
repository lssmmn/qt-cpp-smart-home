#ifndef SAFETY_H
#define SAFETY_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFont>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QGraphicsDropShadowEffect>

// 웹캠
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QImage>
#include <QPixmap>

#include <QMediaDevices>
#include <QCameraDevice>

class Safety : public QWidget
{
    Q_OBJECT

public:
    explicit Safety(QWidget *parent = nullptr);
    ~Safety();

signals:
    void backToMain();  // 메인 화면으로 돌아가는 시그널
    void goToCertified();  // Certified 화면으로 가는 시그널
    void goToSearch();

private slots:
    void OnFireAlarmClicked();
    void onStopAlarmClicked();
    void onWatchHomeClicked();
    void onCallEmergencyClicked();
    void onEndCallClicked();
    void onUserAccountClicked();
    void onHomeClicked();
    void onLockClicked();
    void onSearchClicked();

private:
    void setupUI();
    void setupStyles();
    void setupFonts();
    void startEmergencyCall();
    void startFireAlarm();
    void stopFireAlarm();
    void applyShadowEffect(QWidget* widget);
    QPushButton* createCircleButton(const QString& icon);
    QPixmap loadResourceImage(const QString& imagePath, const QSize& size = QSize());

    // Layout creation methods
    void createHeader(QVBoxLayout *canvasLayout);
    void createSafetyContent(QVBoxLayout *canvasLayout);
    void createMainContentArea(QHBoxLayout *mainLayout);
    void createControlButtons(QHBoxLayout *mainLayout);
    QWidget* createAdditionalButtons();

    // UI Components
    QWidget *headerWidget;
    QLabel *titleLabel;
    QLabel *statusLabel;
    QWidget *cameraArea;
    QLabel *cameraLabel;
    QWidget *mainCanvas;
    QWidget *statusWidget;
    bool isEmergencyCallActive;
    QLabel *fireAlarmBanner;        // 상단 배너
    QLabel *fireAlarmStatusLabel;   // 하단 상태 라벨
    bool isFireAlarmActive;

    // Control buttons
    QPushButton *firealarmButton;
    QPushButton *stopAlarmButton;
    QPushButton *watchHomeButton;
    QPushButton *callEmergencyButton;
    QPushButton *endCallButton;
    QPushButton *userAccountButton;
    QPushButton *homeButton;
    QPushButton *searchButton;
    QPushButton *lockButton;
};

#endif // SAFETY_H
