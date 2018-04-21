#include <opencv2/opencv.hpp>  //!!! OPENCV 3 is used here !!!
#include <stdlib.h>
#include <QFile>
#include <QDir>
#include <QDirIterator>

using namespace std;
using namespace cv;

/* TODO:
 * cut pictures in rectangles out and save them with an ID. For Videos: all cutten frames shall have same size and will be recombined in a video
 * (optional: use video to get a very good image)
 *
 * Test false-positives
 * maybe try canny edge detector
 * https://stackoverflow.com/questions/44633740/opencv-simple-blob-detection-getting-some-undetected-blobs
 *
*/

/* Issues:
 * sky or optical effects on water are detected and have huge contours
 * -> lot of big effects come from Moonbirds cockpit glass -> might not be an issue with the Rpi3-Camera facing air directly
 * -> check diversity of image and remove the ones with low diversity
 * -> actively remove sky
 * especially bigger ships are only detected in parts
 * -> done by merging overlapping rects, but could be improved (merging overlapping rects after merging overlapping rects???)
 * when sea is very reflective, no ships are detected
 * two nearby ships are merged together - wanted or not wanted? - I think this is ok since the images are used to be shown to humans to decide whether these need to be saved or not and not for ship counting
 * merging rects somehow deletes some rects!!!
*/

const string WINDOW_NAME = "Contour Detection";
const string DEBUG_WINDOW_NAME = "Thresholded Image";
const string CUTTEN_FRAME_WINDOW_NAME = "Cutten Frame";
const bool SHOW_INITIAL_RECTS = true;
int pen_size = 10; //make it dependant from image resolution?!
cv::Mat global_image;
int num_showed_contours = 100;
//pure blue color is hsv 240° -> here 120
int water_min_hue = 90;
int water_max_hue = 150; //130 was good too
const int numDebugWindows = 3;
string debugWindows[numDebugWindows];

void getFiles(QString path, std::vector<QString> &images, std::vector<QString> &videos) { //!!! Hard string comparison with jpg, jpeg, png and mp4 !!!
	QDirIterator iterator(path, QDirIterator::Subdirectories);
	while (iterator.hasNext()) {
		QFile file(iterator.next());
		if (file.fileName().endsWith(".jpg", Qt::CaseInsensitive) || file.fileName().endsWith(".jpeg", Qt::CaseInsensitive) || file.fileName().endsWith(".png", Qt::CaseInsensitive)) {
			images.push_back(file.fileName());
		} else if (file.fileName().endsWith(".mp4", Qt::CaseInsensitive)) {
			videos.push_back(file.fileName());
		}
	}
}

//area comparison func
bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 ) {
	double i = fabs( contourArea(cv::Mat(contour1)) );
	double j = fabs( contourArea(cv::Mat(contour2)) );
	return ( i < j );
}

