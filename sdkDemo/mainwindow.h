#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// 在最上方 include 区域加上：
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>

#include <QMainWindow>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QTimer>
#include <QFileDialog>
#include <QStandardPaths>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QDebug>
#include "DeviceInfo.h"
#include "common/CircleBuffer.h"
#include "net/UdpReceiver.h"
#include "Processing.h"
#include "FileRead.h"
#include "FileSave.h"
#include "LhForwardCMD.h"
#include "LhForwardFrame.h"

#ifdef USE_WINDOWS
#include "WS2tcpip.h"
#elif USE_LINUX
#include <unistd.h>
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString m_savePngPath = ""; // 记录用户选择的文件夹
    int m_savedImageCount = 0;  // 记录存了几张图
    int m_frameCounter = 0;     // 帧计数器（用于抽帧间隔）
    int m_frameInterval = 3;    // 抽帧间隔（默认每3帧保存一次）
    qint64 m_lastSaveTime = 0;  // 控制抽帧频率
    QString m_currentDbFile = ""; // 记录当前回放的DB文件路径

private slots:
    // 1. 在这里添加按钮点击的槽函数声明（假设你的按钮在 UI 里命名为 btn_select_path）
    void on_btn_select_path_clicked();

private:
    // ================= YOLO ONNX 核心变量 =================
    Ort::Env* ort_env = nullptr;
    Ort::Session* ort_session = nullptr;
    Ort::MemoryInfo* memory_info = nullptr;
    // =====================================================

    // ... 其他原本的变量 ...
    Ui::MainWindow *ui;
    struct packageInfo m_curFrameInfo;
    CircleBuffer* m_circleBuffer;
    Processing* m_processing;
    FileRead* m_fileRead;
    FileSave* m_fileSave;
    UdpReceiver* m_udpReceiver;
    LhForwardCMD* m_lhForwardCmd;
    LhForwardFrame *m_lhForwardFrame;
    QLabel* m_imageLabel;
    QTextBrowser *browser;
    QPushButton* m_connectPBtn;
    QTimer* m_connectTimer;
    QFileDialog* m_fileDialog;
    QFile m_file;
    QLabel* label_name;
    QPushButton *btn_power;
    bool btn_power_status;
    QComboBox *combox_mode;
    QTextEdit *text_ip;
    QTextEdit *text_gamma;
    QTextEdit *text_distance;
    QPushButton *btn_distance;
    QPushButton *btn_gamma;
    float m_maxTemp;
    QTimer *m_timer;
    uint8_t workMode;
    uint8_t workModeNum;
    bool getWorkMode;

#ifdef USE_WINDOWS
    QVector<SOCKET> m_udpSocketVec;
#elif USE_LINUX
    QVector<int> m_udpSocketVec;
#endif

    QPushButton *playback_btn;
    QPushButton *export_png_btn;
    bool or_text = false;
    void playStart();
    void on_export_png_clicked();

    QPushButton *save_btn;
    int fileNum = 0;
    void saveData();
    void isTargetData();

    void showImage(QImage img);
    void connectPBtnActive();
    void getHostSocket();
    void connectTimerActive();
    int tryConnect(QString ipAddr, QString inHostIp);
    void setDeviceName();
    void setPower();
    void sendCMDReturn(QString arg_type, int ret);
    void setGamma();
    void setMaxDistance();
    void MaxDistanceTimer();
    void getWorkModeNum();
    void changeWorkMode(int index);

};
#endif // MAINWINDOW_H
