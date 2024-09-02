#include "IniHandler.h"
#include <QFile>
#include <QList>
#include <QFileInfo>
#include <QTemporaryFile>
#include "easylogging++.h"
#pragma execution_character_set("utf-8")

#ifndef X
#define X(log) QString::fromLocal8Bit(log)
#endif

void IniHandler::Init()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/config/";
    LoadDir(configPath);
}

QString IniHandler::GetValue(const QString& key, const QString& defValue)
{
    QMutexLocker lock(&m_mtx);
    return m_items.value(key.toLower(), defValue);
}

QString IniHandler::GetValue(const QString& file, const QString& section, const QString& key, const QString& defValue)
{
    QString realkey = QString("/%1/%2/%3").arg(file, section, key).toLower();
    QMutexLocker lock(&m_mtx);
    return m_items.value(realkey, defValue);
}

void IniHandler::SetValue(const QString& file, const QString& section, const QString& key, const QString& val, const QString& comment, int opType)
{
    // 如果修改的文件名包含了路径,则缓存到mItems时只提取文件名
    QFileInfo fileInfo(file);
    // 如果文件不存在,默认从当前的config目录查找,修改其他程序的配置文件时需要指定路径
    bool ok;

    if(!fileInfo.exists() && !fileInfo.isAbsolute())
    {
        QString cfgFile = QCoreApplication::applicationDirPath() + QString("/config/%1").arg(file);
        ok = ModifyConf(cfgFile, section, key, val, comment, opType);
    }
    else
    {
        ok = ModifyConf(file, section, key, val, comment, opType);
    }

    if(ok)
    {
        QString realkey = QString("/%1/%2/%3").arg(fileInfo.baseName(), section, key).toLower();
        QMutexLocker lock(&m_mtx);
        m_items[realkey] = val;
    }
}

bool IniHandler::ExistsKey(const QString& key)
{
    QMutexLocker lock(&m_mtx);
    return m_items.contains(key.toLower());
}

