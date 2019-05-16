#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->pbxS->setScaledContents(true); // �O��ܩ�QLabel�W�v���j���Y���M����ؤo�ۦP
}

MainWindow::~MainWindow()
{
    delete ui;
}

// �Ncv::Mat�v����ܩ�QLabel�W
void MainWindow::ShowFrame(cv::Mat imgS, QLabel *imgT)
{
    Mat imgC;

    cvtColor(imgS,imgC,cv::COLOR_BGR2RGB); // �N�v����m�Ŷ��qBGR�নRGB

    QImage tmp(imgC.data, imgC.cols, imgC.rows, imgC.step,
               QImage::Format_RGB888); // �ͦ�QImage�榡�Ȧs�v��

    imgT->setPixmap(QPixmap::fromImage(tmp)); // �N�v��ø��QLabel�W
}

// ���U�iLoad...�j��
void MainWindow::on_btnLoad_clicked()
{
    fileName = QFileDialog::getOpenFileName(this,tr("Open File")); // �}�Ҷ}�ɹ�ܲ��è��o�ɦW
    QWidget::setWindowTitle (fileName); // �]�w�������D���}���ɮצW��

    imgSrc = cv::imread(fileName.toStdString(),IMREAD_COLOR); // Ū���v���ɨ�cv:Mat

     if(!imgSrc.empty()) { // �Y�ɮפ���
         ShowFrame(imgSrc, ui->pbxS); // �Ncv::Mat�v����ܩ�QLabel�W
         ui->btnBilateral->setEnabled(true); // �O�iBilateral�j����P��
         ui->btnCuda->setEnabled(true); // �O�iCuda�j����P��
     }
     else{ // �Y�ɮפ��s�b
         ui->btnBilateral->setEnabled(false);  // �O�iBilateral�j����T��
         ui->btnCuda->setEnabled(false); // �O�iCuda�j����T��
         QMessageBox::critical(NULL, "Error", "File not exist!"); // �}�ɥ�����ܿ��~�T��
         return;
    }
}

// ���U�iBilatral�j���V�o�i����(CPU�p��)
void MainWindow::on_btnBilateral_clicked()
{
    double t1=(double)getTickCount(); // ���o�ثe�ɶ�(Tick)

    cv::bilateralFilter(imgSrc, imgTrg, 33, 60, 60); // �HCPU�p�����V�o�i���G

    double t2=(double)getTickCount(); // ���o�ثe�ɶ�(Tick)
    double diff_t = (t2 - t1) / getTickFrequency() * 1000; // �N�ɶ��t�নms

    ui->labStatus->setText("Status : " + QString::number(diff_t,'f',3) + " ms"); // ��ܮɶ��t�󪬺A�r��
    ShowFrame(imgTrg, ui->pbxS); // �Ncv::Mat�v����ܩ�QLabel�W
}

// ���U�iCuda�j���V�o�i����(GPU�p��)
void MainWindow::on_btnCuda_clicked()
{
    int num_devices = cv::cuda::getCudaEnabledDeviceCount(); // �ˬd�t�ΤWGPU�˸m�ƶq

    ui->labStatus->setText("Status : Cuda= " + QString::number(num_devices)); // ���GPU�˸m�ƶq���A�r��W

    if(num_devices > 0){ // �Y�˸m�ƶq�j��0
        cv::cuda::DeviceInfo dev_info(0); // ���o��0��GPU�˸m��T
        int enable_device_id = -1; // �]�wGPU�P��˸m�N�X��-1

        if(dev_info.isCompatible()){ // �YGPU�˸m�����`
            enable_device_id = 0; // �]�wGPU�P��˸m�N�X����0��
            cv::cuda::setDevice(0); // �]�w��0��GPU�˸m

            ui->labStatus->setText(ui->labStatus->text() +
                                   ", Name: " + QString(dev_info.name()) +
                                   ", ID: " + QString::number(enable_device_id)); // ���GPU�W�٤ΥN�X�󪬺A�r��

            double t1=(double)getTickCount(); // ���o�ثe�ɶ�(Tick)

            cv::cuda::GpuMat gmatS(imgSrc); // �Ncv::Mat�v���ǤJGPU�O�����
            cv::cuda::GpuMat gmatT; // �ǳƦs��GPU���G�v���O

            double t2=(double)getTickCount(); // ���o�ثe�ɶ�(Tick)

            cv::cuda::bilateralFilter(gmatS, gmatT, 33, 60, 60); // �HCUDA(GPU)�p�����V�o�i���G

            double t3=(double)getTickCount(); // ���o�ثe�ɶ�(Tick)

            gmatT.download(imgTrg); // �NGPU���G�v���Ǧ^cv::Mat

            double t4=(double)getTickCount(); // ���o�ثe�ɶ�(Tick)

            double diff_t1 = (t2 - t1) / getTickFrequency() * 1000; // �p��v�����JGPU�ɶ�
            double diff_t2 = (t3 - t2) / getTickFrequency() * 1000; // �p��GPU�������V�o�i�ɶ�
            double diff_t3 = (t4 - t3) / getTickFrequency() * 1000; // �p��GPU�Ǧ^���G�v���ɶ�
            double diff_t4 = (t4 - t1) / getTickFrequency() * 1000; // �p���`�@����ɶ�
            QString strResult = "Load Image to GPU = " + QString::number(diff_t1,'f',3) + " ms\n" +
                                "Cuda Bilateral = " + QString::number(diff_t2,'f',3) + " ms\n" +
                                "GPU to Mat = " + QString::number(diff_t3,'f',3) + " ms\n" +
                                "Total time = " + QString::number(diff_t4,'f',3) + " ms\n"; // ���ͭp��ɶ����G�r��

            QMessageBox::information(this,"Infomation",strResult); // �Q�ΰT������ܵ��G�r��
            ShowFrame(imgTrg, ui->pbxS); // �Ncv::Mat�v����ܩ�QLabel�W
        }
        else{
            ui->labStatus->setText("Status : "); // �M�Ū��A�r��
            QMessageBox::critical(NULL, "Error", "GPU module isn't built for GPU!"); // �Q�ΰT�������GPU�X�ʼҲե����T�إ߰T��
        }
    }
}
