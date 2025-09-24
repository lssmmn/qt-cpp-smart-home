#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QRandomGenerator>
#include "database.h"

//=============================================================================
// COLOR CONSTANTS - Exact Match to Design
//=============================================================================
const QColor BackgroundGray(0xF4, 0xF2, 0xF2);      // #F4F2F2 - Main background
const QColor DarkNavy(0x2F, 0x3C, 0x56);            // #2F3A56 - Navy blue
const QColor CardWhite(0xFF, 0xFF, 0xFF);           // #FFFFFF - Card background
const QColor TextGray(0x6B, 0x73, 0x80);            // #6B7380 - Secondary text
const QColor PureWhite(0xFF, 0xFF, 0xFF);           // #FFFFFF - White text
const QColor ToggleBlue(0x4F, 0x7A, 0xF0);          // #4F7AF0 - Toggle active
const QColor LightGray(0xE5, 0xE7, 0xEB);           // #E5E7EB - Toggle inactive
const QColor goldColor(218, 165, 32);               // #DAA520 - 똥색

// 센서 상태 색상
const QColor StatusNormal(0xEA, 0xE6, 0xE6);         // #EAE6E6 - 기본 색상
const QColor StatusDanger(0xFF, 0x4C, 0x4C);         // #FF4C4C - 위험 색상
const QColor StatusOptimal(0x4C, 0xAF, 0x50);        // #4CAF50 - 최적 색상

//=============================================================================
// CONSTRUCTOR & INITIALIZATION
//=============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isWindowOpen(false)
    , fireAlert(false)
    , gasAlert(false)
    , tcpClient(nullptr)
    , clockTimer(nullptr)
    , dbUpdateTimer(nullptr)
    , currentPlantStatus(SensorStatus::Normal)
    , currentGasStatus(SensorStatus::Normal)
    , currentFireStatus(SensorStatus::Normal)
    , currentPetPoopDetected(false)
{
    setupUI();
    setupStyles();

    // TCP 클라이언트 초기화 (서버 문서에 맞춰 8080 포트)
    tcpClient = new TcpClient(this);

    // TCP 클라이언트 시그널 연결
    connect(tcpClient, &TcpClient::connected, this, &MainWindow::onTcpConnected);
    connect(tcpClient, &TcpClient::disconnected, this, &MainWindow::onTcpDisconnected);
    connect(tcpClient, &TcpClient::messageReceived, this, &MainWindow::onTcpMessageReceived);
    connect(tcpClient, &TcpClient::errorOccurred, this, &MainWindow::onTcpErrorOccurred);

    // 자동으로 서버에 연결 시도 (127.0.0.1:8080)
    tcpClient->connectToServer();

    // 시계 타이머 초기화 및 시작
    clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    clockTimer->start(1000);
    updateClock();

    // DB 업데이트 타이머 초기화 및 시작
    dbUpdateTimer = new QTimer(this);
    connect(dbUpdateTimer, &QTimer::timeout, this, &MainWindow::updateDbData);
    dbUpdateTimer->start(5000);
    updateDbData();
}

void MainWindow::setupUI()
{
    // Set window to fullscreen with screen size
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    setFixedSize(screenGeometry.size());
    setWindowTitle("Smart Home Dashboard");
    setFocusPolicy(Qt::StrongFocus);

    // Create central widget with background
    centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    // Create stacked widget for page switching
    stackedWidget = new QStackedWidget(centralWidget);

    // Create main dashboard widget
    QWidget *dashboardPage = new QWidget();
    dashboardPage->setObjectName("dashboardPage");

    // Create other pages
    safetyWidget = new Safety();
    certifiedWidget = new Certified();
    searchWidget = new Search();

    // Connect page navigation
    connect(safetyWidget, &Safety::backToMain, this, [this]() {
        stackedWidget->setCurrentIndex(0);
    });
    connect(safetyWidget, &Safety::goToCertified, this, [this]() {
        stackedWidget->setCurrentIndex(2);
    });
    connect(certifiedWidget, &Certified::backToMain, this, [this]() {
        stackedWidget->setCurrentIndex(0);
    });
    connect(certifiedWidget, &Certified::goToSafety, this, [this]() {
        stackedWidget->setCurrentIndex(1);
    });
    connect(searchWidget, &Search::backToMain, this, [this]() {
        stackedWidget->setCurrentIndex(0);
    });
    connect(searchWidget, &Search::goToSafety, this, [this]() {
        stackedWidget->setCurrentIndex(1);
    });
    connect(searchWidget, &Search::goToCertified, this, [this]() {
        stackedWidget->setCurrentIndex(2);
    });
    // Safety에서 Search로 가는 시그널 연결 추가
    connect(safetyWidget, &Safety::goToSearch, this, [this]() {
        stackedWidget->setCurrentIndex(3);
    });

    // Certified에서 Search로 가는 시그널 연결 추가
    connect(certifiedWidget, &Certified::goToSearch, this, [this]() {
        stackedWidget->setCurrentIndex(3);
    });

    // Add pages to stack
    stackedWidget->addWidget(dashboardPage);
    stackedWidget->addWidget(safetyWidget);
    stackedWidget->addWidget(certifiedWidget);
    stackedWidget->addWidget(searchWidget);
    stackedWidget->setCurrentIndex(0);

    // Central layout
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->addWidget(stackedWidget);

    // Setup dashboard layout
    setupDashboardLayout(dashboardPage);

    // Connect button signals
    connect(cameraButton, &QPushButton::clicked, this, &MainWindow::onCameraClicked);
    connect(homeButton, &QPushButton::clicked, this, &MainWindow::onHomeClicked);
    connect(securityButton, &QPushButton::clicked, this, &MainWindow::onSecurityClicked);
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::onSearchClicked);
}

