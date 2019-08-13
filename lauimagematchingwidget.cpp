#include "lauimagematchingwidget.h"

#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace cv;
using namespace cv::xfeatures2d;
using std::cout;
using std::endl;

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

    lftWidget = new LAUImageGLWidget(objectA);
    this->layout()->addWidget(lftWidget);

    rghWidget = new LAUImageGLWidget(objectB);
    this->layout()->addWidget(rghWidget);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QMatrix3x3 LAUImageMatchingWidget::homography(LAUMemoryObject objA, LAUMemoryObject objB)
{
    Mat img_object = objA.toMat();
    Mat img_scene = objB.toMat();

    //-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
    int minHessian = 400;
    Ptr<SURF> detector = SURF::create(minHessian);
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
    if (good_matches.size() > 20) {
        //-- Localize the object
        std::vector<Point2f> obj;
        std::vector<Point2f> scene;
        for (size_t i = 0; i < good_matches.size(); i++) {
            //-- Get the keypoints from the good matches
            obj.push_back(keypoints_object[ good_matches[i].queryIdx ].pt);
            scene.push_back(keypoints_scene[ good_matches[i].trainIdx ].pt);
        }

        H = findHomography(obj, scene, RANSAC);
    }

    QMatrix3x3 T;
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

    return (T);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUImageMatchingWidget::match(LAUMemoryObject objA, LAUMemoryObject objB)
{
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

    return (objB);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMemoryObject LAUImageMatchDialog::batchProcessImages(QStringList inputs, QStringList outputs)
{
    LAUMemoryObject object(3 * outputs.count(), 3 * inputs.count(), 1, sizeof(float));
    for (int row = 0; row < inputs.count() && row < 20; row++) {
        LAUMemoryObject input(inputs.at(row));
        if (input.isValid()) {
            for (int col = 0; col < outputs.count() && col < 20; col++) {
                LAUMemoryObject output(outputs.at(col));
                if (output.isValid()) {
                    qDebug() << inputs.at(row);
                    qDebug() << outputs.at(col);

                    QMatrix3x3 T = LAUImageMatchingWidget::homography(input, output);
                    reinterpret_cast<float *>(object.constScanLine(3 * row + 0))[3 * col + 0] = T(0, 0);
                    reinterpret_cast<float *>(object.constScanLine(3 * row + 0))[3 * col + 1] = T(0, 1);
                    reinterpret_cast<float *>(object.constScanLine(3 * row + 0))[3 * col + 2] = T(0, 2);
                    reinterpret_cast<float *>(object.constScanLine(3 * row + 1))[3 * col + 0] = T(1, 0);
                    reinterpret_cast<float *>(object.constScanLine(3 * row + 1))[3 * col + 1] = T(1, 1);
                    reinterpret_cast<float *>(object.constScanLine(3 * row + 1))[3 * col + 2] = T(1, 2);
                    reinterpret_cast<float *>(object.constScanLine(3 * row + 2))[3 * col + 0] = T(2, 0);
                    reinterpret_cast<float *>(object.constScanLine(3 * row + 2))[3 * col + 1] = T(2, 1);
                    reinterpret_cast<float *>(object.constScanLine(3 * row + 2))[3 * col + 2] = T(2, 2);
                }
            }
        }
        qDebug() << "ROW:" << row;
    }
    return (object);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUImageMatchDialog::onBatchProcessImages()
{
    QStringList inputStrings, outputStrings;

    QSettings settings;
    QString directory = settings.value("LAUImageMatchDialog::sourceImageDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString sourceImageDirectory = QFileDialog::getExistingDirectory(this, QString("Load source directory..."), directory);
    if (sourceImageDirectory.isEmpty() == false) {
        settings.setValue("LAUImageMatchDialog::sourceImageDirectory", sourceImageDirectory);

        // GET A LIST OF IMAGES FROM THE INPUT DIRECTORY
        QStringList filters;
        filters << "*.tif" << "*.tiff";
        inputStrings = QDir(sourceImageDirectory).entryList(filters, QDir::Files);

        // PREPEND THE DIRECTORY TO CREATE ABSOLUTE PATH NAMES
        for (int n = 0; n < inputStrings.count(); n++) {
            QString string = QString("%1/%2").arg(sourceImageDirectory).arg(inputStrings.at(n));
            inputStrings.replace(n, string);
        }
    } else {
        return;
    }

    directory = settings.value("LAUImageMatchDialog::sinkImageDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString sinkImageDirectory = QFileDialog::getExistingDirectory(this, QString("Load sink directory..."), directory);
    if (sinkImageDirectory.isEmpty() == false) {
        settings.setValue("LAUImageMatchDialog::sinkImageDirectory", sinkImageDirectory);

        // GET A LIST OF IMAGES FROM THE INPUT DIRECTORY
        QStringList filters;
        filters << "*.tif" << "*.tiff";
        outputStrings = QDir(sinkImageDirectory).entryList(filters, QDir::Files);

        // PREPEND THE DIRECTORY TO CREATE ABSOLUTE PATH NAMES
        for (int n = 0; n < outputStrings.count(); n++) {
            QString string = QString("%1/%2").arg(sinkImageDirectory).arg(outputStrings.at(n));
            outputStrings.replace(n, string);
        }
    } else {
        return;
    }

    while (inputStrings.count() > 100) {
        inputStrings.takeLast();
    }

    while (outputStrings.count() > 100) {
        outputStrings.takeLast();
    }

    LAUMemoryObject object = batchProcessImages(inputStrings, outputStrings);
    if (object.save()) {
        accept();
    }
}
