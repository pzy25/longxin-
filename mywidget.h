#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QSqlTableModel>
#include "initdb.h"
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QtNetwork>
#include "mqtt/qmqtt.h"
#include <QtNetwork>
#include <QHostAddress>
#include <QPainter>


QT_BEGIN_NAMESPACE
namespace Ui { class myWidget; }
QT_END_NAMESPACE

class myWidget : public QWidget
{
    Q_OBJECT

public:
    myWidget(QWidget *parent = nullptr);
    ~myWidget();
    void initDB();
    void startvideo();
    void stopvideo();
    void InitMQTT();

private slots:
    void addButton_clicked();
    void deleteButton_clicked();

    void videoButton_clicked();
    void dataButton_clicked();

    void startButton_clicked();
    void stopButton_clicked();
    void setupButton_clicked();
    void dataReceivedVideo();
    void dataProcessVideo();

    void recvMessageSlot(QMQTT::Message message);
    void connectSuccessSlot();

private:
    Ui::myWidget *ui;
    QSqlTableModel *model;

    QQueue<QByteArray> dataQueue;           //接收数据流
    QByteArray frameData;
    QByteArray frameBuffer;
    QNetworkReply *Client = nullptr;
    QNetworkAccessManager *Manager;
    bool startflag = false;

    QMQTT::Client *m_client;
    QString m_strProductKey;
    QString m_strDeviceName;
    QString m_strDeviceSecret;
    QString m_strRegionId;
    QString m_strTargetServer;
};
#endif // MYWIDGET_H