void MainWindow::updateDbData()
{
    // 온습도 데이터 조회 및 업데이트
    auto homeInfo = Database::instance().getLatestHomeEnv("1");
    if (homeInfo.has_value()) {
        double temperature = homeInfo.value().first;
        double humidity = homeInfo.value().second;

        // 온도 카드 업데이트
        if (tempCard) {
            QLabel* tempValueLabel = tempCard->findChild<QLabel*>("cardValue");
            if (tempValueLabel) {
                tempValueLabel->setText(QString::number((int)temperature) + "°C");
            }
        }

        // 습도 카드 업데이트
        if (humCard) {
            QLabel* humValueLabel = humCard->findChild<QLabel*>("cardValue");
            if (humValueLabel) {
                humValueLabel->setText(QString::number(humidity, 'f', 0) + "%");
            }
        }
    }

    // 화재 상태 조회 및 업데이트
    auto fireInfo = Database::instance().getLatestFireStatus("1");
    if (fireInfo.has_value()) {
        QString fireStatus = fireInfo.value().first;
        QString gasLevel = fireInfo.value().second;

        // 화재 카드 업데이트
        if (fireCard) {
            QLabel* fireStatusLabel = fireCard->findChild<QLabel*>("statusLabel");
            if (fireStatusLabel) {
                if (fireStatus == "화재") {
                    fireStatusLabel->setText("화재");
                    updateCardColor(fireCard, SensorStatus::Danger);
                } else {
                    fireStatusLabel->setText("정상");
                    updateCardColor(fireCard, SensorStatus::Normal);
                }
            }
        }

        // 가스 카드 업데이트
        if (gasCard) {
            QLabel* gasStatusLabel = gasCard->findChild<QLabel*>("statusLabel");
            if (gasStatusLabel) {
                if (gasLevel == "위험") {
                    gasStatusLabel->setText("위험");
                    updateCardColor(gasCard, SensorStatus::Danger);
                } else {
                    gasStatusLabel->setText("정상");
                    updateCardColor(gasCard, SensorStatus::Normal);
                }
            }
        }
    }

    // 식물 습도 조회 및 업데이트
    auto soilInfo = Database::instance().getLatestSoilMoisture("1");
    if (soilInfo.has_value()) {
        int soilMoisture = soilInfo.value();

        if (plantCard) {
            QLabel* plantValueLabel = plantCard->findChild<QLabel*>("cardValue");
            if (plantValueLabel) {
                plantValueLabel->setText(QString::number(soilMoisture) + "%");
            }

            // 식물 습도 상태에 따른 색상 업데이트
            if (soilMoisture < 36) {
                updateCardColor(plantCard, SensorStatus::Danger);
            } else if (soilMoisture > 55) {
                updateCardColor(plantCard, SensorStatus::Optimal);
            } else {
                updateCardColor(plantCard, SensorStatus::Normal);
            }
        }
    }

    // 펫 상태 조회 및 업데이트
    auto petInfo = Database::instance().getLatestPetToilet("1");
    if (petInfo.has_value()) {
        QString toiletStatus = petInfo.value();
        bool needsCleaning = (toiletStatus == "청소 필요");

        updatePetStatus(needsCleaning);
    }
}

void MainWindow::setupDashboardLayout(QWidget *dashboardPage)
{
    // Main layout with reduced margins (위쪽 여백 줄임)
    QVBoxLayout *mainLayout = new QVBoxLayout(dashboardPage);
    mainLayout->setContentsMargins(40, 30, 40, 80);
    mainLayout->setSpacing(20);

    // Create main canvas (the rounded white container)
    mainCanvas = new QWidget();
    mainCanvas->setObjectName("mainCanvas");
    applyShadowEffect(mainCanvas);

    QVBoxLayout *canvasLayout = new QVBoxLayout(mainCanvas);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->setSpacing(0);

    // Create header
    createHeader(canvasLayout);

    // Create dashboard content
    createDashboardContent(canvasLayout);

    mainLayout->addWidget(mainCanvas);
}

void MainWindow::createHeader(QVBoxLayout *canvasLayout)
{
    headerWidget = new QWidget();
    headerWidget->setObjectName("headerWidget");
    headerWidget->setFixedHeight(120);  // 기준 높이

    helloLabel = new QLabel("Hello, Sumin");
    helloLabel->setObjectName("helloLabel");
    helloLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addWidget(helloLabel);

    canvasLayout->addWidget(headerWidget);
}

void MainWindow::createDashboardContent(QVBoxLayout *canvasLayout)
{
    // Dashboard content widget
    dashboardWidget = new QWidget();
    dashboardWidget->setObjectName("dashboardContent");

    // Main horizontal layout
    QHBoxLayout *mainContentLayout = new QHBoxLayout(dashboardWidget);
    mainContentLayout->setContentsMargins(40, 20, 20, 40);
    mainContentLayout->setSpacing(30);

    // Left side - Cards grid
    createCardsGrid(mainContentLayout);

    // Right side - Cards (Clock, Door Lock, Plant Humidity)
    createRightSideCards(mainContentLayout);

    // Far right side - Control buttons
    createSideButtons(mainContentLayout);

    canvasLayout->addWidget(dashboardWidget, 1);
}

void MainWindow::createCardsGrid(QHBoxLayout *mainLayout)
{
    QWidget *cardsArea = new QWidget();
    QGridLayout *gridLayout = new QGridLayout(cardsArea);
    gridLayout->setSpacing(25);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    // Create cards following exact design layout (Lock 카드 제거)
    tempCard = createSensorCard("Temperature", "20°C", "thermometer");
    humCard = createSensorCard("Humidity", "50%", "droplet");
    petCard = createPetCard();

    fireCard = createStatusCard("Fire Detection");
    gasCard = createStatusCard("Gas");

    // Grid arrangement (Lock 카드 제거, 2x3 그리드로 변경)
    gridLayout->addWidget(tempCard, 0, 0);      // Top-left
    gridLayout->addWidget(humCard, 0, 1);       // Top-center
    gridLayout->addWidget(petCard, 0, 2, 2, 1); // Top-right (spans 2 rows)

    gridLayout->addWidget(fireCard, 1, 0);      // Bottom-left
    gridLayout->addWidget(gasCard, 1, 1);       // Bottom-center

    // Set column stretches
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(2, 1);

    mainLayout->addWidget(cardsArea, 2);
}

