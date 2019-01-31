#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "rcon.h"
#include "settings.h"
#include <QMessageBox>
#include <QScrollBar>

extern QList<ServerInfo *> serverList;

void MainWindow::runCommand(ServerInfo *info, QString command)
{
    if(command.length() != 0)
    {
        int sliderPos = this->ui->commandOutput->verticalScrollBar()->sliderPosition();
        bool shouldAutoScroll = sliderPos == this->ui->commandOutput->verticalScrollBar()->maximum();

        if(this->ui->commandText->text() == "clear")
        {
            this->ui->commandOutput->clear();
            this->ui->commandOutput->moveCursor(QTextCursor::End);
            this->ui->commandOutput->appendPlainText(QString("] %1").arg(this->ui->commandText->text()));
            this->ui->commandOutput->moveCursor(QTextCursor::End);
            info->rcon->execCommand(this->ui->commandText->text());
            this->ui->commandText->setText("");

            return;
        }
        this->ui->commandOutput->appendPlainText(QString("] %1").arg(this->ui->commandText->text()));
        info->rconOutput.append(QString("] %1\n").arg(this->ui->commandText->text()));
        this->ui->commandOutput->moveCursor(QTextCursor::End);
        this->ui->commandOutput->appendPlainText("");
        this->ui->commandOutput->moveCursor(QTextCursor::End);
        info->rcon->execCommand(this->ui->commandText->text());
        this->ui->commandText->setText("");

        if(!shouldAutoScroll)
            this->ui->commandOutput->verticalScrollBar()->setSliderPosition(sliderPos);
        else
           this->ui->commandOutput->verticalScrollBar()->setSliderPosition(this->ui->commandOutput->verticalScrollBar()->maximum());
    }
}

//RCON HANDLING
void MainWindow::processCommand()
{
    if(this->ui->browserTable->selectedItems().size() == 0)
    {
        this->browserTableItemSelected();
        return;
    }

    ServerTableIndexItem *item = this->GetServerTableIndexItem(this->ui->browserTable->currentRow());
    ServerInfo *info = item->GetServerInfo();

    if(info && (info->rcon == NULL || !info->rcon->isAuthed))
    {
        QList<QueuedCommand>cmds;
        cmds.append(QueuedCommand(this->ui->commandText->text(), QueuedCommandType::ConsoleCommand));
        this->rconLoginQueued(cmds);
        return;
    }

    this->runCommand(info, this->ui->commandText->text());
}

void MainWindow::rconSaveToggled(bool checked)
{
    if(this->ui->browserTable->selectedItems().size() == 0)
    {
        this->browserTableItemSelected();
        return;
    }

    ServerTableIndexItem *item = this->GetServerTableIndexItem(this->ui->browserTable->currentRow());
    ServerInfo *info = item->GetServerInfo();

    info->saveRcon = checked;
}

void MainWindow::passwordUpdated(const QString &text)
{
    if(this->ui->browserTable->selectedItems().size() == 0)
    {
        this->browserTableItemSelected();
        return;
    }

    ServerTableIndexItem *item = this->GetServerTableIndexItem(this->ui->browserTable->currentRow());
    ServerInfo *info = item->GetServerInfo();

    info->rconPassword = text;
}

void MainWindow::rconLogin()
{
    if(this->ui->browserTable->selectedItems().size() == 0)
    {
        this->browserTableItemSelected();
        return;
    }

    ServerTableIndexItem *item = this->GetServerTableIndexItem(this->ui->browserTable->currentRow());
    ServerInfo *info = item->GetServerInfo();

    if(info->rcon == NULL)
    {
        info->rcon = new RconQuery(this, NULL, info);
    }
    else if(info->rcon->isAuthed)
    {
        QMessageBox message(this);
        message.setText("Already authenticated");
        message.exec();
        return;
    }
    if(info->rconPassword == 0)
    {
        QMessageBox message(this);
        message.setText("Please enter a password");
        message.exec();
        return;
    }
    info->rcon->auth();
}

void MainWindow::rconLoginQueued(QList<QueuedCommand>queuedcmds)
{
    if(this->ui->browserTable->selectedItems().size() == 0)
    {
        this->browserTableItemSelected();
        return;
    }

    ServerTableIndexItem *item = this->GetServerTableIndexItem(this->ui->browserTable->currentRow());
    ServerInfo *info = item->GetServerInfo();

    if(info->rcon == NULL)
    {
        info->rcon = new RconQuery(this, NULL, info);
    }
    else if(info->rcon->isAuthed)
    {
        QMessageBox message(this);
        message.setText("Already authenticated");
        message.exec();
        return;
    }
    if(info->rconPassword == 0)
    {
        QMessageBox message(this);
        message.setText("Please enter a password");
        message.exec();
        return;
    }
    info->rcon->queuedList = queuedcmds;
    this->rconLogin();

}

