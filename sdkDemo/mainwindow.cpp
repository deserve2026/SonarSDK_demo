#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , btn_power_status(false)
    , m_timer(new QTimer(this))
    , getWorkMode(0)
{
    ui->setupUi(this);

    // ==================== YOLO 大脑通电点火 ====================
    try {
        // 1. 初始化环境 (消音模式，不打印烦人的警告)
        ort_env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "YoloFish");

        // 2. 配置 Session 选项，强行开启刚才配好的 CUDA
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        // 开启 CUDA 加速 (如果你刚才配好的 CUDA DLL 没问题，这里就能成功)
        OrtCUDAProviderOptions cuda_options;
        cuda_options.device_id = 0; // 使用第一张显卡
        session_options.AppendExecutionProvider_CUDA(cuda_options);

        // 3. 加载模型 (注意：把你刚才生成的 best.onnx 复制到 EXE 同级目录下)
        // 注意 Windows 下路径字符串需要是宽字符 L""
        const wchar_t* model_path = L"F:/guangdong/project_related/fish_counting_related/lanheng/sdkDemo/sdkDemo/out/build/x64-Release/best_640.onnx";

        ort_session = new Ort::Session(*ort_env, model_path, session_options);
        memory_info = new Ort::MemoryInfo(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU));

        qDebug() << ">>> 捷报：YOLO GPU 核心加载成功！随时准备识别！";

    }
    catch (const Ort::Exception& e) {
        qDebug() << ">>> 警报：YOLO 核心加载失败！报错信息：" << e.what();
    }
    // ===========================================================

    setWindowTitle(QString::fromLocal8Bit("LhSonarSDKGuiDemoV1.2 Built on 2025/10/31"));
    setFixedHeight(800);
    setFixedWidth(968);
    
    m_circleBuffer = new CircleBuffer(LhForwardSDK::LhForwardFrame::PACK_SIZE, LhForwardSDK::PACK_MAX_NUM);

    m_processing = new Processing(m_circleBuffer);
    // connect(m_processing, &Processing::sigImageOK, this, &MainWindow::isTargetData); // Collision avoidance display
    connect(m_processing, &Processing::sigImageOK, this, &MainWindow::showImage); // QImage display
    connect(m_processing, &Processing::sigDeviceName, this, &MainWindow::setDeviceName); // QImage display
    m_processing->start();

    m_udpReceiver = new UdpReceiver(m_circleBuffer);
    m_udpReceiver->start();

    m_fileRead = new FileRead(m_circleBuffer);
    connect(m_fileRead, &FileRead::sigImageOK, this, &MainWindow::showImage); // QImage display

    m_fileSave = new FileSave(m_circleBuffer);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setFixedWidth(IMAGE_WIDTH);
    m_imageLabel->setFixedHeight(IMAGE_HEIGHT);

    label_name = new QLabel(this);
    label_name->resize(200, 30);
    //    label_name->setStyleSheet("border: 2px solid red;");
    label_name->move(150, 515);

    m_connectPBtn = new QPushButton(this);
    m_connectPBtn->setText("Connect");
    m_connectPBtn->move(130, 550);
    m_connectPBtn->resize(120, 40);
    connect(m_connectPBtn, &QPushButton::clicked, this, &MainWindow::connectPBtnActive);
    m_connectTimer = new QTimer(this);
    connect(m_connectTimer, &QTimer::timeout, this, &MainWindow::connectTimerActive);
    m_connectTimer->setInterval(3e3);

    btn_power = new QPushButton(this);
    btn_power->setText("Transmitter Enable");
    btn_power->move(130, 600);
    btn_power->resize(150, 40);
    btn_power->setStyleSheet("background-color: #2c9678;");
    btn_power->setEnabled(false);
    connect(btn_power, &QPushButton::clicked, this, &MainWindow::setPower);

    combox_mode = new QComboBox(this);
    combox_mode->move(130, 650);
    combox_mode->resize(120, 40);
    // combox_mode->setStyleSheet("QComboBox QAbstractItemView { alignment: center; }");

    connect(combox_mode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::changeWorkMode);

    browser = new QTextBrowser(this);
    browser->move(750, 0);
    browser->resize(200,230);
    browser->setVisible(false);

    QLabel *label_ip = new QLabel(this);
    label_ip->setText("Manual IP:");
    label_ip->move(360, 550);
    text_ip = new QTextEdit(this);
    text_ip->move(460, 550);
    text_ip->resize(190, 33);

    playback_btn = new QPushButton(this);
    playback_btn->move(720,600);
    playback_btn->setText("Playback");
    playback_btn->resize(100,40);
    connect(playback_btn, &QPushButton::clicked, this, &MainWindow::playStart);

    save_btn = new QPushButton(this);
    save_btn->move(720,550);
    save_btn->setText("Save");
    save_btn->resize(100,40);
    save_btn->setEnabled(false);
    connect(save_btn, &QPushButton::clicked, this, &MainWindow::saveData);

    QLabel *label_gamma = new QLabel(this);
    label_gamma->setText("Gamma:");
    label_gamma->move(360, 600);
    text_gamma = new QTextEdit(this);
    text_gamma->move(460, 600);
    text_gamma->resize(100, 33);
    btn_gamma = new QPushButton(this);
    btn_gamma->move(570, 600);
    btn_gamma->setText("set");
    btn_gamma->resize(80, 33);
    btn_gamma->setEnabled(false);
    connect(btn_gamma, &QPushButton::clicked, this, &MainWindow::setGamma);

    QLabel *label_distance = new QLabel(this);
    label_distance->setText("Range:");
    label_distance->move(360, 650);
    text_distance = new QTextEdit(this);
    text_distance->move(460, 650);
    text_distance->resize(100, 33);
    btn_distance = new QPushButton(this);
    btn_distance->move(570, 650);
    btn_distance->setText("set");
    btn_distance->resize(80, 33);
    btn_distance->setEnabled(false);
    connect(btn_distance, &QPushButton::clicked, this, &MainWindow::setMaxDistance);
    connect(m_timer,&QTimer::timeout,this,&MainWindow::MaxDistanceTimer);

    m_lhForwardCmd = NULL;

    connect(m_processing, &Processing::sigPackageInfo, this, [=](struct packageInfo inPackageInfo){
        m_curFrameInfo = inPackageInfo;
    });
}