void MainWindow::createRightSideCards(QHBoxLayout *mainLayout)
{
    QWidget *rightCardsArea = new QWidget();
    rightCardsArea->setFixedWidth(280); // 다른 카드들과 동일한 너비로 설정

    QVBoxLayout *rightLayout = new QVBoxLayout(rightCardsArea);
    rightLayout->setSpacing(25);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    // Create right side cards with fixed width
    clockCard = createClockCard();
    windowCard = createWindowCard();
    plantCard = createSensorCard("Plant Humidity", "60%", "plant");

    rightLayout->addWidget(clockCard);
    rightLayout->addWidget(windowCard);
    rightLayout->addWidget(plantCard);
    rightLayout->addStretch();

    mainLayout->addWidget(rightCardsArea, 0); // stretch factor를 0으로 설정하여 고정 너비 유지
}

void MainWindow::createSideButtons(QHBoxLayout *mainLayout)
{
    QWidget *buttonsArea = new QWidget();
    buttonsArea->setFixedWidth(100);

    QVBoxLayout *buttonsLayout = new QVBoxLayout(buttonsArea);
    buttonsLayout->setSpacing(70);  // 버튼 간격을 40으로 늘림 (25에서 변경)
    buttonsLayout->setContentsMargins(0, 20, 0, 20);

    // 기존의 Qt::AlignTop 대신 Qt::AlignCenter로 변경하여 중앙 정렬
    buttonsLayout->setAlignment(Qt::AlignCenter);

    // Create circle buttons
    cameraButton = createCircleButton("camera");
    homeButton = createCircleButton("home");
    searchButton = createCircleButton("search");
    securityButton = createCircleButton("lock");

    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(cameraButton);
    buttonsLayout->addWidget(homeButton);
    buttonsLayout->addWidget(searchButton);
    buttonsLayout->addWidget(securityButton);
    buttonsLayout->addStretch(1);    // 하단 spacer (상단과 동일한 비율)

    mainLayout->addWidget(buttonsArea, 0);  // stretch factor 0으로 고정 크기 유지
}

QPixmap MainWindow::loadResourceImage(const QString& imagePath, const QSize& size)
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



//=============================================================================
// 센서 데이터 업데이트 메서드들
//=============================================================================
void MainWindow::updatePlantHumidityStatus(int value)
{
    SensorStatus newStatus;
    QString statusText;

    if (value < 0) { // DB에 데이터가 저장되지 않은 경우
        newStatus = SensorStatus::Danger;
        statusText = "데이터 없음";
    } else {
        newStatus = SensorStatus::Normal;
        statusText = QString::number(value) + "%";
    }

    if (currentPlantStatus != newStatus) {
        currentPlantStatus = newStatus;
        updateCardColor(plantCard, newStatus);

        // Plant Humidity 카드의 값 업데이트
        QLabel* valueLabel = plantCard->findChild<QLabel*>("cardValue");
        if (valueLabel) {
            valueLabel->setText(statusText);
        }

        qDebug() << "Plant Humidity status updated:" << statusText;
    }
}

void MainWindow::updatePetStatus(bool poopDetected)
{
    if (currentPetPoopDetected != poopDetected) {
        currentPetPoopDetected = poopDetected;

        // Pet 카드에서 상태 컨테이너 찾기
        QWidget* statusContainer = nullptr;
        QList<QWidget*> widgets = petCard->findChildren<QWidget*>();
        for (QWidget* widget : widgets) {
            QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(widget->layout());
            if (layout && layout->count() == 3) { // Food, Water, Clean 3개 아이템
                statusContainer = widget;
                break;
            }
        }

        if (statusContainer) {
            QHBoxLayout* statusLayout = qobject_cast<QHBoxLayout*>(statusContainer->layout());
            if (statusLayout && statusLayout->count() >= 3) {
                // Clean 아이템 (세 번째 아이템)
                QWidget* cleanItem = qobject_cast<QWidget*>(statusLayout->itemAt(2)->widget());
                if (cleanItem) {
                    QVBoxLayout* itemLayout = qobject_cast<QVBoxLayout*>(cleanItem->layout());
                    if (itemLayout && itemLayout->count() >= 2) {
                        // 아이콘과 텍스트 모두 업데이트
                        QLabel* itemIcon = qobject_cast<QLabel*>(itemLayout->itemAt(0)->widget());
                        QLabel* itemLabel = qobject_cast<QLabel*>(itemLayout->itemAt(1)->widget());

                        if (itemIcon && itemLabel) {
                            if (poopDetected) {
                                // 똥 감지 시: 빨간색 아이콘과 경고 텍스트
                                QPixmap redIcon = loadResourceImage(":/res/poo.png", QSize(70, 70));
                                if (!redIcon.isNull()) {
                                    // 빨간색으로 아이콘 색상 변경
                                    QPixmap coloredIcon = redIcon;
                                    QPainter painter(&coloredIcon);
                                    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
                                    painter.fillRect(coloredIcon.rect(), QColor(218, 165, 32)); // 황금색
                                    painter.end();
                                    itemIcon->setPixmap(coloredIcon);
                                } else {
                                    itemIcon->setText("💩");
                                    itemIcon->setStyleSheet("font-size: 30px; color: #FF4C4C;");
                                }

                                itemLabel->setText("똥을 치워주세요!");
                                itemLabel->setStyleSheet("color: #DAA520; font-weight: bold; font-size: 10px;");
                            } else {
                                // 정상 상태: 기본 색상 복원
                                QPixmap normalIcon = loadResourceImage(":/res/poo.png", QSize(70, 70));
                                if (!normalIcon.isNull()) {
                                    itemIcon->setPixmap(normalIcon);
                                } else {
                                    itemIcon->setText("🧼");
                                    itemIcon->setStyleSheet("font-size: 30px; color: #2F3A56;");
                                }

                                itemLabel->setText("Clean");
                                itemLabel->setStyleSheet("color: #2F3A56; font-weight: normal; font-size: 12px;");
                            }
                        }
                    }
                }
            }
        }

        qDebug() << "Pet status updated - Poop detected:" << poopDetected;
    }
}

void MainWindow::updateGasStatus(int value)
{
    SensorStatus newStatus;
    QString statusText;

    if (value >= 100 && value <= 200) {
        newStatus = SensorStatus::Optimal;
        statusText = "최적";
    } else if (value > 200 && value <= 500) {
        newStatus = SensorStatus::Normal;
        statusText = "정상";
    } else if (value >= 1000) {
        newStatus = SensorStatus::Danger;
        statusText = "위험";
    } else {
        newStatus = SensorStatus::Normal;
        statusText = "정상";
    }

    if (currentGasStatus != newStatus) {
        currentGasStatus = newStatus;
        updateCardColor(gasCard, newStatus);
        updateCardStatusText(gasCard, statusText, newStatus);

        qDebug() << "Gas status updated:" << statusText << "(" << value << ")";
    }
}

