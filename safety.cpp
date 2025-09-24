#include "safety.h"

//=============================================================================
// COLOR CONSTANTS - Match Main Dashboard
//=============================================================================
const QColor BackgroundGray(0xEA, 0xE6, 0xE6);      // #EAE6E6 - Main background
const QColor DarkNavy(0x2F, 0x3C, 0x56);            // #2F3C56 - Navy blue
const QColor CardWhite(0xFF, 0xFF, 0xFF);           // #FFFFFF - Card background
const QColor StatusGray(0xF5, 0xF5, 0xF5);          // #F5F5F5 - Status bar background
const QColor CameraAreaGray(0xE8, 0xE8, 0xE8);      // #E8E8E8 - Camera area background

namespace {
    QCamera*              s_camera   = nullptr;
    QMediaCaptureSession  s_session;
    QVideoSink*           s_sink     = nullptr;
}

Safety::Safety(QWidget *parent) : QWidget(parent), isEmergencyCallActive(false), isFireAlarmActive(false)
{
    setupFonts();
    setupUI();
    setupStyles();
}

Safety::~Safety()
{
}

void Safety::setupFonts()
{
    QFont appFont("sans-serif", 10);
    QApplication::setFont(appFont);
    setFont(appFont);
}

void Safety::setupUI()
{
    // Set fullscreen size to match MainWindow
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    setFixedSize(screenGeometry.size());

    // Main layout with background color
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 80);
    mainLayout->setSpacing(30);

    // Create main canvas (white card container - same as MainWindow)
    mainCanvas = new QWidget();
    mainCanvas->setObjectName("mainCanvas");
    applyShadowEffect(mainCanvas);

    QVBoxLayout *canvasLayout = new QVBoxLayout(mainCanvas);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->setSpacing(0);

    // Create header (same style as MainWindow)
    createHeader(canvasLayout);

    // Create dashboard content
    createSafetyContent(canvasLayout);

    mainLayout->addWidget(mainCanvas);
}

void Safety::createHeader(QVBoxLayout *canvasLayout)
{
    headerWidget = new QWidget();
    headerWidget->setObjectName("headerWidget");
    headerWidget->setFixedHeight(120);

    titleLabel = new QLabel("Safety/Security");
    titleLabel->setObjectName("titleLabel");  // titleLabel로 설정됨 - 맞음
    titleLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addWidget(titleLabel);

    canvasLayout->addWidget(headerWidget);
}

void Safety::createSafetyContent(QVBoxLayout *canvasLayout)
{
    // Safety content widget
    QWidget *contentWidget = new QWidget();
    contentWidget->setObjectName("safetyContent");

    // 기존의 메인 컨텐츠만 포함 (화재경보 배너 제거)
    QHBoxLayout *mainContentLayout = new QHBoxLayout(contentWidget);
    mainContentLayout->setContentsMargins(40, 20, 20, 40);
    mainContentLayout->setSpacing(30);

    // Left side - Main content area
    createMainContentArea(mainContentLayout);

    // Right side - Control buttons
    createControlButtons(mainContentLayout);

    canvasLayout->addWidget(contentWidget, 1);
}

void Safety::createMainContentArea(QHBoxLayout *mainLayout)
{
    QWidget *leftArea = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftArea);
    leftLayout->setSpacing(25);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    // Status bar - READY와 화재경보 배너가 교체될 영역
    statusWidget = new QWidget();
    statusWidget->setObjectName("statusWidget");
    statusWidget->setFixedHeight(80);

    // READY 라벨
    statusLabel = new QLabel("READY");
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);

    // 화재경보 배너 라벨 (초기에는 숨김)
    fireAlarmBanner = new QLabel("아파트 전체 화재경보 발생");
    fireAlarmBanner->setObjectName("fireAlarmBanner");
    fireAlarmBanner->setAlignment(Qt::AlignCenter);
    fireAlarmBanner->setVisible(false);

    // statusWidget에 두 라벨을 겹쳐서 배치
    QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(fireAlarmBanner);

    // Camera area (main content area)
    cameraArea = new QWidget();
    cameraArea->setObjectName("cameraArea");
    cameraArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    cameraLabel = new QLabel("카메라 꺼짐");
    cameraLabel->setObjectName("cameraLabel");
    cameraLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *cameraLayout = new QVBoxLayout(cameraArea);
    cameraLayout->setContentsMargins(40, 20, 20, 40);
    cameraLayout->addWidget(cameraLabel);

    // 화재경보 상태 라벨 (초기에는 숨김)
    fireAlarmStatusLabel = new QLabel("화재경보 방송 중");
    fireAlarmStatusLabel->setObjectName("fireAlarmStatusLabel");
    fireAlarmStatusLabel->setAlignment(Qt::AlignCenter);
    fireAlarmStatusLabel->setVisible(false);
    cameraLayout->addWidget(fireAlarmStatusLabel);

    // Additional control buttons area
    QWidget *additionalButtonsArea = createAdditionalButtons();

    leftLayout->addWidget(statusWidget);
    leftLayout->addWidget(cameraArea, 1);
    leftLayout->addWidget(additionalButtonsArea);

    mainLayout->addWidget(leftArea, 1);
}

