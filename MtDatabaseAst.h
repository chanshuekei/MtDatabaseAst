#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MtDatabaseAst.h"
#include "DataBaseAssistant.h"

class MtDatabaseAst : public QMainWindow
{
    Q_OBJECT

public:
    MtDatabaseAst(QWidget* parent = nullptr);
    ~MtDatabaseAst();
    void Init();
    void Deinit();
public:
    void ShowUILog(const QString& log);
    void ShowDatabaseInfo(const QString& driver, const QString& dbName, const QString& host, int port, const QString& user, const QString& password);
    void UpdateTables(const QStringList& tables);
    void ConnectDatabase();
    void AbortDatabase();
    void ExecuteSQL();
    void QuerySQL();
protected:
    void TestFunction();
private:
    Ui::MtDatabaseAstClass ui;
    DataBaseAssistant* m_dbAssistant{ nullptr };
};
