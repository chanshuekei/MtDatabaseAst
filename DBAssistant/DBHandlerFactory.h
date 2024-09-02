#ifndef DBHANDLERFACTORY_H
#define DBHANDLERFACTORY_H

#include "DBADefine.h"
#include "DataBaseHandler.h"

class DBHandlerFactory
{
public:
    DBHandlerFactory() = default;
    ~DBHandlerFactory() = default;
public:
    static DataBaseHandler* CreateHandler(const TKDBA_SPACE::DATABASE_TYPE type);
    static TKDBA_SPACE::DATABASE_TYPE ConvertInt2DbType(const int& ini);
    static TKDBA_SPACE::DATABASE_TYPE ConvertStr2DbType(const QString& ini);
};

#endif // DBHANDLERFACTORY_H
