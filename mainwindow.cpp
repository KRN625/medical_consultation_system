#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tts.cpp"

#include <qfiledialog.h>
#include <QTreeWidgetItem>
#include <QMessageBox>

QUrl MainWindow::getUrlFromItem(QListWidgetItem *item)
{
    QVariant itemData= item->data(Qt::UserRole);   //获取用户数据
    QUrl source =itemData.value<QUrl>();    //QVariant转换为QUrl类型
    return source;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() != QEvent::KeyPress)      //不是KeyPress事件，退出
        return QWidget::eventFilter(watched,event);

    QKeyEvent *keyEvent=static_cast<QKeyEvent *>(event);
    if (keyEvent->key() != Qt::Key_Delete)      //按下的不是Delete键，退出
        return QWidget::eventFilter(watched,event);

    if (watched==ui->listWidget)
    {
        QListWidgetItem *item= ui->listWidget->takeItem(ui->listWidget->currentRow());
        delete  item;
    }
    return true;    //表示事件已经被处理
}

QList<QTreeWidgetItem *> MainWindow::getAllParents(QTreeWidgetItem *item)
{
    QList<QTreeWidgetItem*> parents;

    QTreeWidgetItem *parent = item->parent();
    while (parent) {
        parents.prepend(parent); // 将父项添加到列表的开头
        parent = parent->parent();
    }

    return parents;
}

void MainWindow::expandToCurItem(QTreeWidgetItem *curItem)
{
    QList<QTreeWidgetItem*> parents=getAllParents(curItem);
    foreach (QTreeWidgetItem* parent, parents) {
        parent->setExpanded(true);
    }
    curItem->setExpanded(true);
}

void MainWindow::initPlayer()
{
    player = new QMediaPlayer(this);
    QAudioOutput *audioOutput = new QAudioOutput(this);   //音频输出，指向默认的音频输出设备
    player->setAudioOutput(audioOutput);    //设置音频输出

    // 监听状态变化信号
    connect(player, &QMediaPlayer::playbackStateChanged,
            this, &MainWindow::do_stateChanged);

    connect(player, &QMediaPlayer::sourceChanged,       //播放源发生变化
            this, &MainWindow::do_sourceChanged);

}




void MainWindow::showTipMessage(const QString text)
{
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.setWindowIcon(QIcon(":/icons/imgs/tip.png"));
    msgBox.setWindowTitle("提示");
    msgBox.setIcon(QMessageBox::Information);

    // 添加一个“确定”按钮
    msgBox.addButton(QMessageBox::Ok);

    // 显示提示框
    msgBox.exec();
    return;
}

QString MainWindow::getTopDir()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = QCoreApplication::applicationDirPath() + "/config.ini";
    // 创建QSettings对象并指定INI文件的路径
    QSettings settings(configFile, QSettings::IniFormat);
    // 读取配置项
    return settings.value("topDir").toString();
}

void MainWindow::setTopDir(QString path)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = QCoreApplication::applicationDirPath() + "/config.ini";
    // 创建QSettings对象并指定INI文件的路径
    QSettings settings(configFile, QSettings::IniFormat);
    // 设置配置项
    settings.setValue("topDir", path);
}

//递归查询顶层节点的子目录，并将其添加到顶层节点
void MainWindow::addSubDirItem(QTreeWidgetItem *parent)
{
    qDebug()<<"[debug] addSubDirItem()";

    //获取文件夹绝对路径
    QString dir=parent->data(0,Qt::UserRole).toString();

    qDebug()<<"[debug] "<<dir;

    QDir folder(dir);

    if(!folder.exists())
        qDebug()<<"[debug] "<<dir<<" not exists";

    QStringList subFolders=folder.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if(!subFolders.isEmpty()){
        foreach (const QString &subFolder, subFolders) {
            QTreeWidgetItem *item=new QTreeWidgetItem(MainWindow::itGroupItem);

            item->setText(0,subFolder);
            item->setIcon(0,QIcon(":/icons/imgs/folder.png"));
            item->setData(0,Qt::UserRole,folder.absoluteFilePath(subFolder));

            addSubDirItem(item);

            parent->addChild(item);
        }
    }else {
        qDebug()<<"[debug] "<<dir<<" don't have any folder";
    }
}

