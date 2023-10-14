#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>
#include <mainwindow.h>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    session = new QMediaCaptureSession(this);
    audioInput = new QAudioInput(this);
    recorder = new QMediaRecorder(this);

    session->setAudioInput(audioInput);
    session->setRecorder(recorder);
    recorder->setQuality(QMediaRecorder::HighQuality);
    recorder->setEncodingMode(QMediaRecorder::ConstantQualityEncoding);


    connect(recorder,SIGNAL(durationChanged(qint64)),
            this,SLOT(do_duration_changed(qint64)));
    connect(recorder,&QMediaRecorder::recorderStateChanged,
            this,&Dialog::do_state_changed);
    qDebug()<<"[ddebug] 进入录音程序";

    ui->btnPause->setEnabled(false);
    ui->btnStop->setEnabled(false);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setPath(QString str){
    path=str;
}

void Dialog::setTreeWidgetItem(QTreeWidgetItem *it)
{
    item=it;
}

void Dialog::on_btnStart_clicked()
{
    qDebug()<<"[ddebug] path: "<<path;
    QString fileName=ui->textEdit->toPlainText();
    if(fileName.isEmpty()){
        QMessageBox::warning(this,"未设置文件名","请在输入框中输入文件名!");
        return;
    }
    qDebug()<<"[[ddebug] filename: "<<fileName;
    QString file=path+"/"+fileName;
    QUrl url;
    url.setScheme("file");
    url.setPath(file);
    qDebug()<<"[[ddebug] 录音文件QUrl: "<<url;
    recorder->setOutputLocation(url);
    recorder->record();
}

void Dialog::on_btnPause_clicked()
{
    recorder->pause();
}


void Dialog::on_btnStop_clicked()
{
    recorder->stop();
    this->hide();
    QMessageBox::information(this,"录音结束","录音已完成!");
    if (MainWindow *mainWindow = qobject_cast<MainWindow *>(parent())) {
        // 调用 MainWindow 中的成员函数
        mainWindow->buildAudioList(item);
    }
    ui->timeLabel->setText("");
    ui->textEdit->setText("");
}

void Dialog::do_duration_changed(qint64 duration)
{
    ui->timeLabel->setText(tr("已录音 %1 秒").arg(duration / 1000));
}

void Dialog::do_state_changed(QMediaRecorder::RecorderState state)
{//录音状态变化
    qDebug()<<"[ddebug] 录音状态变化了";
    if(state==QMediaRecorder::RecordingState){   //正在录制
        ui->btnStart->setEnabled(false);
        ui->btnStop->setEnabled(true);
        ui->btnPause->setEnabled(true);
    }else if(state==QMediaRecorder::PausedState){   //暂停录制
        ui->btnStart->setEnabled(true);
        ui->btnStop->setEnabled(true);
        ui->btnPause->setEnabled(false);
    }else{
        ui->btnStart->setEnabled(true);
        ui->btnStop->setEnabled(false);
        ui->btnPause->setEnabled(false);
    }
}