void MainWindow::updateFireStatus(int value)
{
    SensorStatus newStatus;
    QString statusText;

    if (value < 100) {
        newStatus = SensorStatus::Danger;
        statusText = "화재 위험";
    } else {
        newStatus = SensorStatus::Normal;
        statusText = "정상";
    }

    if (currentFireStatus != newStatus) {
        currentFireStatus = newStatus;
        updateCardColor(fireCard, newStatus);
        updateCardStatusText(fireCard, statusText, newStatus);

        qDebug() << "Fire status updated:" << statusText << "(" << value << ")";
    }
}

void MainWindow::updateCardColor(QWidget* card, SensorStatus status)
{
    if (!card) return;

    QString colorStyle = QString(
                             "QWidget#statusCard, QWidget#sensorCard {"
                             "    background-color: %1;"
                             "    border-radius: 20px;"
                             "}"
                             ).arg(getStatusColor(status));

    card->setStyleSheet(colorStyle);
}

void MainWindow::updateCardStatusText(QWidget* card, const QString& statusText, SensorStatus status)
{
    if (!card) return;

    QLabel* statusLabel = card->findChild<QLabel*>("statusLabel");
    if (statusLabel) {
        statusLabel->setText(statusText);

        QString textColor;
        switch (status) {
        case SensorStatus::Danger:
            textColor = "#FFFFFF"; // 위험 상태일 때 흰색 텍스트
            break;
        case SensorStatus::Optimal:
            textColor = "#FFFFFF"; // 최적 상태일 때 흰색 텍스트
            break;
        default:
            textColor = "#2F3C56"; // 기본 네이비 색상
            break;
        }

        statusLabel->setStyleSheet(QString("color: %1; font-weight: bold;").arg(textColor));
    }
}

QString MainWindow::getStatusColor(SensorStatus status)
{
    switch (status) {
    case SensorStatus::Danger:
        return StatusDanger.name(); // #FF4C4C
    case SensorStatus::Optimal:
        return StatusOptimal.name(); // #4CAF50
    case SensorStatus::Normal:
    default:
        return StatusNormal.name(); // #EAE6E6
    }
}

//=============================================================================
// CARD CREATION METHODS
//=============================================================================
QWidget* MainWindow::createClockCard()
{
    QWidget *card = new QWidget();
    card->setObjectName("clockCard");
    card->setFixedSize(280, 200); // setMinimumSize -> setFixedSize 변경
    applyShadowEffect(card);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignCenter);

    // 날짜 레이블
    QLabel *dateLabel = new QLabel();
    dateLabel->setObjectName("clockDateLabel");
    dateLabel->setAlignment(Qt::AlignCenter);
    QFont dateFont("Arial", 18);
    dateLabel->setFont(dateFont);

    // 시간 레이블
    clockLabel = new QLabel();
    clockLabel->setObjectName("clockTimeLabel");
    clockLabel->setAlignment(Qt::AlignCenter);
    QFont timeFont("Arial", 35, QFont::Bold);
    clockLabel->setFont(timeFont);

    layout->addWidget(dateLabel);
    layout->addWidget(clockLabel);

    return card;
}

QWidget* MainWindow::createWindowCard()
{
    QWidget *card = new QWidget();
    card->setObjectName("windowCard");
    card->setFixedSize(280, 200);
    applyShadowEffect(card);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(25, 20, 25, 20);
    layout->setSpacing(10);

    // Window 라벨
    QLabel *windowTitleLabel = new QLabel("Window");  // "Door" -> "Window"
    windowTitleLabel->setObjectName("windowTitleLabel");  // doorTitleLabel -> windowTitleLabel
    windowTitleLabel->setAlignment(Qt::AlignCenter);
    QFont windowFont("Arial", 16, QFont::Bold);
    windowTitleLabel->setFont(windowFont);

    // Inner navy container
    QWidget *innerContainer = new QWidget();
    innerContainer->setObjectName("windowInnerContainer");  // doorLockInnerContainer -> windowInnerContainer

    QHBoxLayout *containerLayout = new QHBoxLayout(innerContainer);
    containerLayout->setContentsMargins(15, 10, 15, 10);
    containerLayout->setSpacing(10);

    // Window text
    windowLabel = new QLabel(isWindowOpen ? "Open" : "Close");  // Lock/Unlock -> Open/Close
    QFont windowLabelFont("Arial", 14, QFont::Bold);
    windowLabel->setFont(windowLabelFont);
    windowLabel->setStyleSheet("color: white;");

    // Toggle switch
    windowToggle = new CustomToggleSwitch();  // doorLockToggle -> windowToggle
    windowToggle->setChecked(isWindowOpen);   // isLocked -> isWindowOpen
    connect(windowToggle, &CustomToggleSwitch::toggled, this, &MainWindow::toggleWindow);  // toggleLock -> toggleWindow

    containerLayout->addWidget(windowLabel);  // doorLockLabel -> windowLabel
    containerLayout->addStretch();
    containerLayout->addWidget(windowToggle);  // doorLockToggle -> windowToggle

    layout->addWidget(windowTitleLabel);  // doorTitleLabel -> windowTitleLabel
    layout->addWidget(innerContainer);

    return card;
}