void MainWindow::buildDirTree(const QString &path)
{
    if(path.isEmpty())
        return;
    QDir folder(path);
    qDebug()<<"[debug]"<<" on_pushButton_2_clicked()";
    // 确保文件夹存在
    if (folder.exists()) {
        // 获取文件夹中的所有子文件夹
        QStringList subFolders = folder.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        //清除目录树
        ui->treeWidget->clear();

        // 打印所有子文件夹的名称
        foreach (const QString &subFolder, subFolders) {
            qDebug()<<"[debug]" << "Subfolder: " << folder.absoluteFilePath(subFolder);
            QTreeWidgetItem *item=new QTreeWidgetItem(MainWindow::itTopItem);
            item->setText(0,subFolder);
            item->setIcon(0,QIcon(":/icons/imgs/folder.png"));

            //将子目录的绝对路径作为作为用户数据存储在节点中
            item->setData(0,Qt::UserRole,folder.absoluteFilePath(subFolder));

            addSubDirItem(item);

            //将item节点添加为顶层节点
            ui->treeWidget->addTopLevelItem(item);
        }
    } else {
        qDebug() << "[debug] Folder does not exist!";
    }
}

void MainWindow::buildAudioList(QTreeWidgetItem *item)
{
    qDebug()<<"[debug] buildAudioList()";
    ui->listWidget->clear();
    if(isRefresh)
        return;

    QString path=item->data(0,Qt::UserRole).toString();//当前节点的路径
    QDir folder(path);//创建一个QDir对象，表示要搜索的文件夹
    QStringList audioFiles; // 用于存储音频文件的绝对路径列表

    //检查目录是否存在
    if(folder.exists()){
        QStringList audioFileExtensions = {"*.mp3", "*.wav","*.m4a","*wma"}; // 定义音频文件的扩展名

        // 遍历文件夹中的所有文件
        for (const QString &extension : audioFileExtensions) {
            QStringList matchingFiles = folder.entryList(QStringList(extension), QDir::Files);

            // 将匹配的文件的绝对路径添加到列表中
            for (const QString &fileName : matchingFiles) {
                QString filePath = folder.absoluteFilePath(fileName);
                audioFiles.append(filePath);
            }
        }
        // 将音频文件放到listwidget中
        if (audioFiles.count()<1)
            return;

        for (int i=0; i<audioFiles.size();i++)
        {
            QString  aFile=audioFiles.at(i);
            qDebug()<<"[debug] "<<aFile;
            QFileInfo  fileInfo(aFile);
            QListWidgetItem *aItem =new QListWidgetItem(fileInfo.fileName());
            aItem->setIcon(QIcon(":/icons/imgs/note.png"));
            aItem->setData(Qt::UserRole, QUrl::fromLocalFile(aFile));  //设置用户数据，QUrl对象
            ui->listWidget->addItem(aItem);
        }
    }else {
        qDebug() << "Directory does not exist.";
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    dialog=new Dialog(this);

    isRefresh=false;//防止刷新时currentItemChanged信号更新audioList

    buildDirTree(getTopDir());//构建初始目录

    //=================删除dockwidget顶部标题栏================
    QWidget* pTitleBar = ui->dockWidget_3->titleBarWidget();//去除dockwidget顶部标题栏
    QWidget* pEmptyWidget = new QWidget();
    ui->dockWidget_3->setTitleBarWidget(pEmptyWidget);
    delete pTitleBar;
    pTitleBar=NULL;

    //======================设置listWidget===================
    ui->listWidget->installEventFilter(this);       //安装事件过滤器
    ui->listWidget->setDragEnabled(true);           //允许拖放操作
    ui->listWidget->setDragDropMode(QAbstractItemView::InternalMove);   //列表项可移动

    //======================设置状态栏=======================
    audioItemLabel=new QLabel(this);
    audioItemLabel->setText("当前无待播放音频");
//    dirItemLabel=new QLabel(this);
//    dirItemLabel->setText("当前目录树节点");

    ui->statusbar->addPermanentWidget(audioItemLabel);
//    ui->statusbar->addWidget(dirItemLabel);

    connect(ui->treeWidget,&QTreeWidget::currentItemChanged,
            this, &MainWindow::buildAudioList);  

    //设置QTreeWidgetItem的右键菜单
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showContextMenu(const QPoint&)));

    //设置QListWidgetItem的右键菜单
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this,
            SLOT(showListWidgetContextMenu(QPoint)));

    initPlayer();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::do_sourceChanged(const QUrl &media)
{//播放的文件发生变化时的响应
    audioItemLabel->setText(media.fileName());
}

