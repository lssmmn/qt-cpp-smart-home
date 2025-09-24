#include "search.h"
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QApplication>
#include <QDebug>

// 색상 상수들 (다른 파일들과 동일)
const QColor BackgroundGray(0xEA, 0xE6, 0xE6);
const QColor DarkNavy(0x2F, 0x3C, 0x56);
const QColor CardWhite(0xFF, 0xFF, 0xFF);
const QColor StatusGray(0xF5, 0xF5, 0xF5);
const QColor CameraAreaGray(0xE8, 0xE8, 0xE8);

Search::Search(QWidget *parent) : QWidget(parent)
{
    setupFonts();
    setupUI();
    setupStyles();
}

Search::~Search()
{
}

void Search::setupFonts()
{
    QFont appFont("sans-serif", 10);
    QApplication::setFont(appFont);
    setFont(appFont);
}

void Search::setupUI()
{
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    setFixedSize(screenGeometry.size());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 80);
    mainLayout->setSpacing(30);

    mainCanvas = new QWidget();
    mainCanvas->setObjectName("mainCanvas");
    applyShadowEffect(mainCanvas);

    QVBoxLayout *canvasLayout = new QVBoxLayout(mainCanvas);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->setSpacing(0);

    createHeader(canvasLayout);
    createSearchContent(canvasLayout);

    mainLayout->addWidget(mainCanvas);
}

void Search::createHeader(QVBoxLayout *canvasLayout)
{
    headerWidget = new QWidget();
    headerWidget->setObjectName("headerWidget");
    headerWidget->setFixedHeight(120);

    titleLabel = new QLabel("Search");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addWidget(titleLabel);

    canvasLayout->addWidget(headerWidget);
}

void Search::createSearchContent(QVBoxLayout *canvasLayout)
{
    QWidget *contentWidget = new QWidget();
    contentWidget->setObjectName("searchContent");

    QHBoxLayout *mainContentLayout = new QHBoxLayout(contentWidget);
    mainContentLayout->setContentsMargins(40, 20, 20, 40);
    mainContentLayout->setSpacing(30);

    createMainContentArea(mainContentLayout);
    createControlButtons(mainContentLayout);

    canvasLayout->addWidget(contentWidget, 1);
}

void Search::createMainContentArea(QHBoxLayout *mainLayout)
{
    QWidget *leftArea = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftArea);
    leftLayout->setSpacing(25);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    // 기존 검색 컨트롤 영역 생성
    createSearchControls(leftLayout);

    // 기존 검색 결과 테이블 생성
    createSearchTable(leftLayout);

    mainLayout->addWidget(leftArea, 1);
}

