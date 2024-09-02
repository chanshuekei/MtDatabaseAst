INCLUDEPATH += $$PWD

DEFINES += ELPP_QT_LOGGING \
           ELPP_STL_LOGGING \
           ELPP_STRICT_SIZE_CHECK \
           ELPP_MULTI_LOGGER_SUPPORT \
           ELPP_THREAD_SAFE \
           ELPP_UNICODE \
           ELPP_NO_DEFAULT_LOG_FILE

HEADERS += \
        $$PWD/easylogging++.h

SOURCES += \
        $$PWD/easylogging++.cc

