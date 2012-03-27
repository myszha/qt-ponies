#include <QPushButton>
#include <QDateTime>

#include <map>
#include <utility>

#include "debugwindow.h"
#include "ui_debugwindow.h"


static const std::map<QtMsgType, std::pair<QString, QString>> msg_types {
    {QtMsgType::QtDebugMsg, {"black","Debug"}},
    {QtMsgType::QtWarningMsg, {"blue","Warning"}},
    {QtMsgType::QtCriticalMsg, {"red","Error"}},
    {QtMsgType::QtFatalMsg, {"red","Fatal"}},
    {QtMsgType::QtSystemMsg, {"red","System"}}
};

DebugWindow::DebugWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DebugWindow)
{
    ui->setupUi(this);

    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), ui->textEdit, SLOT(clear()));
}

DebugWindow::~DebugWindow()
{
    delete ui;
}

void DebugWindow::handle_message(QtMsgType type, const char *msg)
{
    static QString output("<font color=\"%1\">[%2][%3] %4</font>");
    auto &msg_type = msg_types.find(type)->second;
    ui->textEdit->append(output.arg(msg_type.first,QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate), msg_type.second, msg));
}
