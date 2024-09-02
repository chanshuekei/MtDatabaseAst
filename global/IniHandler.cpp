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
    // ����޸ĵ��ļ���������·��,�򻺴浽mItemsʱֻ��ȡ�ļ���
    QFileInfo fileInfo(file);
    // ����ļ�������,Ĭ�ϴӵ�ǰ��configĿ¼����,�޸���������������ļ�ʱ��Ҫָ��·��
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
        LOG(ERROR) << X("�޷��������ļ�:") << patchFilename;
        return false;
    }

    QFile tempFile(file.fileName() + ".modify");

    if(!tempFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        LOG(ERROR) << X("�޸������ļ�:%1ʧ��,�޸�����:[%2]%3=%4,����ʱ�ļ�ʧ��:%5")
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

        //LOG(INFO) << X("д�������ļ�:") << line;
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
            // �ҵ�section,�����Ҳ���key,���л�sectionʱ׷��
            if(!isFindKey && (opType != CONFIG_DEL_KEY) && (currentSection.compare(section, Qt::CaseInsensitive) == 0))
            {
                if(currentSection.compare(reg.cap(1), Qt::CaseInsensitive) != 0)
                {
                    //LOG(WARNING) << X("�����ļ���[%1]������:%2=%3").arg(currentSection).arg(key).arg(value);
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

        // ����ע��,�ȱ���ע��,�������ɾ������,��д��
        if(isCommentLine(line))
        {
            lastComment << line;
            line = "";
            continue;
        }

        // ������ע����,����֮ǰ��ע��
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
            LOG(WARNING) << X("��������:%1,ɾ��Section:%2,����:%3").arg(opType).arg(currentSection).arg(line);
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
            LOG(WARNING) << X("��������:%1,ɾ��Key:%2").arg(opType).arg(key);
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
        //LOG(WARNING) << X("�����ļ���[%1]���޸�:%2=%3").arg(currentSection).arg(key).arg(value);
        flushTemp(line);
}

    // �������������ɾ��,�Ҳ���Section/Key����Ҫ����
    if((opType != CONFIG_DEL_SECTION) && (opType != CONFIG_DEL_KEY))
    {
        if(!isFindKey)
        {
            LOG(WARNING) << X("�����ļ�����[%1]:%2=%3").arg(currentSection).arg(key).arg(value);

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
            LOG(DEBUG) << X("��ȡ�����ļ�:") << file.fileName();
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
                //���⴦����Ҫ�Ǳ���ɾ��URL��ַ��http://xxx.xx.xx.xx
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
