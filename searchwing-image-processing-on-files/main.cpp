#include <opencv2/opencv.hpp>  //!!! OPENCV 3 is used here !!!
#include <stdlib.h>
#include <QFile>
#include <QDir>
#include <QDirIterator>

using namespace std;
using namespace cv;

/* TODO:
 * recursively read all images/videos in a directory and display them v
 * make blob detection on each picture/ frame and draw a rectangle around them
 * cut pictures in rectangles out and save them with an ID. For Videos: all cutten frames shall have same size and will be recombined in a video
 * (optional: use video to get a very good image)
 *
*/

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
//	params.minDistBetweenBlobs = 10000;
	params.filterByColor = false;
	params.blobColor = 200;
	params.filterByArea = true;
	params.minArea = 1000;
	params.maxArea = 6000000;
	params.filterByCircularity = true;
	params.minCircularity = 0.1;
	params.maxCircularity = 0.8;
	params.filterByConvexity = false;
	params.filterByInertia = true;
	params.maxInertiaRatio = 0.8;
	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params); //This is OpenCV3 Style, OpenCV2 would be: SimpleBlobDetector detector; detector.detect(...);
	vector<KeyPoint> keypoints;
	detector->detect(img, keypoints);
	Mat img_with_keypoints;
	drawKeypoints(img, keypoints, img_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	return img_with_keypoints;
}

int main(int argc, char *argv[])
{
	const int NUM_ARGS = 1;
	const string WINDOW_NAME = "test";
	if (argc == NUM_ARGS + 1) {
		string image_path = argv[1];
		cout << "using images/videos from path \"" << image_path << "\"" << endl;

		std::vector<QString> images;
		std::vector<QString> videos;
		getFiles(QString().fromStdString(image_path), images, videos);
		cv::namedWindow(WINDOW_NAME, cv::WINDOW_NORMAL);
		for (unsigned int i = 0; i < images.size(); i++) {
			cv::Mat image = cv::imread(images[i].toStdString()); //options like cv::IMREAD_GRAYSCALE possible
			image = blobDetection(image);
			cv::imshow(WINDOW_NAME, image);
			cout << images[i].toStdString() << endl;
			if (cv::waitKey(1000) == 27) {return 0;}
		}
		for (unsigned int i = 0; i < videos.size(); i++) {
			cout << videos[i].toStdString() << endl; // TODO: Show Video
		}
	} else {
		cout << "Wrong amount of parameters: needed " << NUM_ARGS << ", got " << (argc-1) << "\nusage: \"searchwing-image-processing <image/video directory>\"" << endl;
	}
	return 0;
}