MainWindow::~MainWindow()
{
    // ================= 拔掉电源 =================
    if (ort_session) delete ort_session;
    if (ort_env) delete ort_env;
    if (memory_info) delete memory_info;
    // ===========================================

    
    if (m_lhForwardCmd != NULL) {
        m_lhForwardCmd->sendCMD(LhForwardCMD::startStop, (uint8_t)0);  // Stop sonar
        delete m_lhForwardCmd;
    }

    m_udpReceiver->stop();
    delete m_udpReceiver;

    m_processing->stop();
    delete m_processing;

    m_fileRead->stop();
    delete m_fileRead;

    m_fileSave->stop();
    delete m_fileSave;

    delete ui;
}

void MainWindow::playStart()
{
    or_text = !or_text;

    if(or_text){
        QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        QString filePath = QFileDialog::getOpenFileName(this, tr("Select playback file"), desktopPath, tr("Data format(*.DB)"));

        if(!filePath.isEmpty()) {
            // Selected file
            playback_btn->setText("Stop");
            m_connectPBtn->setEnabled(false);

            if(m_fileRead->f_readFile(filePath)){
                // File opened successfully
                m_udpReceiver->stop();    // Stop UDP receiving thread
                m_fileRead->start();      // Open replay thread
            }
            else{
                // File opening failed
                return;
            }
            m_fileRead->m_startFlag = or_text; // Change flag position
        }
        else {
            // Cancel button
            or_text = !or_text;
        }
    }
    else{
        playback_btn->setText("Playback");
        m_fileRead->m_startFlag = or_text;
        m_connectPBtn->setEnabled(true);
        m_udpReceiver->start();
        m_fileRead->stop();
    }
}

