#include "databaseassistant.h"
#include "IniHandler.h"
#include "easylogging++.h"
#include "DBHandlerFactory.h"
#include <QMutexLocker>
#pragma execution_character_set("utf-8")
using namespace::TKDBA_SPACE;

#ifndef X
#define X(log) QString::fromLocal8Bit(log)
#endif // !X


DataBaseAssistant::DataBaseAssistant(QObject* parent)
    : QObject { parent }
{
    // 启动线程
    m_thread = new QThread();
    this->moveToThread(m_thread);
    m_thread->start();
}

DataBaseAssistant::~DataBaseAssistant()
{
    Deinit();
}

bool DataBaseAssistant::Init(TKDBA_SPACE::DATABASE_TYPE type)
{
    QMutexLocker locker(&m_mutex);
    // 创建控制模块
    m_dbHander = DBHandlerFactory::CreateHandler(type);
    if(nullptr == m_dbHander)
    {
        return false;
    }
    // 连接数据库
    auto ok = m_dbHander->Open();
    if(ok)
    {
        auto db = m_dbHander->GetSqlDatabase();
        auto driver = db.driverName();
        auto dbName = db.databaseName();
        auto host = db.hostName();
        auto port = db.port();
        auto user = db.userName();
        auto password = db.password();
        auto connName = db.connectionName();
        emit sigShowDatabaseInfo(driver, dbName, host, port, user, password);
        LOG(DEBUG) << X("databaseName:%1, driverName:%2, hostName:%3, port:%4, userName:%5, password:%6, connectionName:%7")
            .arg(dbName)
            .arg(driver)
            .arg(host)
            .arg(port)
            .arg(user)
            .arg(password)
            .arg(connName);
    }
    return ok;
}

void DataBaseAssistant::Deinit()
{
    QMutexLocker locker(&m_mutex);
    if(nullptr != m_dbHander)
    {
        m_dbHander->Close();
        delete m_dbHander;
        m_dbHander = nullptr;
    }

    if(nullptr != m_thread)
    {
        m_thread->requestInterruption();
        m_thread->quit();
        m_thread->wait();
        m_thread->deleteLater();
    }
}

bool DataBaseAssistant::Open(const QString& driver, const QString& dbName, const QString& host, int port, const QString& user, const QString& password)
{
    return m_dbHander->Open(driver, dbName, host, port, user, password);
}

bool DataBaseAssistant::Close()
{
    return m_dbHander->Close();
}

bool DataBaseAssistant::ExeSql(const QString& sql, QString& error)
{
    QMutexLocker locker(&m_mutex);
    if(nullptr == m_dbHander || !m_dbHander->IsEnable())
    {
        error = X("数据库状态异常");
        return false;
    }

    return m_dbHander->ExeSql(sql, error);
}

bool DataBaseAssistant::QuerySql(const QString& sql, QSqlQuery& query, QString& error)
{
    QMutexLocker locker(&m_mutex);
    if(nullptr == m_dbHander || !m_dbHander->IsEnable())
    {
        emit sigShowLog(X("数据库状态异常"));
        return false;
    }

    return m_dbHander->QuerySql(sql, query, error);
}

QStringList DataBaseAssistant::GetTableList()
{
    QMutexLocker locker(&m_mutex);
    if(nullptr == m_dbHander || !m_dbHander->IsEnable())
    {
        emit sigShowLog(X("数据库状态异常"));
        return QStringList();
    }

    QStringList tablelist;
    auto db = m_dbHander->GetSqlDatabase();
    QSqlQuery query(db);
    if(query.exec("SELECT TABLE_NAME FROM USER_TABLES"))
    {
        while(query.next())
        {
            tablelist.append(query.value(0).toString());
        }

    }
    else
    {
        emit sigShowLog(X("获取数据库表失败:") + query.lastError().text());
        return QStringList();
    }
    // 保存数据
    m_tableList = tablelist;
    emit sigShowLog(X("获取数据库表:") + tablelist.join("|"));
    return tablelist;
}

bool DataBaseAssistant::CheckTable(const QString& table)
{
    // SQLITE不支持
    if(nullptr == m_dbHander || !m_dbHander->IsEnable())
    {
        return false;
    }

    if(m_tableList.isEmpty())
    {
        GetTableList();
    }

    // 不区分大小写
    return m_tableList.contains(table, Qt::CaseInsensitive);
}

void DataBaseAssistant::UpdateTableList()
{
    emit sigShowTables(m_tableList);
}

void DataBaseAssistant::TestFunction()
{
    GetTableList();
    UpdateTableList();
    //TestStationUserTable();
    TestConnectionNames();
}

void DataBaseAssistant::TestConnectionNames()
{
    QMutexLocker locker(&m_mutex);
    if(nullptr == m_dbHander || !m_dbHander->IsEnable())
    {
        return;
    }

    auto db = m_dbHander->GetSqlDatabase();
    auto showConnNames = db.connectionNames().join("|");
    emit sigShowLog(X("连接数据库名:") + showConnNames);
}

void DataBaseAssistant::TestStationUserTable()
{
    QMutexLocker locker(&m_mutex);
    if(nullptr == m_dbHander || !m_dbHander->IsEnable())
    {
        emit sigShowLog(X("数据库状态异常"));
        return;
    }

    // UPLOAD_IMAGE_RECORD|UPLOAD_EXCEL_RECORD|TPM_EQUIPMENT_STATUS_INFO|TPM_EQUIPMENT_OP_TIME|TPM_EQUIPMENT_OP_INFO|TPM_EQUIPMENT_ALARM_INFO|CENTRALUSER|CACIBRATION_FLAG|T_CHKRESULTMAIN|T_CHKRESULTITEM|STATIONUSER|STATIONDETECTION|PRODUCT_TESTED|PRODUCT_OP_RESULT|PRODUCT_OP|PM_EQUIPMENT_FAULT_INFO|T_OUT_PKTOTLE|T_CHKTASK|MATERIAL_CONSUMPTION|EXCEPTION_REPORT|EVENTLOG|EARYWARNING_PARAM|DAILY_PRODUCTING_TIME
    auto db = m_dbHander->GetSqlDatabase();
    QSqlQuery query(db);
    if(query.exec("SELECT * FROM STATIONUSER"))
    {
        while(query.next())
        {
            emit sigShowLog(X("获取到ID=%1,LOGIN_NAME=%2,LOGIN_PASSWORD=%3")
                            .arg(query.value("ID").toInt())
                            .arg(query.value("LOGIN_NAME").toString())
                            .arg(query.value("LOGIN_PASSWORD").toString()));
        }
    }
    else
    {
        emit sigShowLog(X("获取数据库表失败:") + query.lastError().text());
    }
}
