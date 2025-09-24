// search.h에서 수정할 부분

#ifndef SEARCH_H
#define SEARCH_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QGraphicsDropShadowEffect>
#include <QComboBox>
#include <QDateTimeEdit>  // QDateEdit에서 QDateTimeEdit으로 변경
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDate>
#include <QDateTime>      // QDateTime 헤더 추가
#include "database.h"

class Search : public QWidget
{
    Q_OBJECT

public:
    explicit Search(QWidget *parent = nullptr);
    ~Search();

signals:
    void backToMain();  // 메인 화면으로 돌아가는 시그널
    void goToCertified();
    void goToSafety();

private slots:
    void onHomeClicked();
    void onLockClicked();
    void onSafetyClicked();
    void onSearchClicked();  // 조회 버튼 클릭
    void onCategoryChanged(const QString& category);  // 카테고리 변경

private:
    void setupUI();
    void setupStyles();
    void setupFonts();
    void applyShadowEffect(QWidget* widget);
    QPushButton* createCircleButton(const QString& icon);
    QPixmap loadResourceImage(const QString& imagePath, const QSize& size = QSize());
    QString formatDateTime(const QString& dateTimeStr);

    // Layout creation methods
    void createHeader(QVBoxLayout *canvasLayout);
    void createSearchContent(QVBoxLayout *canvasLayout);
    void createMainContentArea(QHBoxLayout *mainLayout);
    void createControlButtons(QHBoxLayout *mainLayout);
    void createSearchControls(QVBoxLayout *leftLayout);
    void createSearchTable(QVBoxLayout *leftLayout);

    // Data loading methods
    void loadSearchData();
    void populateHomeData(const QString& startDateTime, const QString& endDateTime);
    void populateFireData(const QString& startDateTime, const QString& endDateTime);
    void populateGasData(const QString& startDateTime, const QString& endDateTime);
    void populatePlantData(const QString& startDateTime, const QString& endDateTime);
    void populatePetData(const QString& startDateTime, const QString& endDateTime);
    void clearTable();

    // UI Components
    QWidget *headerWidget;
    QLabel *titleLabel;
    QWidget *mainCanvas;
    QWidget *contentArea;

    QComboBox *categoryComboBox;
    QDateTimeEdit *startDateTimeEdit;  // 변경된 멤버 변수명
    QDateTimeEdit *endDateTimeEdit;    // 변경된 멤버 변수명
    QPushButton *searchButton;

    // Search results table
    QTableWidget *resultsTable;

    // Control buttons
    QPushButton *cameraButton;
    QPushButton *homeButton;
    QPushButton *searchNavButton;
    QPushButton *lockButton;
};

#endif // SEARCH_H
