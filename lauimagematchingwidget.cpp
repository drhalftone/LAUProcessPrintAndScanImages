#include "lauimagematchingwidget.h"
#include "laudefaultdirectorieswidget.h"

#include <QProgressDialog>

#include <iostream>
#ifdef USEOPENCV
#include "opencv2/core.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace cv;
using namespace cv::xfeatures2d;
#endif
using std::cout;
using std::endl;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImageMatchDialog::LAUImageMatchDialog(QWidget *parent) : QDialog(parent)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);
    this->setWindowTitle(QString("Image Matching Dialog"));

    // GET A LIST OF IMAGES FROM THE INPUT DIRECTORY
    QStringList filters;
    filters << "*.tif";

    prestineFiles = QDir(LAUDefaultDirectoriesDialog::prestineThumbnailDirectory).entryInfoList(filters, QDir::Files);
    printedFiles = QDir(LAUDefaultDirectoriesDialog::printedThumbnailsDirectory).entryInfoList(filters, QDir::Files);

    qDebug() << printedFiles.first().absoluteFilePath();
    qDebug() << prestineFiles.first().absoluteFilePath();

    widget = new LAUImageMatchingWidget(LAUMemoryObject(printedFiles.first().absoluteFilePath()), LAUMemoryObject(prestineFiles.first().absoluteFilePath()));
    widget->setMinimumSize(480, 320);
    this->layout()->addWidget(widget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    this->layout()->addWidget(buttonBox);

    QPushButton *button = new QPushButton("Batch Process");
    connect(button, SIGNAL(pressed()), this, SLOT(onBatchProcessImages()));
    buttonBox->addButton(button, QDialogButtonBox::ActionRole);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImageMatchingWidget::LAUImageMatchingWidget(LAUMemoryObject objA, LAUMemoryObject objB, QWidget *parent) : objectA(objA), objectB(objB), QWidget(parent)
{
    this->setLayout(new QHBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);

    if (objectA.isNull()) {
        objectA = LAUMemoryObject((QString()));
        if (objectA.isNull()) {
            return;
        }
    }

    if (objectB.isNull()) {
        objectB = LAUMemoryObject((QString()));
        if (objectB.isNull()) {
            return;
        }
    }

    lftWidget = new LAUImageGLWidget(objectB);
    this->layout()->addWidget(lftWidget);

    rghWidget = new LAUImageGLWidget(objectA);
    this->layout()->addWidget(rghWidget);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QMatrix3x3 LAUImageMatchingWidget::homography(LAUMemoryObject objA, LAUMemoryObject objB)
{
    QMatrix3x3 T;

#ifdef USEOPENCV
    Mat img_object = objA.toMat();
    Mat img_scene = objB.toMat();

    //-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
    int minHessian = 400;
    Ptr<SIFT> detector = SIFT::create(minHessian);
    std::vector<KeyPoint> keypoints_object, keypoints_scene;
    Mat descriptors_object, descriptors_scene;
    detector->detectAndCompute(img_object, noArray(), keypoints_object, descriptors_object);
    detector->detectAndCompute(img_scene, noArray(), keypoints_scene, descriptors_scene);

    //-- Step 2: Matching descriptor vectors with a FLANN based matcher
    // Since SURF is a floating-point descriptor NORM_L2 is used
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    std::vector< std::vector<DMatch> > knn_matches;
    matcher->knnMatch(descriptors_object, descriptors_scene, knn_matches, 2);

    //-- Filter matches using the Lowe's ratio test
    const float ratio_thresh = 0.75f;
    std::vector<DMatch> good_matches;
    for (size_t i = 0; i < knn_matches.size(); i++) {
        if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance) {
            good_matches.push_back(knn_matches[i][0]);
        }
    }

    Mat H(3, 3, CV_64F, 0.0);
    if (good_matches.size() > 10) {
        //-- Localize the object
        std::vector<Point2f> obj;
        std::vector<Point2f> scene;
        for (size_t i = 0; i < good_matches.size(); i++) {
            //-- Get the keypoints from the good matches
            obj.push_back(keypoints_object[ good_matches[i].queryIdx ].pt);
            scene.push_back(keypoints_scene[ good_matches[i].trainIdx ].pt);

            qDebug() << keypoints_object[ good_matches[i].queryIdx ].pt.x << keypoints_object[ good_matches[i].queryIdx ].pt.y << keypoints_scene[ good_matches[i].queryIdx ].pt.x << keypoints_scene[ good_matches[i].queryIdx ].pt.y;
        }

        H = findHomography(obj, scene, LMEDS);
    }

    if (H.rows == 3 && H.cols == 3 && H.depth() == CV_64F) {
        T(0, 0) = H.at<double>(0, 0);
        T(0, 1) = H.at<double>(0, 1);
        T(0, 2) = H.at<double>(0, 2);
        T(1, 0) = H.at<double>(1, 0);
        T(1, 1) = H.at<double>(1, 1);
        T(1, 2) = H.at<double>(1, 2);
        T(2, 0) = H.at<double>(2, 0);
        T(2, 1) = H.at<double>(2, 1);
        T(2, 2) = H.at<double>(2, 2);
    } else {
        T(0, 0) = 0.0f;
        T(0, 1) = 0.0f;
        T(0, 2) = 0.0f;
        T(1, 0) = 0.0f;
        T(1, 1) = 0.0f;
        T(1, 2) = 0.0f;
        T(2, 0) = 0.0f;
        T(2, 1) = 0.0f;
        T(2, 2) = 0.0f;
    }
#endif
    return (T);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUImageMatchingWidget::match(LAUMemoryObject objA, LAUMemoryObject objB)
{
    // FORCE A DEEP COPY OF THE TARGET IMAGE
    objB.deepCopy();

#ifdef USEOPENCV
    Mat img_object = objA.toMat();
    Mat img_scene = objB.toMat();

    QMatrix3x3 T = homography(objA, objB);

    Mat H(3, 3, CV_32F);
    H.at<float>(0, 0) = T(0, 0);
    H.at<float>(0, 1) = T(0, 1);
    H.at<float>(0, 2) = T(0, 2);
    H.at<float>(1, 0) = T(1, 0);
    H.at<float>(1, 1) = T(1, 1);
    H.at<float>(1, 2) = T(1, 2);
    H.at<float>(2, 0) = T(2, 0);
    H.at<float>(2, 1) = T(2, 1);
    H.at<float>(2, 2) = T(2, 2);

    warpPerspective(img_object, img_scene, H, img_scene.size());
#endif
    return (objB);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUImageMatchingWidget::matchByReduction(LAUMemoryObject objA, LAUMemoryObject objB)
{
#ifdef USEOPENCV
    Mat imgA, imgB;

    imgA = objB.toMat(true);
    if (objB.colors() == 3) {
        cvtColor(imgA, imgB, COLOR_RGB2GRAY);
    } else if (objB.colors() == 4) {
        cvtColor(imgA, imgB, COLOR_RGBA2GRAY);
    }

    imshow("image B", imgB);

    Mat imgX, imgY;
    reduce(imgB, imgX, 0, REDUCE_AVG, CV_32SC1);
    reduce(imgB, imgY, 1, REDUCE_AVG, CV_32SC1);

    int lftEdge = imgX.cols - 1, rghEdge = 0;
    for (int n = 0; n < imgX.cols; n++) {
        qDebug() << imgX.at<int>(n);
        if (imgX.at<int>(n) < 180) {
            lftEdge = qMin(n, lftEdge);
            rghEdge = qMax(n, rghEdge);
        }
    }

    int topEdge = imgY.rows - 1, botEdge = 0;
    for (int n = 0; n < imgY.rows; n++) {
        if (imgY.at<int>(n) < 180) {
            topEdge = qMin(n, topEdge);
            botEdge = qMax(n, botEdge);
        }
    }

    if ((rghEdge - lftEdge) > 100 && (botEdge - topEdge) > 100) {
        imgB = imgA.rowRange(topEdge + 1, botEdge - 1).colRange(lftEdge + 1, rghEdge - 1);
        cv::resize(imgB, imgB, cv::Size((int)objA.width(), (int)objA.height()));
    }

    LAUMemoryObject object((unsigned int)imgB.cols, (unsigned int)imgB.rows, objB.colors(), objB.depth());
    for (unsigned int row = 0; row < object.height(); row++) {
        memcpy(object.constScanLine(row), imgB.ptr((int)row), qMin((int)object.step(), (int)imgB.step));
    }
    return (object);
#else
    return (LAUMemoryObject());
#endif
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUImageMatchDialog::onBatchProcessImages()
{
    int numFiles = qMax(printedFiles.count(), prestineFiles.count());

    QProgressDialog dialog(QString("Processing thumbnails..."), QString("Abort"), 0, numFiles, this);
    dialog.show();
    for (int n = 0; n < numFiles; n++) {
        if (dialog.wasCanceled()) {
            break;
        } else {
            dialog.setValue(n);
            qApp->processEvents();
        }
        //LAUMemoryObject object = LAUImageMatchingWidget::match(LAUMemoryObject(printedFiles.at(n).absoluteFilePath()), LAUMemoryObject(prestineFiles.at(n).absoluteFilePath()));
        LAUMemoryObject object = widget->matchByReduction(LAUMemoryObject(prestineFiles.at(n).absoluteFilePath()), LAUMemoryObject(printedFiles.at(n).absoluteFilePath()));
        if (object.isValid()) {
            QString string = QString("%1/warp%2").arg(LAUDefaultDirectoriesDialog::warpedThumbnailsDirectory).arg(printedFiles.at(n).fileName().right(18));
            object.save(string);
        }
    }
}
