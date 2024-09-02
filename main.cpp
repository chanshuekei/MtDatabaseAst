#include "MtDatabaseAst.h"
#include <QtWidgets/QApplication>
#include <QDir>
#include <QSharedMemory>
#include <QMessageBox>
#include <QTextCodec>
#include <QDebug>
#include "easylogging++.h"
#pragma execution_character_set("utf-8")

// ��ʼ����־
INITIALIZE_EASYLOGGINGPP
void SetupEasyLogging(int argc, char* argv[])
{
    auto logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir dir(logDir);

    if (!dir.exists())
    {
        dir.mkpath(dir.absolutePath());
    }

    auto filePath = QCoreApplication::applicationDirPath().toStdString() + std::string("/config/log.conf");
    QFile f(QString::fromStdString(filePath));

    if (f.exists())
    {
        el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
        el::Configurations conf(filePath);
        el::Loggers::reconfigureAllLoggers(conf);
        el::Helpers::installPreRollOutCallback([](const char* fname, std::size_t currFileSize)
            {
                Q_UNUSED(currFileSize);
                // �������ƵĴ�С,�����е��ļ�������
                QFileInfo fileInfo(fname);
                QDir dir(QCoreApplication::applicationDirPath() + "/logs");
                QFileInfoList files = dir.entryInfoList({ fileInfo.baseName() + R"(.*.log)", }, QDir::Files);
                QString newName = fileInfo.path() + "/" + fileInfo.baseName() + QString(".%1.log").arg(files.size() + 1);
                QFile::rename(fileInfo.filePath(), newName);
            });
    }
    else
    {
        el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
        el::Configurations conf;
        conf.setGlobally(el::ConfigurationType::Format, "%datetime{%H:%m:%s.%g} [%levshort] [%thread] [%line@%fbase] %msg");
        std::string logFile = logDir.toStdString() + "/MtDBA_%datetime{%Y%M%d}.log";
        conf.setGlobally(el::ConfigurationType::Filename, logFile);
        el::Loggers::reconfigureLogger("default", conf);
    }

    START_EASYLOGGINGPP(argc, argv);
}


int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    qDebug() << "Typing Under The Moon And Thinking Over The Moon.";
    // �����ڴ�,Ψһʵ��
    static QSharedMemory shared(a.applicationName());
    shared.create(1);

    // �ж��Ƿ��Ѿ�������Ӧ��ʾ������
    if (shared.attach())
    {
        QMessageBox::critical(nullptr, "����", "ֻ��������һʵ��,�������������ʵ��!");
        LOG(ERROR) << X("��ֹ�������ʵ��!!!");
        return -1;
    }
    // ���ó������:UTF-8
    QTextCodec* codec = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
    // ������־
    SetupEasyLogging(argc, argv);
    LOG(DEBUG) << X("��������");
    // ��������
    MtDatabaseAst w;
    w.show();
    return a.exec();
}
