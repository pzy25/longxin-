#include "initdb.h"


bool createConnection() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("books.db");

    if (!db.open()) {
        qDebug() << "Cannot open database:" << db.lastError();
        return false;
    }
    return true;
}

void createTable() {
    QSqlQuery query;
        if (!query.exec("CREATE TABLE IF NOT EXISTS books ("
                        "书名 VARCHAR(100), "
                        "索书号 VARCHAR(20), "
                        "书架号 VARCHAR(20), "
                        "是否正确 VARCHAR(10))"))
        {
            qDebug() << "Failed to create table:" << query.lastError();
        }

}