QWidget* Safety::createAdditionalButtons()
{
    QWidget *buttonsArea = new QWidget();
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsArea);
    buttonsLayout->setSpacing(20);
    buttonsLayout->setAlignment(Qt::AlignRight);

    // "집 안 보기" 버튼
    watchHomeButton = new QPushButton("집 안 보기");
    watchHomeButton->setObjectName("watchHomeButton");
    watchHomeButton->setFixedSize(140, 60);

    // "신고하기" 버튼
    callEmergencyButton = new QPushButton("신고하기");
    callEmergencyButton->setObjectName("callEmergencyButton");
    callEmergencyButton->setFixedSize(140, 60);

    // "화재경보알림" 버튼
    firealarmButton = new QPushButton("화재경보알림");
    firealarmButton->setObjectName("firealarmButton");
    firealarmButton->setFixedSize(140, 60);

    // "통화 종료" 버튼 (초기에는 숨김)
    endCallButton = new QPushButton("통화 종료");
    endCallButton->setObjectName("endCallButton");
    endCallButton->setFixedSize(140, 60);
    endCallButton->setVisible(false);

    // "경보 중지" 버튼 (초기에는 숨김)
    stopAlarmButton = new QPushButton("경보 중지");
    stopAlarmButton->setObjectName("stopAlarmButton");
    stopAlarmButton->setFixedSize(140, 60);
    stopAlarmButton->setVisible(false);

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(watchHomeButton);
    buttonsLayout->addWidget(callEmergencyButton);
    buttonsLayout->addWidget(firealarmButton);
    buttonsLayout->addWidget(endCallButton);
    buttonsLayout->addWidget(stopAlarmButton);

    // Connect signals
    connect(watchHomeButton, &QPushButton::clicked, this, &Safety::onWatchHomeClicked);
    connect(callEmergencyButton, &QPushButton::clicked, this, &Safety::onCallEmergencyClicked);
    connect(firealarmButton, &QPushButton::clicked, this, &Safety::OnFireAlarmClicked);
    connect(endCallButton, &QPushButton::clicked, this, &Safety::onEndCallClicked);
    connect(stopAlarmButton, &QPushButton::clicked, this, &Safety::onStopAlarmClicked);

    return buttonsArea;
}

