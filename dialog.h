#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtMultimedia>
#include <QTreeWidgetItem>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    void setPath(QString str);
    void setTreeWidgetItem(QTreeWidgetItem *it);

private slots:
    void on_btnStart_clicked();

    void on_btnPause_clicked();

    void on_btnStop_clicked();

    void do_duration_changed(qint64 duration);

    void do_state_changed(QMediaRecorder::RecorderState state);

private:
    Ui::Dialog *ui;

    QString path;
    QTreeWidgetItem *item;

    QMediaCaptureSession* session;
    QAudioInput* audioInput;
    QMediaRecorder* recorder;
};

#endif // DIALOG_H
