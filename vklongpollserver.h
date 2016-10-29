#ifndef VKLONGPOLLSERVER_H
#define VKLONGPOLLSERVER_H

#include <QWidget>
#include <QObject>
#include <QThread>
#include <QString>
#include <QMessageBox>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QEventLoop>
#include <QMap>
#include <QVariant>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class VKLongPollServer : public QThread
{
    Q_OBJECT

public:
    VKLongPollServer(QString token);

    QString longPollKey;
    QString longPollAdress;
    QString longPollTs;
    QString tokenLongpoll;
    bool mainCycleStatus;
    QWidget messageWidget;

    void run();
private:
    QByteArray GET(QUrl);

public:
    void connect_to_longpoll();

signals:
    void send_message(QStringList);
};

#endif // VKLONGPOLLSERVER_H
