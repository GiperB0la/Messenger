#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_Messenger.h"

#include <QInputDialog>
#include <QKeyEvent>

#include <QMenu>
#include <QMenuBar>
#include <QAction>

#include "../include/Client.h"


class Messenger : public QMainWindow
{
    Q_OBJECT

public:
    Messenger(QWidget *parent = nullptr);
    ~Messenger();

private slots:
    void inputNameInBox();
    void onUpdateChat(QString message);
    void appendMessage(const QString& message, bool isSent);
    void keyPressEvent(QKeyEvent* event) override;
    void onClientActionTriggered();
    void showClientsMenu();
    void addClient(const QString& clientName);
    void updateShowMenu();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::MessengerClass* ui;
    Client* client;
    QMenu* clientsMenu;
};