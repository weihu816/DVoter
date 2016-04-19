// We need to include a couple Qt classes that we'll use:
#include <QMainWindow>

// This is the include file that Qt generates for us from the
// GUI we built in Designer  
#include "ui_MainWindow.h"
#include "stdincludes.h"
#include "DNode.h"

/*
 * This is the MainWindow class that we have told to inherit from
 * our Designer MainWindow (ui::MainWindow)
 */
class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    volatile bool isAlive;
    DNode * node;
    // Every QWidget needs a constructor, and they should allow
    // the user to pass a parent QWidget (or not).
    MainWindow(QWidget *parent=0);

    virtual ~MainWindow() {
        isAlive = false;
        delete node;
    }


private slots:

    // This function gets called when a user clicks on the
    // loginButton on the front page (which you placed there
    // with Designer)
    void on_loginButton_clicked();

    // This gets called when you click the sayButton on
    // the chat page.
    void on_sayButton_clicked();

    void slot_msg(QString msg);
    void slot_list();

signals:
    void signal_msg(QString msg);

private:

    void displayMsg();
    void recvMsg();
    void sendMsgToLeader();
    void sendMsg();
    void heartBeatRoutine();

};
