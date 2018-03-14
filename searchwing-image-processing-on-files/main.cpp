#include <opencv2/opencv.hpp>  //!!! OPENCV 3 is used here !!!
#include <stdlib.h>
#include <QFile>
#include <QDir>
#include <QDirIterator>

using namespace std;
using namespace cv;

/* TODO:
 * make blob detection on each picture/ frame and draw a rectangle around them
 * cut pictures in rectangles out and save them with an ID. For Videos: all cutten frames shall have same size and will be recombined in a video
 * (optional: use video to get a very good image)
 *
*/

const string WINDOW_NAME = "Simple Blob Detection";
cv::Mat global_image;
int medianBlurValue = 1;
//int minThreshold = 20;
//int maxThreshold = 200;
int minDistBetweenBlobs = 100;
int blobColorSlider = 255;
int minAreaSlider = 10;
int maxAreaSlider = 6000000;
int minCircularity = 10;
int maxCircularity = 90;
int minConvexity = 0;
int maxConvexity = 100;
int minInertia = 0;
int maxInertia = 100;

void getFiles(QString path, std::vector<QString> &images, std::vector<QString> &videos) { //!!! Hard string comparison with jpg, jpeg and mp4 !!!
	QDirIterator iterator(path, QDirIterator::Subdirectories);
	while (iterator.hasNext()) {
		QFile file(iterator.next());
		if (file.fileName().endsWith(".jpg", Qt::CaseInsensitive) || file.fileName().endsWith(".jpeg", Qt::CaseInsensitive)) {
			images.push_back(file.fileName());
		} else if (file.fileName().endsWith(".mp4", Qt::CaseInsensitive)) {
			videos.push_back(file.fileName());
		}
	}
}

Mat blobDetection(Mat img) {
	cv::SimpleBlobDetector::Params params;
//	params.minThreshold = minThreshold;
//	params.maxThreshold = maxThreshold;
	params.minDistBetweenBlobs = minDistBetweenBlobs;
	params.filterByColor = false;
	params.blobColor = blobColorSlider;
	params.filterByArea = true;
	params.minArea = minAreaSlider;
	params.maxArea = maxAreaSlider;
	params.filterByCircularity = true;
	params.minCircularity = minCircularity / 100.f;
	params.maxCircularity = maxCircularity / 100.f;
	params.filterByConvexity = true;
	params.minConvexity = minConvexity / 100.f;
	params.maxConvexity = maxConvexity / 100.f;
	params.filterByInertia = true;
	params.minInertiaRatio = minInertia / 100.f;
	params.maxInertiaRatio = maxInertia / 100.f;
	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params); //This is OpenCV3 Style, OpenCV2 would be: SimpleBlobDetector detector; detector.detect(...);
	vector<KeyPoint> keypoints;
	detector->detect(img, keypoints);
	Mat img_with_keypoints;
	drawKeypoints(img, keypoints, img_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	return img_with_keypoints;
}

void update() {
	cv::Mat showed_image;
	cv::medianBlur(global_image, showed_image, medianBlurValue * 2 + 1);
	cv::imshow(WINDOW_NAME, blobDetection(showed_image));
	cout << "updated" << endl;
}

void sliderCallback(int value, void*) {
//	update();
}

int main(int argc, char *argv[])
{
	const int NUM_ARGS = 1;
	if (argc == NUM_ARGS + 1) {
		string image_path = argv[1];
		cout << "using images/videos from path \"" << image_path << "\"" << endl;

		std::vector<QString> images;
		std::vector<QString> videos;
		getFiles(QString().fromStdString(image_path), images, videos);
		cv::namedWindow(WINDOW_NAME, cv::WINDOW_NORMAL);
		cv::createTrackbar("medianBlurValue", WINDOW_NAME, &medianBlurValue, 180, sliderCallback);
//		cv::createTrackbar("minThreshold", WINDOW_NAME, &minThreshold, 1000, sliderCallback);
//		cv::createTrackbar("maxThreshold", WINDOW_NAME, &maxThreshold, 1000, sliderCallback);
		cv::createTrackbar("minDistBetweenBlobs", WINDOW_NAME, &minDistBetweenBlobs, 10000, sliderCallback);
		cv::createTrackbar("blobColor", WINDOW_NAME, &blobColorSlider, 255, sliderCallback);
		cv::createTrackbar("minArea", WINDOW_NAME, &minAreaSlider, 100000, sliderCallback);
		cv::createTrackbar("maxArea", WINDOW_NAME, &maxAreaSlider, 12000000, sliderCallback);
		cv::createTrackbar("minCircularity", WINDOW_NAME, &minCircularity, 100, sliderCallback);
		cv::createTrackbar("maxCircularity", WINDOW_NAME, &maxCircularity, 100, sliderCallback);
		cv::createTrackbar("minConvexity", WINDOW_NAME, &minConvexity, 100, sliderCallback);
		cv::createTrackbar("maxConvexity", WINDOW_NAME, &maxConvexity, 100, sliderCallback);
		cv::createTrackbar("minInertia", WINDOW_NAME, &minInertia, 100, sliderCallback);
		cv::createTrackbar("maxInertia", WINDOW_NAME, &maxInertia, 100, sliderCallback);
		unsigned int i = 0;
		global_image = cv::imread(images[i].toStdString()); //options like cv::IMREAD_GRAYSCALE possible
		cout << images[i].toStdString() << endl;
		update();
		while (i >= 0 && i < images.size()) {
			int keyPressed = cv::waitKey(0);
			switch (keyPressed) {
				case 27: return 0; //ESC
				case 119: //w
				case 97: //a
				case 82: //up
					if (i > 0) {i--;}
					global_image = cv::imread(images[i].toStdString()); //options like cv::IMREAD_GRAYSCALE possible
					cout << images[i].toStdString() << endl;
					update();
					break;
				case 115: //s
				case 100: //d
				case 84: //down
					if (i < (images.size() - 1)) {i++;}
					global_image = cv::imread(images[i].toStdString()); //options like cv::IMREAD_GRAYSCALE possible
					cout << images[i].toStdString() << endl;
					update();
					break;
				case 81: //left
				case 83: //right
					break;
				case 117: //u
					update();
					break;
				default:
					cout << keyPressed << endl;
			}
		}
		for (unsigned int i = 0; i < videos.size(); i++) {
			cout << videos[i].toStdString() << endl; // TODO: Show Video
		}
	} else {
		cout << "Wrong amount of parameters: needed " << NUM_ARGS << ", got " << (argc-1) << "\nusage: \"searchwing-image-processing <image/video directory>\"" << endl;
	}
	return 0;
}