void Search::createSearchControls(QVBoxLayout *leftLayout)
{
    // 검색 컨트롤 컨테이너
    QWidget *searchControlsWidget = new QWidget();
    searchControlsWidget->setObjectName("searchControls");
    searchControlsWidget->setFixedHeight(80);

    QHBoxLayout *controlsLayout = new QHBoxLayout(searchControlsWidget);
    controlsLayout->setContentsMargins(30, 20, 30, 20);
    controlsLayout->setSpacing(20);

    controlsLayout->addSpacing(50);

    // 카테고리 콤보박스
    categoryComboBox = new QComboBox();
    categoryComboBox->setObjectName("categoryComboBox");
    categoryComboBox->setFixedSize(150, 40);
    categoryComboBox->addItems({"온도", "화재", "가스", "식물", "펫"});

    // 시작일시 - QDateTimeEdit으로 변경
    startDateTimeEdit = new QDateTimeEdit();
    startDateTimeEdit->setObjectName("dateTimeEdit");
    startDateTimeEdit->setFixedSize(200, 40);  // 너비를 늘려서 시간까지 표시
    startDateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-7)); // 일주일 전
    startDateTimeEdit->setDisplayFormat("yyyy.MM.dd hh:mm"); // 시간까지 표시
    startDateTimeEdit->setCalendarPopup(true);

    // 하이픈 라벨
    QLabel *dashLabel = new QLabel("-");
    dashLabel->setObjectName("dashLabel");
    dashLabel->setAlignment(Qt::AlignCenter);
    dashLabel->setFixedSize(20, 40);

    // 종료일시 - QDateTimeEdit으로 변경
    endDateTimeEdit = new QDateTimeEdit();
    endDateTimeEdit->setObjectName("dateTimeEdit");
    endDateTimeEdit->setFixedSize(200, 40);  // 너비를 늘려서 시간까지 표시
    endDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    endDateTimeEdit->setDisplayFormat("yyyy.MM.dd hh:mm"); // 시간까지 표시
    endDateTimeEdit->setCalendarPopup(true);

    // 조회 버튼
    searchButton = new QPushButton("조회");
    searchButton->setObjectName("searchButton");
    searchButton->setFixedSize(100, 40);

    controlsLayout->addWidget(categoryComboBox);
    controlsLayout->addWidget(startDateTimeEdit);
    controlsLayout->addWidget(dashLabel);
    controlsLayout->addWidget(endDateTimeEdit);
    controlsLayout->addWidget(searchButton);
    controlsLayout->addStretch();

    // 시그널 연결
    connect(searchButton, &QPushButton::clicked, this, &Search::onSearchClicked);
    connect(categoryComboBox, &QComboBox::currentTextChanged, this, &Search::onCategoryChanged);

    leftLayout->addWidget(searchControlsWidget);
}

void Search::createSearchTable(QVBoxLayout *leftLayout)
{
    // 테이블 위젯 생성
    resultsTable = new QTableWidget();
    resultsTable->setObjectName("resultsTable");
    resultsTable->setColumnCount(3);

    // 테이블 헤더 설정
    QStringList headers;
    headers << "날짜" << "분류" << "값";
    resultsTable->setHorizontalHeaderLabels(headers);

    // 테이블 스타일 설정
    resultsTable->horizontalHeader()->setStretchLastSection(true);
    resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    // 컬럼 너비 설정
    resultsTable->setColumnWidth(0, 200);
    resultsTable->setColumnWidth(1, 150);

    // 테이블 속성 설정
    resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable->setAlternatingRowColors(true);
    resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsTable->verticalHeader()->setVisible(false);

    leftLayout->addWidget(resultsTable, 1);

    // 초기 데이터 로드
    loadSearchData();
}

void Search::createControlButtons(QHBoxLayout *mainLayout)
{
    QWidget *buttonsArea = new QWidget();
    buttonsArea->setFixedWidth(100);

    QVBoxLayout *buttonsLayout = new QVBoxLayout(buttonsArea);
    buttonsLayout->setSpacing(70);
    buttonsLayout->setContentsMargins(0, 20, 0, 20);
    buttonsLayout->setAlignment(Qt::AlignCenter);

    cameraButton = createCircleButton("camera");
    homeButton = createCircleButton("home");
    searchNavButton = createCircleButton("search");
    lockButton = createCircleButton("lock");

    // search 버튼을 다른 색으로 표시
    searchNavButton->setObjectName("grayCircleButton");

    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(cameraButton);
    buttonsLayout->addWidget(homeButton);
    buttonsLayout->addWidget(searchNavButton);
    buttonsLayout->addWidget(lockButton);
    buttonsLayout->addStretch(1);

    connect(homeButton, &QPushButton::clicked, this, &Search::onHomeClicked);
    connect(lockButton, &QPushButton::clicked, this, &Search::onLockClicked);
    connect(cameraButton, &QPushButton::clicked, this, &Search::onSafetyClicked);

    mainLayout->addWidget(buttonsArea, 0);
}

void Search::onSearchClicked()
{
    loadSearchData();
}

void Search::onCategoryChanged(const QString& category)
{
    // 카테고리 변경 시 즉시 검색 실행
    loadSearchData();
}

