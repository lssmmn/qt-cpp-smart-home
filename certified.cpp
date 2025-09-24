#include "certified.h"
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QPixmap>
#include <QBuffer>
#include <QInputDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QShowEvent>
#include <QHideEvent>

//=============================================================================
// COLOR CONSTANTS - Match Main Dashboard
//=============================================================================
const QColor BackgroundGray(0xEA, 0xE6, 0xE6);      // #EAE6E6 - Main background
const QColor DarkNavy(0x2F, 0x3C, 0x56);            // #2F3C56 - Navy blue
const QColor CardWhite(0xFF, 0xFF, 0xFF);           // #FFFFFF - Card background
const QColor StatusGray(0xF5, 0xF5, 0xF5);          // #F5F5F5 - Status bar background
const QColor CameraAreaGray(0xE8, 0xE8, 0xE8);      // #E8E8E8 - Camera area background

Certified::Certified(QWidget *parent) : QWidget(parent)
{
    setupFonts();
    setupUI();
    setupStyles();
    connect(registerButton, &QPushButton::clicked, this, &Certified::onRegisterClicked);
    setIdleReady();
}

Certified::~Certified()
{
}

void Certified::setupFonts()
{
    QFont appFont("sans-serif", 10);
    QApplication::setFont(appFont);
    setFont(appFont);
}

void Certified::setupUI()
{
    // Set fullscreen size to match MainWindow exactly
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    setFixedSize(screenGeometry.size());

    // Main layout with background color - same as MainWindow
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

    // Create certified content
    createCertifiedContent(canvasLayout);

    mainLayout->addWidget(mainCanvas);
}

void Certified::createHeader(QVBoxLayout *canvasLayout)
{
    headerWidget = new QWidget();
    headerWidget->setObjectName("headerWidget");
    headerWidget->setFixedHeight(120);

    titleLabel = new QLabel("Certified");
    titleLabel->setObjectName("titleLabel");  // titleLabel로 설정됨 - 맞음
    titleLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addWidget(titleLabel);

    canvasLayout->addWidget(headerWidget);
}

void Certified::createCertifiedContent(QVBoxLayout *canvasLayout)
{
    // Certified content widget
    QWidget *contentWidget = new QWidget();
    contentWidget->setObjectName("certifiedContent");

    // Main horizontal layout
    QHBoxLayout *mainContentLayout = new QHBoxLayout(contentWidget);
    mainContentLayout->setContentsMargins(40, 20, 20, 40);
    mainContentLayout->setSpacing(30);

    // Left side - Main content area
    createMainContentArea(mainContentLayout);

    // Right side - Control buttons (EXACTLY same position as MainWindow)
    createControlButtons(mainContentLayout);

    canvasLayout->addWidget(contentWidget, 1);
}

void Certified::createMainContentArea(QHBoxLayout *mainLayout)
{
    QWidget *leftArea = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftArea);
    leftLayout->setSpacing(25);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    // Status bar - "카메라 시작 (얼굴 인식 가능)" -> "READY"로 변경
    statusWidget = new QWidget();
    statusWidget->setObjectName("statusWidget");
    statusWidget->setFixedHeight(80);

    statusLabel = new QLabel("READY");
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->addWidget(statusLabel);

    // Camera area (main content area)
    cameraArea = new QWidget();
    cameraArea->setObjectName("cameraArea");
    cameraArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    cameraLabel = new QLabel("카메라");
    cameraLabel->setObjectName("cameraLabel");
    cameraLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *cameraLayout = new QVBoxLayout(cameraArea);
    cameraLayout->setContentsMargins(80, 20, 80, 20);
    cameraLayout->addWidget(cameraLabel);

    // Bottom buttons area
    QWidget *bottomButtonsArea = createBottomButtons();

    leftLayout->addWidget(statusWidget);
    leftLayout->addWidget(cameraArea, 1);
    leftLayout->addWidget(bottomButtonsArea);

    mainLayout->addWidget(leftArea, 1);
}

