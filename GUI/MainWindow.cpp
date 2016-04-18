#include "MainWindow.h"

#include <QThread>
#include <qtconcurrentrun.h>
/**
 * FUNCTION NAME: displayMsg
 *
 * DESCRIPTION: thread in charge of displaying messages
 */
void displayMsg(DNode * node) {
    while (1) {
        std::string msg = node->msgLoop();
        std::cout << msg << std::endl;
    }
}


/**
 * FUNCTION NAME: recvMsg
 *
 * DESCRIPTION: thread keep listening to incoming messages
 */
void recvMsg(DNode * node) {
    while (1) {
        node->recvLoop();
    }
}


///**
// * FUNCTION NAME: heartBeatRoutine
// *
// * DESCRIPTION: send and check heartbeat
// */
//void heartBeatRoutine(DNode * node) {
//    while (1) {
//        std::chrono::milliseconds sleepTime(HEARTFREQ); // check every 3 seconds
//        std::this_thread::sleep_for(sleepTime);
//        node->nodeLoopOps();
//    }
//}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);
    stackedWidget->setCurrentWidget(loginPage);

}

void MainWindow::on_loginButton_clicked()
{
    node = new DNode(userLineEdit->text().toStdString(), serverLineEdit->text().toStdString());
    // Node is up, introduced to the group
    if (node->nodeStart() == FAILURE) {
        delete node;
        std::cout << "Fail to start the node" << std::endl;
        std::exit(1);
    }
    // Node is up, introduced to the group
    if (node->nodeStart() == FAILURE) {
        std::cout << "Fail to start the node" << std::endl;
        std::exit(1);
    }
    stackedWidget->setCurrentWidget(chatPage);
    QFuture<void> t1 = QtConcurrent::run(displayMsg, node);
    QFuture<void> t2 = QtConcurrent::run(recvMsg, node);
//    QFuture<void> t3 = QtConcurrent::run(heartBeatRoutine, node);
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