void Search::loadSearchData()
{
    // yyyy-MM-dd HH:mm:ss 형식으로 변경
    QString startDateTime = startDateTimeEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString endDateTime = endDateTimeEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString category = categoryComboBox->currentText();

    clearTable();

    if (category == "온도") {
        populateHomeData(startDateTime, endDateTime);
    } else if (category == "화재") {
        populateFireData(startDateTime, endDateTime);
    } else if (category == "가스") {
        populateGasData(startDateTime, endDateTime);
    } else if (category == "식물") {
        populatePlantData(startDateTime, endDateTime);
    } else if (category == "펫") {
        populatePetData(startDateTime, endDateTime);
    }
}

void Search::populateHomeData(const QString& startDateTime, const QString& endDateTime)
{
    std::optional<QVector<QVector<QString>>> homeList = Database::instance().getSearchListHome(startDateTime, endDateTime);
    if (homeList) {
        resultsTable->setRowCount(homeList->size());

        for (int i = 0; i < homeList->size(); ++i) {
            const QVector<QString>& row = homeList->at(i);

            // 날짜시간 (감지시각) - 형식 변경
            QString formattedDateTime = row.size() > 3 ? formatDateTime(row[3]) : "";
            QTableWidgetItem *dateItem = new QTableWidgetItem(formattedDateTime);
            resultsTable->setItem(i, 0, dateItem);

            // 분류
            QTableWidgetItem *categoryItem = new QTableWidgetItem("온도");
            resultsTable->setItem(i, 1, categoryItem);

            // 값 (온도)
            QString value = row.size() > 0 ? (row[0] + "°C") : "";
            QTableWidgetItem *valueItem = new QTableWidgetItem(value);
            resultsTable->setItem(i, 2, valueItem);
        }
    }
}

void Search::populateFireData(const QString& startDateTime, const QString& endDateTime)
{
    std::optional<QVector<QVector<QString>>> fireList = Database::instance().getSearchListFire(startDateTime, endDateTime);
    if (fireList) {
        resultsTable->setRowCount(fireList->size());

        for (int i = 0; i < fireList->size(); ++i) {
            const QVector<QString>& row = fireList->at(i);

            // 날짜시간 - 형식 변경
            QString formattedDateTime = row.size() > 2 ? formatDateTime(row[2]) : "";
            QTableWidgetItem *dateItem = new QTableWidgetItem(formattedDateTime);
            resultsTable->setItem(i, 0, dateItem);

            // 분류
            QTableWidgetItem *categoryItem = new QTableWidgetItem("화재");
            resultsTable->setItem(i, 1, categoryItem);

            // 값 (화재상태 + 화재수치)
            QString value = "";
            if (row.size() > 1) {
                value = row[0] + " (" + row[1] + ")";
            }
            QTableWidgetItem *valueItem = new QTableWidgetItem(value);
            resultsTable->setItem(i, 2, valueItem);
        }
    }
}

void Search::populateGasData(const QString& startDateTime, const QString& endDateTime)
{
    std::optional<QVector<QVector<QString>>> gasList = Database::instance().getSearchListGas(startDateTime, endDateTime);
    if (gasList) {
        resultsTable->setRowCount(gasList->size());

        for (int i = 0; i < gasList->size(); ++i) {
            const QVector<QString>& row = gasList->at(i);

            // 날짜시간 - 형식 변경
            QString formattedDateTime = row.size() > 2 ? formatDateTime(row[2]) : "";
            QTableWidgetItem *dateItem = new QTableWidgetItem(formattedDateTime);
            resultsTable->setItem(i, 0, dateItem);

            // 분류
            QTableWidgetItem *categoryItem = new QTableWidgetItem("가스");
            resultsTable->setItem(i, 1, categoryItem);

            // 값 (가스누출상태 + 가스수치)
            QString value = "";
            if (row.size() > 1) {
                value = row[0] + " (" + row[1] + ")";
            }
            QTableWidgetItem *valueItem = new QTableWidgetItem(value);
            resultsTable->setItem(i, 2, valueItem);
        }
    }
}

