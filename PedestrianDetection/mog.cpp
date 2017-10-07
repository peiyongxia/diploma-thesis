#include "mog.h"

Mog::Mog()
{
    //pMOG = cv::bgsegm::createBackgroundSubtractorMOG(); //MOG approach
    pMOG2 = cv::createBackgroundSubtractorMOG2(); //MOG2 approach
}

cv::Mat Mog::processMat(cv::Mat &frame)
{
    cv::Mat fgMaskMOG; //fg mask generated by MOG method
    if (frame.empty()) {
        //QErrorMessage().showMessage("Unable to read next frame.");
        exit(EXIT_FAILURE);
    }
    pMOG2->apply(frame, fgMaskMOG);
    //pMOG1->apply(frame, fgMaskMOG);
    return fgMaskMOG;
}

