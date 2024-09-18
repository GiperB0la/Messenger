#include "Messenger.h"


Messenger::Messenger(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MessengerClass()), client(new Client(this))
{
    ui->setupUi(this);
    this->setStyleSheet(
        "QMainWindow { background-color: rgb(30, 30, 30); color: rgb(240, 240, 240); }"
        "QGroupBox { background-color: rgb(30, 30, 30); color: rgb(240, 240, 240); }"
        "QPushButton { background-color: rgb(80, 80, 80); color: rgb(240, 240, 240); }"
    );

    ui->inputText->setStyleSheet("background-color: rgb(45, 45, 45); color: rgb(240, 240, 240);");
    ui->textMessanger->setStyleSheet("background-color: rgb(30, 30, 30); color: rgb(240, 240, 240);");
    ui->textMessanger->setReadOnly(true);

    clientsMenu = new QMenu(this);
    ui->groupBox->installEventFilter(this);

    inputNameInBox();
}

Messenger::~Messenger()
{}

void Messenger::inputNameInBox()
{
    QString name;
    while (name.isEmpty()) {
        QInputDialog inputDialog(this);
        inputDialog.setWindowTitle(tr("Messenger"));
        inputDialog.setLabelText(tr("Input your name:"));
        inputDialog.setStyleSheet(
            "QInputDialog { background-color: rgb(30, 30, 30); color: rgb(240, 240, 240); }"
            "QLabel { color: rgb(240, 240, 240); }"
            "QLineEdit { background-color: rgb(30, 30, 30); color: rgb(240, 240, 240); }"
            "QPushButton { background-color: rgb(80, 80, 80); color: rgb(240, 240, 240); }"
        );

        if (inputDialog.exec() == QDialog::Accepted) {
            name = inputDialog.textValue().trimmed();

            if (!name.isEmpty()) {
                client->name = name.toStdString();

                connect(client, &Client::updateChat, this, &Messenger::onUpdateChat);
                connect(ui->buttonSend, &QPushButton::clicked, [this]() {
                    if (ui->inputText->text().isEmpty())
                        return;
                    QString message = QString::fromStdString(client->whom) + ui->inputText->text();
                    appendMessage(message, true);
                    client->sendMessage(message.toStdString());
                    ui->inputText->clear();
                    });

                client->start();
            }
        }
        else {
            appendMessage("No name entered. Exiting...", false);
            close();
            return;
        }
    }
}

void Messenger::appendMessage(const QString& message, bool isSent)
{
    QString newMessage;
    QString whom;
    for (QChar c : message) {
        if (c != '{' && c != '}' && !c.isDigit()) {
            newMessage += c;
        }
        else {
            if (c.isDigit()) {
                whom += c;
            }
        }
    }

    if (!newMessage.isEmpty()) {
        if (isSent) {
            if (!message.isEmpty())
                ui->textMessanger->append("You> " + newMessage);
        }
        else {
            ui->textMessanger->append(QString::fromStdString(client->otherClients[whom.toStdString()]) + newMessage);
        }
    }
}

void Messenger::onUpdateChat(QString message)
{
    appendMessage(message, false);
}

void Messenger::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        ui->buttonSend->click();
        event->accept();
    }
    else {
        QMainWindow::keyPressEvent(event);
    }
}

void Messenger::onClientActionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString clientName = action->text();
        appendMessage("Selected " + clientName, false);
        client->whom = "{" + client->otherClients[clientName.toStdString()] + "}";
    }
}

bool Messenger::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->groupBox && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            showClientsMenu();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void Messenger::showClientsMenu()
{
    updateShowMenu();

    clientsMenu->setStyleSheet(
        "QMenu { "
        "    background-color: black; "
        "    color: white; "
        "    border: 1px solid white; "
        "} "
        "QMenu::item { "
        "    padding: 5px 20px; "
        "    background-color: black; "
        "    color: white; "
        "    text-align: center; "
        "} "
        "QMenu::item:selected { "
        "    background-color: rgb(80, 80, 80); "
        "}"
    );

    QRect groupBoxRect = ui->groupBox->rect();

    QPoint globalPos = ui->groupBox->mapToGlobal(groupBoxRect.bottomLeft() + QPoint(48, -ui->groupBox->height()));

    clientsMenu->exec(globalPos);
}

void Messenger::addClient(const QString& clientName)
{
    QAction* clientAction = new QAction(clientName, clientsMenu);
    connect(clientAction, &QAction::triggered, this, &Messenger::onClientActionTriggered);
    clientsMenu->addAction(clientAction);
}

void Messenger::updateShowMenu()
{
    clientsMenu->clear();

    for (auto clientName : client->otherClients) {
        addClient(QString::fromStdString(clientName.first));
    }

    client->whom.clear();
}