void getRectsByThresholding(std::vector<cv::Rect>& rects, Mat& working_hsv_image,std::vector<int> lowerBounds, std::vector<int> upperBounds, const int MAX_CONTOURS, bool inverted = false) {
	cv::inRange(working_hsv_image, lowerBounds, upperBounds, working_hsv_image);
	if (inverted) {cv::bitwise_not(working_hsv_image, working_hsv_image);}
	//find contours
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(working_hsv_image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// sort contours ascending by area https://stackoverflow.com/questions/13495207/opencv-c-sorting-contours-by-their-contourarea
	std::sort(contours.begin(), contours.end(), compareContourAreas);

	for (int i = contours.size() - 1; i > std::max(((int) contours.size() - 1 - MAX_CONTOURS), 0); i--) {
		rects.push_back(cv::boundingRect(contours[i]));
	}
}

void getRectsByThresholding(std::vector<cv::Rect>& rects, Mat& working_hsv_image, cv::Scalar lowerBounds, cv::Scalar upperBounds, const int MAX_CONTOURS, bool inverted = false) {
	cv::inRange(working_hsv_image, lowerBounds, upperBounds, working_hsv_image);
	if (inverted) {cv::bitwise_not(working_hsv_image, working_hsv_image);}
	//find contours
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(working_hsv_image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// sort contours ascending by area https://stackoverflow.com/questions/13495207/opencv-c-sorting-contours-by-their-contourarea
	std::sort(contours.begin(), contours.end(), compareContourAreas);

	for (int i = contours.size() - 1; i > std::max(((int) contours.size() - 1 - MAX_CONTOURS), 0); i--) {
		rects.push_back(cv::boundingRect(contours[i]));
	}
}

Mat contourDetection2(Mat img) {
	cv::Scalar red = cv::Scalar(0, 0, 255);
	cv::Scalar green = cv::Scalar(0, 255, 0);
	cv::Scalar blue = cv::Scalar(255, 0, 0);
	cv::Scalar white = cv::Scalar(255, 255, 255);
	Mat working_hsv_image;
	Mat displayed_image;
	img.copyTo(displayed_image);
	cv::cvtColor(img, working_hsv_image, CV_BGR2HSV);
	std::vector<int> lowerBounds = {water_min_hue, 0, 0};
	std::vector<int> upperBounds = {water_max_hue, 255, 255};

	//detect contours
	std::vector<cv::Rect> rects;
	getRectsByThresholding(rects, working_hsv_image, lowerBounds, upperBounds, 100, true);
	std::cout << "Num rects: " << rects.size() << std::endl;
	imshow(DEBUG_WINDOW_NAME, working_hsv_image);

	//return if no contours were found
	if (rects.size() == 0) {return displayed_image;}

	//merge overlapping rects
	cv::Mat working_image_2 = Mat(working_hsv_image.rows, working_hsv_image.cols, CV_8UC3, cv::Scalar(0,0,0));
	for (unsigned int i = 0; i < rects.size(); i++) {
		cv::rectangle(working_image_2, rects[i], red, pen_size);
	}
	std::vector<cv::Rect> rects2;
	getRectsByThresholding(rects2, working_image_2, red, red, 100, false);
	std::cout << "Num rects (Merge1): " << rects2.size() << std::endl;
	imshow(debugWindows[0], working_image_2);

	//merge overlapping rects2
	cv::Mat working_image_3 = Mat(working_hsv_image.rows, working_hsv_image.cols, CV_8UC3, cv::Scalar(0,0,0));
	for (unsigned int i = 0; i < rects2.size(); i++) {
		cv::rectangle(working_image_3, rects2[i], red, pen_size);
	}
	std::vector<cv::Rect> rects3;
	getRectsByThresholding(rects3, working_image_3, red, red, 100, false);
	std::cout << "Num rects (Merge2): " << rects3.size() << std::endl;
	imshow(debugWindows[1], working_image_3);

	//show final merged rects in debug window
	cv::Mat working_image_4 = Mat(working_hsv_image.rows, working_hsv_image.cols, CV_8UC3, cv::Scalar(0,0,0));
	for (unsigned int i = 0; i < rects3.size(); i++) {
		cv::rectangle(working_image_4, rects3[i], white, pen_size);
	}
	imshow(debugWindows[2], working_image_4);


	//cut frame for largest contour
	if (rects3.size() > 0) {
		Rect rect = rects3[0];
		rect.x = std::max((int)(rect.x - rect.width * 0.5), 0);
		rect.y = std::max((int)(rect.y - rect. height * 0.5), 0);
		rect.width = std::min(rect.width * 2, displayed_image.cols - rect.x);
		rect.height = std::min(rect.height * 2, displayed_image.rows - rect.y);
		Mat rectContent(displayed_image, rect);
		Mat cuttenFrame;
		rectContent.copyTo(cuttenFrame);
		imshow(CUTTEN_FRAME_WINDOW_NAME, cuttenFrame);

		cv::rectangle(displayed_image, rect, Scalar(0, 255, 255), pen_size);
	} else {
		//display black screen
		cv::Mat na_image = Mat(50, 50, CV_8UC3, cv::Scalar(0,0,0));
		cv::putText(na_image, "N/A", cv::Point(10, 30), 1, CV_FONT_HERSHEY_PLAIN , cv::Scalar(255, 255, 255));
		cv::imshow(CUTTEN_FRAME_WINDOW_NAME, na_image);
	}

	//display largest rects
	if (SHOW_INITIAL_RECTS) {
		for (int i = 0; i < std::min(num_showed_contours, (int) rects.size()); i++) {
			cv::rectangle(displayed_image, rects[i], red, pen_size);
		}
		for (int i = 0; i < std::min(num_showed_contours, (int) rects2.size()); i++) {
			cv::rectangle(displayed_image, rects2[i], blue, pen_size);
		}
	}
	for (int i = 0; i < std::min(num_showed_contours, (int) rects3.size()); i++) {
		cv::rectangle(displayed_image, rects3[i], green, pen_size);
	}

	return displayed_image;
}

void update() {
	cv::imshow(WINDOW_NAME, contourDetection2(global_image));
	cout << "updated" << endl;
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
		cv::namedWindow(CUTTEN_FRAME_WINDOW_NAME, cv::WINDOW_NORMAL);
		cv::createTrackbar("water min hue", DEBUG_WINDOW_NAME, &water_min_hue, 179);
		cv::createTrackbar("water max hue", DEBUG_WINDOW_NAME, &water_max_hue, 179);
		for (int i = 0; i < numDebugWindows; i++) {
			debugWindows[i] = "Debug Window " + to_string(i);
			cv::namedWindow(debugWindows[i], cv::WINDOW_NORMAL);
		}
		unsigned int i = 0;
		global_image = cv::imread(images[i].toStdString()); //options like cv::IMREAD_GRAYSCALE possible
		cout << images[i].toStdString() << endl;
		update();
		while (i >= 0 && i < images.size()) {
			int keyPressed = cv::waitKey(0);
			switch (keyPressed) {
				case 27: return 0; //ESC
				case 119: //w
				case 82: //up
					if (i > 0) {i--;}
					global_image = cv::imread(images[i].toStdString()); //options like cv::IMREAD_GRAYSCALE possible
					cout << images[i].toStdString() << endl;
					update();
					break;
				case 115: //s
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
				case 48: num_showed_contours = 0; update(); break; //0, to clear all additional drawing and only show original image
				case 49: num_showed_contours = 1; update(); break; //1
				case 50: num_showed_contours = 2; update(); break; //2
				case 51: num_showed_contours = 3; update(); break; //3
				case 52: num_showed_contours = 4; update(); break; //4
				case 53: num_showed_contours = 5; update(); break; //5
				case 54: num_showed_contours = 6; update(); break; //6
				case 55: num_showed_contours = 7; update(); break; //7
				case 56: num_showed_contours = 8; update(); break; //8
				case 57: num_showed_contours = 9; update(); break; //9
				case 100: num_showed_contours = 100; update(); break; //100
				case 97: num_showed_contours = 1000; update(); break; //all!!
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