void MainWindow::saveData()
{
    if(save_btn->text() == "Save"){
        qDebug() << "Start saving";
        save_btn->setText("Stop");

        fileNum++;
        QString fileName = QString::number(fileNum) + ".DB";
        if(m_fileSave->f_saveFile(fileName)){
            m_fileSave->start();
        }else {
            return;
        }

    }else if(save_btn->text() == "Stop"){
        qDebug() << "Stop saving";
        save_btn->setText("Save");
        m_fileSave->stop();
    }
}

// collision avoidance
void MainWindow::isTargetData()
{
    QString title = "X,Y,intensity\n";
    if(!m_processing->l_avoidCollisionMsgs.isEmpty())
    {
        browser->setVisible(true);
        QString lineData = title + m_processing->l_avoidCollisionMsgs.replace(";", "\n");   // Symbol replacement
        browser->setPlainText(lineData);
    }
    else{
        browser->setVisible(false);
    }
}

void MainWindow::showImage(QImage img)
{
    // ==================== 翻转外科手术 ====================
    // 强行把传进来的原始图像翻转！参数1: 水平不翻转(false)；参数2: 垂直翻转(true)
    img = img.mirrored(false, true);
    // ======================================================
    // ==================== 按“绝对时间”抽帧的外科手术 ====================
    static int savedImageCount = 0; // 记录总共保存了多少张图
    static qint64 lastSaveTime = 0; // 记录上一次保存图片的时间戳

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch(); // 获取当前真实时间（毫秒）

    // 500 表示 500 毫秒（即 0.5 秒）。这个数值你可以根据鱼游动的速度自己改。
    // 如果大于 500 毫秒，就存一张图
    if (currentTime - lastSaveTime >= 3000) {
        // 强制使用 PNG 无损格式，保留最原始的像素点喂给 YOLO！
        QString savePath = QString("F:/experimental_data/mend26s/raw_data/frame_%1.png").arg(savedImageCount++, 6, 10, QChar('0'));
        img.save(savePath, "PNG");

        lastSaveTime = currentTime; // 更新上一次保存的时间
    }
    // ==================== 抽帧外科手术代码结束 ====================

    // ==================== YOLO 拦截手术区 (计数功能将在这里诞生) ====================

    // 第一步：将 QImage 转换为 OpenCV 的 Mat
    cv::Mat cv_img;
    if (img.format() == QImage::Format_ARGB32 || img.format() == QImage::Format_RGB32) {
        cv_img = cv::Mat(img.height(), img.width(), CV_8UC4, (void*)img.constBits(), img.bytesPerLine());
        cv::cvtColor(cv_img, cv_img, cv::COLOR_BGRA2BGR); // 丢弃透明通道，转为标准的 BGR
    }
    else if (img.format() == QImage::Format_RGB888) {
        cv_img = cv::Mat(img.height(), img.width(), CV_8UC3, (void*)img.constBits(), img.bytesPerLine());
        cv::cvtColor(cv_img, cv_img, cv::COLOR_RGB2BGR);
    }
    else {
        cv_img = cv::Mat(img.height(), img.width(), CV_8UC1, (void*)img.constBits(), img.bytesPerLine());
        cv::cvtColor(cv_img, cv_img, cv::COLOR_GRAY2BGR);
    }
    
    // ======================================================================
    // ！！！前方高能：YOLO 640x640 张量推理与解析核心区 ！！！

    try {
        // 【温柔版防爆门】：如果通电了才执行推理，如果没通电，直接略过，绝不 return 阻断画图！
        if (ort_session != nullptr && memory_info != nullptr) {

            // 1. 图像预处理 (硬核锁定 640x640)
            cv::Mat blob = cv::dnn::blobFromImage(cv_img, 1.0 / 255.0, cv::Size(640, 640), cv::Scalar(), true, false);

            // 2. 构建 ONNX 输入张量
            size_t input_tensor_size = 1 * 3 * 640 * 640;
            std::vector<float> input_tensor_values(blob.ptr<float>(), blob.ptr<float>() + input_tensor_size);
            std::vector<int64_t> input_node_dims = { 1, 3, 640, 640 };
            Ort::Value input_tensor = Ort::Value::CreateTensor<float>(*memory_info, input_tensor_values.data(), input_tensor_size, input_node_dims.data(), 4);

            // 3. 执行 GPU 推理
            const char* input_names[] = { "images" };
            const char* output_names[] = { "output0" };
            auto output_tensors = ort_session->Run(Ort::RunOptions{ nullptr }, input_names, &input_tensor, 1, output_names, 1);

            // 4. 获取输出数据指针 (640 尺寸下，输出维度是 [1, 6, 8400])
            float* raw_output = output_tensors[0].GetTensorMutableData<float>();

            // 5. 矩阵解码与坐标还原
            std::vector<cv::Rect> boxes;
            std::vector<float> confs;
            std::vector<int> classIds;

            // 计算缩放比例，锁定 640.0f
            float x_factor = (float)cv_img.cols / 640.0f;
            float y_factor = (float)cv_img.rows / 640.0f;

            // 遍历所有 8400 个候选框
            for (int i = 0; i < 8400; ++i) {
                float class0_score = raw_output[4 * 8400 + i];
                float class1_score = raw_output[5 * 8400 + i];
                float max_score = std::max(class0_score, class1_score);

                if (max_score > 0.5f) {
                    float cx = raw_output[0 * 8400 + i];
                    float cy = raw_output[1 * 8400 + i];
                    float w = raw_output[2 * 8400 + i];
                    float h = raw_output[3 * 8400 + i];

                    int left = int((cx - 0.5 * w) * x_factor);
                    int top = int((cy - 0.5 * h) * y_factor);
                    int width = int(w * x_factor);
                    int height = int(h * y_factor);

                    boxes.push_back(cv::Rect(left, top, width, height));
                    confs.push_back(max_score);
                }
            }

            // 6. NMS 非极大值抑制
            std::vector<int> indices;
            cv::dnn::NMSBoxes(boxes, confs, 0.5f, 0.45f, indices);

            // 7. 画框与实事求是的计数
            int fish_count = indices.size();
            for (int idx : indices) {
                cv::Rect box = boxes[idx];
                cv::rectangle(cv_img, box, cv::Scalar(0, 0, 255), 2); // 画红色框
            }

            // 8. 把鱼的数量写在画面左上角
            QString count_text = QString("Fish Count: %1").arg(fish_count);
            cv::putText(cv_img, count_text.toStdString(), cv::Point(30, 60), cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(0, 255, 0), 3);
        }
    }
    catch (const Ort::Exception& e) {
        qDebug() << "推理崩溃，报错信息：" << e.what();
    }
    // ======================================================================
    // ======================================================================

    // 拦截完毕，把画好框的 cv::Mat 重新转回 QImage，交还给原厂代码去显示
    cv::Mat rgb_img;
    cv::cvtColor(cv_img, rgb_img, cv::COLOR_BGR2RGB);
    QImage result_img((const unsigned char*)(rgb_img.data), rgb_img.cols, rgb_img.rows, rgb_img.step, QImage::Format_RGB888);
    // ======================================================================

    // 下面是原厂界面显示代码，千万别动
    // 下面是原厂界面显示代码，千万别动 (把原来的 img 替换成 result_img)
    if(result_img.height() > m_imageLabel->height()){
        result_img = result_img.scaledToHeight(m_imageLabel->height());
    }
    int lImgStartX = -(m_imageLabel->width() - result_img.width()) * 0.5;
    int lImgStartY = -(m_imageLabel->height() - result_img.height()) * 0.5;
    result_img = result_img.copy(lImgStartX, lImgStartY, m_imageLabel->width(), m_imageLabel->height());

    m_imageLabel->setPixmap(QPixmap::fromImage(result_img));
    m_imageLabel->show();

    // Get initial working mode
    if(getWorkMode == 0){
        combox_mode->setCurrentIndex(m_curFrameInfo.workMode);
        getWorkMode = 1;
    }
}

