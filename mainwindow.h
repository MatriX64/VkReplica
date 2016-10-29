#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QWebEngineView>
#include <QRegularExpression>

#include "vklongpollserver.h"

typedef QMap<QString,QString> User;

#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QApplication *a, QWidget *parent = 0);
    ~MainWindow();

signals:
    void auth_success();
    void longpoll_successed();
    void friends_loaded(bool);

public slots:
    void url_changed(QUrl u);
    void get_friends();

private slots:
    void on_SEND_clicked();
    void close_app();
    void on_STOPSEND_clicked();
    void send_message(QStringList);

private:
    Ui::MainWindow *ui;

private:
    bool sendMoonStatus;
    QString token;
    QString receiverId;
    QString receivedMessage;
    QString messageToReceive;
    User users;
    QByteArray GET(QUrl);
    void vait_for_receive();
    QUrl moon();
    QUrl parse_message();

    VKLongPollServer *m_server;
};

#endif // MAINWINDOW_H
