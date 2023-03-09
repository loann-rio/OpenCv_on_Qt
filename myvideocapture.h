#ifndef MYVIDEOCAPTURE_H
#define MYVIDEOCAPTURE_H

#include <QPixmap>
#include <QImage>
#include <QThread>

#include <opencv2/opencv.hpp>

#include <vector>
#include <chrono>


#define ID_CAMERA 0

class MyVideoCapture : public QThread
{
    Q_OBJECT
public:
    MyVideoCapture(QObject *parent = nullptr);

    QPixmap pixmap() const
    {
        return mPixmap;
    }
signals:
    void newPixmapCaptured(); //frame
protected:
    void run() override;

private:
    QPixmap mPixmap;
    cv::Mat mFrame;
    cv::VideoCapture mVideoCap;

    // face detection variables
    std::vector<cv::Rect> faceRectangles;
    cv::Rect foreheadROI;
    cv::CascadeClassifier faceDetector;

    // setup variables for heartbeat detection:
    float FPS;
    int samp_f = 10;
    int DISCARD_DURATION = 2;
    int BUFFER_DURATION = 10;

    // variable use to discard the first values
    bool isDiscardData = true;
    int countDiscard = 0;

    // storage of green channel signal
    std::vector<float> greenSignal;


    QImage cvMatToQImage( const cv::Mat &inMat );
    QPixmap cvMatToQPixmap( const cv::Mat &inMat );

    bool loadcascade();
    void detect_face(cv::Mat & mFrame);
    void get_Heartbeat(cv::Mat & mFrame);
    std::vector<float> normalize(std::vector<float> & greenSignal);
    std::vector<float> FFT(std::vector<float> & greenSignalNormalized);
    int get_index_max_value(std::vector<float> & greenFFTModule, float duration);

};

#endif // MYVIDEOCAPTURE_H