QWidget* Certified::createBottomButtons()
{
    QWidget *buttonsArea = new QWidget();
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsArea);
    buttonsLayout->setSpacing(20);
    buttonsLayout->setAlignment(Qt::AlignCenter);

    // "등록" 버튼
    registerButton = new QPushButton("등록");
    registerButton->setObjectName("registerButton");
    registerButton->setFixedSize(120, 50);

    // "인증용 사진 저장 준비 완료" 텍스트
    readyLabel = new QLabel("인증용 사진 저장 준비 완료");
    readyLabel->setObjectName("readyLabel");
    readyLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(registerButton);
    buttonsLayout->addWidget(readyLabel);
    buttonsLayout->addStretch();

    return buttonsArea;
}

QPixmap Certified::loadResourceImage(const QString& imagePath, const QSize& size)
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

void Certified::createControlButtons(QHBoxLayout *mainLayout)
{
    QWidget *buttonsArea = new QWidget();
    buttonsArea->setFixedWidth(100);  // EXACTLY same width as MainWindow

    QVBoxLayout *buttonsLayout = new QVBoxLayout(buttonsArea);
    buttonsLayout->setSpacing(70);    // 4개 버튼을 위해 간격 조정
    buttonsLayout->setContentsMargins(0, 20, 0, 20);  // EXACTLY same margins as MainWindow
    buttonsLayout->setAlignment(Qt::AlignCenter);

    // Create circle buttons - EXACT same as MainWindow
    userAccountButton = createCircleButton("camera");
    homeButton = createCircleButton("home");
    searchButton = createCircleButton("search");
    lockButton = createCircleButton("lock");

    // Set different style for lock button (gray background) - as shown in design
    lockButton->setObjectName("grayCircleButton");

    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(userAccountButton);
    buttonsLayout->addWidget(homeButton);
    buttonsLayout->addWidget(searchButton);
    buttonsLayout->addWidget(lockButton);
    buttonsLayout->addStretch(1);

    // Connect signals
    connect(userAccountButton, &QPushButton::clicked, this, &Certified::onLockClicked);
    connect(homeButton, &QPushButton::clicked, this, &Certified::onHomeClicked);
    connect(searchButton, &QPushButton::clicked, this, &Certified::onSearchClicked);

    mainLayout->addWidget(buttonsArea, 0);
}

QPushButton* Certified::createCircleButton(const QString& iconPath)
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

void Certified::applyShadowEffect(QWidget* widget)
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(15);
    shadow->setXOffset(0);
    shadow->setYOffset(5);
    shadow->setColor(QColor(0, 0, 0, 30));
    widget->setGraphicsEffect(shadow);
}

void Certified::setupStyles()
{
    QString styles = QString(
                         // Main background
                         "Certified { background-color: %1; }"

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
                         "QLabel#titleLabel {"  // headerTitle -> titleLabel로 수정
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

                         // Register button
                         "QPushButton#registerButton {"
                         "    background-color: %2;"
                         "    color: white;"
                         "    font-size: 14px;"
                         "    font-weight: bold;"
                         "    border: none;"
                         "    border-radius: 12px;"
                         "}"
                         "QPushButton#registerButton:hover {"
                         "    background-color: %5;"
                         "}"
                         "QPushButton#registerButton:pressed {"
                         "    background-color: %6;"
                         "}"

                         // Ready label
                         "QLabel#readyLabel {"
                         "    color: %2;"
                         "    font-size: 12px;"
                         "    background: transparent;"
                         "}"

                         // Circle buttons (same as MainWindow)
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

                         // Gray circle button (lock button)
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


// 여기부터 (추가)--------------------------------------------------------------

// 보일 때 자동 ON
void Certified::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    // 페이지 2로 전환되어 이 위젯이 보여질 때 호출됨
    startCamera(0);        // 이미 켜져 있으면 내부에서 무시되도록 구현돼 있어야 함
    setIdleReady();        // READY / "인증용 사진 저장 준비 완료" 기본 상태로
}

// 숨겨질 때 자동 OFF
void Certified::hideEvent(QHideEvent* e) {
    QWidget::hideEvent(e);
    // 다른 페이지로 넘어가며 이 위젯이 숨겨질 때 호출됨
    stopCamera();          // 촬영 중이면 burst=false 처리 및 상태 복귀까지
}


// ====== 유틸: Mat->QImage ======
QImage Certified::matToQImage(const cv::Mat& mat) {
    if (mat.empty()) return QImage();
    if (mat.type() == CV_8UC1) {
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8).copy();
    }
    cv::Mat rgb;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    }
    cv::Mat gray;
    if (mat.channels() == 3) cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
    else mat.convertTo(gray, CV_8U);
    return QImage(gray.data, gray.cols, gray.rows, gray.step, QImage::Format_Grayscale8).copy();
}