void MainWindow::connectPBtnActive()
{
    if (m_connectPBtn->text() == QString("Connect")) {
        m_connectPBtn->setText("Connecting...");
        playback_btn->setEnabled(false);
        m_connectTimer->start();
    }
    else if(m_connectPBtn->text() == QString("Disconnect")){
        if(true == btn_power_status){
            setPower();
        }
        m_lhForwardCmd->sendCMD(LhForwardCMD::startStop, (uint8_t)0);  // Stop sonar
        //        combox_mode->clear();
        delete m_lhForwardCmd;
        m_lhForwardCmd = NULL;
        m_connectPBtn->setText("Connect");
        playback_btn->setEnabled(true);
        btn_power->setEnabled(false);
        save_btn->setEnabled(false);
        btn_gamma->setEnabled(false);
        btn_distance->setEnabled(false);
    }
    else if (m_connectPBtn->text() == QString("Connecting...")) {
        m_connectTimer->stop();
        m_connectPBtn->setText("Connect");
        playback_btn->setEnabled(true);
        btn_power->setEnabled(false);
        save_btn->setEnabled(false);
        btn_gamma->setEnabled(false);
        btn_distance->setEnabled(false);
    }
}

// Get host socket
void MainWindow::getHostSocket()
{
    QStringList l_hostIpAddrs;
    QList <QHostAddress> l_allIpAddrs = QNetworkInterface::allAddresses();
    for(int i = 0; i < l_allIpAddrs.length(); i++){
        if(l_allIpAddrs.at(i).protocol() == QAbstractSocket::IPv4Protocol){ // Determine whether it is IPv4
            // qDebug() << "The IP address owned by this device:" <<l_allIpAddrs.at(i).toString();
            l_hostIpAddrs.append(l_allIpAddrs.at(i).toString());
        }
    }

    // Clear sockets before the container
    for(int i = 0; i < m_udpSocketVec.length(); i++){
        if(m_udpSocketVec.at(i) != INVALID_SOCKET){
#ifdef USE_WINDOWS
            closesocket(m_udpSocketVec.at(i));
#elif USE_LINUX
            shutdown(m_udpSocketVec.at(i),SHUT_RDWR);
            ::close(m_udpSocketVec.at(i));
#endif
        }
    }
    m_udpSocketVec.clear();

    // Add socket to container
    for(int i = 0; i < l_hostIpAddrs.length(); i++){
#ifdef USE_WINDOWS
        SOCKET l_tempSocket = socket(AF_INET,SOCK_DGRAM,0);
#elif USE_LINUX
        int l_tempSocket = socket(AF_INET,SOCK_DGRAM,0);
#endif
        int netBuffer = 1024*1024*10;
        setsockopt(l_tempSocket, SOL_SOCKET, SO_RCVBUF, (const char *)&netBuffer, sizeof(int));

#ifdef USE_WINDOWS
        int outTime = 50;
        bool broad = true;
        bool Reuseaddr=true;
#elif USE_LINUX
        struct timeval outTime;
        outTime.tv_sec = 0;
        outTime.tv_usec = 50000;
        int broad = true;
        int Reuseaddr=true;
#endif
        setsockopt(l_tempSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&outTime, sizeof(outTime));

        setsockopt(l_tempSocket, SOL_SOCKET, SO_BROADCAST, (const char *)&broad, sizeof(broad));

        setsockopt(l_tempSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&Reuseaddr, sizeof(Reuseaddr));

#ifdef USE_WINDOWS
        SOCKADDR_IN tempSockAddr;
#elif USE_LINUX
        struct sockaddr_in tempSockAddr;
#endif
        tempSockAddr.sin_family = AF_INET;
        tempSockAddr.sin_port = htons(0);   // Set the port to 0, and the system selects the available ports to use
#ifdef USE_WINDOWS
        tempSockAddr.sin_addr.S_un.S_addr = inet_addr(l_hostIpAddrs.at(i).toStdString().c_str());
#elif USE_LINUX
        tempSockAddr.sin_addr.s_addr = inet_addr(l_hostIpAddrs.at(i).toStdString().c_str());
#endif
        if(SOCKET_ERROR == ::bind(l_tempSocket,(sockaddr*)&tempSockAddr,sizeof(tempSockAddr))){
            qDebug() << "Binding failed:" <<l_hostIpAddrs.at(i);
            continue;
        }
        else{
            // qDebug() <<"Binding successful:" <<l_hostIpAddrs.at(i);
        }

        m_udpSocketVec.append(l_tempSocket);
    }
}

