#pragma once
#include <QtCore>
#include <QMap>

class IniHandler
{
    enum
    {
        CONFIG_MODIFY = 0,
        CONFIG_ADD = 1,
        CONFIG_DEL_SECTION = 2,
        CONFIG_DEL_KEY = 3,
    };
private:
    explicit IniHandler() = default;
public:
    ~IniHandler() = default;
    static IniHandler& GetInstance()
    {
        static IniHandler ins;
        return ins;
    }
    void Init();
    void LoadDir(const QString& dirpath);
    void LoadFile(const QString& file);
    QString GetValue(const QString& key, const QString& defValue = QString(""));
    QString GetValue(const QString& file, const QString& section, const QString& key, const QString& defValue = QString(""));
    void SetValue(const QString& file, const QString& section, const QString& key, const QString& val, const QString& comment = "", int opType = 0);
    bool ExistsKey(const QString& key);
    bool ModifyConf(const QString& filename, const QString& section, const QString& key, const QString& value, const QString& comment, int opType);
private:
    QMap<QString, QString>  m_items;
    QMutex                  m_mtx { QMutex::Recursive };
};

#define GetIniValue IniHandler::GetInstance().GetValue
#define SetIniValue IniHandler::GetInstance().SetValue