// ====== 얼굴 검출기 로드 ======
bool Certified::loadFaceCascade() {
    if (cascadeLoaded) return true;
    const QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/haarcascade_frontalface_default.xml",
        QDir::currentPath() + "/haarcascade_frontalface_default.xml",
        "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
        "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml"
    };
    for (const auto& p : candidates) {
        if (QFile::exists(p)) {
            if (faceCascade.load(p.toStdString())) {
                cascadeLoaded = true;
                break;
            }
        }
    }
    return cascadeLoaded;
}

// ====== 128x128 얼굴/센터 크롭 ======
static cv::Rect keepIn(const cv::Rect& r, const cv::Size& s) {
    int x = std::max(0, r.x);
    int y = std::max(0, r.y);
    int w = std::min(r.width,  s.width  - x);
    int h = std::min(r.height, s.height - y);
    return cv::Rect(x, y, w, h);
}

cv::Mat Certified::cropFace128(const cv::Mat& bgr) {
    cv::Mat img = bgr;
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    cv::Rect roi;
    if (loadFaceCascade()) {
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(60,60));
        if (!faces.empty()) {
            // 가장 큰 얼굴
            roi = *std::max_element(faces.begin(), faces.end(),
                                    [](const cv::Rect& a, const cv::Rect& b){ return a.area() < b.area(); });
        }
    }

    if (roi.area() == 0) {
        // 센터 정사각 크롭
        int minSide = std::min(img.cols, img.rows);
        int cx = img.cols/2, cy = img.rows/2;
        roi = cv::Rect(cx - minSide/2, cy - minSide/2, minSide, minSide);
    }
    roi = keepIn(roi, img.size());
    cv::Mat crop = img(roi).clone();
    cv::resize(crop, crop, cv::Size(128,128), 0,0, cv::INTER_AREA); // 컬러 유지
    return crop;
}

// ====== DB 연결 ======
bool Certified::openDb() {
    if (db.isValid() && db.isOpen()) return true;

    // 연결명: EnrollConnection (팀 결정사항)
    if (QSqlDatabase::contains("hometer"))
        db = QSqlDatabase::database("hometer");
    else
        db = QSqlDatabase::addDatabase("QMYSQL", "hometer");

    // ★ 현재는 요청하신 값으로 직접 연결 (후에 JSON/env 로 전환 가능)
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("hometer");
    db.setUserName("root");
    db.setPassword("1111");  // <- 추후 ConfigLoader로 치환 권장

    if (!db.open()) {
        if (statusLabel) statusLabel->setText("DB 연결 실패: " + db.lastError().text());
        qWarning() << "DB open failed:" << db.lastError().text();
        return false;
    }
    return true;
}

// ====== 1장 INSERT ======
bool Certified::insertFaceImage(int uid, const QString& uname, const cv::Mat& bgr) {
    if (!openDb()) return false;

    // 1) 얼굴/센터 크롭 128x128
    cv::Mat crop = cropFace128(bgr);

    // 2) PNG 인메모리 인코딩 → QByteArray
    std::vector<uchar> buf;
    if (!cv::imencode(".png", crop, buf)) {
        qWarning() << "PNG encode failed";
        return false;
    }
    QByteArray ba(reinterpret_cast<const char*>(buf.data()), static_cast<int>(buf.size()));

    // 3) INSERT
    QSqlQuery q(db);
    q.prepare("INSERT INTO face_images (user_id, user_name, face_data) VALUES (?, ?, ?)");
    q.addBindValue(uid);
    q.addBindValue(uname);
    q.addBindValue(ba);

    if (!q.exec()) {
        qWarning() << "INSERT failed:" << q.lastError().text();
        if (statusLabel) statusLabel->setText("DB 저장 실패: " + q.lastError().text());
        return false;
    }
    return true;
}

