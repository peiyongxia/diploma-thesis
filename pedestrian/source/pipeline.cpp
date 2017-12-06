#include "pipeline.h"
#include <sstream>
#include <ctime>

int Pipeline::allDetections = 0;

Pipeline::Pipeline()
{
	hog = Hog("48_96_16_8_8_9_01.yml");

}

void Pipeline::executeImages(std::string testSamplesPath)
{
	allDetections = 0;
	assert(!testSamplesPath.empty());
	cv::Mat frame;
	std::fstream sampleFile(testSamplesPath);
	std::string oSample;
	while (sampleFile >> oSample) {
		frame = cv::imread(oSample, CV_32FC3);
        if(frame.empty()) {
			sampleFile.close();
            break;
        }
       processStandaloneIm(frame);
	   cv::waitKey(0);
       frame.release();
    }
    cv::destroyWindow("Result");
}

void Pipeline::execute(int cameraFeed = 99)
{
	allDetections = 0;
    vs = new VideoStream(cameraFeed);
	std::cout << "Camera initialized." << std::endl;
	vs->openCamera();
    for(int i = 0; ; i++ ) {
        cv::Mat frame = vs->getFrame();
        if(frame.empty()) {
            break;
        }
        process(frame, i);
        frame.release();
		cv::waitKey(5);
    }
  //  cv::destroyWindow("Test");
}

std::stringstream ss;

void Pipeline::execute(std::string cameraFeed)
{
	allDetections = 0;
    vs = new VideoStream(cameraFeed);
    vs->openCamera();
	std::cout << "Videostream initialized." << std::endl;
	rects2Eval = std::vector < std::vector < std::vector < cv::Rect > > >(vs->totalFrames);

    for(int i = 0; ; i++) {
        cv::Mat frame = vs->getFrame();
        if(frame.empty()) {
			delete vs;
			saveResults("test.txt");
			break;
        }
		//time_t time = clock();
        process(frame, i);
		//time = clock() - time;
	//	std::cout << static_cast<float>(time) / CLOCKS_PER_SEC << std::endl;

        frame.release();
        
		cv::waitKey(5);
		ss << "../img/mat_" << i << ".jpg";
		cv::imwrite(ss.str(), localFrame);
		ss.str("");
		ss.clear();
		localFrame.release();
    }

     cv::destroyWindow("Test");
}


void Pipeline::process(cv::Mat &frame, int cFrame)
{
	localFrame = frame.clone();
	preprocessing(frame);
	frame = mog.processMat(frame);
	preprocessing(frame, true);
	///cv::blur(frame, frame, cv::Size(9, 9));
	
	//cv::imshow("MOG", frame);
	//cv::imwrite("test.jpg", frame);
	std::vector< cv::Rect > rect = ch.wrapObjects(localFrame, frame);

	if (rect.size() != 0) {
		std::vector< CroppedImage > croppedImages;
		for (size_t i = 0; i < rect.size(); i++) {
			croppedImages.emplace_back(CroppedImage(i, localFrame.clone(), rect[i]));
		}
		std::vector < std::vector < cv::Rect > > foundRect;
		
	//	foundRect = fhog.detect(croppedImages);
		foundRect = hog.detect(croppedImages);
		//foundRect = cc.detect(croppedImages);
		rects2Eval[cFrame] = foundRect;
	//	draw2mat(croppedImages, foundRect);
	foundRect.clear();
	}
	// if(Settings::showVideoFrames)
	//cv::imshow("Result", localFrame);
	frame.release();
	rect.clear();
}


void Pipeline::processStandaloneIm(cv::Mat &frame)
{
	localFrame = frame.clone();
	preprocessing(frame);
	std::vector < cv::Rect  > foundRect;
	//foundRect = fhog.detect(frame);
		foundRect = hog.detect(frame);
	//foundRect = cc.detect(frame);
	draw2mat(foundRect);

	// if(Settings::showVideoFrames)
	cv::imshow("Result", localFrame);
	foundRect.clear();
}

void Pipeline::preprocessing(cv::Mat& frame, bool afterMog)
{
	
	if(afterMog)
	{
		int dilation_type = cv::MORPH_RECT;
		int erosion_type = cv::MORPH_CROSS;
		int dilation_size = 4;
		int erosion_size = 3;
		cv::Mat dilMat = getStructuringElement(dilation_type,
			cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
			cv::Point(dilation_size, dilation_size));
		cv::Mat eroMat = getStructuringElement(erosion_type,
			cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
			cv::Point(erosion_size, erosion_size));
		cv::dilate(frame, frame, dilMat);
		cv::erode(frame, frame, eroMat);
	}
	else
	{
		cv::cvtColor(frame, frame, CV_BGR2GRAY);
		frame.convertTo(frame, CV_8UC1);

	}
	//cv::medianBlur(frame, frame, 3);
	
	//cv::Mat dst(frame.rows, frame.cols, CV_8UC1);
	//cv::bilateralFilter(frame, dst, 10, 1.5, 1.5, cv::BORDER_DEFAULT);
	//cv::GaussianBlur(frame, frame, cv::Size(9, 9), 2,4 , cv::BORDER_DEFAULT);
	//cv::blur(frame, frame, cv::Size(6, 6)); //medianBlur
	//cv::imshow("Blur", frame);
}

