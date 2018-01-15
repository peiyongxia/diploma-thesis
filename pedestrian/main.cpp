#include <iostream>
#include <cstdlib>
#include <opencv2/core/utility.hpp>
#include "source/train/trainhog.h"
#include "source/pipeline.h"
#include "source/train/combinedTrainHog.h"
#include "source/utils/extractorROI.h"
#include "source/utils/utils.h"
#include "source/test/testClass.h"
#include <csignal>
#include <signal.h>
////////////////////////////////////////////////////////
//		DATA		 //
//////////////////////

std::string posSamples = "samples/posSamples.txt";
std::string negSamples = "samples/negSamples.txt";

///////////////////////
//					//
///////////////////////////////////////////////////////
//	 DECLARATION	 //
//////////////////////
namespace mainFun {
	void train(cv::CommandLineParser parser);
	void type(cv::CommandLineParser parser);
	void camera(cv::CommandLineParser parser);
	void video(cv::CommandLineParser parser);
	void image(cv::CommandLineParser parser);
	void extract(cv::CommandLineParser parser);
	void createSample(cv::CommandLineParser parser);
	/**
	* Print the results on screen
	*
	* @param timer represent time of the duration of the algorithm
	*
	*/
	void printResults(clock_t timer);
}


///////////////////////
//					//
///////////////////////////////////////////////////////
//		 MAIN		 //
//////////////////////

/* 
 * @TODO command line parser
 * @TODO docs on trainfHog, hog, videostream, mediafile, utils,pipeline, fhog, cascadeClass
 * @TODO add choose to set all params
 * @TODO ROC curves
 * 
 * @TODO clear all global variables
 * @TODO calc confidence
 * @TODO clear bad samples from dataset
 * @TODO train on siluette samples
 * @TODO parse main body
 * @TODO train Dlib SVM from OpenCV HOG features
 * 
 * @TODO train cascade classificator
 * @TODO HAAR cascade classificator
 * @TODO LBP cascade classificator
 * @TODO ADA BOOST train
 * @TODO LBP train
 * @TODO HAAR train
 * @TODO replace convex hull with something more effiness
 * 
 * 
 * @TODO optimalize pipeline for all algorithms
 * @TODO refactor Utils class
 * @TODO implement cv::groupRectangles();
 * @TODO own implementation of detectMultiScale()

 */

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		std::cout << "\tType -help for help" << std::endl;
		return 0;
	}
	const cv::String keys =
		"{ help h ?           |        |  print help message                       }"
		"{ alg	              |    1   |  alg type                                 }"
		"{ video v            |        |  use video as input                       }"
		"{ image i            |        |  use list of images as input              }"
		"{ camera c           |        |  enable camera capturing                  }"
		"{ class svm          |    0   |  trained clasifier path                   }"
		"{ type  t            |        |  type of alg (train, test, video, picture }"
		"{ extract e          |        |  extract ROI from videostream             }"
		"{ vizualize          |    0   |  show result in window                    }"
		"{ createSample cs    |    0   |  creating samples from image              }"
		;
	
	cv::CommandLineParser parser(argc, argv, keys);
	parser.about("DIPLOMA THESIS - Pedestrian Detection v1.0.0");

	if (parser.has("help"))	{
		parser.printMessage();
		return 0;
	}
	if (parser.has("type"))	{
		mainFun::type(parser);
	}
	else if (parser.has("camera"))	{
		mainFun::camera(parser);
	}
	else if (parser.has("video"))	{
		mainFun::video(parser);
	}
	else if (parser.has("image"))	{
		mainFun::image(parser);
	}
	else if (parser.has("extract")) {
		mainFun::extract(parser);
	}
	else if (parser.has("createSample")) {
		mainFun::createSample(parser);
	}
	
	return 0;
}

void mainFun::type(cv::CommandLineParser parser)
{
	std::string type = parser.get<std::string>("type");
	if (!type.compare("test"))	{
		TestClass tc;
		tc.initTesting();
	}
	int chosenType;
	if (!type.compare("train"))	{

		std::cout << "\n1) openCV SVM train \n2) combined train (extract features by opencv HOG and train by dlib SVM) \n3) dlib SVM train \nType of train: ";
		std::cin >> chosenType;

		if (chosenType == 0 )	{  // @TODO train from mat and select own params?
			//TrainHog th;
			TrainHog th = TrainHog(450, 3, 0, 100, 1.e-06, 0, 3, 0.0005, 0, 0, 0.0001, "2111_79_98.4.yml");
			//TrainHog th = TrainHog(114, 3, 0, 100, 1.e-06, 0, 3, 0.1, 0.313903, 0.212467, 0.130589, "2111_79_98.4.yml");
			//th.trainFromMat("test.yml", "labels.txt");
			th.train(posSamples, negSamples, false);
		}
		else if (chosenType == 1 )	{
			CombinedTrainHog cth;
			cth.train(posSamples, negSamples);
		}
		else if(chosenType == 2)	{
			TrainFHog tfh;
			tfh.train(posSamples, negSamples);
		}
	}
}

void mainFun::camera(cv::CommandLineParser parser)
{
	Pipeline pl;
	std::cout << "camera" << std::endl;
	pl.execute(0);
}

void mainFun::image(cv::CommandLineParser parser)
{
	Pipeline pl;
	pl.executeImages(parser.get<std::string>("image"));
	std::cout << parser.get<std::string>("image") << std::endl;
	cv::waitKey(0);
}

void mainFun::video(cv::CommandLineParser parser)
{
	Pipeline pl;
	int typeAlg;
	clock_t timer;
	
	std::cout << "\nSelect detection algorithm: \n 1) Only HoG (openCV) \n 2) Mixtured HoG (openCV) \n 3) only FHoG (dlib) \n 4) mixtured FHoG (dlib) \n 5) cascade classificator \n 6) TEST MODE \n" << std::endl;
	std::cin >> typeAlg;
	
	if(typeAlg == 0 || (unsigned) typeAlg > 6)	{
		std::cout << "Bad selection.";
		return;
	}

	timer = clock();
	pl.execute(parser.get<std::string>("video"),typeAlg);
	timer = clock() - timer;
	printResults(timer);
	pl.evaluate("test.txt", "trained.txt");
	cv::waitKey(0);
}

void mainFun::extract(cv::CommandLineParser parser)
{
	std::cout << "extracting ROI" << std::endl;
	ExtractorROI eroi = ExtractorROI(2);
	eroi.extractROI(parser.get<std::string>("extract"));
}

void mainFun::createSample(cv::CommandLineParser parser)
{
	std::cout << "Creating samples from img" << std::endl;
	clock_t timer = clock();
	Utils::createSamplesFromImage(parser.get<std::string>("createSample"), "makedSamples");
	timer = clock() - timer;
	std::cout << "Parsing took " << static_cast<float>(timer) / CLOCKS_PER_SEC << "s." << std::endl;
}

void mainFun::printResults(clock_t timer)
{
	std::cout << "FPS: " << VideoStream::fps << "." << std::endl;
	std::cout << "Total frames: " << VideoStream::totalFrames << "." << std::endl;
	std::cout << "Video duration: " << VideoStream::totalFrames / static_cast<float>(VideoStream::fps) << "s."<< std::endl;
	std::cout << "Detection took " << static_cast<float>(timer) / CLOCKS_PER_SEC << "s." << std::endl;
	std::cout << "Possibly detection: " << Pipeline::allDetections << " frames." << std::endl;
}


///////////////////////
//		END			//
///////////////////////////////////////////////////////