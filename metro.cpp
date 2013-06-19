#include "metro.h"
#include <QtGui/QApplication>
#include <QDesktopWidget>
#include <QStringList>
#include <QtGui/QDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QColorDialog>
#include <QtGui/QFontDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QProcess>
#include <QtGui/QKeyEvent>
#include "lua/lua.hpp"
#include "lmetrolib.h"

Unix* UNIX = new Unix();
MFile* MFILE = new MFile();

Metro::Metro(QWidget *parent)
    : QWebView(parent)
{
    QWebSettings* defaultSettings = QWebSettings::globalSettings();  
    QWebSettings::enablePersistentStorage("");  
    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true);  
    // Plug-ins must be set to be enabled to use plug-ins.  
    defaultSettings->setAttribute(QWebSettings::PluginsEnabled,true);  
    defaultSettings->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls,true);  
    defaultSettings->setObjectCacheCapacities(0, 0, 0); 
    setWindowTitle("QtMetro - Cubway");
//    setWindowFlags(Qt::WindowStaysOnBottomHint | Qt::FramelessWindowHint);
    if(QApplication::arguments().length() <= 1)
        load(QUrl("http://erhandsome.org/subway"));
    else
        load(QUrl(QApplication::arguments()[1]));
//    showFullScreen();
    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            this, SLOT(javaScriptWindowObjectCleared()));
    lua = luaL_newstate();
    luaL_openmetrolib(lua);
    Mainview = this;
    RunLua("libmetro.lua");
}
Metro* Metro::Mainview = NULL;

Metro::~Metro()
{
    lua_close(lua);
    Mainview = NULL;
}

void Metro::javaScriptWindowObjectCleared()
{
    page()->mainFrame()->addToJavaScriptWindowObject("MetroView", this);
    page()->mainFrame()->addToJavaScriptWindowObject("MetroFile", MFILE);
    page()->mainFrame()->addToJavaScriptWindowObject("UNIX", UNIX);
}

QString Metro::System(QString str)
{
    QProcess *qp = new QProcess;
    qp->start(str);
    if (!qp->waitForStarted())
      return "1";
    if (!qp->waitForFinished())
      return "2";
    QByteArray result = qp->readAll();
    return QString(result);
}

QString Metro::Exec(QString str,QString args,int wait)
{
    QProcess *qp = new QProcess;
    if (!args.isNull()||!args.isEmpty()){
    QStringList Args=args.split(' ');
    qp->start(str,Args);
    }else qp->start(str);

    if (wait==0){
    if (!qp->waitForStarted())
      return "1";
    if (!qp->waitForFinished())
      return "2";
    QByteArray result = qp->readAll();
    return QString(result);
    } else return "-1";
}

QString Metro::OpenFile()
{
    return QFileDialog::getOpenFileName(this,tr("Open File"));
}

QString Metro::GetColor()
{
    QColor color = QColorDialog::getColor();
    if(color.isValid()){
    int r = color.red();
    int g = color.green();
    int b = color.blue();
    return "rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+")";
    }else{
    return "-1";
    }
}

QString Metro::GetFont(QString family,int size,QString weight,QString style)
{
    bool ok;
    bool _style;
    QFont font;
    QString font_style;
    QString font_weight;
    QString font_family;
    if (style=="italic") _style=true; else _style=false; 
    if(weight=="bold")
    font = QFontDialog::getFont(&ok,QFont(family, size,QFont::Bold,_style),this);
    else
    font = QFontDialog::getFont(&ok,QFont(family, size,QFont::Normal,_style),this);
    if (ok) {
    font_family=font.family();
    if(font.italic()) font_style="italic"; else font_style="normal";
    if(font.bold()) font_weight="bold"; else font_weight="normal";
//    return "font-family:"+font_family.replace(QString(" "), QString("-"))+";font-size:"+QString::number(font.pointSize())+";font-style:"+font_style+";font-weight:"+font_weight+";";
//    return font_family.replace(QString(" "), QString("-"))+" "+QString::number(font.pointSize())+"px "+font_style+" "+font_weight;
    return font_style+" "+font_weight+" "+QString::number(font.pointSize())+"px "+'"'+font_family+'"';
    }else{
    return "-1";
    }
}

void Metro::WinTitle(QString title)
{
    Mainview->setWindowTitle(title);
}

void Metro::WinResize(int w,int h)
{
    Mainview->resize(w,h);
}

void Metro::WinPos(int x,int y)
{
    QDesktopWidget* desktop = QApplication::desktop();
    if(x==-1&&y==-1){
    Mainview->move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);
    }else{
    Mainview->move(x,y);
    }
}
/*
void Metro::WinFlag(QString flag)
{
    if(flag=="below") Mainview->setWindowFlags(Qt::WindowStaysOnBottomHint);
    else if(flag=="above") Mainview->setWindowFlags(Qt::WindowStaysOnTopHint);
    else Mainview->setWindowFlags(Qt::Window);
}
*/

void Metro::WinFullScreen()
{
    Mainview->showFullScreen();
}

void Metro::WinMaximize()
{
    Mainview->showMaximized();
}

void Metro::WinMinimize()
{
    Mainview->showMinimized();
}

void Metro::WinNormal()
{
    Mainview->showNormal();
}

void Metro::QtAlert(QString str)
{
    QMessageBox::information(this,"QtAlert",str);
}


void Metro::RunLua(QString str)
{
    luaL_loadfile(lua,str.toAscii());
    lua_pcall(lua,0,LUA_MULTRET,0);
}

void Metro::RunLuaString(QString str)
{
    luaL_loadstring(lua,str.toAscii());
    lua_pcall(lua,0,LUA_MULTRET,0);
}

void Metro::keyPressEvent(QKeyEvent *ke)
{
    //if(ke->key() == Qt::Key_Super_L || ke->key() == Qt::Key_Super_R)
    //if(ke->key() == Qt::Key_Alt)
    {

    }
    //else
    {
        QWebView::keyPressEvent(ke);
    }
}

void Metro::Hide()
{
    hide();
}

//MetroFile
void MFile::DownLoad(QString url,QString tofile)
{
    QProcess *qp = new QProcess;
    qp->start("wget",QStringList() << url << "-O" << tofile);
}

//UNIX

void Unix::SendNotify(QString str,QString icon)
{
    QProcess *qp = new QProcess;
    qp->start("notify-send",QStringList() << str << "-i" << icon);
}