void Search::populatePlantData(const QString& startDateTime, const QString& endDateTime)
{
    std::optional<QVector<QVector<QString>>> plantList = Database::instance().getSearchListPlant(startDateTime, endDateTime);
    if (plantList) {
        resultsTable->setRowCount(plantList->size());

        for (int i = 0; i < plantList->size(); ++i) {
            const QVector<QString>& row = plantList->at(i);

            // 날짜시간 - 형식 변경
            QString formattedDateTime = row.size() > 1 ? formatDateTime(row[1]) : "";
            QTableWidgetItem *dateItem = new QTableWidgetItem(formattedDateTime);
            resultsTable->setItem(i, 0, dateItem);

            // 분류
            QTableWidgetItem *categoryItem = new QTableWidgetItem("식물");
            resultsTable->setItem(i, 1, categoryItem);

            // 값 (토양습도)
            QString value = row.size() > 0 ? (row[0] + "%") : "";
            QTableWidgetItem *valueItem = new QTableWidgetItem(value);
            resultsTable->setItem(i, 2, valueItem);
        }
    }
}

void Search::populatePetData(const QString& startDateTime, const QString& endDateTime)
{
    std::optional<QVector<QVector<QString>>> petList = Database::instance().getSearchListPet(startDateTime, endDateTime);
    if (petList) {
        resultsTable->setRowCount(petList->size() * 3); // 급식, 급수, 배변으로 3개 행

        int tableRow = 0;
        for (int i = 0; i < petList->size(); ++i) {
            const QVector<QString>& row = petList->at(i);
            QString formattedDateTime = row.size() > 3 ? formatDateTime(row[3]) : "";

            // 급식 행
            resultsTable->setItem(tableRow, 0, new QTableWidgetItem(formattedDateTime));
            resultsTable->setItem(tableRow, 1, new QTableWidgetItem("펫(급식)"));
            resultsTable->setItem(tableRow, 2, new QTableWidgetItem(row.size() > 0 ? row[0] : ""));
            tableRow++;

            // 급수 행
            resultsTable->setItem(tableRow, 0, new QTableWidgetItem(formattedDateTime));
            resultsTable->setItem(tableRow, 1, new QTableWidgetItem("펫(급수)"));
            resultsTable->setItem(tableRow, 2, new QTableWidgetItem(row.size() > 1 ? row[1] : ""));
            tableRow++;

            // 배변 행
            resultsTable->setItem(tableRow, 0, new QTableWidgetItem(formattedDateTime));
            resultsTable->setItem(tableRow, 1, new QTableWidgetItem("펫(배변)"));
            resultsTable->setItem(tableRow, 2, new QTableWidgetItem(row.size() > 2 ? row[2] : ""));
            tableRow++;
        }
    }
}

void Search::clearTable()
{
    resultsTable->setRowCount(0);
}

QString Search::formatDateTime(const QString& dateTimeStr)
{
    // "2025-09-10T11:27:57.000Z" -> "2025-09-10 11:27"
    QDateTime dt = QDateTime::fromString(dateTimeStr, Qt::ISODate);
    if (dt.isValid()) {
        return dt.toString("yyyy-MM-dd hh:mm");
    }

    // ISO 형식이 아닌 경우 다른 형식 시도
    dt = QDateTime::fromString(dateTimeStr, "yyyy-MM-ddThh:mm:ss.zzzZ");
    if (dt.isValid()) {
        return dt.toString("yyyy-MM-dd hh:mm");
    }

    // MySQL DATETIME 형식 시도 (YYYY-MM-DD HH:MM:SS)
    dt = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd hh:mm:ss");
    if (dt.isValid()) {
        return dt.toString("yyyy-MM-dd hh:mm");
    }

    // 초단위까지 포함된 형식 시도
    dt = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd hh:mm:ss.zzz");
    if (dt.isValid()) {
        return dt.toString("yyyy-MM-dd hh:mm");
    }

    // 변환 실패 시 원본 반환
    return dateTimeStr;
}

