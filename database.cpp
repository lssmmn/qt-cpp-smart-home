#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Database& Database::instance()
{
    static Database db;
    return db;
}

bool Database::connect(const QString& host,
                       const QString& dbName,
                       const QString& user,
                       const QString& password,
                       int port)
{
    if (m_db.isOpen())
        return true;

    m_db = QSqlDatabase::addDatabase("QMYSQL");
    m_db.setHostName(host);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);
    m_db.setPort(port);

    if (!m_db.open()) {
        qDebug() << "DB 연결 실패:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "DB 연결 성공";
    return true;
}

void Database::disconnect()
{
    if (m_db.isOpen()) {
        m_db.close();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
}

Database::~Database()
{
    disconnect();
}


// 온습도 조회
std::optional<QPair<double, double>> Database::getLatestHomeEnv(const QString& homeId)
{
    QSqlQuery query;
    query.prepare("SELECT temperature, humidity FROM home_env "
                  "ORDER BY measured_at DESC LIMIT 1");
    // query.bindValue(":homeId", homeId);

    if (!query.exec()) {
        qDebug() << "쿼리 실패:" << query.lastError().text();
        return std::nullopt;
    }

    if (query.next())
        return QPair<double, double>(query.value(0).toDouble(),
                                     query.value(1).toDouble());

    return std::nullopt;
}

// fire_events 조회
std::optional<QPair<QString, QString>> Database::getLatestFireStatus(const QString& homeId)
{
    QSqlQuery query;
    query.prepare("SELECT fire_status, level_status FROM fire_events "
                  "ORDER BY detected_at DESC LIMIT 1");
    // query.bindValue(":homeId", homeId);

    if (!query.exec()) {
        qDebug() << "쿼리 실패:" << query.lastError().text();
        return std::nullopt;
    }

    if (query.next())
        return QPair<QString, QString>(query.value(0).toString(),
                                       query.value(1).toString());
    return std::nullopt;
}


// 토양수분 조회
std::optional<int> Database::getLatestSoilMoisture(const QString& homeId)
{
    QSqlQuery query;
    query.prepare("SELECT soil_moisture FROM plant_env "
                  "ORDER BY measured_at DESC LIMIT 1");
    // query.bindValue(":homeId", homeId);

    if (!query.exec()) {
        qDebug() << "쿼리 실패:" << query.lastError().text();
        return std::nullopt;
    }

    if (query.next())
        return query.value(0).toInt();

    return std::nullopt;
}

// 반려동물 상태 조회
std::optional<QString> Database::getLatestPetToilet(const QString& homeId)
{
    QSqlQuery query;
    query.prepare("SELECT toilet FROM pet_status "
                  "ORDER BY measured_at DESC LIMIT 1");
    // query.bindValue(":homeId", homeId);

    if (!query.exec()) {
        qDebug() << "쿼리 실패:" << query.lastError().text();
        return std::nullopt;
    }

    if (query.next())
        return query.value(0).toString();

    return std::nullopt;
}

// 현관문 상태
std::optional<QString> Database::getLatestDoorStatus(const QString& homeId)
{
    QSqlQuery query;
    query.prepare("SELECT status FROM door_status "
                  "ORDER BY measured_at DESC LIMIT 1");
    //query.bindValue(":homeId", homeId);

    if (!query.exec()) {
        qDebug() << "쿼리 실패:" << query.lastError().text();
        return std::nullopt;
    }

    if (query.next())
        return query.value(0).toString();

    return std::nullopt;
}

// ---------------------- 집 상태 ----------------------
// 1. 집 상태 검색 메서드 수정
std::optional<QVector<QVector<QString>>> Database::getSearchListHome(const QString& firstDateTime, const QString& lastDateTime)
{
    QVector<QVector<QString>> results;
    QSqlQuery query;

    // DATE() 함수를 제거하고 직접 datetime 비교로 변경
    query.prepare(R"(
        SELECT temperature, humidity, illumination, measured_at
        FROM home_env
        WHERE measured_at >= :firstDateTime
          AND measured_at <= :lastDateTime
        ORDER BY measured_at DESC
    )");
    query.bindValue(":firstDateTime", firstDateTime);
    query.bindValue(":lastDateTime", lastDateTime);

    if (!query.exec()) {
        qDebug() << "getSearchListHome 실패:" << query.lastError().text();
        return std::nullopt;
    }

    while (query.next()) {
        QVector<QString> row;
        row << query.value(0).toString()  // 온도
            << query.value(1).toString()  // 습도
            << query.value(2).toString()  // 조도
            << query.value(3).toString(); // 감지시간
        results.push_back(row);
    }

    return results;
}

// 2. 화재 검색 메서드 수정
std::optional<QVector<QVector<QString>>> Database::getSearchListFire(const QString& firstDateTime, const QString& lastDateTime)
{
    QVector<QVector<QString>> results;
    QSqlQuery query;

    // DATE() 함수를 제거하고 직접 datetime 비교로 변경
    query.prepare(R"(
        SELECT fire_status, fire_level, detected_at
        FROM fire_events
        WHERE detected_at >= :firstDateTime
          AND detected_at <= :lastDateTime
        ORDER BY detected_at DESC
    )");
    query.bindValue(":firstDateTime", firstDateTime);
    query.bindValue(":lastDateTime", lastDateTime);

    if (!query.exec()) {
        qDebug() << "getSearchListFire 실패:" << query.lastError().text();
        return std::nullopt;
    }

    while (query.next()) {
        QVector<QString> row;
        row << query.value(0).toString()  // 화재상태
            << query.value(1).toString()  // 화재수치
            << query.value(2).toString(); // 감지시간
        results.push_back(row);
    }

    return results;
}

// 3. 가스 검색 메서드 수정
std::optional<QVector<QVector<QString>>> Database::getSearchListGas(const QString& firstDateTime, const QString& lastDateTime)
{
    QVector<QVector<QString>> results;
    QSqlQuery query;

    // DATE() 함수를 제거하고 직접 datetime 비교로 변경
    query.prepare(R"(
        SELECT level_status, level, detected_at
        FROM fire_events
        WHERE detected_at >= :firstDateTime
          AND detected_at <= :lastDateTime
        ORDER BY detected_at DESC
    )");
    query.bindValue(":firstDateTime", firstDateTime);
    query.bindValue(":lastDateTime", lastDateTime);

    if (!query.exec()) {
        qDebug() << "getSearchListGas 실패:" << query.lastError().text();
        return std::nullopt;
    }

    while (query.next()) {
        QVector<QString> row;
        row << query.value(0).toString()  // 가스누출상태
            << query.value(1).toString()  // 가스수치
            << query.value(2).toString(); // 감지시간
        results.push_back(row);
    }

    return results;
}

// 4. 식물 검색 메서드 수정
std::optional<QVector<QVector<QString>>> Database::getSearchListPlant(const QString& firstDateTime, const QString& lastDateTime)
{
    QVector<QVector<QString>> results;
    QSqlQuery query;

    // DATE() 함수를 제거하고 직접 datetime 비교로 변경
    query.prepare(R"(
        SELECT soil_moisture, measured_at
        FROM plant_env
        WHERE measured_at >= :firstDateTime
          AND measured_at <= :lastDateTime
        ORDER BY measured_at DESC
    )");
    query.bindValue(":firstDateTime", firstDateTime);
    query.bindValue(":lastDateTime", lastDateTime);

    if (!query.exec()) {
        qDebug() << "getSearchListPlant 실패:" << query.lastError().text();
        return std::nullopt;
    }

    while (query.next()) {
        QVector<QString> row;
        row << query.value(0).toString()  // 토양습도
            << query.value(1).toString(); // 감지시간
        results.push_back(row);
    }

    return results;
}

// 5. 펫 검색 메서드 수정
std::optional<QVector<QVector<QString>>> Database::getSearchListPet(const QString& firstDateTime, const QString& lastDateTime)
{
    QVector<QVector<QString>> results;
    QSqlQuery query;

    // DATE() 함수를 제거하고 직접 datetime 비교로 변경
    query.prepare(R"(
        SELECT food, water, toilet, measured_at
        FROM pet_status
        WHERE measured_at >= :firstDateTime
          AND measured_at <= :lastDateTime
        ORDER BY measured_at DESC
    )");
    query.bindValue(":firstDateTime", firstDateTime);
    query.bindValue(":lastDateTime", lastDateTime);

    if (!query.exec()) {
        qDebug() << "getSearchListPet 실패:" << query.lastError().text();
        return std::nullopt;
    }

    while (query.next()) {
        QVector<QString> row;
        row << query.value(0).toString()  // 급식
            << query.value(1).toString()  // 급수
            << query.value(2).toString()  // 배변
            << query.value(3).toString(); // 감지시간
        results.push_back(row);
    }

    return results;
}