// Broadcast
void MainWindow::connectTimerActive()
{
#ifdef USE_WINDOWS
    // Initialize the broadcast UDP socket
    SOCKET udpSearchSocket;
    udpSearchSocket = socket(AF_INET, SOCK_DGRAM, 0);
    bool broad = true;
    setsockopt(udpSearchSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&broad, sizeof(broad));
    bool Reuseaddr = true;
    setsockopt(udpSearchSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&Reuseaddr, sizeof(Reuseaddr));
    // Send search broadcast
    SOCKADDR_IN sonarAddr;
    sonarAddr.sin_family = AF_INET;
    sonarAddr.sin_port = htons(UDP_DEV_ASK_PORT);
    if(text_ip->toPlainText().isEmpty()){
        sonarAddr.sin_addr.S_un.S_addr = INADDR_BROADCAST;
    }
    else{
        QString ipAddressString = text_ip->toPlainText();  // Replace here with QString containing IP address
        sonarAddr.sin_addr.S_un.S_addr = inet_addr(ipAddressString.toStdString().c_str());
    }
    getHostSocket();
    QVector<SOCKET> lSocketVec = m_udpSocketVec;
#elif USE_LINUX
    // Initialize the broadcast UDP socket
    int udpSearchSocket;
    udpSearchSocket = socket(AF_INET, SOCK_DGRAM, 0);

    struct timeval timeOut;
    timeOut.tv_sec = 0;
    timeOut.tv_usec = 50e3;  //50ms
    setsockopt(udpSearchSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeOut, sizeof(timeOut));

    int broad = 1;
    setsockopt(udpSearchSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&broad, sizeof(int));
    bool Reuseaddr = true;
    setsockopt(udpSearchSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&Reuseaddr, sizeof(Reuseaddr));
    // Send search broadcast
    struct sockaddr_in sonarAddr;
    sonarAddr.sin_family = AF_INET;
    sonarAddr.sin_port = htons(UDP_DEV_ASK_PORT);
    if(text_ip->toPlainText().isEmpty()){
        sonarAddr.sin_addr.s_addr = INADDR_BROADCAST;
    }
    else{
        QString ipAddressString = text_ip->toPlainText();  // Replace here with QString containing IP address
        sonarAddr.sin_addr.s_addr = inet_addr(ipAddressString.toStdString().c_str());
    }
    getHostSocket();
    QVector<int> lSocketVec = m_udpSocketVec;
#endif  
    char searchSonar[] = { "SVSearching" };

    for(int i = 0; i < lSocketVec.length(); i++){
#ifdef USE_WINDOWS
        sendto(lSocketVec[i], searchSonar, sizeof(searchSonar), 0, (SOCKADDR*)&sonarAddr, sizeof(sonarAddr));
        // Receive sonar response
        char buffer[1024] = { 0 };
        SOCKADDR_IN recvAddr;
        int recvAddrSize = sizeof(recvAddr);
        Sleep(100);
        int recvNum = recvfrom(lSocketVec[i], buffer, 1024, 0, (SOCKADDR*)&recvAddr, &recvAddrSize);
#elif USE_LINUX
        sendto(lSocketVec[i], searchSonar, sizeof(searchSonar), 0, (struct sockaddr *)&sonarAddr, sizeof(sonarAddr));
        // Receive sonar response
        char buffer[1024] = { 0 };
        struct sockaddr_in recvAddr;
        socklen_t recvAddrSize = sizeof(recvAddr);
        usleep(100000);
        int recvNum = recvfrom(lSocketVec[i], buffer, 1024, 0, (struct sockaddr *)&recvAddr, &recvAddrSize);
#endif
        if (recvNum == -1) {
            continue;
        }
        else {
            // get a reply
            QByteArray deviceData;
            deviceData.resize(recvNum);
            memcpy(deviceData.data(), buffer, (size_t)recvNum);
            if (deviceData.left(1).toHex() == "a0" || deviceData.left(3).toHex() == "005356") {
                // Determine as a response message from sonar
                QString ipAddr((char*)inet_ntoa(recvAddr.sin_addr));    // Obtain sonar IP

                deviceTotalInfo devInfo;
                memcpy(&devInfo, deviceData.data(), sizeof(deviceTotalInfo));
                workModeNum = devInfo.workModeNum;

                // Obtain work mode
                getWorkModeNum();

                struct sockaddr_in l_addr;
                char l_hIp[128] = {0};
#ifdef USE_WINDOWS
                int l_addrLength = sizeof(sockaddr_in);
#elif USE_LINUX
                unsigned int l_addrLength = sizeof(sockaddr_in);
#endif
                getsockname(lSocketVec[i], (struct sockaddr *)&l_addr, &l_addrLength);
                inet_ntop(AF_INET, (void *)&l_addr.sin_addr, l_hIp, sizeof(l_hIp));

                if (1 == tryConnect(ipAddr, l_hIp)) {
                    m_connectTimer->stop();
                    m_connectPBtn->setText("Disconnect");
                    btn_power->setEnabled(true);
                    save_btn->setEnabled(true);
                    btn_gamma->setEnabled(true);
                    btn_distance->setEnabled(true);
                }
            }
            break;
        }
    }

    for(int i = 0; i < lSocketVec.length(); i++){
        if(lSocketVec.at(i) != INVALID_SOCKET){
#ifdef USE_WINDOWS
            closesocket(lSocketVec.at(i));
#elif USE_LINUX
            shutdown(lSocketVec.at(i),SHUT_RDWR);
            ::close(lSocketVec.at(i));
#endif
        }
    }
    lSocketVec.clear();
}

