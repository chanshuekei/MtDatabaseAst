#ifndef DATABASEASSISTANT_H
#define DATABASEASSISTANT_H

#include <QObject>
#include "DBADefine.h"
#include "DataBaseHandler.h"
#include <QThread>
#include <QMutex>
#include <QStringList>

class DataBaseAssistant : public QObject
{
    Q_OBJECT
public:
    explicit DataBaseAssistant(QObject* parent = nullptr);
    ~DataBaseAssistant();
signals:
    void sigShowLog(QString);
    void sigShowTables(QStringList);
    // 数据库驱动|数据库名|主机地址|主机端口|用户名|用户密码
    void sigShowDatabaseInfo(QString, QString, QString, int, QString, QString);
public:
    bool Init(TKDBA_SPACE::DATABASE_TYPE type);
    void Deinit();
    bool Open(const QString& driver, const QString& dbName, const QString& host, int port, const QString& user, const QString& password);
    bool Close();
    bool ExeSql(const QString& sql, QString& error);
    bool QuerySql(const QString& sql, QSqlQuery& query, QString& error);
    QStringList GetTableList();
    bool CheckTable(const QString& table);
    void UpdateTableList();
public:
    void TestFunction();
    void TestStationUserTable();
    void TestConnectionNames();
private:
    QMutex m_mutex;
    QThread* m_thread { nullptr };
    DataBaseHandler* m_dbHander { nullptr };
    QStringList m_tableList;
};


#endif // DATABASEASSISTANT_H
