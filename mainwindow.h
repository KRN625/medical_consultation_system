#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMultimedia>
#include <QListWidgetItem>
#include <QEvent>
#include <QLabel>
#include <QProgressBar>
#include <QInputDialog>
#include <QMessageBox>
#include "dialog.h"

class QTreeWidgetItem;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    void showTipMessage(const QString text);//显示提示信息

    //======================状态栏=====================
    QLabel *audioItemLabel;
    QLabel *dirItemLabel;
    QProgressBar *progressBar;

    //======================目录树======================
    //枚举类型treeItemType， 用于创建 QTreeWidgetItem 时作为节点的type, 自定义类型必须大于1000
    //itTopItem 顶层节点;  itGroupItem 组节点； itImageItem 图片
    enum    treeItemType{itTopItem=1001,itGroupItem,itAudioItem};

    QString getTopDir();                            //获取配置文件中配置的顶层目录
    void setTopDir(QString path);                               //设置配置文件中的顶层目录

    void addSubDirItem(QTreeWidgetItem *parent);    //递归查询顶层节点的子目录，并将其添加到顶层节点
    void buildDirTree(const QString &path);         //构建左侧目录树


    //=======================音乐列表====================
    QMediaPlayer  *player;      //播放器
    bool    loopPlay=true;      //是否循环播放
    QString  durationTime;      //文件总长度，mm:ss字符串
    QString  positionTime;      //当前播放到位置，mm:ss字符串

    QUrl getUrlFromItem(QListWidgetItem *item);         //获取item的用户数据
    bool eventFilter(QObject *watched, QEvent *event);  //事件过滤处理

    bool isRefresh;//应对刷新时发射currentItemChanged信号

    QList<QTreeWidgetItem*> getAllParents(QTreeWidgetItem *item);//获取指定项目的父链
    void expandToCurItem(QTreeWidgetItem *curItem);

    void initPlayer();//初始化player


public:
    void buildAudioList(QTreeWidgetItem *item);     //构建右侧音频文件列表
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //自定义槽函数

    void do_sourceChanged(const QUrl &media);   //文件发生变化

    void do_stateChanged(QMediaPlayer::PlaybackState state);

    void showContextMenu(const QPoint &pos);    //显示QTreeWidgetItem右键菜单

    void showListWidgetContextMenu(const QPoint &pos);//显示QListWidgetItem右键菜单

    //自动生成的槽函数
    void on_actOpenDir_triggered();

    void on_actRefresh_triggered();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_actAddDir_triggered();

    void on_actDelDir_triggered();

    void on_actAddFile_triggered();

    void on_actDelFile_triggered();

    void on_actTape_triggered();

    void on_actTts_triggered();

private:
    Ui::MainWindow *ui;

    Dialog *dialog;
};
#endif // MAINWINDOW_H
