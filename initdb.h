#ifndef INITDB_H
#define INITDB_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

bool createConnection();
void createTable();

#endif // INITDB_H