// ====== 카메라 제어 ======
void Certified::startCamera(int index) {
    if (cameraRunning) return;
    
    // 첫 번째 카메라 사용 (index 0)
    int cameraIndex = 0;
    
    if (!cap.open(cameraIndex)) {
        if (statusLabel) statusLabel->setText("카메라 열기 실패");
        qWarning() << "Failed to open camera index" << cameraIndex;
        return;
    }
    
    cameraRunning = true;

    if (!cameraTimer) {
        cameraTimer = new QTimer(this);
        connect(cameraTimer, &QTimer::timeout, this, &Certified::onFrameTick);
    }
    cameraTimer->start(33); // ~30fps
    if (statusLabel) statusLabel->setText("READY");
}

void Certified::stopCamera() {
    if (!cameraRunning) return;
    if (cameraTimer) cameraTimer->stop();
    cap.release();
    cameraRunning = false;
    if (statusLabel) statusLabel->setText("카메라 OFF");
}

// ====== 프레임 틱 ======
void Certified::onFrameTick() {
    if (!cameraRunning) return;

    cv::Mat frame;
    if (!cap.read(frame) || frame.empty()) {
        if (statusLabel) statusLabel->setText("프레임 수신 실패");
        return;
    }

    // 프리뷰
    if (cameraLabel) {
        QImage img = matToQImage(frame);
        QPixmap pix = QPixmap::fromImage(img).scaled(
            cameraLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        cameraLabel->setPixmap(pix);
    }

    // 버스트 저장
    if (burst) {
        bool ok = insertFaceImage(currentUserId, currentUserName, frame);
        if (ok) ++burstSaved;

        if (burstSaved >= burstTarget) {
            burst = false;
            if (readyLabel)  readyLabel->setText("저장완료");
            if (statusLabel) statusLabel->setText("COMPLETE");

            QTimer::singleShot(3000, this, [this](){
                this->setIdleReady();
            });
        } else {
            if (readyLabel)  readyLabel->setText(QString("저장 중... (%1/%2)").arg(burstSaved).arg(burstTarget));
        }
    }
}

// ====== 등록 버튼 ======
void Certified::onRegisterClicked() {
    // ì‚¬ìš©ìž ID ìž…ë ¥ - 커스텀 다이얼로그 생성
    QDialog idDialog(this);
    idDialog.setWindowTitle("사용자 ID");
    idDialog.setFixedSize(420, 200);
    idDialog.setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(&idDialog);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 제목 라벨
    QLabel *titleLabel = new QLabel("user_id:");
    titleLabel->setObjectName("dialogTitle");

    // 입력 필드
    QLineEdit *idLineEdit = new QLineEdit();
    idLineEdit->setObjectName("dialogInput");
    idLineEdit->setText("1");
    idLineEdit->selectAll();

    // 버튼 레이아웃
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    QPushButton *cancelBtn = new QPushButton("Cancel");
    QPushButton *okBtn = new QPushButton("OK");
    cancelBtn->setObjectName("dialogButton");
    okBtn->setObjectName("dialogButton");

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(okBtn);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(idLineEdit);
    mainLayout->addLayout(buttonLayout);

    // 스타일시트 적용
    QString dialogStyle = QString(
        "QDialog {"
        "    background-color: #EAE6E6;"
        "    border-radius: 15px;"
        "}"
        "QLabel#dialogTitle {"
        "    color: #2F3C56;"
        "    font-family: 'sans-serif';"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QLineEdit#dialogInput {"
        "    background-color: white;"
        "    border: 2px solid #D1D5DB;"
        "    border-radius: 8px;"
        "    padding: 8px 12px;"
        "    font-size: 14px;"
        "    color: #2F3C56;"
        "}"
        "QLineEdit#dialogInput:focus {"
        "    border-color: #2F3C56;"
        "}"
        "QPushButton#dialogButton {"
        "    background-color: #2F3C56;"
        "    color: #EAE6E6;"
        "    font-family: 'sans-serif';"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 6px 16px;"
        "    min-width: 70px;"
        "    min-height: 28px;"
        "}"
        "QPushButton#dialogButton:hover {"
        "    background-color: #394863;"
        "}"
        "QPushButton#dialogButton:pressed {"
        "    background-color: #252F45;"
        "}"
        );

    idDialog.setStyleSheet(dialogStyle);

    // 시그널 연결
    connect(cancelBtn, &QPushButton::clicked, &idDialog, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, &idDialog, &QDialog::accept);
    connect(idLineEdit, &QLineEdit::returnPressed, &idDialog, &QDialog::accept);

    // 다이얼로그 실행
    bool ok1 = false;
    int uid = 1;
    if (idDialog.exec() == QDialog::Accepted) {
        uid = idLineEdit->text().toInt(&ok1);
        if (uid <= 0) ok1 = false;
    }

    if (!ok1) {
        if (statusLabel) statusLabel->setText("등록 취소(사용자 ID 미입력)");
        return;
    }

    // ì‚¬ìš©ìž ì´ë¦„ ìž…ë ¥ - 커스텀 다이얼로그 생성
    QDialog nameDialog(this);
    nameDialog.setWindowTitle("사용자 이름");
    nameDialog.setFixedSize(420, 200);
    nameDialog.setModal(true);

    QVBoxLayout *nameMainLayout = new QVBoxLayout(&nameDialog);
    nameMainLayout->setContentsMargins(20, 20, 20, 20);
    nameMainLayout->setSpacing(15);

    // 제목 라벨
    QLabel *nameTitleLabel = new QLabel("user_name:");
    nameTitleLabel->setObjectName("dialogTitle");

    // 입력 필드
    QLineEdit *nameLineEdit = new QLineEdit();
    nameLineEdit->setObjectName("dialogInput");

    // 버튼 레이아웃
    QHBoxLayout *nameButtonLayout = new QHBoxLayout();
    nameButtonLayout->setSpacing(10);

    QPushButton *nameCancelBtn = new QPushButton("Cancel");
    QPushButton *nameOkBtn = new QPushButton("OK");
    nameCancelBtn->setObjectName("dialogButton");
    nameOkBtn->setObjectName("dialogButton");

    nameButtonLayout->addStretch();
    nameButtonLayout->addWidget(nameCancelBtn);
    nameButtonLayout->addWidget(nameOkBtn);

    nameMainLayout->addWidget(nameTitleLabel);
    nameMainLayout->addWidget(nameLineEdit);
    nameMainLayout->addLayout(nameButtonLayout);

    nameDialog.setStyleSheet(dialogStyle); // 동일한 스타일 적용

    // 시그널 연결
    connect(nameCancelBtn, &QPushButton::clicked, &nameDialog, &QDialog::reject);
    connect(nameOkBtn, &QPushButton::clicked, &nameDialog, &QDialog::accept);
    connect(nameLineEdit, &QLineEdit::returnPressed, &nameDialog, &QDialog::accept);

    // 다이얼로그 실행
    bool ok2 = false;
    QString uname;
    if (nameDialog.exec() == QDialog::Accepted) {
        uname = nameLineEdit->text().trimmed();
        ok2 = !uname.isEmpty();
    }

    if (!ok2) {
        if (statusLabel) statusLabel->setText("등록 취소(사용자 이름 미입력)");
        return;
    }

    currentUserId = uid;
    currentUserName = uname;

    if (!openDb()) return;

    if (!cameraRunning) startCamera(0);

    burst = true;
    burstSaved = 0;

    if (readyLabel)  readyLabel->setText("저장 중... (0/15)");
    if (statusLabel) statusLabel->setText("START");
}

// 상태 초기화 헬퍼 함수
void Certified::setIdleReady() {
    if (statusLabel) statusLabel->setText("READY");
    if (readyLabel)  readyLabel->setText("인증용 사진 저장 준비 완료");
}

//=============================================================================
// EVENT HANDLERS
//=============================================================================

void Certified::onUserAccountClicked()
{
    // 사용자 계정 기능 구현 (카메라 버튼)
}

void Certified::onHomeClicked()
{
    // 메인 화면으로 돌아가는 시그널 발생
    emit backToMain();
}

void Certified::onLockClicked()
{
    emit goToSafety();
}

void Certified::onSearchClicked()
{
    // Search 화면으로 가는 시그널 발생
    emit goToSearch();
}