// Attempt to connect
int MainWindow::tryConnect(QString ipAddr, QString inHostIp)
{
    uint8_t ip1, ip2, ip3, ip4;
    ip1 = ip2 = ip3 = ip4 = 0;

    QStringList ipParts = inHostIp.split('.');

    if (ipParts.size() == 4) {
        ip1 = ipParts[0].toUInt();
        ip2 = ipParts[1].toUInt();
        ip3 = ipParts[2].toUInt();
        ip4 = ipParts[3].toUInt();
    }
    else {
        qDebug() << "ip error!";
    }


    m_lhForwardCmd = new LhForwardCMD(ipAddr.toStdString(), TCP_CMD_PORT);
    int ret = m_lhForwardCmd->connectDevice(inHostIp.toStdString());
    if (LhForwardCMD::ok == ret) {
        ret = m_lhForwardCmd->sendCMD(LhForwardCMD::dataSource, (uint8_t)3);  // Set the upload data type to png
        sendCMDReturn("Data Source", ret);
#ifdef USE_WINDOWS
        Sleep(1e3);
#elif USE_LINUX
        usleep(1e3 * 1000);
#endif
        uint8_t ip[4];
        ip[0] = ip1;
        ip[1] = ip2;
        ip[2] = ip3;
        ip[3] = ip4;
        ret = m_lhForwardCmd->sendCMD(LhForwardCMD::udpData, ip, UDP_REC_SDK_PORT); // Set local IP address
        sendCMDReturn("Local IP", ret);
#ifdef USE_WINDOWS
        Sleep(1e3);
#elif USE_LINUX
        usleep(1e3 * 1000);
#endif
        ret = m_lhForwardCmd->sendCMD(LhForwardCMD::startStop, (uint8_t)1);
        sendCMDReturn("Start Stop", ret);
        return 1;
    }
    return 0;
}

