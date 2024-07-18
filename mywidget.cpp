#include "mywidget.h"
#include "ui_mywidget.h"
#include "centereddelegate.h"

myWidget::myWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::myWidget)
{
    ui->setupUi(this);

    initDB();
    InitMQTT();

    QPixmap backgroundPixmap(":/prefix1/images1.jpg");
       QPalette palette;
       palette.setBrush(QPalette::Background, QBrush(backgroundPixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
       ui->page->setAutoFillBackground(true);
       ui->page->setPalette(palette);

    connect(ui->addButton,&QPushButton::clicked,this,&myWidget::addButton_clicked);
    connect(ui->deleteButton,&QPushButton::clicked,this,&myWidget::deleteButton_clicked);

    connect(ui->videoButton,&QPushButton::clicked,this,&myWidget::videoButton_clicked);
    connect(ui->dataButton,&QPushButton::clicked,this,&myWidget::dataButton_clicked);

    connect(ui->startButton,&QPushButton::clicked,this,&myWidget::startButton_clicked);
    connect(ui->stopButton,&QPushButton::clicked,this,&myWidget::stopButton_clicked);

}

myWidget::~myWidget()
{
    delete ui;
}

void myWidget::initDB()
{
    if (createConnection()) {
        createTable();
    }

    model = new QSqlTableModel(this);
    model->setTable("books");
    model->setEditStrategy(QSqlTableModel::OnFieldChange); // 自动保存更改
    model->select();

    // 设置表头
    model->setHeaderData(0, Qt::Horizontal, tr("书名"));
    model->setHeaderData(1, Qt::Horizontal, tr("索书号"));
    model->setHeaderData(2, Qt::Horizontal, tr("书架号"));
    model->setHeaderData(3, Qt::Horizontal, tr("是否正确"));

    ui->tableView->setModel(model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectItems); // 设置为单个单元格选择
    // 自动填充空位，并调整列宽适应内容
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableView->horizontalHeader()->setStyleSheet("QHeaderView::section { font-size: 17px; }");

    // 设置单元格居中显示和减小字体大小
    ui->tableView->setStyleSheet(
        "QTableView {"
        "    font-size: 15px;"
        "}"
        "QTableView::item {"
        "    text-align: center;"
        "}"
    );

    // 应用自定义委托来居中显示单元格内容
     CenteredDelegate* delegate = new CenteredDelegate(this);
     ui->tableView->setItemDelegate(delegate);
}

void myWidget::startvideo()
{
    if(startflag) {
        return ;
    }

    //启动视频流传输前，需要检查IP地址是否有效
      QString ipAddress = "192.168.25.202";
    //视频流的http请求命令格式，例如："http://192.168.1.8:81/stream"
    QNetworkRequest request;
  //  QString url="http://" + ipAddress + ":46221//mjpeg/1";
    QString url="http://" + ipAddress + ":80//mjpeg/1";
    request.setUrl(QUrl(url));
    request.setRawHeader("Connection", "Keep-Alive");
    request.setRawHeader("User-Agent", "1601");

    Manager = new QNetworkAccessManager();
    Client = Manager->get(request);

    connect(Client, &QNetworkReply::readyRead, this, &myWidget::dataReceivedVideo);

    startflag = true;
    qDebug()<<url;
}

void myWidget::stopvideo()
{
    qDebug() << "===== STOP =====";
    if(!startflag) { //只有开启视频流才可以停止，或者防止多次触发停止视频流
        return;
    }

    Client->close();
    Client->deleteLater();
    startflag = false;
}

void myWidget::InitMQTT()
{
    m_strTargetServer = "esp.icce.top";
    QString clientId="abcdefg";       //表示客户端ID，建议使用设备的MAC地址或SN码，64字符内。
    QString signmethod = "hmacsha1";    //加密方式
    QString message ="clientId"+clientId+"deviceName"+m_strDeviceName+"productKey"+m_strProductKey;

    QHostAddress m_address(m_strTargetServer);
    m_client = new QMQTT::Client(m_address,1883);
    m_client->setHostName(m_strTargetServer);
    m_client->setPort(1883);

    connect(m_client,&QMQTT::Client::connected,this,&myWidget::connectSuccessSlot);
    connect(m_client,SIGNAL(received(QMQTT::Message)),this,SLOT(recvMessageSlot(QMQTT::Message)));

    m_client->connectToHost();
}

void myWidget::addButton_clicked()
{
    QSqlRecord record = model->record();
    record.setValue("title", "New Book");
    record.setValue("index_number", "0000");
    record.setValue("number", 0);
    record.setValue("available", true);
    if (!model->insertRecord(-1, record)) {
        qDebug() << "Failed to add record:" << model->lastError();
    }
    model->submitAll(); // 提交更改
    model->select(); // 刷新模型
}

void myWidget::deleteButton_clicked()
{
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes(); // 获取所有选中项

     QSet<int> rowsToDelete; // 使用QSet确保每行只删除一次
     for (const QModelIndex &index : selectedIndexes) {
         rowsToDelete.insert(index.row());
     }


     QList<int> rowsList = rowsToDelete.values();
     std::sort(rowsList.begin(), rowsList.end(), std::greater<int>()); // 从高到低排序

     for (int row : rowsList) {
         model->removeRow(row); // 删除行
     }

     if (!model->submitAll()) {
         qDebug() << "Failed to delete record:" << model->lastError();
     }
     model->select(); // 刷新模型
}

void myWidget::videoButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void myWidget::dataButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void myWidget::startButton_clicked()
{
    startvideo();
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
}

void myWidget::stopButton_clicked()
{
    stopvideo();
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void myWidget::dataReceivedVideo()
{
    QByteArray buffer = Client->readAll();
    QString data(buffer);

    //对接收的到的数据放到队列中
    int pos = data.indexOf("Content-Type");
    if(pos != -1)
    {
        frameData.append(buffer.left(pos));
        dataQueue.enqueue(frameData);
        frameData.clear();
        frameData.append(buffer.mid(pos));

    } else {
        frameData.append(buffer);
    }

    //触发void dataProcess()函数，合成图片和显示
    if (!dataQueue.isEmpty()) {
        frameBuffer = dataQueue.dequeue();
        dataProcessVideo();
    }
}

void myWidget::dataProcessVideo()
{
    QString data = QString::fromUtf8(frameBuffer.data(), 50); //截取前面50个字符

//    qDebug() <<frameBuffer.left(100)<<"......";
//    qDebug() <<frameBuffer.right(100);
//    qDebug() <<"------------------------";
//      qDebug()<<frameBuffer;
//      qDebug() <<"------------------------";

    const QString lengthKeyword = "Content-Length: ";

    int lengthIndex = data.indexOf(lengthKeyword);
    if (lengthIndex >= 0){
        int endIndex = data.indexOf("\r\n", lengthIndex);
        int length = data.midRef(lengthIndex + 16, endIndex - (lengthIndex + 16 - 1)).toInt(); //取出Content-Length后的数字
        QPixmap pixmap;
        auto loadStatus = pixmap.loadFromData(frameBuffer.mid(endIndex + 4, length));

        //合成失败
        if (!loadStatus)
        {
            qDebug() << "Video load failed";
            frameBuffer.clear();
            return;
        }
        frameBuffer.clear();
        QPixmap pps = pixmap.scaled(ui->videoLabel->width(), ui->videoLabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        ui->videoLabel->setPixmap(pps);
     }

}

void myWidget::recvMessageSlot(QMQTT::Message message)
{
    qDebug() << "Received message from topic" << message.topic() << ":" << message.payload();
    // 将 QByteArray 转换为 QString
    QString payloadString = QString::fromUtf8(message.payload());

    QString currentText = ui->textEdit->toPlainText();
    QString newText = QString("topic %1: %2").arg(message.topic(), payloadString);
    ui->textEdit->setText(currentText + "\n" + newText);

    if (message.topic() == "Car")
       {
            if (payloadString.startsWith("angle:"))
            {
                QString angle = payloadString.mid(6); // 提取角度值
                // 处理角度值
                qDebug() << "Angle value:" << angle;
                ui->agLineEdit->setText(angle);
            }
       }
       else if (message.topic() == "location")
       {
                qDebug() << "Location value:" << payloadString;
                ui->wbLineEdit->setText(payloadString);
       }

}

void myWidget::connectSuccessSlot()
{
    qDebug() << "success" <<endl;
    QStringList subscriptions = {"Car", "location"}; // 添加多个主题
    for (const QString &subscription : subscriptions)
    {
        m_client->subscribe(subscription);
        qDebug() << "Subscribed to topic:" << subscription;
    }
}