bool IniHandler::ModifyConf(const QString& filename, const QString& section, const QString& key, const QString& value, const QString& comment,
                            int opType)
{
    QString patchFilename(filename);

    if(!filename.endsWith(".ini"))
    {
        patchFilename.append(".ini");
    }

    QFile file(patchFilename);

    if(!file.open(QIODevice::ReadWrite))
    {
        LOG(ERROR) << X("无法打开配置文件:") << patchFilename;
        return false;
    }

    QFile tempFile(file.fileName() + ".modify");

    if(!tempFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        LOG(ERROR) << X("修改配置文件:%1失败,修改内容:[%2]%3=%4,打开临时文件失败:%5")
            .arg(filename).arg(section).arg(key).arg(value).arg(tempFile.errorString());
        return false;
    }

    auto isCommentLine = [] (const QString& line)
    {
        const QStringList comment { "//", ";", "#" };

        for(auto it = comment.begin(); it != comment.end(); ++it)
        {
            if(line.startsWith(*it))
            {
                return true;
            }
        }

        return false;
    };
    QTextStream stream(&file);
    //stream.setCodec("GBK");
    stream.setCodec("UTF-8");
    QTextStream streamTemp(&tempFile);
    //streamTemp.setCodec("GBK");
    streamTemp.setCodec("UTF-8");
    auto flushTemp = [&] (const QString& line)
    {
        if(line.isEmpty())
        {
            return;
        }

        //LOG(INFO) << X("写入配置文件:") << line;
        streamTemp << line << "\r\n";
        streamTemp.flush();
    };
    QRegExp reg(R"(^\[(.*)\].*)");
    reg.setCaseSensitivity(Qt::CaseInsensitive);
    QString currentSection;
    QStringList lastComment;
    bool isFindKey = false;

    while(true)
    {
        if(stream.atEnd())
        {
            break;
        }

        bool isSync = true;
        QString line = stream.readLine().trimmed();
#if 0

        if(isSync)
        {
            flushTemp(line);
    };

#endif
        if(line.isNull() || line.isEmpty())
        {
            continue;
        }

        if(reg.exactMatch(line))
        {
            // 找到section,但是找不到key,在切换section时追加
            if(!isFindKey && (opType != CONFIG_DEL_KEY) && (currentSection.compare(section, Qt::CaseInsensitive) == 0))
            {
                if(currentSection.compare(reg.cap(1), Qt::CaseInsensitive) != 0)
                {
                    //LOG(WARNING) << X("配置文件在[%1]下新增:%2=%3").arg(currentSection).arg(key).arg(value);
                    if(!comment.isEmpty())
                    {
                        auto commentLines = comment.split("\\n");

                        foreach(const QString & commentLine, commentLines)
                        {
                            streamTemp << X("; %1").arg(commentLine) << "\r\n";
                        }
                    }

                    streamTemp << X("%1=%2").arg(key).arg(value) << "\r\n";
                    streamTemp.flush();
                    isFindKey = true;
                }
            }

            if(!currentSection.isEmpty())
            {
                streamTemp << "\r\n";
                streamTemp.flush();
            }

            currentSection = reg.cap(1);
        }

        // 过滤注释,先保存注释,如果不是删除操作,再写入
        if(isCommentLine(line))
        {
            lastComment << line;
            line = "";
            continue;
        }

        // 遇到非注释行,处理之前的注释
        if(isSync)
        {
            foreach(const QString & commentLine, lastComment)
            {
                flushTemp(commentLine);
            }

            lastComment.clear();
        };

        if(currentSection.compare(section, Qt::CaseInsensitive) != 0)
        {
            flushTemp(line);
            continue;
        }

        if(opType == CONFIG_DEL_SECTION)
        {
            LOG(WARNING) << X("操作类型:%1,删除Section:%2,内容:%3").arg(opType).arg(currentSection).arg(line);
            lastComment.clear();
            isFindKey = true;
            isSync = false;
            continue;
        }

        QRegExp reg(R"(^(.*)=(\s*)(\S*)(.*))");
        reg.setCaseSensitivity(Qt::CaseInsensitive);
        QString matchKey;
        QString matchValue;

        if(!reg.exactMatch(line))
        {
            flushTemp(line);
            continue;
        }

        matchKey = reg.cap(1).trimmed();
        matchValue = reg.cap(3).trimmed();

        if(matchKey.compare(key, Qt::CaseInsensitive) != 0)
        {
            flushTemp(line);
            continue;
        }

        if(opType == CONFIG_DEL_KEY)
        {
            LOG(WARNING) << X("操作类型:%1,删除Key:%2").arg(opType).arg(key);
            lastComment.clear();
            isFindKey = true;
            isSync = false;
            continue;
        }

        isFindKey = true;
        line = "";

        if(!comment.isEmpty())
        {
            lastComment.clear();
            auto commentLines = comment.split("\\n");

            foreach(const QString & commentLine, commentLines)
            {
                line += X("; %1\r\n").arg(commentLine);
            }
        }

        line += QString::fromLocal8Bit("%1=%2").arg(matchKey).arg(value);
        //LOG(WARNING) << X("配置文件在[%1]下修改:%2=%3").arg(currentSection).arg(key).arg(value);
        flushTemp(line);
}

    // 如果操作类型是删除,找不到Section/Key不需要新增
    if((opType != CONFIG_DEL_SECTION) && (opType != CONFIG_DEL_KEY))
    {
        if(!isFindKey)
        {
            LOG(WARNING) << X("配置文件新增[%1]:%2=%3").arg(currentSection).arg(key).arg(value);

            if(currentSection.compare(section, Qt::CaseInsensitive) != 0)
            {
                streamTemp << X("\r\n[%1]").arg(section) << "\r\n";
            }

            if(!comment.isEmpty())
            {
                auto commentLines = comment.split("\\n");

                foreach(const QString & commentLine, commentLines)
                {
                    streamTemp << X("; %1").arg(commentLine) << "\r\n";
                }
            }

            streamTemp << X("%1=%2").arg(key).arg(value) << "\r\n";
            streamTemp.flush();
        }
    }

    QFile::remove(file.fileName().append(".old"));
    file.close();
    tempFile.close();

    if(!file.rename(file.fileName().append(".old")))
    {
        return false;
    }

    return tempFile.rename(tempFile.fileName().replace(".modify", ""));
}

void IniHandler::LoadDir(const QString& dirpath)
{
    QDir dir(dirpath);

    if(!dir.exists())
    {
        return;
    }

    QFileInfoList files = dir.entryInfoList(QDir::Files);

    for(QFileInfo& file : files)
    {
        if(file.fileName().toLower().endsWith(".ini"))
        {
            LOG(DEBUG) << X("读取配置文件:") << file.fileName();
            LoadFile(file.absoluteFilePath());
        }
    }
}


void IniHandler::LoadFile(const QString& file)
{
    QMutexLocker lock(&m_mtx);
    QSettings ini(file, QSettings::IniFormat);
    //ini.setIniCodec("GBK");
    ini.setIniCodec("UTF-8");
    QString basefilename = QFileInfo(file).baseName();
    QStringList keys = ini.allKeys();

    for(QString& key : keys)
    {
        if(key.startsWith("//") || key.startsWith(";"))
        {
            continue;
        }

        QVariant value = ini.value(key);
        QString val;

        if(value.type() == QMetaType::QStringList)
        {
            val = value.toStringList().join(",");
        }
        else
        {
            val = value.toString();
        }

        int pos = val.indexOf("//");

        while(pos > 0)
        {
            if(val[pos - 1] == ':')
            {
                //特殊处理，主要是避免删除URL地址，http://xxx.xx.xx.xx
                pos = val.indexOf("//", pos + 1);
            }
            else
            {
                val.truncate(pos);
                break;
            }
        }

        if(val.indexOf(";") >= 0)
        {
            val.truncate(val.indexOf(";"));
        }

        if(val.indexOf("#") >= 0)
        {
            val.truncate(val.indexOf("#"));
        }

        m_items[("/" + basefilename + "/" + key).toLower()] = val.trimmed();
    }
}
