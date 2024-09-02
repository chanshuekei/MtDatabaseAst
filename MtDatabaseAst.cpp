#include "MtDatabaseAst.h"
#include "easylogging++.h"
#include "IniHandler.h"
#include "DBHandlerFactory.h"
#include <QDateTime>
#pragma execution_character_set("utf-8")

MtDatabaseAst::MtDatabaseAst(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowTitle(X("MTDBA(���ݿ�����)@CHANSHUEKEI"));
    ui.pteLOG->setReadOnly(true);
    Init();
}

MtDatabaseAst::~MtDatabaseAst()
{
    Deinit();
}

void MtDatabaseAst::Init()
{
    // ��ʼ�������ļ�
    IniHandler::GetInstance().Init();
    // ��ȡ���ݿ�����
    // auto type = IniHandler::GetInstance().GetValue("dbassistant", "Database", "Type");
    auto driver = IniHandler::GetInstance().GetValue("dbassistant", "Database", "Driver");
    auto dbType = DBHandlerFactory::ConvertStr2DbType(driver);
    m_dbAssistant = new DataBaseAssistant();
    //connect(ui.btnExeSql, &QPushButton::clicked, this, &MtDatabaseAst::ExecuteSQL);
    connect(ui.btnExeSql, &QPushButton::clicked, this, &MtDatabaseAst::QuerySQL);
    connect(ui.btnTest, &QPushButton::clicked, this, &MtDatabaseAst::TestFunction);
    connect(ui.btnConn, &QPushButton::clicked, this, &MtDatabaseAst::ConnectDatabase);
    connect(ui.btnAbort, &QPushButton::clicked, this, &MtDatabaseAst::AbortDatabase);
    connect(m_dbAssistant, &DataBaseAssistant::sigShowLog, this, &MtDatabaseAst::ShowUILog);
    connect(m_dbAssistant, &DataBaseAssistant::sigShowTables, this, &MtDatabaseAst::UpdateTables);
    connect(m_dbAssistant, &DataBaseAssistant::sigShowDatabaseInfo, this, &MtDatabaseAst::ShowDatabaseInfo);
    auto ok = m_dbAssistant->Init(dbType);
    if (ok)
    {
        LOG(INFO) << X("��ʼ���ɹ�");

    }
    else
    {
        LOG(FATAL) << X("��ʼ��ʧ��!");
        //exit(-1);
    }
}

void MtDatabaseAst::Deinit()
{
    if (nullptr != m_dbAssistant)
    {
        m_dbAssistant->Deinit();
        m_dbAssistant->deleteLater();
    }
}

void MtDatabaseAst::ShowUILog(const QString& log)
{
    LOG(INFO) << log;
    ui.pteLOG->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + " " + log.trimmed());
    ui.pteLOG->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}

void MtDatabaseAst::ShowDatabaseInfo(const QString& driver, const QString& dbName, const QString& host, int port, const QString& user, const QString& password)
{
    ui.leDbType->setText(driver);
    ui.leDbName->setText(dbName);
    ui.leDbHost->setText(host);
    ui.leDbPort->setText(QString::number(port));
    ui.leUser->setText(user);
    ui.lePassword->setText(password);
}

void MtDatabaseAst::UpdateTables(const QStringList& tables)
{
    ui.cbTableList->clear();
    ui.cbTableList->addItems(tables);
}

void MtDatabaseAst::ConnectDatabase()
{
    auto driver = ui.leDbType->text().trimmed();
    auto dbName = ui.leDbName->text().trimmed();
    auto host = ui.leDbHost->text().trimmed();
    auto port = ui.leDbPort->text().trimmed().toInt();
    auto user = ui.leUser->text().trimmed();
    auto password = ui.lePassword->text().trimmed();
    auto ok = m_dbAssistant->Open(driver, dbName, host, port, user, password);
    if (ok)
    {
        ShowUILog(X("�������ݿ�ɹ�!"));
    }
    else
    {
        ShowUILog(X("�������ݿ�ʧ��,�������Ӳ���!"));
    }
}

void MtDatabaseAst::AbortDatabase()
{
    if (nullptr == m_dbAssistant)
    {
        ShowUILog(X("���ݿ����ģ���쳣!"));
        return;
    }

    auto ok = m_dbAssistant->Close();
    if (ok)
    {
        ShowUILog(X("�Ͽ����ݿ�ɹ�!"));
    }
    else
    {
        ShowUILog(X("�Ͽ����ݿ�ʧ��!"));
    }
}

void MtDatabaseAst::ExecuteSQL()
{
    if (nullptr == m_dbAssistant)
    {
        ShowUILog(X("���ݿ����ģ���쳣!"));
        return;
    }
    auto sql = ui.pteSQL->toPlainText().trimmed();
    QString error;
    auto ok = m_dbAssistant->ExeSql(sql, error);
    if (ok)
    {
        ShowUILog(X("SQL���(%1)ִ�гɹ�!").arg(sql));
    }
    else
    {
        ShowUILog(X("SQL���(%1)ִ��ʧ��:%2").arg(sql).arg(error));
    }
}

void MtDatabaseAst::QuerySQL()
{
    if (nullptr == m_dbAssistant)
    {
        ShowUILog(X("���ݿ����ģ���쳣!"));
        return;
    }

    auto sql = ui.pteSQL->toPlainText().trimmed();
    QString error;
    QSqlQuery query;
    auto ok = m_dbAssistant->QuerySql(sql, query, error);
    if (ok)
    {
        ShowUILog(X("SQL���(%1)ִ�гɹ�!").arg(sql));
        // ��ӡ����
        int columnCount = query.record().count();

        while (query.next())
        {
            for (int i = 0; i < columnCount; ++i)
            {
                // ʹ��record().fieldName(i)����ȡ�ֶ����������Ҫ�Ļ�
                ShowUILog(query.record().fieldName(i) + ": " + query.value(i).toString());
            }

            ShowUILog(X("----------[���Ƿָ���]----------"));
        }
    }
    else
    {
        ShowUILog(X("SQL���(%1)ִ��ʧ��:%2").arg(sql).arg(error));
    }
}

void MtDatabaseAst::TestFunction()
{
    if (nullptr == m_dbAssistant)
    {
        ShowUILog(X("���ݿ����ģ���쳣!"));
        return;
    }

    m_dbAssistant->TestFunction();
}
