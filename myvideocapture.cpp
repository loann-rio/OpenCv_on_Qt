#include "myvideocapture.h"

#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QDebug>
#include <QFile>
#include <QUrl>
#include <QResource>

#include <chrono>
#include <vector>


using namespace cv;
using namespace std::chrono;
using namespace std;


MyVideoCapture::MyVideoCapture(QObject *parent) :QThread { parent }, mVideoCap { ID_CAMERA }
{

}

void MyVideoCapture::run()
{
    if (mVideoCap.isOpened()) {

        // timer
        auto start = high_resolution_clock::now();

        // load cascade
        const string cascadePath = ":/cascade/haarcascade_frontalface_alt.xml"; // your cascade file path here

        if(!loadcascade())
        {
            qDebug() << "[ERROR] Unable to load face cascade";
            return;
        };

        while (true) {
            mVideoCap >> mFrame;

            if (!mFrame.empty())
            {
                resize(mFrame, mFrame,Size(480, 360));

                if (isDiscardData) {
                    countDiscard++;
                    if (countDiscard == DISCARD_DURATION*samp_f){
                        isDiscardData = false;
                        start = high_resolution_clock::now();
                    }
                }
                else
                {
                    // get vector of detected faces
                    detect_face(mFrame);

                    if (faceRectangles.empty())
                    {
                        qDebug() << "no detected faces";
                        continue;
                    }

                    Scalar colorsavg = mean(mFrame(foreheadROI));

                    // save color value
                    greenSignal.push_back(colorsavg[1]);

                    // when we have enough data, get result
                    int N = greenSignal.size();

                    if ( N > samp_f*BUFFER_DURATION) {

                        vector<float> greenSignalNormalized = normalize(greenSignal);

                        // get duration to have an approximation of FPS
                        auto stop = high_resolution_clock::now();
                        auto duration1 = chrono::duration_cast<chrono::milliseconds>(stop-start);
                        float duration = duration1.count() /1000; // convert from type 'duration' to float
                        qDebug() << "duration" << duration;
                        FPS = N/duration;
                        //float duration = BUFFER_DURATION;

                        vector<float> greenFFTModule = FFT(greenSignalNormalized);

                        vector<float> frequency;
                        for (int i = 0; i < N; i ++){
                            frequency.push_back(i*FPS/N);
                        }

                        int indexHR = get_index_max_value(greenFFTModule, duration);
                        qDebug() << indexHR;


                        qDebug() <<"your heart rate is "<<indexHR/duration*60;//dividing by duration since i is f*time


                        // clear green signal to reuse it
                        greenSignal.clear();

                        // clear the module as we dont need it anymore
                        greenFFTModule.clear();

                        start = high_resolution_clock::now();
                    }

                }
            }
            mPixmap = cvMatToQPixmap(mFrame);
            emit newPixmapCaptured();
        }
    }
}

QImage MyVideoCapture::cvMatToQImage( const cv::Mat &inMat )
{
    switch ( inMat.type() )
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_ARGB32 );

        return image;
    }

        // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_RGB888 );

        return image.rgbSwapped();
    }

        // 8-bit, 1 channel
    case CV_8UC1:
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_Grayscale8 );
#else
        static QVector<QRgb>  sColorTable;

        // only create our color table the first time
        if ( sColorTable.isEmpty() )
        {
            sColorTable.resize( 256 );

            for ( int i = 0; i < 256; ++i )
            {
                sColorTable[i] = qRgb( i, i, i );
            }
        }

        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_Indexed8 );

        image.setColorTable( sColorTable );
#endif

        return image;
    }

    default:
        qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
        break;
    }

    return QImage();
}

QPixmap MyVideoCapture::cvMatToQPixmap( const cv::Mat &inMat )
{
    return QPixmap::fromImage( cvMatToQImage( inMat ) );
}

bool MyVideoCapture::loadcascade()
{
    QUrl *location_cascade= new QUrl("C:/Users/riolo/opencv/sources/data/haarcascades/haarcascade_frontalface_alt.xml");
    std::string file=location_cascade->toString().toStdString();
    bool loaded = faceDetector.load(file.c_str());
    return loaded;
}

void MyVideoCapture::detect_face(Mat & mFrame)
{
    // get vector of detected faces
    Mat frame_gray; // create a grey frame
    cvtColor(mFrame, frame_gray, COLOR_RGB2GRAY);

    // detect the face:
    faceDetector.detectMultiScale(frame_gray, faceRectangles, 1.1, 3, 0, Size(20, 20));

    // reduce the face rect to get only the forehead
    foreheadROI = faceRectangles[0];
    foreheadROI.height *= 0.4;

    rectangle(mFrame, foreheadROI, Scalar(0, 255, 0), 1, 1, 0);
}

vector<float> MyVideoCapture::normalize(vector<float> & greenSignal)
{
    // normalize the signal
    vector<float> greenSignalNormalized;
    Scalar mean, stddev;
    meanStdDev(greenSignal, mean, stddev);
    for (int element : greenSignal) {
        greenSignalNormalized.push_back((element - mean[0])/stddev[0]);
    }
    return greenSignalNormalized;
}

vector<float> MyVideoCapture::FFT(vector<float> & greenSignalNormalized)
{
    // FFT
    int N = greenSignalNormalized.size();

    Mat greenFFT;
    vector<float> greenFFTModule;
    dft(greenSignalNormalized,greenFFT,DFT_ROWS|DFT_COMPLEX_OUTPUT);

    // get real part of the result
    Mat planes[] = {Mat::zeros(N ,1, CV_64F), Mat::zeros(N,1, CV_64F)};

    split(greenFFT, planes);
    // planes[0] = Re(DFT(I),
    //planes[1] = Im(DFT(I));

    for (int l=0; l < planes[1].cols; l++)
    {
        float moduleFFT = pow(planes[1].at<float>(0,l),2) +
        pow(planes[0].at<float>(0,l),2);
        greenFFTModule.push_back(sqrt(moduleFFT));
    }
    return greenFFTModule;
}

int MyVideoCapture::get_index_max_value(vector<float> & greenFFTModule, float duration)
{
    qDebug() << greenFFTModule.size() << " " << duration;
    float maxValue = -1;
    float lw_lim = 0.85;
    float up_lim = 2;
    int indexHR = 0; //the frequency associated to a weight given by the FFT
    for (int i = lw_lim*duration; i < up_lim*duration; i++){ //i is the corresponding frame in the FFT module
        qDebug() << greenFFTModule[i];
        if (greenFFTModule[i] > maxValue){
            maxValue = greenFFTModule[i];
            indexHR = i;
        }
    }

    return indexHR;
}

