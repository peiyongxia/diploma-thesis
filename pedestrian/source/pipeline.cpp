#include "pipeline.h"
#include <sstream>

int Pipeline::allDetections = 0;

Pipeline::Pipeline()
{

}

void Pipeline::execute(std::vector<cv::Mat> frames)
{
	allDetections = 0;

    for(auto &frame: frames) {
        if(frame.empty()) {
            break;
        }
       process(frame);
       frame.release();
    }
    cv::destroyWindow("Result");
}

void Pipeline::execute(int cameraFeed = 99)
{
	allDetections = 0;
    vs = new VideoStream(cameraFeed);
    vs->openCamera();
    for( ; ; ) {
        cv::Mat frame = vs->getFrame();
        if(frame.empty()) {
            break;
        }
        process(frame);
        frame.release();
    }
  //  cv::destroyWindow("Test");
}

void Pipeline::execute(std::string cameraFeed)
{
	allDetections = 0;
    vs = new VideoStream(cameraFeed);
    vs->openCamera();
    int i = 0;
    for( ; ; ) {
        cv::Mat frame = vs->getFrame();
        if(frame.empty()) {
			delete vs;
			break;
        }
        process(frame);
        frame.release();
        std::stringstream ss;
        ss <<  "../img/mat_" << i << ".jpg";
        cv::imwrite(ss.str(),localFrame);
        localFrame.release();
        i++;
		cv::waitKey(5);
    }
     cv::destroyWindow("Test");
}

void Pipeline::process(cv::Mat frame)
{
	localFrame = frame.clone();
	cv::cvtColor(frame, frame, CV_BGR2GRAY);
	cv::blur(frame, frame, cv::Size(6, 6));
	cv::imshow("Blur", frame);
	frame = mog.processMat(frame);
	cv::blur(frame, frame, cv::Size(9, 9));
	//cv::imshow("MOG", frame);
	std::vector< std::vector< cv::Rect > > rect = ch.wrapObjects(localFrame, frame);

	std::vector< CroppedImage > croppedImages;
	if (rect.size() != 0) {
		for (uint j = 0; j < rect.size(); j++) {
			for (uint i = 0; i < rect[j].size(); i++) {
				croppedImages.emplace_back(CroppedImage(i, localFrame.clone(), rect[j][i]));
			}
		}
	}
	//found_filtered = hog.detect(croppedImages);
	found_filtered = cc.detect(croppedImages);
	draw2mat(croppedImages);
	// if(Settings::showVideoFrames)
	cv::imshow("Result", localFrame);
	frame.release();
	rect.clear();
	found_filtered.clear();
}

void Pipeline::draw2mat(std::vector< CroppedImage > croppedImages)
{
    for (uint j = 0; j < found_filtered.size(); j++) {
        for (uint i = 0; i < found_filtered[j].size(); i++) {
            cv::Rect r = found_filtered[j][i];
            r.x += cvRound(croppedImages[j].offsetX);
            //r.width = cvRound(croppedImages[j].croppedImg.cols);
            r.y += cvRound(croppedImages[j].offsetY);
            //r.height = cvRound(croppedImages[j].croppedImg.rows);
            cv::rectangle(localFrame, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
        }
		allDetections += found_filtered[j].size();
    }
}
