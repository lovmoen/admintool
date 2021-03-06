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

#define GF_VERSION "1.0.0"

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

enum CommandLineParseResult
{
    CommandLineOk,
    CommandLineError,
    CommandLineVersionRequested,
    CommandLineHelpRequested
};

struct CmdLineValues
{
    bool testOptionSet;
    bool daemonOptionSet;
    QString serversFile;
    QString restEndpoint;
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, CmdLineValues* pValues, QString *errorMessage)
{
    const QCommandLineOption testOption(QStringList() << "t" << "test", "Run tests");
    parser.addOption(testOption);
    const QCommandLineOption daemonOption(QStringList() << "d" << "daemon", "Run as daemon");
    parser.addOption(daemonOption);
    const QCommandLineOption serversFileOption(QStringList() << "s" << "servers", "The file to read servers from.", "serversFile");
    parser.addOption(serversFileOption);
    const QCommandLineOption restOption(QStringList() << "r" << "rest", "The rest endpoint to post to.", "restEndpoint");
    parser.addOption(restOption);
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    if (!parser.parse(QCoreApplication::arguments())) {
        *errorMessage = parser.errorText();
        return CommandLineError;
    }

    if (parser.isSet(versionOption))
        return CommandLineVersionRequested;

    if (parser.isSet(helpOption))
        return CommandLineHelpRequested;

    pValues->daemonOptionSet = parser.isSet(daemonOption);
    pValues->testOptionSet = parser.isSet(testOption);

    if (parser.isSet(serversFileOption)) {
        const QString serversFile = parser.value(serversFileOption);
        if (!QFileInfo::exists(serversFile) || !QFileInfo(serversFile).isFile()) {
            *errorMessage = QString("Servers file %1 does not exist.").arg(serversFile);
            return CommandLineError;
        }
        pValues->serversFile = serversFile;
    }

    if (parser.isSet(restOption)) {
        const QString restString = parser.value(restOption);
        QUrl restUrl(restString);
        if (!restUrl.isValid()) {
            *errorMessage = "Bad URL: " + restString;
            return CommandLineError;
        }
        pValues->restEndpoint = restString;
    }

    return CommandLineOk;
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
    QCoreApplication::setApplicationVersion(GF_VERSION);
    QCoreApplication::setApplicationName("Server Monitor (GF)");

    QCommandLineParser parser;
    parser.setApplicationDescription("A daemon-ized version of SourceAdminTool.");
    CmdLineValues cmdLineValues;
    cmdLineValues.serversFile = "servers.json";
    cmdLineValues.restEndpoint = "http://localhost:51347/api/gameserver";
    QString errorMessage;
    switch (parseCommandLine(parser, &cmdLineValues, &errorMessage))
    {
        case CommandLineOk:
            break;
        case CommandLineError:
            cerr << errorMessage.toStdString() << endl << endl;
            cerr << parser.helpText().toStdString() << endl;
            return 1;
        case CommandLineVersionRequested:
            cout << QCoreApplication::applicationName().toStdString() << " " << QCoreApplication::applicationVersion().toStdString() << endl;
            return 0;
        case CommandLineHelpRequested:
            parser.showHelp();
            Q_UNREACHABLE();
    }

    MainTask task(&a, cmdLineValues.serversFile, cmdLineValues.restEndpoint, cmdLineValues.testOptionSet);
    QObject::connect(&task, &MainTask::finished, &a, &QCoreApplication::quit);
    QTimer::singleShot(0, &task, &MainTask::initialize);

    return a.exec();
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (strncmp(argv[i], "--daemon", strlen(argv[i])) == 0 ||
            strncmp(argv[i], "-d", strlen(argv[i])) == 0)
        {
            return RunCLI(argc, argv);
        }
    }
    return RunUI(argc, argv);
}