QPixmap Safety::loadResourceImage(const QString& imagePath, const QSize& size)
{
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        // Navy 색상으로 변경
        QPixmap coloredPixmap = pixmap;
        QPainter painter(&coloredPixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(coloredPixmap.rect(), QColor(0x2F, 0x3C, 0x56)); // Navy 색상
        painter.end();

        if (!size.isEmpty()) {
            return coloredPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        return coloredPixmap;
    }
    return pixmap;
}

void Safety::createControlButtons(QHBoxLayout *mainLayout)
{
    QWidget *buttonsArea = new QWidget();
    buttonsArea->setFixedWidth(100);

    QVBoxLayout *buttonsLayout = new QVBoxLayout(buttonsArea);
    buttonsLayout->setSpacing(70);  // 4개 버튼을 위해 간격 조정
    buttonsLayout->setContentsMargins(0, 20, 0, 20);
    buttonsLayout->setAlignment(Qt::AlignCenter);

    // Create circle buttons - 4개 버튼
    userAccountButton = createCircleButton("camera");
    homeButton = createCircleButton("home");
    searchButton = createCircleButton("search");
    lockButton = createCircleButton("lock");

    // Set different style for lock button (gray background)
    lockButton->setObjectName("grayCircleButton");

    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(userAccountButton);
    buttonsLayout->addWidget(homeButton);
    buttonsLayout->addWidget(searchButton);  // 추가
    buttonsLayout->addWidget(lockButton);
    buttonsLayout->addStretch(1);

    // Connect signals
    connect(userAccountButton, &QPushButton::clicked, this, &Safety::onUserAccountClicked);
    connect(homeButton, &QPushButton::clicked, this, &Safety::onHomeClicked);
    connect(searchButton, &QPushButton::clicked, this, &Safety::onSearchClicked);  // 추가
    connect(lockButton, &QPushButton::clicked, this, &Safety::onLockClicked);

    mainLayout->addWidget(buttonsArea, 0);
}

QPushButton* Safety::createCircleButton(const QString& iconPath)
{
    QPushButton *button = new QPushButton();
    button->setObjectName("circleButton");
    button->setFixedSize(60, 60);

    // 리소스에서 이미지 로딩
    QPixmap iconPixmap;
    if (iconPath == "camera") {
        iconPixmap = loadResourceImage(":/res/cctv.png", QSize(30, 30));
    } else if (iconPath == "home") {
        iconPixmap = loadResourceImage(":/res/home.png", QSize(30, 30));
    } else if (iconPath == "search") {
        iconPixmap = loadResourceImage(":/res/search.png", QSize(30, 30));
    } else if (iconPath == "lock") {
        iconPixmap = loadResourceImage(":/res/lock.png", QSize(30, 30));
    }

    // 이미지가 성공적으로 로딩되었으면 아이콘 설정, 아니면 텍스트 사용
    if (!iconPixmap.isNull()) {
        button->setIcon(QIcon(iconPixmap));
        button->setIconSize(QSize(30, 30));
    } else {
        // 폴백: 이모지 사용
        if (iconPath == "camera") {
            button->setText("📹");
        } else if (iconPath == "home") {
            button->setText("🏠");
        } else if (iconPath == "search") {
            button->setText("🔍");
        } else if (iconPath == "lock") {
            button->setText("🔒");
        }
    }

    return button;
}

void Safety::applyShadowEffect(QWidget* widget)
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(15);
    shadow->setXOffset(0);
    shadow->setYOffset(5);
    shadow->setColor(QColor(0, 0, 0, 30));
    widget->setGraphicsEffect(shadow);
}

