#include "MtDatabaseAst.h"
#include <QtWidgets/QApplication>
#include <QDir>
#include <QSharedMemory>
#include <QMessageBox>
#include <QTextCodec>
#include <QDebug>
#include "easylogging++.h"
#pragma execution_character_set("utf-8")

// 初始化日志
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
                // 超过限制的大小,将现有的文件重命名
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
    // 共享内存,唯一实例
    static QSharedMemory shared(a.applicationName());
    shared.create(1);

    // 判断是否已经启动对应的示例进程
    if (shared.attach())
    {
        QMessageBox::critical(nullptr, "错误", "只能启动单一实例,不允许启动多个实例!");
        LOG(ERROR) << X("禁止启动多个实例!!!");
        return -1;
    }
    // 设置程序编码:UTF-8
    QTextCodec* codec = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
    // 启动日志
    SetupEasyLogging(argc, argv);
    LOG(DEBUG) << X("启动程序");
    // 启动程序
    MtDatabaseAst w;
    w.show();
    return a.exec();
}
