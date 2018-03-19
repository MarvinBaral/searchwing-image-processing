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
 * https://stackoverflow.com/questions/44633740/opencv-simple-blob-detection-getting-some-undetected-blobs
 *
*/

const string WINDOW_NAME = "Contour Detection";
const string DEBUG_WINDOW_NAME = "Debug";
cv::Mat global_image;

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

Mat contourDetection(Mat img) {
	Mat working_hsv_image;
	cv::cvtColor(img, working_hsv_image, CV_BGR2HSV);
//	cv::medianBlur(working_hsv_image, working_hsv_image, 45);
	std::vector<short int> lowerBounds = {90, 0, 0};
	std::vector<short int> upperBounds = {130, 255, 255};
	cv::inRange(working_hsv_image, lowerBounds, upperBounds, working_hsv_image);
	cv::bitwise_not(working_hsv_image, working_hsv_image);

	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(working_hsv_image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	cv::Scalar color = cv::Scalar(0, 255, 0);
	for (int i = 0; i < contours.size(); i++) {
		cv::drawContours(img, contours, i, color);
	}
	//display longest contour
	cv::Scalar color2 = cv::Scalar(0, 0, 255);
	int maxContourLength = 0;
	int longestContour = 0;
	int i = 0;
	for (; i < contours.size(); i++) {
		if ((int) contours[i].size() > maxContourLength) {
			maxContourLength = std::max(maxContourLength, (int) contours[i].size());
			longestContour = i;
		}
	}
//	cv::drawContours(img, contours, longestContour, color2);
	cv::rectangle(img, cv::boundingRect(contours[longestContour]), color2, 10);

	cout << "number of contours " << contours.size() << endl;
	imshow(DEBUG_WINDOW_NAME, working_hsv_image);
	return img;
}

void update() {
	cv::imshow(WINDOW_NAME, contourDetection(global_image));
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
		cv::namedWindow(DEBUG_WINDOW_NAME, cv::WINDOW_NORMAL);

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
