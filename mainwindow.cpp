#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QApplication *a, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->WEB->load(QUrl("https://oauth.vk.com/authorize?client_id=5683779&scope=offline,messages,friends,video&redirect_uri=https://oauth.vk.com/blank.html&display=wap&response_type=token"));

    connect(ui->WEB, SIGNAL(urlChanged(QUrl)), this, SLOT(url_changed(QUrl)));
    connect(this, SIGNAL(auth_success()), this, SLOT(get_friends()));
    connect(this, SIGNAL(friends_loaded(bool)), ui->LIST, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(friends_loaded(bool)), ui->ANSWER_METHOD, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(friends_loaded(bool)), ui->SEND, SLOT(setEnabled(bool)));
    //connect(this, SIGNAL(friends_loaded(bool)), ui->SEND, SIGNAL(clicked(bool)));
    connect(a, SIGNAL(lastWindowClosed()), this, SLOT(close_app()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::url_changed(QUrl u)
{
    u = u.toString().replace("#", "?");
    QUrlQuery query(u);
    token = query.queryItemValue("access_token");

    if (token.isEmpty())
    {
        return;
    }
    emit auth_success();
}

void MainWindow::get_friends()
{
    QUrl current("https://api.vk.com/method/friends.get");
    QUrlQuery query;
    query.addQueryItem("v","5.59");
    query.addQueryItem("access_token", token);
    query.addQueryItem("fields", "first_name, last_name");
    current.setQuery(query);
    QByteArray answer = GET(current);
    if (answer.isEmpty())
    {
        qDebug() << "No answer";
        exit(-1);
    }
    /*{"response":{"count":1,"items":[{"id":97943655,"first_name":"FirstName","last_name":"LastName","online":0}]}}*/
    QVariantList list = QJsonDocument::fromJson(answer).object().toVariantMap().value("response").toMap().value("items").toList();

    for (int i = 0; i < list.size(); i++)
    {
        QVariantMap current = list[i].toMap();
        QString tmp = current.value("first_name").toString() + " " + current.value("last_name").toString();
        users[tmp] = current.value("id").toString();
    }
    for (User::iterator itr = users.begin(); itr!=users.end(); itr++)
    {
        ui->LIST->addItem(itr.key());
    }

    emit friends_loaded(true);

}

QByteArray MainWindow::GET(QUrl r)
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

void MainWindow::send_message(QStringList messageIdList)
{
    receivedMessage = messageIdList.at(0);
    receiverId = messageIdList.at(1);

    QUrl current;
    if (ui->ANSWER_METHOD->currentText() == "Moon")
    {
        current = moon();
    } else if (ui->ANSWER_METHOD->currentText() == "Parse")
    {
        current = parse_message();
    }

    if (current.toString() == "access_denied")
    {
        qDebug() << "access_denied";
        return;
    }

    QByteArray answer = GET(current);

    if (answer.contains("response"))
    {
        qDebug() << "moon sended";
    } else
    {
        qDebug() << "cannot send moon";
    }
}

QUrl MainWindow::moon()
{
    QString id = users.value(ui->LIST->currentText());
    if (sendMoonStatus == false)
    {
        messageToReceive = "&#127770;";
        sendMoonStatus = true;
    } else
    {
        messageToReceive = "&#127773;";
        sendMoonStatus = false;
    }

    QUrl current("https://api.vk.com/method/messages.send");
    QUrlQuery query;
    if (id.isEmpty())
    {
        query.addQueryItem("user_id", receiverId);
    } else if (id == receiverId)
    {
        query.addQueryItem("user_id", id);
    } else
    {
        current = "access_denied";
        return current;
    }
    query.addQueryItem("message", messageToReceive);
    query.addQueryItem("access_token", token);
    current.setQuery(query);
    return current;
}

QUrl MainWindow::parse_message()
{
    QString id = users.value(ui->LIST->currentText());

    QRegularExpression expMeme("meme \\d");
    QRegularExpression expPron("pron \\d");

    if (!(receivedMessage.contains(expMeme) || receivedMessage.contains(expPron)))
    {
        QUrl current("access_denied");
        return current;
    }

    if (receivedMessage.contains(expMeme))
    {
        QStringList requestString;
        requestString = receivedMessage.split(" ");
        QString groupsNumber = requestString.at(1);

        if ((groupsNumber.toInt() < 1) || (groupsNumber.toInt() > 6))
        {
            QUrl current("access_denied");
            return current;
        }

        QUrl helloMessage("https://api.vk.com/method/messages.send");
        QUrlQuery queryHello;
        if (id.isEmpty())
        {
            queryHello.addQueryItem("user_id", receiverId);
        } else if (id == receiverId)
        {
            queryHello.addQueryItem("user_id", id);
        } else
        {
            QUrl current("access_denied");
            return current;
        }
        queryHello.addQueryItem("message", "Подождите. Запрос обрабатывается...");
        queryHello.addQueryItem("access_token", token);
        helloMessage.setQuery(queryHello);
        QByteArray answerHello = GET(helloMessage);


        QUrl current("https://api.vk.com/method/groups.get");
        QUrlQuery query;
        if (id.isEmpty())
        {
            query.addQueryItem("user_id", receiverId);
        } else if (id == receiverId)
        {
            query.addQueryItem("user_id", id);
        } else
        {
            QUrl current("access_denied");
            return current;
        }
        query.addQueryItem("count", groupsNumber);
        query.addQueryItem("access_token", token);
        current.setQuery(query);
        QByteArray answer = GET(current);

        //{\"response\":[3,69319700,30315369]}
        QVariantList list = QJsonDocument::fromJson(answer).object().toVariantMap().value("response").toList();
        QStringList wallsToSend;
        QStringList groupList;
        for (int i = 1; i < list.size(); i++)
        {
            QString currentGroup = list.at(i).toString();
            QUrl currentWall("https://api.vk.com/method/wall.get");
            QUrlQuery queryWall;
            queryWall.addQueryItem("access_token", token);
            queryWall.addQueryItem("owner_id", "-" + currentGroup);
            queryWall.addQueryItem("offset", "1");
            queryWall.addQueryItem("count", "2");
            currentWall.setQuery(queryWall);
            QByteArray answerWall = GET(currentWall);
            QVariantList wallList = QJsonDocument::fromJson(answerWall).object().toVariantMap().value("response").toList();
            int likesComparator  = 0;
            QString resultId;
            for (int j = 1; j < wallList.size(); j++)
            {
                QString postId = wallList[j].toMap().value("id").toString();
                int postLikes = wallList[j].toMap().value("likes").toMap().value("count").toInt();
                if (postLikes >= likesComparator)
                {
                    likesComparator = postLikes;
                    resultId = postId;
                }
            }
            //qDebug() << likesComparator << ": " << resultId;
            groupList << currentGroup;
            wallsToSend << resultId;
        }
        QUrl resultUrl("https://api.vk.com/method/messages.send");
        QString attachment;
        for (int i = 0; i < groupList.size(); i++)
        {
            attachment.append("wall-" + groupList.at(i) + "_" + wallsToSend.at(i));
            if (groupList.at(i) == groupList.last())
                break;
            attachment.append(",");
        }
        //qDebug() << attachment;
        QUrlQuery resultQuery;
        if (id.isEmpty())
        {
            resultQuery.addQueryItem("user_id", receiverId);
        } else if (id == receiverId)
        {
            resultQuery.addQueryItem("user_id", id);
        } else
        {
            QUrl current("access_denied");
            return current;
        }
        resultQuery.addQueryItem("access_token", token);
        resultQuery.addQueryItem("attachment", attachment);
        resultQuery.addQueryItem("message", "Ваши мемесы, сир");
        resultUrl.setQuery(resultQuery);
        return resultUrl;
    } else if (receivedMessage.contains(expPron))
    {
        QStringList requestString;
        requestString = receivedMessage.split(" ");
        QString videoTimer = requestString.at(1);

        if ((videoTimer.toInt() < 1) || (videoTimer.toInt() > 1000))
        {
            QUrl current("access_denied");
            return current;
        }

        QString secondsHighTimer = QString::number((videoTimer.toInt()*60));

        int randOffset = qrand() % 300;
        QUrl currentVideo("https://api.vk.com/method/video.search");
        QUrlQuery queryVideo;
        queryVideo.addQueryItem("q", "porn");
        queryVideo.addQueryItem("adult", "1");
        queryVideo.addQueryItem("access_token", token);
        queryVideo.addQueryItem("offset", QString::number(randOffset));
        queryVideo.addQueryItem("shorter", secondsHighTimer);
        queryVideo.addQueryItem("count", "1");
        currentVideo.setQuery(queryVideo);
        QByteArray answerVideo = GET(currentVideo);
        if (answerVideo.contains("id") && answerVideo.contains("owner_id"))
        {
            QUrl helloMessage("https://api.vk.com/method/messages.send");
            QUrlQuery queryHello;
            if (id.isEmpty())
            {
                queryHello.addQueryItem("user_id", receiverId);
            } else if (id == receiverId)
            {
                queryHello.addQueryItem("user_id", id);
            } else
            {
                QUrl current("access_denied");
                return current;
            }
            queryHello.addQueryItem("message", "Подождите. Запрос обрабатывается...");
            queryHello.addQueryItem("access_token", token);
            helloMessage.setQuery(queryHello);
            QByteArray answerHello = GET(helloMessage);

            QString videoId = QJsonDocument::fromJson(answerVideo).object().toVariantMap().value("response").toList().at(0).toMap().value("id").toString();
            QString videoOwnerId = QJsonDocument::fromJson(answerVideo).object().toVariantMap().value("response").toList().at(0).toMap().value("owner_id").toString();

            QUrl resultUrl("https://api.vk.com/method/messages.send");
            QString attachment("video" + videoOwnerId + "_" + videoId);
            QUrlQuery resultQuery;
            if (id.isEmpty())
            {
                resultQuery.addQueryItem("user_id", receiverId);
            } else if (id == receiverId)
            {
                resultQuery.addQueryItem("user_id", id);
            } else
            {
                QUrl current("access_denied");
                return current;
            }
            resultQuery.addQueryItem("access_token", token);
            resultQuery.addQueryItem("attachment", attachment);
            resultQuery.addQueryItem("message", "Ваш прон, сир");
            resultUrl.setQuery(resultQuery);
            return resultUrl;
      } else
        {
            QUrl current("access_denied");
            return current;
        }
    } else
    {
        QUrl current("access_denied");
        return current;
    }
}

void MainWindow::close_app()
{
    m_server->mainCycleStatus = false;
    m_server->quit();
}

void MainWindow::on_SEND_clicked()
{
    ui->SEND->setDisabled(true);
    ui->STOPSEND->setEnabled(true);
    ui->LIST->setDisabled(true);
    ui->ANSWER_METHOD->setDisabled(true);

    m_server = new VKLongPollServer(token);
    connect(m_server, SIGNAL(send_message(QStringList)), this, SLOT(send_message(QStringList)));
    m_server->mainCycleStatus = true;
    m_server->start();
}

void MainWindow::on_STOPSEND_clicked()
{    
    ui->STOPSEND->setDisabled(true);
    ui->SEND->setEnabled(true);
    ui->LIST->setEnabled(true);
    ui->ANSWER_METHOD->setEnabled(true);

    m_server->mainCycleStatus = false;
    m_server->quit();
}