void Safety::setupStyles()
{
    QString styles = QString(
                         // Main background
                         "Safety { background-color: %1; }"

                         // Main canvas (rounded white container)
                         "QWidget#mainCanvas {"
                         "    background-color: white;"
                         "    border-radius: 28px;"
                         "}"

                         // Header
                         "QWidget#headerWidget {"
                         "    background-color: %2;"
                         "    border-top-left-radius: 28px;"
                         "    border-top-right-radius: 28px;"
                         "}"
                         "QLabel#titleLabel {"
                         "    font-family: 'sans-serif' !important;"
                         "    font-size: 40px !important;"
                         "    font-weight: bold !important;"
                         "    color: white !important;"
                         "}"

                         // Status bar
                         "QWidget#statusWidget {"
                         "    background-color: %3;"
                         "    border-radius: 15px;"
                         "    margin: 0px 40px;"
                         "}"
                         "QLabel#statusLabel {"
                         "    color: %2;"
                         "    font-size: 18px;"
                         "    font-weight: bold;"
                         "    padding: 15px;"
                         "}"

                         // 화재경보 배너 (statusWidget 내부에서 사용)
                         "QLabel#fireAlarmBanner {"
                         "    background-color: #dc3545;"  // 빨간색 배경
                         "    color: white;"
                         "    font-size: 18px;"
                         "    font-weight: bold;"
                         "    padding: 15px;"
                         "    border-radius: 15px;"
                         "    margin: 0px 40px;"
                         "}"

                         // Camera area
                         "QWidget#cameraArea {"
                         "    background-color: %4;"
                         "    border-radius: 20px;"
                         "}"
                         "QLabel#cameraLabel {"
                         "    color: %2;"
                         "    font-size: 24px;"
                         "    font-weight: bold;"
                         "}"

                         // Additional buttons
                         "QPushButton#watchHomeButton, QPushButton#callEmergencyButton, QPushButton#gasbelvButton, QPushButton#firealarmButton {"
                         "    background-color: %2;"
                         "    color: white;"
                         "    font-size: 14px;"
                         "    font-weight: bold;"
                         "    border: none;"
                         "    border-radius: 12px;"
                         "}"
                         "QPushButton#watchHomeButton:hover, QPushButton#callEmergencyButton:hover, QPushButton#gasbelvButton:hover, QPushButton#firealarmButton:hover {"
                         "    background-color: %5;"
                         "}"
                         "QPushButton#callEmergencyButton:hover {"
                         "    background-color: #e53e3e;"
                         "}"
                         "QPushButton#endCallButton {"
                         "    background-color: #e53e3e;"
                         "    color: white;"
                         "    font-size: 14px;"
                         "    font-weight: bold;"
                         "    border: none;"
                         "    border-radius: 12px;"
                         "}"
                         "QPushButton#endCallButton:hover {"
                         "    background-color: #c53030;"
                         "}"
                         "QPushButton#endCallButton:pressed {"
                         "    background-color: #9b2c2c;"
                         "}"

                         // 화재경보 상태 라벨
                         "QLabel#fireAlarmStatusLabel {"
                         "    color: #dc3545;"
                         "    font-size: 18px;"
                         "    font-weight: bold;"
                         "    background-color: transparent;"
                         "    padding: 10px;"
                         "}"

                         // 경보 중지 버튼
                         "QPushButton#stopAlarmButton {"
                         "    background-color: #dc3545;"
                         "    color: white;"
                         "    font-size: 14px;"
                         "    font-weight: bold;"
                         "    border: none;"
                         "    border-radius: 12px;"
                         "}"
                         "QPushButton#stopAlarmButton:hover {"
                         "    background-color: #c82333;"
                         "}"
                         "QPushButton#stopAlarmButton:pressed {"
                         "    background-color: #bd2130;"
                         "}"

                         // Circle buttons
                         "QPushButton#circleButton {"
                         "    background-color: #EAE6E6;"
                         "    border-radius: 30px;"
                         "    font-size: 20px;"
                         "    border: none;"
                         "}"
                         "QPushButton#circleButton:hover {"
                         "    background-color: %5;"
                         "}"
                         "QPushButton#circleButton:pressed {"
                         "    background-color: %6;"
                         "}"

                         // Gray circle button
                         "QPushButton#grayCircleButton {"
                         "    background-color: #EAE6E6;"
                         "    color: %2;"
                         "    border-radius: 30px;"
                         "    font-size: 20px;"
                         "    border: none;"
                         "}"
                         "QPushButton#grayCircleButton:hover {"
                         "    background-color: %6;"
                         "}"
                         "QPushButton#grayCircleButton:pressed {"
                         "    background-color: %6;"
                         "}"

                         ).arg(BackgroundGray.name(),                    // %1 - Background
                              DarkNavy.name(),                           // %2 - Navy
                              StatusGray.name(),                         // %3 - Status background
                              CameraAreaGray.name(),                     // %4 - Camera area background
                              DarkNavy.lighter(120).name(),              // %5 - Hover
                              DarkNavy.darker(120).name());              // %6 - Pressed

    setStyleSheet(styles);
}

//=============================================================================
// EVENT HANDLERS
//=============================================================================
void Safety::onWatchHomeClicked()
{
    if (watchHomeButton->text() == "집 안 보기") {
        // 버튼 토글
        watchHomeButton->setText("카메라 끄기");

        // 이미 켜져 있으면 무시
        if (s_camera && s_camera->isActive()) return;

        // 카메라/싱크 생성 및 연결 - 세 번째 카메라 지정 (인덱스 2)
        if (!s_camera) {
            // 수정된 코드
            QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
            if (cameras.size() > 0) {
                // 첫 번째 카메라 장치 선택 (index 0)
                s_camera = new QCamera(cameras.at(0), this);
            } else {
                // 카메라가 없으면 기본 카메라 사용
                s_camera = new QCamera(this);
            }
        }
        if (!s_sink) s_sink = new QVideoSink(this);

        // 프레임 콜백: cameraLabel에 표시
        // (중복 연결 방지 위해 일단 끊고 다시 연결)
        QObject::disconnect(s_sink, nullptr, this, nullptr);
        QObject::connect(s_sink, &QVideoSink::videoFrameChanged, this,
                         [this](const QVideoFrame& vf){
                             QVideoFrame frame(vf);
                             if (!frame.isValid()) return;
                             QImage img = frame.toImage();
                             if (img.isNull())     return;

                             // 라벨 크기에 맞춰 부드럽게 스케일
                             cameraLabel->setPixmap(
                                 QPixmap::fromImage(img)
                                     .scaled(cameraLabel->size(),
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation)
                                 );
                         });

        // 세션 연결 및 시작
        s_session.setCamera(s_camera);
        s_session.setVideoOutput(s_sink);
        s_camera->start();

        // 상태 텍스트 업데이트
        statusLabel->setText("READY");
        cameraLabel->setText(QString()); // "카메라 꺼짐" 삭제
    } else {
        // 끄기
        watchHomeButton->setText("집 안 보기");

        if (s_camera) s_camera->stop();
        if (s_sink) {
            QObject::disconnect(s_sink, nullptr, this, nullptr);
            s_sink->deleteLater();  s_sink = nullptr;
        }
        if (s_camera) { s_camera->deleteLater(); s_camera = nullptr; }

        cameraLabel->setPixmap(QPixmap()); // 화면 지우기
        cameraLabel->setText("카메라 꺼짐");
    }
}