void MainWindow::do_stateChanged(QMediaPlayer::PlaybackState state)
{
    if(state==QMediaPlayer::PlayingState){
        qDebug()<<"[debug] PlayingState";
        ui->pushButton->setIcon(QIcon(":/icons/imgs/suspend1.png"));
    }else if(state==QMediaPlayer::PausedState){
        qDebug()<<"[debug] PausedState";
    }else if(state==QMediaPlayer::StoppedState){
        qDebug()<<"[debug] StoppedState";
        ui->pushButton->setIcon(QIcon(":/icons/imgs/start1.png"));
    }else{
        qDebug()<<"[debug] OtherState";
    }
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->treeWidget->itemAt(pos);

    if (item)
    {
        QMenu contextMenu(this);
        QAction *action1 = contextMenu.addAction(QIcon(":/icons/imgs/addfile.png"),"添加文件");
        QAction *action2 = contextMenu.addAction(QIcon(":/icons/imgs/tts.png"),"语音合成");
        QAction *action3 = contextMenu.addAction(QIcon(":/icons/imgs/tape.png"),"录音");
        contextMenu.addSeparator();
        QAction *action4 = contextMenu.addAction(QIcon(":/icons/imgs/addfolder.png"),"新建文件夹");
        QAction *action5 = contextMenu.addAction(QIcon(":/icons/imgs/delfolder.png"),"删除文件夹");

        connect(action1, &QAction::triggered, this,
                &MainWindow::on_actAddFile_triggered);

        connect(action2, &QAction::triggered, this,
                &MainWindow::on_actTts_triggered);

        connect(action3, &QAction::triggered, this,
                &MainWindow::on_actTape_triggered);

        connect(action4, &QAction::triggered, this,
                &MainWindow::on_actAddDir_triggered);

        connect(action5, &QAction::triggered, this,
                &MainWindow::on_actDelDir_triggered);

        // 显示菜单在鼠标附近
        contextMenu.exec(QCursor::pos());
    }
}

void MainWindow::showListWidgetContextMenu(const QPoint &pos)
{
    QListWidgetItem *item=ui->listWidget->itemAt(pos);
    if (item)
    {
        QMenu contextMenu(this);
        QAction *action = contextMenu.addAction(QIcon(":/icons/imgs/delfile.png"),"删除文件");

        connect(action, &QAction::triggered, this,
                &MainWindow::on_actDelFile_triggered);

        // 显示菜单在鼠标附近
        contextMenu.exec(QCursor::pos());
    }
}

void MainWindow::on_actOpenDir_triggered()
{
    QString topDir = QFileDialog::getExistingDirectory();
    setTopDir(topDir);
    on_actRefresh_triggered();
}


void MainWindow::on_actRefresh_triggered()
{

    qDebug()<<"[debug] on_actRefresh_triggered( topDir: "<<getTopDir();
    isRefresh=true;
    buildDirTree(getTopDir());
    isRefresh=false;
}


void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    qDebug()<<"[debug] on_listWidget_itemDoubleClicked()";

    player->setSource(getUrlFromItem(item));
    player->play();
}


void MainWindow::on_pushButton_clicked()
{
    QUrl curUrl=player->source();
    if(ui->listWidget->currentRow()<0){
        if(!curUrl.isEmpty()){
            //当前有音频在播放
            if(player->playbackState() == QMediaPlayer::PlayingState){
                qDebug()<<"[debug] curUrl is empty";
                player->stop();
            }else{
                qDebug()<<"[debug] curUrl is not empty";
                player->play();
            }
            return;
        }
        // 创建一个信息提示框
        showTipMessage("请选择一个音频！");
        return;
    }
    // 获取当前播放器的状态
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        qDebug() << "[debug] 播放器正在播放。";
        player->setSource(getUrlFromItem(ui->listWidget->currentItem()));
        player->stop();

    } else if (player->playbackState() == QMediaPlayer::PausedState
               ||player->playbackState() == QMediaPlayer::StoppedState) {
        qDebug() << "[debug] 播放器已暂停、播放器已停止。";
        player->setSource(getUrlFromItem(ui->listWidget->currentItem()));
        player->play();
    }
}


void MainWindow::on_pushButton_4_clicked()
{
    bool mute=player->audioOutput()->isMuted();
    player->audioOutput()->setMuted(!mute);
    if(mute)
        ui->pushButton_4->setIcon(QIcon(":/icons/imgs/mute.png"));
    else
        ui->pushButton_4->setIcon(QIcon(":icons/imgs/volume.png"));
}


