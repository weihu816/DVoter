#include "MainWindow.h"

#include <QThread>
#include <qtconcurrentrun.h>
#include <QTimer>

/**
 * FUNCTION NAME: sendMsg
 *
 * DESCRIPTION: thread keep listening to user input
 */
void MainWindow::sendMsg() {
    while (isAlive) {
        /* Simply exit */
        if (std::cin.eof()) { // Control-D / EOF: shutdown
            isAlive = false;
            node->nodeLeave();
            delete node;
            std::exit(0);
        }
        char msg[MAXBUFLEN];
        std::cin.getline(msg, MAXBUFLEN - 1);
        if (strlen(msg) != 0) {
            node->sendMsg(std::string(msg));
        }
    }
}

void MainWindow::sendMsgToLeader() {
    std::cout << "???" << isAlive << std::endl;
    while (isAlive) {
        std::cout << "???" << std::endl;
        node->sendMsgToLeader();
    }
}

/**
 * FUNCTION NAME: recvMsg
 *
 * DESCRIPTION: thread keep listening to incoming messages
 */
void MainWindow::recvMsg() {
    while (isAlive) {
        node->recvLoop();
    }
}

/**
 * FUNCTION NAME: displayMsg
 *
 * DESCRIPTION: thread in charge of displaying messages
 */
void MainWindow::displayMsg() {
    while (isAlive) {
        const QString msg = QString::fromStdString(node->msgLoop());
        std::cout << msg.toStdString() << std::endl;
        emit signal_msg(msg);
    }
}

void MainWindow::slot_msg(QString msg) {
    roomTextEdit->append(msg);
}


void MainWindow::slot_list() {
    if (!isAlive) return;
    const QString list = QString::fromStdString(node->getMember()->getMemberListGUI());
    userTextEdit->setText(list);
}


/**
 * FUNCTION NAME: heartBeatRoutine
 *
 * DESCRIPTION: send and check heartbeat
 */
void MainWindow::heartBeatRoutine() {
    while (isAlive) {
        std::chrono::milliseconds sleepTime(HEARTFREQ); // check every HEARTFREQ seconds
        std::this_thread::sleep_for(sleepTime);
        node->nodeLoopOps();
    }
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);
    stackedWidget->setCurrentWidget(loginPage);
    QObject::connect(this, SIGNAL(signal_msg(QString)), this, SLOT(slot_msg(QString)));
    QTimer * timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_list()));
    timer->start(5000);
}

void MainWindow::on_loginButton_clicked()
{
    node = new DNode(userLineEdit->text().toStdString(), serverLineEdit->text().toStdString());
    if (node->nodeStart() == FAILURE) {
        delete node;
        std::exit(1);
    }
    stackedWidget->setCurrentWidget(chatPage);
    QString name = QString::fromStdString(node->getUsername() + " " + node->getMember()->getAddress());
    nameLabel->setText(name);
    isAlive = true;
    std::thread t1(&MainWindow::displayMsg, this);
    t1.detach();
    std::thread t2(&MainWindow::recvMsg, this);
    t2.detach();
    std::thread t3(&MainWindow::heartBeatRoutine, this);
    t3.detach();
    std::thread t4(&MainWindow::sendMsgToLeader, this);
    t4.detach();
}


void MainWindow::on_sayButton_clicked() {
    QString message = sayLineEdit->text().trimmed();
    if(!message.isEmpty())
    {
        node->sendMsg(message.toStdString());
    }
    sayLineEdit->clear();
    sayLineEdit->setFocus();
}