void Pipeline::draw2mat(std::vector< CroppedImage > &croppedImages, std::vector < std::vector < cv::Rect > > &rect)
{
	for (uint j = 0; j < rect.size(); j++) {
		for (uint i = 0; i < rect[j].size(); i++) {
			cv::Rect r = rect[j][i];
			r.x += cvRound(croppedImages[j].offsetX);
			//r.width = cvRound(croppedImages[j].croppedImg.cols);
			r.y += cvRound(croppedImages[j].offsetY);
			//r.height = cvRound(croppedImages[j].croppedImg.rows);
			cv::rectangle(localFrame, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
		}
		allDetections += rect[j].size();
	}
	rect.clear();

}

void Pipeline::draw2mat(std::vector < cv::Rect > &rect)
{
	for (uint i = 0; i < rect.size(); i++) {
		cv::rectangle(localFrame, rect[i], cv::Scalar(0, 255, 0), 3);
	}
	allDetections += rect.size();
}

void Pipeline::saveResults(std::string filePath)
{
	std::ofstream fs;
	fs.open(filePath);
	fs << rects2Eval.size() << std::endl;
	for (uint i = 0; i < rects2Eval.size(); i++)	{
		for (uint j = 0; j < rects2Eval[i].size(); j++)	{
			for (uint k = 0; k < rects2Eval[i][j].size(); k++)	{
				if (rects2Eval[i][j][k].area() == 0) continue;
				fs << i << " " << rects2Eval[i][j][k].tl().x << " " << rects2Eval[i][j][k].tl().y << " " << rects2Eval[i][j][k].br().x << " " << rects2Eval[i][j][k].br().y << std::endl;
			}
		}
	}
	fs.close();
}

void Pipeline::loadRects(std::string filePath, std::vector< std::vector<cv::Rect> > & rects)
{
	std::ifstream ifs;
	ifs.open(filePath);
	int x1, x2, y1, y2, cFrame;
	std::string frame;
	std::string fileContents;
	unsigned int curLine;
	int i = 0;
	if (ifs.is_open())
	{
		getline(ifs, fileContents, '\x1A');
		ifs.close();
	}
	//	std::getline(ifs, fileContents, '\n');
	std::istringstream iss(fileContents);
	iss >> curLine;
	rects = std::vector< std::vector<cv::Rect> >(curLine);
	while (!iss.eof()) {

		iss >> cFrame >> x1 >> y1 >> x2 >> y2;
		cv::Point p1(x1, y1);
		cv::Point p2(x2, y2);
		rects[cFrame].emplace_back(cv::Rect(p1, p2));
	}
}

void Pipeline::evaluate(std::string testResultPath, std::string trainedPosPath)
{
	std::vector< std::vector<cv::Rect> > trained;
	std::vector< std::vector<cv::Rect> > test;
	loadRects("test.txt", test);
	loadRects("trained.txt", trained);
	int truePos = 0, falsePos = 0;
	int trueNeg = 0, falseNeg = 0;
	for (int i = 0; i < test.size(); i++) {
		if (test[i].empty() && trained[i].empty()) { trueNeg++;  continue; } // There is no pedestrian - OK
		if (test[i].empty() && !trained[i].empty()) { falseNeg++; continue; } // There is no pedestrian but something detect
		if (!test[i].empty() && trained[i].empty()) { falsePos++; continue; } // There is pedestrian but no detect

		for (int j = 0; j < test[i].size(); j++) {
			if (cv::norm(test[i][j].x - trained[i][j].x) < 51 &&  cv::norm(test[i][j].y - trained[i][j].y) < 51)
			{
				truePos++; // Pedestrian detected !
			}
			else
			{
				falsePos++; // Pedestrian no detected !
			}
		}

	}
	float acc = (float)(truePos + trueNeg) /
		(float)(truePos + trueNeg + falsePos + falseNeg);

	std::cout << "True Positive: " << truePos << std::endl;
	std::cout << "False Positive: " << falsePos << std::endl;
	std::cout << "True Negative: " << trueNeg << std::endl;
	std::cout << "False Negative: " << falseNeg << std::endl;
	std::cout << "Accuracy: " << acc << std::endl;

	std::ofstream ofs;
	ofs.open("results.txt");
	ofs << "Results" << std::endl;
	ofs << "True Positive: " << truePos << std::endl;
	ofs << "False Positive: " << falsePos << std::endl;
	ofs << "True Negative: " << trueNeg << std::endl;
	ofs << "False Negative: " << falseNeg << std::endl;
	ofs << "Accuracy: " << acc << std::endl;

}