void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    player->audioOutput()->setVolume(value/100.0);
}


void MainWindow::on_actAddDir_triggered()
{
    QTreeWidgetItem *curItem=ui->treeWidget->currentItem();
    if(curItem==NULL){
        showTipMessage("请选择一个文件夹！");
    }else{

        auto list=getAllParents(curItem);
        foreach (auto item, list) {
            qDebug()<<"cur: "<<item->data(0,Qt::UserRole);
        }

        QString path=curItem->data(0,Qt::UserRole).toString();
        qDebug()<<"[debug] on_actAddDir_triggered(): "<<path;

        // 创建输入对话框
        QInputDialog inputDialog;
        inputDialog.setWindowTitle("输入提示框!");
        inputDialog.setLabelText("请输入文件夹名称:");
        inputDialog.setInputMode(QInputDialog::TextInput);

        // 修改对话框按钮文字
        inputDialog.setWindowIcon(QIcon(QIcon(":/icons/imgs/tip.png")));

        // 显示对话框
        bool ok = inputDialog.exec();

        // 检查用户是否点击了"确定"按钮
        if (ok) {
            QString userInput = inputDialog.textValue();
            qDebug() << "[debug] 用户输入的文本是：" << userInput;
            QString absolutePath=path+"/"+userInput;
            qDebug()<<"[debug] 文件夹绝对路径为：" <<absolutePath;

            // 创建QDir对象并使用mkdir()方法创建文件夹
            QDir dir;
            if (!dir.exists(absolutePath)) {
                if (dir.mkdir(absolutePath)) {
                    qDebug() << "[debug] 文件夹创建成功";

                    QTreeWidgetItem *item=new QTreeWidgetItem(MainWindow::itGroupItem);

                    item->setText(0,userInput);
                    item->setIcon(0,QIcon(":/icons/imgs/folder.png"));
                    item->setData(0,Qt::UserRole,absolutePath);

                    curItem->addChild(item);
                    curItem->setExpanded(true);
                } else {
                    qDebug() << "[debug] 文件夹创建失败";
                    showTipMessage("文件夹已存在!");
                }
            } else {
                qDebug() << "[debug] 文件夹已经存在";
                showTipMessage("文件夹已经存在!");
            }
        } else {
            qDebug() << "[debug] 用户取消了输入操作";
        }
    }
}

void MainWindow::on_actDelDir_triggered()
{
    QTreeWidgetItem *curItem=ui->treeWidget->currentItem();
    if(curItem==NULL){
        showTipMessage("请选择一个文件夹！");
    }else{
        if(MainWindow::itTopItem==curItem->type()){
            QMessageBox::critical(this,"删除失败","本程序没有删除顶层节点的权限!");
            return;
        }
        QString path=curItem->data(0,Qt::UserRole).toString();
        qDebug()<<"[debug] on_actAddDir_triggered(): "<<path;

        // 创建询问对话框
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
            this,
            "确认删除",
            "是否要删除文件夹 '" + path + "'？",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            QDir dir(path);
            QTreeWidgetItem* parentItem=curItem->parent();
            if (dir.exists()) {
                // 删除文件夹及其内容
                if (dir.removeRecursively()) {
                    parentItem->removeChild(curItem);
                } else {
                    QMessageBox::critical(this, "删除失败", "该文件夹中的文件正在使用，无法删除！");
                }
            } else {
                QMessageBox::critical(this, "删除失败", "文件夹不存在");
            }
        } else {
            // 用户选择了“否”或关闭了对话框，取消删除操作
        }

    }
}


void MainWindow::on_actAddFile_triggered()
{
    QTreeWidgetItem *curItem=ui->treeWidget->currentItem();
    if(curItem==NULL){
        showTipMessage("请选择一个文件夹！");
        return;
    }
    QString targetFolderPath =curItem->data(0,Qt::UserRole).toString();
    // 创建文件对话框
    QFileDialog fileDialog;

    // 设置文件对话框的模式，以选择文件
    fileDialog.setFileMode(QFileDialog::ExistingFile);

    // 设置对话框的标题
    fileDialog.setWindowTitle("选择文件");

    // 设置过滤器以限制可选择的文件类型
    fileDialog.setNameFilter("音频文件 (*.wav *.mp3 *.wmv *.m4a);;所有文件 (*)");

    // 如果用户点击了"打开"按钮
    if (fileDialog.exec()) {
        // 获取所选文件的路径
        QStringList selectedFiles = fileDialog.selectedFiles();

        // 输出所选文件的路径
        foreach (QString sourceFilePath, selectedFiles) {
            qDebug() << "[debug] 选择的文件路径: " << sourceFilePath;
            QFileInfo fileInfo(sourceFilePath);
            QString targetFilePath = targetFolderPath + "/" + fileInfo.fileName();
            qDebug()<<"[cur] "<<targetFilePath;
            if (QFile::copy(sourceFilePath, targetFilePath)) {
                buildAudioList(curItem);
            } else {
                QMessageBox::critical(this,"添加文件","文件添加失败,请查看是否已有同名文件!");
            }
        }
    } else {
        // 用户取消了文件选择
        qDebug() << "[debug] 用户取消了文件选择";
    }
}