QWidget* MainWindow::createSensorCard(const QString& title, const QString& value, const QString& iconPath)
{
    QWidget *card = new QWidget();

    // Temperature 카드만 특별한 objectName 설정
    if (title == "Temperature") {
        card->setObjectName("temperatureCard");
    } else {
        card->setObjectName("sensorCard");
    }

    // Plant Humidity 카드의 경우 높이 조정
    if (title == "Plant Humidity") {
        card->setFixedSize(280, 340);
    } else {
        card->setMinimumSize(280, 180);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    applyShadowEffect(card);

    // Temperature와 Humidity 카드는 가로 레이아웃
    if (title == "Temperature" || title == "Humidity") {
        QHBoxLayout *layout = new QHBoxLayout(card);
        layout->setContentsMargins(15, 20, 45, 20);
        layout->setSpacing(0);

        // 아이콘 설정
        QLabel *iconLabel = new QLabel();
        iconLabel->setObjectName("cardIcon");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setFixedSize(200, 250);
        iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QPixmap iconPixmap;
        if (title == "Temperature") {
            iconPixmap = loadResourceImage(":/res/thermometer.png", QSize(200, 250));
        } else if (title == "Humidity") {
            iconPixmap = loadResourceImage(":/res/blur.png", QSize(200, 250));
        }

        if (!iconPixmap.isNull()) {
            iconLabel->setPixmap(iconPixmap);
        } else {
            if (title == "Temperature") {
                iconLabel->setText("🌡️");
            } else if (title == "Humidity") {
                iconLabel->setText("💧");
            }
            iconLabel->setStyleSheet("font-size: 60px; color: #2F3A56;");
        }

        // 텍스트 영역
        QWidget *textArea = new QWidget();
        textArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QVBoxLayout *textLayout = new QVBoxLayout(textArea);
        textLayout->setContentsMargins(-120, 0, 0, 0);
        textLayout->setSpacing(0);
        textLayout->setAlignment(Qt::AlignCenter);

        QLabel *valueLabel = new QLabel(value);
        valueLabel->setObjectName("cardValue");
        valueLabel->setAlignment(Qt::AlignLeft);
        QFont valueFont("Arial", 60, QFont::Bold);
        valueLabel->setFont(valueFont);

        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("cardTitle");
        titleLabel->setAlignment(Qt::AlignLeft);
        QFont titleFont("Arial", 20);
        titleLabel->setFont(titleFont);

        textLayout->addWidget(valueLabel);
        textLayout->addWidget(titleLabel);

        layout->addWidget(iconLabel, 0);
        layout->addWidget(textArea, 1);
    } else if (title == "Plant Humidity") {
        // Plant Humidity 전용 가로 레이아웃 (더 큰 카드용)
        QHBoxLayout *layout = new QHBoxLayout(card);
        layout->setContentsMargins(15, 40, 45, 40);  // 세로 여백 더 크게
        layout->setSpacing(0);

        // 아이콘 (Plant Humidity용 크기)
        QLabel *iconLabel = new QLabel();
        iconLabel->setObjectName("cardIcon");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setFixedSize(130, 160);  // 높이를 더 크게
        iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QPixmap iconPixmap = loadResourceImage(":/res/sprout.png", QSize(130, 160));
        if (!iconPixmap.isNull()) {
            iconLabel->setPixmap(iconPixmap);
        } else {
            iconLabel->setText("🌱");
            iconLabel->setStyleSheet("font-size: 80px; color: #2F3A56;");  // 이모지도 더 크게
        }

        // 텍스트 영역
        QWidget *textArea = new QWidget();
        textArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QVBoxLayout *textLayout = new QVBoxLayout(textArea);
        textLayout->setContentsMargins(-120, 0, 0, 0);
        textLayout->setSpacing(0);
        textLayout->setAlignment(Qt::AlignCenter);

        QLabel *valueLabel = new QLabel(value);
        valueLabel->setObjectName("cardValue");
        valueLabel->setAlignment(Qt::AlignLeft);
        QFont valueFont("Arial", 33, QFont::Bold);  // 폰트도 더 크게
        valueLabel->setFont(valueFont);

        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("cardTitle");
        titleLabel->setAlignment(Qt::AlignLeft);
        QFont titleFont("Arial", 11);  // 제목도 조금 더 크게
        titleLabel->setFont(titleFont);

        textLayout->addWidget(valueLabel);
        textLayout->addWidget(titleLabel);

        layout->addWidget(iconLabel, 0);
        layout->addWidget(textArea, 1);
    } else {
        // 기존 세로 레이아웃 (다른 센서 카드들용)
        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setContentsMargins(25, 25, 25, 25);
        layout->setSpacing(15);
        layout->setAlignment(Qt::AlignCenter);

        // Icon
        QLabel *iconLabel = new QLabel();
        iconLabel->setObjectName("cardIcon");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setFixedSize(60, 60);

        QPixmap iconPixmap;
        if (iconPath == "droplet") {
            iconPixmap = loadResourceImage(":/res/blur.png", QSize(60, 60));
        } else if (iconPath == "plant") {
            iconPixmap = loadResourceImage(":/res/sprout.png", QSize(60, 60));
        }

        if (!iconPixmap.isNull()) {
            iconLabel->setPixmap(iconPixmap);
        } else {
            if (iconPath == "droplet") {
                iconLabel->setText("💧");
            } else if (iconPath == "plant") {
                iconLabel->setText("🌱");
            }
            iconLabel->setStyleSheet("font-size: 36px; color: #2F3A56;");
        }

        // Value
        QLabel *valueLabel = new QLabel(value);
        valueLabel->setObjectName("cardValue");
        valueLabel->setAlignment(Qt::AlignCenter);

        // Title
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("cardTitle");
        titleLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(titleLabel);
    }

    return card;
}

QWidget* MainWindow::createStatusCard(const QString& title)
{
    QWidget *card = new QWidget();
    card->setObjectName("statusCard");
    card->setMinimumSize(280, 240);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    applyShadowEffect(card);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(25, 25, 25, 35);
    layout->setSpacing(15);
    layout->setAlignment(Qt::AlignCenter);

    // Icon
    QLabel *iconLabel = new QLabel();
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(200, 200);

    QPixmap iconPixmap;
    if (title == "Fire Detection") {
        iconPixmap = loadResourceImage(":/res/fire.png", QSize(180, 180));
    } else if (title == "Gas") {
        iconPixmap = loadResourceImage(":/res/stove.png", QSize(200, 200));
    }

    if (!iconPixmap.isNull()) {
        iconLabel->setPixmap(iconPixmap);
    } else {
        if (title == "Fire Detection") {
            iconLabel->setText("🔥");
        } else {
            iconLabel->setText("⚙️");
        }
        iconLabel->setStyleSheet("font-size: 48px; color: #2F3A56;");
    }

    // Status (initially Normal)
    QLabel *statusLabel = new QLabel("Normal");
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    QFont statusFont("Arial", 20, QFont::Bold);
    statusLabel->setFont(statusFont);

    // Title
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("cardTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont("Arial", 14);
    titleLabel->setFont(titleFont);

    layout->addWidget(iconLabel);
    layout->addWidget(statusLabel);
    layout->addWidget(titleLabel);

    return card;
}

QWidget* MainWindow::createPetCard()
{
    QWidget *card = new QWidget();
    card->setObjectName("petCard");
    card->setMinimumSize(280, 410);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    applyShadowEffect(card);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(25, 25, 25, 25);
    layout->setSpacing(20);
    layout->setAlignment(Qt::AlignCenter);

    // Pet title
    QLabel *titleLabel = new QLabel("Pet");
    titleLabel->setObjectName("cardTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont("Arial", 20, QFont::Bold);
    titleLabel->setFont(titleFont);

    // Pet icon container (navy circle)
    QWidget *petIconContainer = new QWidget();
    petIconContainer->setFixedSize(350, 300);

    QLabel *petIcon = new QLabel();
    petIcon->setAlignment(Qt::AlignCenter);

    QPixmap petPixmap = loadResourceImage(":/res/pet1.png", QSize(350, 300));
    if (!petPixmap.isNull()) {
        petIcon->setPixmap(petPixmap);
    } else {
        petIcon->setText("🐕");
        petIcon->setStyleSheet("font-size: 80px; color: white;");
    }

    QVBoxLayout *iconContainerLayout = new QVBoxLayout(petIconContainer);
    iconContainerLayout->setContentsMargins(0, 0, 0, 0);
    iconContainerLayout->addWidget(petIcon);

    // Status indicators with images
    QWidget *statusContainer = new QWidget();
    QHBoxLayout *statusLayout = new QHBoxLayout(statusContainer);
    statusLayout->setSpacing(20);
    statusLayout->setAlignment(Qt::AlignCenter);

    QStringList statusItems = {"Food", "Water", "Clean"};
    QStringList statusImages = {":/res/food.png", ":/res/foodwater.png", ":/res/poo.png"};
    QStringList fallbackIcons = {"🍽️", "🥤", "🧼"};

    for (int i = 0; i < statusItems.size(); ++i) {
        QWidget *statusItem = new QWidget();
        QVBoxLayout *itemLayout = new QVBoxLayout(statusItem);
        itemLayout->setSpacing(8);
        itemLayout->setAlignment(Qt::AlignCenter);

        QLabel *itemIcon = new QLabel();
        itemIcon->setAlignment(Qt::AlignCenter);
        itemIcon->setFixedSize(70, 70);

        QPixmap statusPixmap = loadResourceImage(statusImages[i], QSize(70, 70));
        if (!statusPixmap.isNull()) {
            itemIcon->setPixmap(statusPixmap);
        } else {
            itemIcon->setText(fallbackIcons[i]);
            itemIcon->setStyleSheet("font-size: 30px; color: #2F3A56;");
        }

        QLabel *itemLabel = new QLabel(statusItems[i]);
        itemLabel->setAlignment(Qt::AlignCenter);
        QFont itemFont("Arial", 12);
        itemLabel->setFont(itemFont);
        itemLabel->setStyleSheet("color: #2F3A56;");

        itemLayout->addWidget(itemIcon);
        itemLayout->addWidget(itemLabel);
        statusLayout->addWidget(statusItem);
    }

    layout->addStretch(1);        // 위쪽 여백
    layout->addWidget(titleLabel);
    layout->addStretch(1);        // 제목과 펫 사이 여백
    layout->addWidget(petIconContainer);
    layout->addStretch(1);        // 펫과 상태 사이 여백
    layout->addWidget(statusContainer);
    layout->addStretch(1);        // 아래쪽 여백

    return card;
}

QPushButton* MainWindow::createCircleButton(const QString& iconPath)
{
    QPushButton *button = new QPushButton();
    button->setObjectName("circleButton");
    button->setFixedSize(60, 60);

    QPixmap iconPixmap;
    if (iconPath == "camera") {
        iconPixmap = loadResourceImage(":/res/cctv.png", QSize(30, 30));
    } else if (iconPath == "home") {
        iconPixmap = loadResourceImage(":/res/home.png", QSize(30, 30));
    } else if (iconPath == "lock") {
        iconPixmap = loadResourceImage(":/res/lock.png", QSize(30, 30));
    } else if (iconPath == "search") {
        iconPixmap = loadResourceImage(":/res/search.png", QSize(30, 30));
    }

    if (!iconPixmap.isNull()) {
        button->setIcon(QIcon(iconPixmap));
        button->setIconSize(QSize(30, 30));
    } else {
        // 폴백: 이모지 사용
        if (iconPath == "camera") {
            button->setText("📹");
        } else if (iconPath == "home") {
            button->setText("🏠");
        } else if (iconPath == "lock") {
            button->setText("🔒");
        } else if (iconPath == "search") {
            button->setText("🔍");
        }
    }

    return button;
}

//=============================================================================
// STYLING
//=============================================================================
void MainWindow::setupStyles()
{
    QString styles = QString(
                         // Main background
                         "QWidget#centralWidget {"
                         "    background-color: %1;"
                         "}"
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
                         "QLabel#helloLabel {"
                         "    font-family: 'sans-serif' !important;"
                         "    font-size: 40px !important;"
                         "    font-weight: bold !important;"
                         "    color: white !important;"
                         "}"
                         "QWidget#sensorCard {"
                         "    background-color: #EAE6E6;"  // 밝은 회색 배경
                         "    border-radius: 20px;"
                         "}"

                         // Temperature 카드 전용 스타일 추가
                         "QWidget#temperatureCard {"
                         "    background-color: #EAE6E6;"
                         "    border-radius: 20px;"
                         "    padding: 15px 10px;"

                         "}"

                         "QLabel#tempCardValue {"
                         "    color: #2E3A59;"  // 짙은 회색
                         "    font-size: 80px;"
                         "    font-weight: bold;"
                         "}"

                         "QLabel#tempCardTitle {"
                         "    color: #2E3A59;"  // 짙은 회색
                         "    font-size: 14px;"
                         "    font-weight: normal;"
                         "}"

                         "QLabel#tempCardIcon {"
                         "    background-color: transparent;"
                         "}"
                         // Cards
                         "QWidget#statusCard, QWidget#petCard, QWidget#lockCard, QWidget#clockCard, QWidget#windowCard {"
                         "    background-color: #EAE6E6;"
                         "    border-radius: 20px;"
                         "}"

                         // Card text
                         "QLabel#cardValue {"
                         "    color: %2;"
                         "    font-weight: bold;"
                         "}"
                         "QLabel#cardTitle {"
                         "    color: %3;"
                         "}"
                         "QLabel#statusLabel {"
                         "    color: %2;"
                         "}"

                         // Clock card specific styles
                         "QLabel#clockDateLabel {"
                         "    color: %3;"
                         "    font-size: 20px;"
                         "}"
                         "QLabel#clockTimeLabel {"
                         "    color: %2;"
                         "    font-size: 45px;"
                         "    font-weight: bold;"
                         "}"

                         // Door lock card specific styles
                         "QLabel#windowTitleLabel {"
                         "    color: %2;"
                         "    font-size: 20px;"
                         "    font-weight: bold;"
                         "}"

                         // Pet icon container
                         "QWidget#petIconContainer {"
                         "    background-color: %2;"
                         "    border-radius: 20px;"
                         "}"

                         // Lock inner container
                         "QWidget#lockInnerContainer, QWidget#windowInnerContainer {"
                         "    background-color: %2;"
                         "    border-radius: 15px;"
                         "}"

                         // Circle buttons
                         "QPushButton#circleButton {"
                         "    background-color: #EAE6E6;"
                         "    border-radius: 30px;"
                         "    font-size: 20px;"
                         "    border: none;"
                         "}"
                         "QPushButton#circleButton:hover {"
                         "    background-color: %4;"
                         "}"
                         "QPushButton#circleButton:pressed {"
                         "    background-color: %5;"
                         "}"
                         "QPushButton#connectButton {"
                         "    background-color: %2;"
                         "    color: white;"
                         "    font-size: 14px;"
                         "    font-weight: bold;"
                         "    border: none;"
                         "    border-radius: 8px;"
                         "}"
                         "QPushButton#connectButton:hover {"
                         "    background-color: %4;"
                         "}"
                         "QPushButton#connectButton:pressed {"
                         "    background-color: %5;"
                         "}"
                         "QLineEdit#inputField {"
                         "    border: 2px solid #E9ECEF;"
                         "    border-radius: 8px;"
                         "    padding: 8px 12px;"
                         "    font-size: 14px;"
                         "    background-color: white;"
                         "}"
                         "QLineEdit#inputField:focus {"
                         "    border-color: %2;"
                         "}"
                         "QPushButton#sendButton {"
                         "    background-color: %2;"
                         "    color: white;"
                         "    font-size: 14px;"
                         "    font-weight: bold;"
                         "    border: none;"
                         "    border-radius: 8px;"
                         "}"
                         "QPushButton#sendButton:hover {"
                         "    background-color: %4;"
                         "}"
                         "QPushButton#sendButton:pressed {"
                         "    background-color: %5;"
                         "}"
                         "QPushButton#sendButton:disabled {"
                         "    background-color: #CED4DA;"
                         "    color: #6C757D;"
                         "}"
                         "QTextEdit#logView {"
                         "    border: 2px solid #E9ECEF;"
                         "    border-radius: 8px;"
                         "    background-color: white;"
                         "    font-family: 'Consolas', 'Monaco', monospace;"
                         "    font-size: 12px;"
                         "    padding: 8px;"
                         "}"

                         ).arg(BackgroundGray.name(),      // %1 - Background
                              DarkNavy.name(),            // %2 - Navy
                              TextGray.name(),            // %3 - Text gray
                              DarkNavy.lighter(120).name(), // %4 - Hover
                              DarkNavy.darker(120).name()); // %5 - Pressed

    setStyleSheet(styles);
}

void MainWindow::applyShadowEffect(QWidget* widget)
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(15);
    shadow->setXOffset(0);
    shadow->setYOffset(5);
    shadow->setColor(QColor(0, 0, 0, 30));
    widget->setGraphicsEffect(shadow);
}

void MainWindow::animateCardAlert(QWidget* card, bool enable)
{
    QLabel* statusLabel = card->findChild<QLabel*>("statusLabel");
    if (statusLabel) {
        if (enable) {
            statusLabel->setText("ALERT!");
            statusLabel->setStyleSheet("color: #F44336; font-weight: bold;");
            card->setStyleSheet("QWidget#statusCard { background-color: #FFEBEE; border: 2px solid #F44336; border-radius: 20px; }");
        } else {
            statusLabel->setText("Normal");
            statusLabel->setStyleSheet("color: #2F3A56; font-weight: bold;");
            card->setStyleSheet("QWidget#statusCard { background-color: white; border-radius: 20px; }");
        }
    }
}

void MainWindow::updateClock()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timeString = currentDateTime.toString("hh:mm");
    QString dateString = currentDateTime.toString("yyyy.MM.dd");

    if (clockLabel) {
        clockLabel->setText(timeString);
    }

    if (clockCard) {
        QLabel* dateLabel = clockCard->findChild<QLabel*>("clockDateLabel");
        if (dateLabel) {
            dateLabel->setText(dateString);
        }
    }
}

void MainWindow::toggleWindow()
{
    isWindowOpen = windowToggle->isChecked();

    if (windowLabel) {
        windowLabel->setText(isWindowOpen ? "Open" : "Close");
    }

    // TCP 서버로 창문 제어 명령 전송 (서버 문서 기준)
    if (tcpClient && tcpClient->isConnected()) {
        if (isWindowOpen) {
            tcpClient->sendWindowOpen(); // "window_open" 명령
            qDebug() << "Window open command sent to server";
        } else {
            tcpClient->sendWindowClose(); // "window_close" 명령
            qDebug() << "Window close command sent to server";
        }
    } else {
        qWarning() << "TCP client not connected - window command not sent";
    }
}

void MainWindow::requestWindowStatus()
{
    if (tcpClient && tcpClient->isConnected()) {
        tcpClient->sendWindowStatus(); // "window_status" 명령
        qDebug() << "Window status request sent to server";
    }
}

void MainWindow::setWindowAngle(int angle)
{
    if (tcpClient && tcpClient->isConnected()) {
        tcpClient->setWindowAngle(angle); // "set_open_angle=N" 명령
        qDebug() << "Window angle set command sent to server:" << angle;
    }
}

void MainWindow::onCameraClicked()
{
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::onHomeClicked()
{
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::onSecurityClicked()
{
    stackedWidget->setCurrentIndex(2);
}

void MainWindow::onSearchClicked()
{
    stackedWidget->setCurrentIndex(3);
}

void MainWindow::toggleFireAlert()
{
    fireAlert = !fireAlert;
    animateCardAlert(fireCard, fireAlert);
}

void MainWindow::toggleGasAlert()
{
    gasAlert = !gasAlert;
    animateCardAlert(gasCard, gasAlert);
}

void MainWindow::onTcpConnected()
{
    qDebug() << "[TCP] Connected to Smart Home Server (127.0.0.1:8080)";

    // 연결 성공 후 창문 상태 조회
    if (tcpClient) {
        QTimer::singleShot(1000, this, [this]() {
            requestWindowStatus(); // 초기 상태 확인
        });
    }
}

void MainWindow::onTcpDisconnected()
{
    qDebug() << "[TCP] Disconnected from Smart Home Server";

    // 연결 끊어진 경우 자동 재연결 시도
    QTimer::singleShot(5000, this, [this]() {
        if (tcpClient && !tcpClient->isConnected()) {
            qDebug() << "[TCP] Attempting to reconnect to Smart Home Server...";
            tcpClient->connectToServer();
        }
    });
}

void MainWindow::onTcpMessageReceived(const QString& message)
{
    qDebug() << "[TCP] Received:" << message;

    // 서버 문서에 따른 응답 처리
    if (message.startsWith("ACK")) {
        // 명령 확인 응답: "ACK OPEN", "ACK CLOSE" 등
        QStringList parts = message.split(" ");
        if (parts.size() >= 2) {
            QString command = parts[1];
            qDebug() << "Server acknowledged command:" << command;
        }
    }
    else if (message.startsWith("EVT")) {
        // 이벤트 응답: "EVT OPENED", "EVT CLOSED" 등
        QStringList parts = message.split(" ");
        if (parts.size() >= 2) {
            QString event = parts[1];
            qDebug() << "Server event:" << event;

            // UI 상태 업데이트
            if (event == "OPENED") {
                isWindowOpen = true;
                if (windowToggle) windowToggle->setChecked(true);
                if (windowLabel) windowLabel->setText("Open");
            }
            else if (event == "CLOSED") {
                isWindowOpen = false;
                if (windowToggle) windowToggle->setChecked(false);
                if (windowLabel) windowLabel->setText("Close");
            }
        }
    }
    else if (message.startsWith("{") && message.endsWith("}")) {
        // JSON 상태 응답: {"pose":"OPEN","angle":100}
        qDebug() << "Window status JSON:" << message;

        // JSON 파싱 (간단한 방법)
        if (message.contains("\"pose\":\"OPEN\"")) {
            isWindowOpen = true;
            if (windowToggle) windowToggle->setChecked(true);
            if (windowLabel) windowLabel->setText("Open");
        }
        else if (message.contains("\"pose\":\"CLOSE\"")) {
            isWindowOpen = false;
            if (windowToggle) windowToggle->setChecked(false);
            if (windowLabel) windowLabel->setText("Close");
        }
    }
    else {
        // 센서 데이터 처리 (기존 로직 유지)
        QStringList parts = message.split(":");
        if (parts.size() >= 2) {
            QString sensor = parts[0].toUpper();
            QString value = parts[1];

            if (sensor == "PLANT") {
                bool ok;
                int plantValue = value.toInt(&ok);
                if (ok) {
                    updatePlantHumidityStatus(plantValue);
                }
            } else if (sensor == "GAS") {
                bool ok;
                int gasValue = value.toInt(&ok);
                if (ok) {
                    updateGasStatus(gasValue);
                }
            } else if (sensor == "FIRE") {
                bool ok;
                int fireValue = value.toInt(&ok);
                if (ok) {
                    updateFireStatus(fireValue);
                }
            } else if (sensor == "PET") {
                bool poopDetected = (value.toUpper() == "POOP");
                updatePetStatus(poopDetected);
            }
        }
    }
}

void MainWindow::onTcpErrorOccurred(const QString& errorString)
{
    qDebug() << "[TCP] Smart Home Server Error:" << errorString;
}

//=============================================================================
// CUSTOM TOGGLE SWITCH IMPLEMENTATION
//=============================================================================
CustomToggleSwitch::CustomToggleSwitch(QWidget *parent)
    : QWidget(parent)
    , checked(false)
    , handlePosition(Margin)
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(Width, Height);

    animation = new QPropertyAnimation(this, "position", this);
    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
}

void CustomToggleSwitch::setChecked(bool checked)
{
    if (this->checked != checked) {
        this->checked = checked;
        updateAnimation();
        emit toggled(checked);
    }
}

void CustomToggleSwitch::setPosition(int pos)
{
    handlePosition = pos;
    update();
}

void CustomToggleSwitch::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor = checked ? QColor(0x4F, 0x7A, 0xF0) : QColor(0xE5, 0xE7, 0xEB);
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), Height/2, Height/2);

    painter.setBrush(Qt::white);
    painter.setPen(QPen(QColor(0xD1, 0xD5, 0xDB), 1));
    QRect handleRect(handlePosition, Margin, HandleSize, HandleSize);
    painter.drawEllipse(handleRect);
}

void CustomToggleSwitch::mousePressEvent(QMouseEvent *)
{
    toggle();
}

QSize CustomToggleSwitch::sizeHint() const
{
    return QSize(Width, Height);
}

void CustomToggleSwitch::toggle()
{
    setChecked(!checked);
}

void CustomToggleSwitch::updateAnimation()
{
    int startPos = handlePosition;
    int endPos = checked ? (Width - HandleSize - Margin) : Margin;

    animation->setStartValue(startPos);
    animation->setEndValue(endPos);
    animation->start();
}
