#include "mog.h"
#include "../settings.h"
#include <opencv2/video/background_segm.hpp>

Mog::Mog()
{
   // pMOG1 = cv::bgsegm::createBackgroundSubtractorMOG(150,13,0.1,1); //MOG approach
    pMOG2 = cv::createBackgroundSubtractorMOG2(150,18,false); //MOG2 approach /*Settings::mogHistory, Settings::mogThreshold*/
} 

void Mog::processMat(cv::Mat &frame)
{
    pMOG2->apply(frame, frame);
   // pMOG1->apply(frame, frame);
}