// Set device name
void MainWindow::setDeviceName()
{
    label_name->setText(m_processing->device_name);
}

// Switch emission (by setting power)
void MainWindow::setPower()
{
    uint8_t temp;
    if(false == btn_power_status){
        temp = 255;
        btn_power_status = true;
        btn_power->setText("Transmitter Disable");
        btn_power->setStyleSheet("background-color: #ed5a65;");
    }
    else{
        temp = 0;
        btn_power_status = false;
        btn_power->setText("Transmitter Enable");
        btn_power->setStyleSheet("background-color: #2c9678;");
    }
    LhForwardCMD::LfCMDRet ret = LhForwardCMD::ok;
    ret = m_lhForwardCmd->sendCMD(LhForwardCMD::power, temp);
    sendCMDReturn("Transmitter Enable", ret);
}

// Send cmd and return judgment
void MainWindow::sendCMDReturn(QString arg_type, int ret)
{
    switch (ret) {
    case 1:qDebug() << arg_type << "Device connection error";break;
    case 2:qDebug() << arg_type << "Device not connected error";break;
    case 3:qDebug() << arg_type << "Command parameter error";break;
    case 4:qDebug() << arg_type << "Command sending error";break;
    case 5:qDebug() << arg_type << "Ming replied incorrectly today";break;
    case 6:qDebug() << arg_type << "No command reply";break;
    default:
        break;
    }
}