QPushButton* Search::createCircleButton(const QString& iconPath)
{
    QPushButton *button = new QPushButton();
    button->setObjectName("circleButton");
    button->setFixedSize(60, 60);

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

    if (!iconPixmap.isNull()) {
        button->setIcon(QIcon(iconPixmap));
        button->setIconSize(QSize(30, 30));
    } else {
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

QPixmap Search::loadResourceImage(const QString& imagePath, const QSize& size)
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

void Search::applyShadowEffect(QWidget* widget)
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(15);
    shadow->setXOffset(0);
    shadow->setYOffset(5);
    shadow->setColor(QColor(0, 0, 0, 30));
    widget->setGraphicsEffect(shadow);
}

void Search::setupStyles()
{
    QString styles = QString(
                         "Search { background-color: %1; }"

                         "QWidget#mainCanvas {"
                         "    background-color: white;"
                         "    border-radius: 28px;"
                         "}"

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

                         // 검색 컨트롤 영역
                         "QWidget#searchControls {"
                         "    background-color: %3;"
                         "    border-radius: 15px;"
                         "    margin: 0px 40px;"
                         "}"

                         // 콤보박스 스타일 - 선택 가시성 개선
                         "QComboBox#categoryComboBox {"
                         "    background-color: white;"
                         "    border: 2px solid %3;"
                         "    border-radius: 8px;"
                         "    padding: 8px 12px;"
                         "    font-size: 14px;"
                         "    font-weight: bold;"
                         "    color: %2;"
                         "}"

                         "QComboBox#categoryComboBox::drop-down {"
                         "    border: none;"
                         "    width: 30px;"
                         "}"

                         "QComboBox#categoryComboBox::down-arrow {"
                         "    image: none;"
                         "    border-left: 5px solid transparent;"
                         "    border-right: 5px solid transparent;"
                         "    border-top: 5px solid %2;"
                         "    width: 0px;"
                         "    height: 0px;"
                         "}"

                         // 드롭다운 전체 컨테이너 - selection 속성 제거
                         "QComboBox#categoryComboBox QAbstractItemView {"
                         "    background-color: white;"
                         "    border: 2px solid %3;"
                         "    border-radius: 8px;"
                         "    outline: none;"
                         "    font-weight: bold;"
                         "    padding: 4px;"
                         "    alternate-background-color: #FAFAFA;"
                         "}"

                         // 개별 항목 기본 상태 - 명확한 색상 정의
                         "QComboBox#categoryComboBox QAbstractItemView::item {"
                         "    padding: 12px 16px;"
                         "    border: 1px solid transparent;"
                         "    color: #2F3C56;"              // 직접 네이비 색상 지정
                         "    background-color: white;"     // 직접 흰색 지정
                         "    margin: 1px;"
                         "    border-radius: 4px;"
                         "}"

                         // 호버 상태 - 완전히 다른 색상 사용
                         "QComboBox#categoryComboBox QAbstractItemView::item:hover {"
                         "    background-color: #F5F5F5;"   // 연한 회색
                         "    color: #2F3C56;"              // 네이비
                         "    border: 2px solid #2F3C56;"
                         "}"

                         "QComboBox#categoryComboBox QAbstractItemView::item:selected {"
                         "    background-color: #2F3C56;"   // 네이비 배경
                         "    color: white;"                // 흰색 텍스트
                         "    border: 2px solid #4CAF50;"   // 녹색 테두리로 구분
                         "    font-weight: bold;"
                         "}"

                         // 현재 선택된 항목이 강조된 상태
                         "QComboBox#categoryComboBox QAbstractItemView::item:selected:focus {"
                         "    background-color: #0D47A1;"   // 더 진한 파란색
                         "    color: white;"
                         "    outline: none;"
                         "}"

                         // 날짜 선택기 스타일
                         "QDateTimeEdit#dateTimeEdit {"
                         "    background-color: white;"
                         "    border: 2px solid %3;"
                         "    border-radius: 8px;"
                         "    padding: 8px 12px;"
                         "    font-size: 12px;"  // 폰트 크기를 약간 줄여서 시간까지 표시
                         "    font-weight: bold;"
                         "    color: %2;"
                         "}"

                         "QDateTimeEdit#dateTimeEdit::drop-down {"
                         "    border: none;"
                         "    width: 30px;"
                         "}"

                         "QDateTimeEdit#dateTimeEdit::down-arrow {"
                         "    image: none;"
                         "    border-left: 5px solid transparent;"
                         "    border-right: 5px solid transparent;"
                         "    border-top: 5px solid %2;"
                         "    width: 0px;"
                         "    height: 0px;"
                         "}"

                         // 하이픈 라벨
                         "QLabel#dashLabel {"
                         "    color: %2;"
                         "    font-size: 18px;"
                         "    font-weight: bold;"
                         "}"

                         // 검색 버튼
                         "QPushButton#searchButton {"
                         "    background-color: %2;"
                         "    color: white;"
                         "    font-size: 14px;"
                         "    font-weight: bold;"
                         "    border: none;"
                         "    border-radius: 8px;"
                         "}"

                         "QPushButton#searchButton:hover {"
                         "    background-color: %4;"
                         "}"

                         "QPushButton#searchButton:pressed {"
                         "    background-color: %5;"
                         "}"

                         // 테이블 스타일
                         "QTableWidget#resultsTable {"
                         "    background-color: white;"
                         "    border: 2px solid %3;"
                         "    border-radius: 15px;"
                         "    gridline-color: %3;"
                         "    selection-background-color: %6;"
                         "    selection-color: %2;"
                         "    font-size: 13px;"
                         "    margin: 0px;"
                         "}"

                         "QTableWidget#resultsTable::item {"
                         "    padding: 10px;"
                         "    border-bottom: 1px solid %3;"
                         "}"

                         "QTableWidget#resultsTable::item:selected {"
                         "    background-color: %6;"
                         "    color: %2;"
                         "}"

                         "QTableWidget#resultsTable::item:alternate {"
                         "    background-color: #FAFAFA;"
                         "}"

                         // 테이블 헤더
                         "QHeaderView::section {"
                         "    background-color: %2;"
                         "    color: white;"
                         "    font-size: 14px;"
                         "    font-weight: bold;"
                         "    padding: 12px;"
                         "    border: none;"
                         "    border-right: 1px solid %3;"
                         "}"

                         "QHeaderView::section:first {"
                         "    border-top-left-radius: 13px;"
                         "}"

                         "QHeaderView::section:last {"
                         "    border-top-right-radius: 13px;"
                         "    border-right: none;"
                         "}"

                         // 원형 버튼들
                         "QPushButton#circleButton {"
                         "    background-color: #EAE6E6;"
                         "    border-radius: 30px;"
                         "    font-size: 20px;"
                         "    border: none;"
                         "}"

                         "QPushButton#circleButton:hover {"
                         "    background-color: %4;"
                         "}"

                         "QPushButton#grayCircleButton {"
                         "    background-color: #EAE6E6;"
                         "    color: %2;"
                         "    border-radius: 30px;"
                         "    font-size: 20px;"
                         "    border: none;"
                         "}"

                         ).arg(BackgroundGray.name(),               // %1 - Background
                              DarkNavy.name(),                     // %2 - Navy (#2F3C56)
                              StatusGray.name(),                   // %3 - Light gray (#F5F5F5)
                              DarkNavy.lighter(120).name(),        // %4 - Hover (밝은 네이비)
                              DarkNavy.darker(120).name(),         // %5 - Pressed (어두운 네이비)
                              DarkNavy.lighter(150).name());       // %6 - Selection (매우 밝은 네이비)

    setStyleSheet(styles);
}

void Search::onHomeClicked()
{
    emit backToMain();
}

void Search::onLockClicked()
{
    // Certified 화면으로 가는 시그널 발생
    emit goToCertified();
}

void Search::onSafetyClicked()
{
    // Safety 화면으로 가는 시그널 발생
    emit goToSafety();
}