void MainWindow::on_actDelFile_triggered()
{
    if(ui->listWidget->currentRow()<0){
        showTipMessage("请选择一个待删除的音频文件！");
        return;
    }
    QListWidgetItem *item=ui->listWidget->currentItem();
    QVariant data=item->data(Qt::UserRole);
    QString filePath=data.value<QUrl>().toLocalFile();
    qDebug()<<"[debug] on_actDelFile_triggered(): "<<filePath;

    // 创建一个消息框询问用户是否要删除文件
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "删除文件", "是否要删除文件?", QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 用户选择删除文件
        QFile file(filePath);

        if (file.remove()) {
            QListWidgetItem *delItem=ui->listWidget->takeItem(ui->listWidget->row(item));
            delete delItem;
            qDebug() << "[debug] on_actDelFile_triggered(): 文件删除成功";
        } else {
            QMessageBox::critical(this, "删除失败", "该文件正在使用中，无法删除!");
            qDebug() << "[debug] on_actDelFile_triggered(): 文件删除失败";
        }
    } else {
        // 用户选择不删除文件
        qDebug() << "[debug] on_actDelFile_triggered(): 文件未删除";
    }
}


void MainWindow::on_actTape_triggered()
{
    QTreeWidgetItem *curItem=ui->treeWidget->currentItem();
    if(curItem==NULL){
        showTipMessage("请选择一个文件夹！");
        return;
    }else{
        QString path=curItem->data(0,Qt::UserRole).toString();
        dialog->setPath(path);
        dialog->setTreeWidgetItem(curItem);
    }
    dialog->show();
}


void MainWindow::on_actTts_triggered()
{
    QTreeWidgetItem *curItem=ui->treeWidget->currentItem();
    if(curItem==NULL){
        showTipMessage("请选择一个文件夹！");
    }else{
        QString path=curItem->data(0,Qt::UserRole).toString();
        qDebug()<<"[debug] on_actAddDir_triggered(): "<<path;

        // 创建输入对话框
        QInputDialog inputDialog;
        inputDialog.setWindowTitle("输入提示框!");
        inputDialog.setLabelText("请输入音频内容:");
        inputDialog.setInputMode(QInputDialog::TextInput);

        // 修改对话框图标
        inputDialog.setWindowIcon(QIcon(QIcon(":/icons/imgs/tip.png")));

        // 显示对话框
        bool ok = inputDialog.exec();

        // 检查用户是否点击了"确定"按钮
        if (ok) {
            QString userInput = inputDialog.textValue();
            qDebug() << "[debug] 用户输入的文本是：" << userInput;
            QString absolutePath=path+"/"+userInput+".wav";
            qDebug()<<"[debug] 文件夹绝对路径为：" <<absolutePath;

            // 创建QFile对象并使用文件路径初始化
            QFile file(absolutePath);

            // 使用exists()函数来检查文件是否存在
            if (file.exists()) {
                QMessageBox::warning(this,"创建文件失败","该文件夹下已有一个同名文件!");
                return;
            }

            // 使用科大讯飞sdk生成文本对应的音频文件
            tts(absolutePath,userInput.toStdString().c_str());

            QFileInfo  fileInfo(absolutePath);
            QListWidgetItem *aItem =new QListWidgetItem(fileInfo.fileName());
            aItem->setIcon(QIcon(":/icons/imgs/note.png"));
            aItem->setData(Qt::UserRole, QUrl::fromLocalFile(absolutePath));  //设置用户数据，QUrl对象
            ui->listWidget->addItem(aItem);
            return;
        } else {
            qDebug() << "[debug] 用户取消了输入操作";
        }
    }

}