void Safety::OnFireAlarmClicked()
{
    if (isFireAlarmActive) {
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("화재경보알림");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("아파트 전체에 화재 알림을 울릴까요?");
    msgBox.setInformativeText("(데모: 실제 경보 발생 안 함)");

    QPushButton *yesButton = msgBox.addButton("Yes", QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton("No", QMessageBox::NoRole);
    msgBox.setDefaultButton(noButton);

    // 동일한 고정 크기 설정
    msgBox.setFixedSize(420, 200);

    // 스타일시트 적용 (기존 코드와 동일)
    QString customStyle = QString(
                              "QMessageBox {"
                              "    background-color: #EAE6E6;"
                              "    font-family: 'sans-serif';"
                              "    border-radius: 15px;"
                              "}"
                              "QMessageBox {"
                              "    background-color: #EAE6E6;"
                              "    font-family: 'sans-serif';"
                              "    border-radius: 15px;"
                              "}"
                              "QMessageBox::title {"
                              "    background-color: %1;"
                              "    color: #EAE6E6;"
                              "    font-family: 'sans-serif';"
                              "    font-size: 18px;"
                              "    font-weight: bold;"
                              "    padding: 15px;"
                              "    border-top-left-radius: 15px;"
                              "    border-top-right-radius: 15px;"
                              "}"
                              "QMessageBox QLabel {"
                              "    background-color: transparent;"
                              "    color: %1;"
                              "    font-family: 'sans-serif';"
                              "    font-size: 14px;"
                              "    font-weight: bold;"
                              "    padding: 10px;"
                              "}"
                              "QMessageBox QPushButton {"
                              "    background-color: %1;"
                              "    color: #EAE6E6;"
                              "    font-family: 'sans-serif';"
                              "    font-size: 14px;"
                              "    font-weight: bold;"
                              "    border: none;"
                              "    border-radius: 8px;"
                              "    padding: 8px 20px;"
                              "    min-width: 80px;"
                              "    min-height: 30px;"
                              "}"
                              "QMessageBox QPushButton:hover {"
                              "    background-color: %2;"
                              "}"
                              "QMessageBox QPushButton:pressed {"
                              "    background-color: %3;"
                              "}"
                              ).arg(DarkNavy.name(),
                                   DarkNavy.lighter(120).name(),
                                   DarkNavy.darker(120).name());

    msgBox.setStyleSheet(customStyle);

    msgBox.exec();

    if (msgBox.clickedButton() == yesButton) {
        startFireAlarm();
    }
}

void Safety::startFireAlarm()
{
    isFireAlarmActive = true;

    // READY 라벨 숨기고 화재경보 배너 표시
    statusLabel->setVisible(false);
    fireAlarmBanner->setVisible(true);

    // 하단 상태 라벨 표시
    fireAlarmStatusLabel->setVisible(true);

    // 화재경보알림 버튼 숨기고 경보 중지 버튼 표시
    firealarmButton->setVisible(false);
    stopAlarmButton->setVisible(true);
}

// 화재경보 중지 함수
void Safety::stopFireAlarm()
{
    isFireAlarmActive = false;

    // 화재경보 배너 숨기고 READY 라벨 다시 표시
    fireAlarmBanner->setVisible(false);
    statusLabel->setVisible(true);

    // 하단 상태 라벨 숨김
    fireAlarmStatusLabel->setVisible(false);

    // 경보 중지 버튼 숨기고 화재경보알림 버튼 다시 표시
    stopAlarmButton->setVisible(false);
    firealarmButton->setVisible(true);
}

// 경보 중지 버튼 클릭 함수
void Safety::onStopAlarmClicked()
{
    stopFireAlarm();
}

void Safety::onCallEmergencyClicked()
{
    if (isEmergencyCallActive) {
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("119 신고");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("정말 119에 신고하시겠습니까?");
    msgBox.setInformativeText("(데모: 실제 통화/신고 안 함)");

    QPushButton *yesButton = msgBox.addButton("Yes", QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton("No", QMessageBox::NoRole);
    msgBox.setDefaultButton(noButton);

    // 고정 크기 설정 (화재경보 팝업과 동일하게)
    msgBox.setFixedSize(420, 200);  // 너비 420px, 높이 200px

    // 스타일시트 적용 (기존 코드와 동일)
    QString customStyle = QString(
                              "QMessageBox {"
                              "    background-color: #EAE6E6;"
                              "    font-family: 'sans-serif';"
                              "    border-radius: 15px;"
                              "}"
                              // 전체 다이얼로그 배경
                              "QMessageBox {"
                              "    background-color: #EAE6E6;"
                              "    font-family: 'sans-serif';"
                              "    border-radius: 10px;"
                              "}"

                              // 타이틀바 (윈도우 제목)
                              "QMessageBox QLabel#qt_msgbox_label {"
                              "    background-color: #EAE6E6;"
                              "    color: %1;"
                              "    font-family: 'sans-serif';"
                              "    font-size: 16px;"
                              "    font-weight: bold;"
                              "    padding: 10px;"
                              "}"

                              // 메인 텍스트
                              "QMessageBox QLabel#qt_msgbox_informativelabel {"
                              "    background-color: #EAE6E6;"
                              "    color: %1;"
                              "    font-family: 'sans-serif';"
                              "    font-weight: bold;"
                              "    font-size: 14px;"
                              "    padding: 5px;"
                              "}"

                              // 경고 아이콘 영역
                              "QMessageBox QLabel#qt_msgboxex_icon_label {"
                              "    background-color: #EAE6E6;"
                              "}"

                              // Yes/No 버튼들
                              "QMessageBox QPushButton {"
                              "    background-color: %1;"
                              "    color: #EAE6E6;"
                              "    font-family: 'sans-serif';"
                              "    font-size: 14px;"
                              "    font-weight: bold;"
                              "    border: none;"
                              "    border-radius: 8px;"
                              "    padding: 8px 20px;"
                              "    min-width: 80px;"
                              "    min-height: 30px;"
                              "}"

                              // 버튼 호버 효과
                              "QMessageBox QPushButton:hover {"
                              "    background-color: %2;"
                              "}"

                              // 버튼 클릭 효과
                              "QMessageBox QPushButton:pressed {"
                              "    background-color: %3;"
                              "}"

                              // 기본 버튼 (No 버튼) 특별 스타일
                              "QMessageBox QPushButton:default {"
                              "    border: 2px solid %1;"
                              "}"

                              ).arg(DarkNavy.name(),                    // %1 - Navy (#2F3C56)
                                   DarkNavy.lighter(120).name(),       // %2 - Hover
                                   DarkNavy.darker(120).name());       // %3 - Pressed

    msgBox.setStyleSheet(customStyle);
    msgBox.exec();

    if (msgBox.clickedButton() == yesButton) {
        startEmergencyCall();
    }
}

void Safety::startEmergencyCall()
{
    isEmergencyCallActive = true;

    // 상태 라벨 업데이트
    statusLabel->setText("119 신고 진행 중");

    // 카메라 라벨 업데이트
    cameraLabel->setText("신고 접수됨 - 대기 중");

    // 신고하기 버튼 숨기고 통화 종료 버튼 표시
    callEmergencyButton->setVisible(false);
    endCallButton->setVisible(true);
}

void Safety::onEndCallClicked()
{
    isEmergencyCallActive = false;

    // 상태 라벨 원래대로 복원
    statusLabel->setText("READY");

    // 카메라 라벨 원래대로 복원
    cameraLabel->setText("카메라 꺼짐");

    // 통화 종료 버튼 숨기고 신고하기 버튼 다시 표시
    endCallButton->setVisible(false);
    callEmergencyButton->setVisible(true);
}

void Safety::onUserAccountClicked()
{
    // 사용자 계정 기능 구현 (카메라 버튼)
}

void Safety::onHomeClicked()
{
    // 메인 화면으로 돌아가는 시그널 발생
    emit backToMain();
}

void Safety::onLockClicked()
{
    // Certified 화면으로 가는 시그널 발생
    emit goToCertified();
}

void Safety::onSearchClicked()
{
    // Search 화면으로 가는 시그널 발생
    emit goToSearch();
}