void MainWindow::RconAuthReady(ServerInfo *info, QList<QueuedCommand>queuedcmds)
{
    if(this->ui->browserTable->selectedItems().size() == 0)
    {
        this->browserTableItemSelected();
        return;
    }

    ServerTableIndexItem *item = this->GetServerTableIndexItem(this->ui->browserTable->currentRow());
    ServerInfo *currentInfo = item->GetServerInfo();

    if(!info->rcon->isAuthed)
    {
        QMessageBox message(this);
        message.setText(QString("Failed to authenticate %1").arg(info->hostPort));
        message.exec();
        return;
    }
    else
    {
        info->rcon->execCommand("echo Welcome user!", false);
        QueuedCommand queuedcmd;
        foreach(queuedcmd, queuedcmds)
        {
            if(queuedcmd.commandType == QueuedCommandType::GetLogCommand)//Get log
            {
                info->rcon->execCommand(queuedcmd.command, false);
                pLogHandler->addServer(info);
            }
            else if(queuedcmd.commandType == QueuedCommandType::ContextCommand)
            {
                info->rcon->execCommand(queuedcmd.command, false);
            }
            else //command
            {
                if(info == currentInfo)//If current server print the stuff
                    this->runCommand(info, queuedcmd.command);
                else //Not current store it only.
                {
                    info->rconOutput.append(QString("] %1\n").arg(queuedcmd.command));
                    info->rcon->execCommand(queuedcmd.command, true);
                }
            }
        }
    }
}

void MainWindow::RconOutput(ServerInfo *info, QByteArray result)
{
    if(this->ui->browserTable->selectedItems().size() == 0)
    {
        this->browserTableItemSelected();
        return;
    }

    if(result.length() == 0)
    {
        return;
    }

    ServerTableIndexItem *item = this->GetServerTableIndexItem(this->ui->browserTable->currentRow());
    ServerInfo *currentInfo = item->GetServerInfo();

    if(info == currentInfo)
    {
        int sliderPos = this->ui->commandOutput->verticalScrollBar()->sliderPosition();
        bool shouldAutoScroll = sliderPos == this->ui->commandOutput->verticalScrollBar()->maximum();

        this->ui->commandOutput->moveCursor(QTextCursor::End);
        this->ui->commandOutput->insertPlainText(result);
        this->ui->commandOutput->moveCursor(QTextCursor::End);

        if(!shouldAutoScroll)
            this->ui->commandOutput->verticalScrollBar()->setSliderPosition(sliderPos);
        else
           this->ui->commandOutput->verticalScrollBar()->setSliderPosition(this->ui->commandOutput->verticalScrollBar()->maximum());
    }

    if(info && !result.isEmpty())
    {
        while(info->rconOutput.size() > 100)
            info->rconOutput.removeFirst();

        info->rconOutput.append(result);
    }
}

void MainWindow::showRconClicked(bool checked)
{
    if(checked)
        ui->rconPassword->setEchoMode(QLineEdit::Normal);
    else
        ui->rconPassword->setEchoMode(QLineEdit::Password);
}

void MainWindow::SetRconSignals(bool block)
{
    this->ui->rconSave->blockSignals(block);
    this->ui->rconPassword->blockSignals(block);
    this->ui->commandOutput->blockSignals(block);
}

void MainWindow::RestoreRcon(ServerInfo *info)
{
    if(info == nullptr)
        return;

    this->ui->rconSave->setChecked(info->saveRcon);
    this->ui->rconPassword->setText(info->rconPassword);
    this->ui->commandOutput->setPlainText(info->rconOutput.join(""));
    this->ui->logOutput->setPlainText(info->logOutput.join(""));
    this->ui->chatOutput->setHtml(info->chatOutput.join(""));
    this->ui->logOutput->moveCursor(QTextCursor::End);
    this->ui->commandOutput->moveCursor(QTextCursor::End);
    this->ui->chatOutput->moveCursor(QTextCursor::End);
}

void MainWindow::SetRconEnabled(bool enabled)
{
    this->ui->rconSave->setEnabled(enabled);
    this->ui->rconPassword->setEnabled(enabled);
    this->ui->commandOutput->setEnabled(enabled);
    this->ui->commandText->setEnabled(enabled);
    this->ui->rconLogin->setEnabled(enabled);
    this->ui->logGetLog->setEnabled(enabled);
    this->ui->logOutput->setEnabled(enabled);
    this->ui->chatOutput->setEnabled(enabled);
    this->ui->sendChat->setEnabled(enabled);
}
