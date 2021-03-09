#include "util.h"
#include <fstream>

cv::Mat read_csvf(std::string inputFileName)
{
    std::ifstream inputFile(inputFileName);
    std::string currentLine;
    std::vector<std::vector<float>> allData;

    while (std::getline(inputFile, currentLine))
    {
        std::vector<float> vals;
        std::stringstream tmp(currentLine);
        std::string singleVal;
        while (std::getline(tmp, singleVal, ','))
        {
            vals.push_back(atof(singleVal.c_str()));
        }
        allData.push_back(vals);
    }

    cv::Mat mat((int)allData.size(), (int)allData[0].size(), CV_32FC1);
    for (int row = 0; row < (int)allData.size(); row++)
    {
        for (int col = 0; col < (int)allData[0].size(); col++)
        {
            mat.at<float>(row, col) = allData[row][col];
        }
    }

    return mat;
}
