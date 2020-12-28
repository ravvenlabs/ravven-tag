#include "TagDisplay.h"

void showStep5(TagDetector::Step_5 step5, int rows, int cols)
{
    cv::Mat sizes(rows, cols, CV_16U);
    cv::Mat to_print(rows, cols, CV_32FC1);
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            *(sizes.ptr<unsigned>(y, x)) = step5.uf.getSetSize(y * cols + x);
        }
    }
    cv::normalize(sizes, to_print, 0, 1, cv::NORM_MINMAX, CV_32FC1);
    imshow("Step 5", to_print);
}

void showStep6(TagDetector::Step_6 step6, int rows, int cols)
{
    cv::Mat to_print(rows, cols, CV_8UC3, cv::Vec3b(0, 0, 0));
    for (std::map<int, std::vector<AprilTag::XYWeight>>::iterator it = step6.clusters.begin(); it != step6.clusters.end(); it++)
    {
        cv::Vec3b color = cv::Vec3b((rand() % 190) + 50, (rand() % 190) + 50, (rand() % 190) + 50);
        for (std::vector<AprilTag::XYWeight>::iterator point = it->second.begin(); point != it->second.end(); point++)
        {
            *(to_print.ptr<cv::Vec3b>(point->y, point->x)) = color;
        }
    }
    imshow("Step 6", to_print);
}

void showStep7(TagDetector::Step_7 step7, TagDetector::Step_1 step1)
{
    cv::Mat baseImage(step1.fimOrig.rows, step1.fimOrig.cols, CV_8UC3);
    cv::cvtColor(step1.fimOrig, baseImage, cv::COLOR_GRAY2BGR, 3);
    for (std::vector<AprilTag::Segment>::iterator segment = step7.segments.begin(); segment != step7.segments.end(); segment++)
    {
        cv::Scalar color((rand() % 255) / 255., (rand() % 255) / 255., (rand() % 255) / 255.);
        cv::Point2d startPoint(segment->getX0(), segment->getY0());
        cv::Point2d endPoint(segment->getX1(), segment->getY1());
        cv::line(baseImage, startPoint, endPoint, color, 3);
    }
    imshow("Step 7", baseImage);
}

void showStep8(TagDetector::Step_7 step7, TagDetector::Step_1 step1)
{
    cv::Mat baseImage(step1.fimOrig.rows, step1.fimOrig.cols, CV_8UC3);
    cv::cvtColor(step1.fimOrig, baseImage, cv::COLOR_GRAY2BGR, 3);
    std::vector<int> segmentIdList(step7.segments.size());
    for (int i = 0; i < step7.segments.size(); i++)
        segmentIdList.push_back(i);
    int currSegId;
    int clusterCount = 0;
    while (segmentIdList.size() > 0)
    {
        cv::Scalar color((rand() % 255) / 255., (rand() % 255) / 255., (rand() % 255) / 255.);

        currSegId = segmentIdList.back();
        segmentIdList.pop_back();
        AprilTag::Segment segment = step7.segments[currSegId];
        for (std::vector<AprilTag::Segment*>::iterator segmentChild = segment.children.begin(); segmentChild != segment.children.end(); segmentChild++)
        {
            segmentIdList.erase(std::remove(segmentIdList.begin(), segmentIdList.end(), (*segmentChild)->getId()), segmentIdList.end());
            cv::Point2d startPoint((*segmentChild)->getX0(), (*segmentChild)->getY0());
            cv::Point2d endPoint((*segmentChild)->getX1(), (*segmentChild)->getY1());
            cv::line(baseImage, startPoint, endPoint, color, 3);
        }
        cv::Point2d startPoint(segment.getX0(), segment.getY0());
        cv::Point2d endPoint(segment.getX1(), segment.getY1());
        cv::line(baseImage, startPoint, endPoint, color, 3);
    }
    imshow("Step 8", baseImage);
}

void showStep9(TagDetector::Step_9 step9, TagDetector::Step_1 step1)
{
    cv::Mat baseImage(step1.fimOrig.rows, step1.fimOrig.cols, CV_8UC3);
    cv::cvtColor(step1.fimOrig, baseImage, cv::COLOR_GRAY2BGR, 3);
    for (std::vector<AprilTag::Quad>::iterator quad = step9.quads.begin(); quad != step9.quads.end(); quad++)
    {
        cv::Scalar color((rand() % 255) / 255., (rand() % 255) / 255., (rand() % 255) / 255.);
        cv::Rect2d rect(
            cv::Point2d(quad->quadPoints[0].first, quad->quadPoints[0].second),
            cv::Point2d(quad->quadPoints[2].first, quad->quadPoints[2].second)
        );
        cv::rectangle(baseImage, rect, color, 3);
    }
    imshow("Step 9", baseImage);
}

void showStep10(TagDetector::Step_10 step10, TagDetector::Step_1 step1)
{
    cv::Mat baseImage(step1.fimOrig.rows, step1.fimOrig.cols, CV_8UC3);
    cv::cvtColor(step1.fimOrig, baseImage, cv::COLOR_GRAY2BGR, 3);
    for (std::vector<AprilTag::TagDetection>::iterator detect = step10.detections.begin(); detect != step10.detections.end(); detect++)
    {
        detect->draw(baseImage);
    }
    imshow("Step 10", baseImage);
}

void showStep11(TagDetector::Step_11 step11, TagDetector::Step_1 step1)
{
    cv::Mat baseImage(step1.fimOrig.rows, step1.fimOrig.cols, CV_8UC3);
    cv::cvtColor(step1.fimOrig, baseImage, cv::COLOR_GRAY2BGR, 3);
    for (std::vector<AprilTag::TagDetection>::iterator detect = step11.goodDetections.begin(); detect != step11.goodDetections.end(); detect++)
    {
        detect->draw(baseImage);
    }
    imshow("Step 11", baseImage);
}
