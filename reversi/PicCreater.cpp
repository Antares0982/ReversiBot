//
// Created by antares on 4/26/22.
//

#include "PicCreater.h"
#include <opencv2/opencv.hpp>

void PicCreater::save(const std::string &_path) {
    cv::Mat image(height, width, CV_8UC3, pic_char.get());
    cv::imwrite(_path, image);
}
