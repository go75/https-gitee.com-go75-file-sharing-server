#include "tcp_server.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置窗口标题
    setWindowTitle("P2P Server");

    // 内容列表
    ContentListWidget = new QListWidget;

    myWidget = new QWidget;
    PortLabel = new QLabel("端口:");
    // 端口输入框
    PortLineEdit = new QLineEdit;
    // 启动服务器的按钮
    CreateBtn = new QPushButton("打开服务器");

    // 在线用户列表
    resourceTree = new QTreeWidget();
    resourceTree->setHeaderLabel("在线用户");
    resourceTree->clear();

    mainLayout = new QGridLayout();

    mainLayout->addWidget(ContentListWidget,0,0,1,2);
    mainLayout->addWidget(PortLabel,1,0);
    mainLayout->addWidget(PortLineEdit,1,1);
    mainLayout->addWidget(CreateBtn,2,0,1,2);
    mainLayout->addWidget(resourceTree,0,2,3,1);
    myWidget->setLayout(mainLayout);
    setCentralWidget(myWidget);

    port = 8010;
    PortLineEdit->setText(QString::number(port));

    // 给启动服务器的按钮绑定点击函数
    connect(CreateBtn,SIGNAL(clicked()),this,SLOT(slotCreateServer()));

    server = 0;
}

// 启动服务端实例的回调函数
void MainWindow::slotCreateServer(){
    server = new Server(this, port);
    server->db = QSqlDatabase::addDatabase("QMYSQL");
    server->db.setHostName("127.0.0.1");
    server->db.setPort(3306);
    server->db.setDatabaseName("ShareFile");
    server->db.setUserName("root");
    server->db.setPassword("1234");
    if (!server->db.open()) {
        QMessageBox::critical(0, QObject::tr("无法打开数据库"),
        "无法创建数据库连接！ ", QMessageBox::Cancel);
        return;
    } else {
        qDebug() << "数据库打开成功";
    }

    connect(server,SIGNAL(UpdateServer(QString,int,Server::MsgKind)),this,SLOT(UpdateServer(QString,int,Server::MsgKind)));
    CreateBtn->setEnabled(false);
}

// 调整界面
void MainWindow::UpdateServer(QString msg, int length, Server::MsgKind flag){
    switch (flag) {
    case Server::UpdateMsg:
    {
        ContentListWidget->addItem(msg.left(length));
        break;
    }
    case Server::UPDATEMETA:
    {
        ContentListWidget->addItem(msg + ": update meta");
        break;
    }
    case Server::UpdateName:
    {
        QTreeWidgetItem *item_name = new QTreeWidgetItem(resourceTree);
        item_name->setText(0,msg);
        break;
    }
    case Server::RemoveName:
    {
        QPoint p(length,0);
        resourceTree->removeItemWidget(resourceTree->itemAt(p),0);
        delete resourceTree->itemAt(p);
        break;
    }
    }
}

// 析构函数
MainWindow::~MainWindow()
{
    delete PortLabel;
    delete PortLineEdit;
    delete CreateBtn;
    delete ContentListWidget;
    resourceTree->clear();
    delete resourceTree;
    delete mainLayout;
    delete myWidget;
    if(server != 0){
        server->db.close();
        delete server;
    }
}
