#include "vklongpollserver.h"

VKLongPollServer::VKLongPollServer(QString token) : mainCycleStatus(false)
{
    tokenLongpoll = token;
}

void VKLongPollServer::run()
{
    connect_to_longpoll();

    while (mainCycleStatus)
    {
        QString requestUrl("https://{$server}?act=a_check&key={$key}&ts={$ts}&wait=25&mode=98");
        requestUrl.replace("{$server}", longPollAdress);
        requestUrl.replace("{$key}", longPollKey);
        requestUrl.replace("{$ts}", longPollTs);
        QUrl current(requestUrl);

        QByteArray answer = GET(current);

        QVariantMap map = QJsonDocument::fromJson(answer).object().toVariantMap();

        longPollTs = map.value("ts").toString();

        QVariantList updates = map.value("updates").toList();

        if (!updates.size())
        {
            continue;
        }

        for (int i = 0; i < updates.size(); i++)
        {
            QVariantList cur = updates.at(i).toList();

            if (cur.size() < 2)
                continue;

            int type = cur.at(0).toInt();
            if (type == 4)
            {
                if (cur.count() < 6)
                continue;

                int message_flags = cur.at(2).toInt();
                QString message = cur.at(6).toString();
                QString id = cur.at(3).toString();
                QStringList receiveMessageID;
                receiveMessageID << message << id;
                if (message_flags & 2)
                {
                     //......
                } else
                {
                    send_message(receiveMessageID);
                }
            }
        }
    }
}

QByteArray VKLongPollServer::GET(QUrl r)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->get(QNetworkRequest(r));
    QEventLoop wait;
    connect(manager, SIGNAL(finished(QNetworkReply*)), &wait, SLOT(quit()));
    QTimer::singleShot(10000, &wait, SLOT(quit()));
    wait.exec();

    QByteArray answer = reply->readAll();
    reply->deleteLater();
    return answer;
}

void VKLongPollServer::connect_to_longpoll()
{
    QUrl current("https://api.vk.com/method/messages.getLongPollServer");
    QUrlQuery query;

    query.addQueryItem("access_token", tokenLongpoll);

    current.setQuery(query);
    QByteArray answer = GET(current);
    if (!answer.contains("response"))
    {
        QMessageBox::critical(&messageWidget, tr("Ошибка"), tr("Не удалось подключиться к LP серверу"),
                             QMessageBox::Ok);
        qDebug() << "No answer";
        exit(-1);
    }

    longPollKey = QJsonDocument::fromJson(answer).object().toVariantMap().value("response").toMap().value("key").toString();
    longPollAdress = QJsonDocument::fromJson(answer).object().toVariantMap().value("response").toMap().value("server").toString();
    longPollTs = QJsonDocument::fromJson(answer).object().toVariantMap().value("response").toMap().value("ts").toString();
}
