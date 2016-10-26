#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QEventLoop>
#include "json.h"
#include <QMap>
#include <QMessageBox>

typedef QMap<QString,QString> User;

#include <QDebug>
using namespace QtJson;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void auth_success();
    void longpoll_successed();
    void friends_loaded(bool);
public slots:
    void url_changed(QUrl u);
    void connect_to_longpoll();
    void get_friends();
private slots:
    void on_SEND_clicked();

    void on_STOPSEND_clicked();

private:
    Ui::MainWindow *ui;

private:
    bool mainCycleStatus;
    bool sendMoonStatus;
    QString token;
    QString longPollKey;
    QString longPollAdress;
    QString longPollTs;
    QString receiverId;
    QString receivedMessage;
    QString messageToReceive;
    User users;
    QStringList groups;
    QByteArray GET(QUrl);
    void send_message();
    void vait_for_receive();
    QUrl moon();
    QUrl parse_message();
};

#endif // MAINWINDOW_H