// Set gamma coefficient
void MainWindow::setGamma()
{
    QString arg = text_gamma->toPlainText();
    LhForwardCMD::LfCMDRet ret = LhForwardCMD::ok;
    float temp2 = arg.toFloat();

    ret = m_lhForwardCmd->sendCMD(LhForwardCMD::gamma, temp2);
    sendCMDReturn("Gamma Coefficient", ret);
}

// Set range
void MainWindow::setMaxDistance()
{
    QString choice = text_distance->toPlainText();
    LhForwardCMD::LfCMDRet ret = LhForwardCMD::ok;
    float temp = choice.toFloat();
    m_maxTemp = temp;
    if(abs((double)(temp - m_curFrameInfo.maxDist)) > 0.01)
    {
        if(temp < ((float)750 * m_curFrameInfo.workCycle))
        {
            ret = m_lhForwardCmd->sendCMD(LhForwardCMD::maxDist, temp);
            sendCMDReturn("Range", ret);
        }
        else if(temp >= ((float)750 * m_curFrameInfo.workCycle))
        {
            float tempRes = 0;
            ret = m_lhForwardCmd->sendCMD(LhForwardCMD::perid, tempRes);
            sendCMDReturn("Period", ret);
            m_timer->start(500);
        }
    }
    else {
        return;
    }
}

void MainWindow::MaxDistanceTimer()
{
    LhForwardCMD::LfCMDRet ret = LhForwardCMD::ok;
    ret = m_lhForwardCmd->sendCMD(LhForwardCMD::maxDist, m_maxTemp);
    sendCMDReturn("Maximum Distance Timing", ret);

    m_timer->stop();
}

// Obtain the number of working modes
void MainWindow::getWorkModeNum()
{
    combox_mode->clear();
    if(workModeNum == 1){
        combox_mode->addItem("Mode 0");
    }
    else if(workModeNum == 2){
        combox_mode->addItem("Mode 0");
        combox_mode->addItem("Mode 1");
    }
}

// Change work mode
void MainWindow::changeWorkMode(int index)
{
    if(m_lhForwardCmd == NULL){
        return ;
    }
    LhForwardCMD::LfCMDRet ret = LhForwardCMD::ok;
    uint8_t temp = (uint8_t)index;

    ret = m_lhForwardCmd->sendCMD(LhForwardCMD::workMode, temp);
    sendCMDReturn("Working Mode", ret);
}

