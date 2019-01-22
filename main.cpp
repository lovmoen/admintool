#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <iostream>
#include "mainwindow.h"
#include "maintask.h"

using namespace std;

QPalette defaultPalette;
static int RunUI(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));

    defaultPalette = a.palette();
    MainWindow w;

    return a.exec();
}

static int RunCLI(int argc, char *argv[])
{
#ifdef _WIN32
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif

    QCoreApplication a(argc, argv);

    MainTask *task = new MainTask(&a);
    QObject::connect(task, &MainTask::finished, &a, &QCoreApplication::quit);
    QTimer::singleShot(0, task, &MainTask::initialize);

    return a.exec();
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (strncmp(argv[i], "--cli", strlen(argv[i])) == 0)
        {
            return RunCLI(argc, argv);
        }
    }
    return RunUI(argc, argv);
}
