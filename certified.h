#ifndef CERTIFIED_H
#define CERTIFIED_H

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
#include <QTimer>
#include <QImage>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <opencv2/opencv.hpp>

class Certified : public QWidget
{
    Q_OBJECT

public:
    explicit Certified(QWidget *parent = nullptr);
    ~Certified();


signals:
    void backToMain();  // 메인 화면으로 돌아가는 시그널
    void goToSafety();  // Safety 화면으로 가는 시그널
    void goToSearch();  // Search 화면으로 가는 시그널 추가

private slots:
    void onUserAccountClicked();
    void onHomeClicked();
    void onLockClicked();
    void onSearchClicked();

    //카메라(추가)
    void startCamera(int index = 0);
    void stopCamera();
    void onFrameTick();
    void onRegisterClicked();

protected: //(추가)
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

private:
    void setupUI();
    void setupStyles();
    void setupFonts();
    void applyShadowEffect(QWidget* widget);
    QPushButton* createCircleButton(const QString& icon);
    QPixmap loadResourceImage(const QString& imagePath, const QSize& size = QSize());

    // Layout creation methods
    void createHeader(QVBoxLayout *canvasLayout);
    void createCertifiedContent(QVBoxLayout *canvasLayout);
    void createMainContentArea(QHBoxLayout *mainLayout);
    void createControlButtons(QHBoxLayout *mainLayout);
    QWidget* createBottomButtons();

    // UI Components
    QWidget *headerWidget;
    QLabel *titleLabel;
    QLabel *statusLabel; // 등록할 때 씀
    QWidget *cameraArea;
    QLabel *cameraLabel; // 등록할 때 씀
    QLabel *readyLabel; // 등록할 때 씀
    QWidget *mainCanvas;
    QWidget *statusWidget;

    // Control buttons
    QPushButton *registerButton; // 등록할 때 씀
    QPushButton *userAccountButton;
    QPushButton *homeButton;
    QPushButton *searchButton;
    QPushButton *lockButton;

    // (추가)
    // ---------- 카메라 ----------
    QTimer *cameraTimer = nullptr;
    cv::VideoCapture cap;
    bool cameraRunning = false;

    // ---------- 버스트 저장 ----------
    bool  burst = false;
    int   burstSaved = 0;
    int   burstTarget = 15;

    // ---------- DB ----------
    QSqlDatabase db;
    bool openDb();                           // DB 오픈 (MySQL)
    bool insertFaceImage(int uid, const QString& uname, const cv::Mat& bgr); // 1장 insert

    int currentUserId = -1;
    QString currentUserName;

    // ---------- 비전 유틸 ----------
    QImage matToQImage(const cv::Mat& m);
    bool loadFaceCascade();
    cv::Mat cropFace128(const cv::Mat& bgr); // 얼굴 있으면 가장 큰 얼굴, 없으면 센터 크롭
    cv::CascadeClassifier faceCascade;
    bool cascadeLoaded = false;
    void setIdleReady();

};

#endif // CERTIFIED_H
