#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "opencv2/opencv.hpp"
#include "opencv2/videoio/videoio_c.h"
#include <cstdint>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace py = pybind11;

std::vector<py::array_t<uint8_t>> cap_to_array(char *path, int target) {

  cv::VideoCapture cap(path);
  int frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
  int frameWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
  int frameHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

  uint8_t** frames = new uint8_t *[target];
  for (size_t i = 0; i < frameCount; i++)
  {
    cv::Mat frame;
    bool ret = cap.read(frame);
    frames[i] = frame.data;
    frame.addref();
  }
  cv::Mat black_image(frameHeight, frameWidth, CV_8UC3, cv::Scalar(0, 0, 0));
  for (size_t i = frameCount; i < target; i++)
  {
    frames[i] = black_image.data;
  }
  black_image.addref();

    /*for(size_t i = 10; i < 300; i++) {
      cv::Mat d(1080, 1920, CV_8UC3, frames[i]);
      std::cout << i << "\n";
      cv::imshow("ok i pull up", d);
      cv::waitKey(0);
    }*/
  std::cout << "frames populated\n";
  std::vector<py::array_t<uint8_t>> arr;
  for (size_t i = 0; i < target; i++)
  {
    //std::cout << i << std::endl;
    //std::cout << sizeof(std::vector<int>) + (sizeof(int) * arr.size()) << std::endl;
    arr.push_back(py::array(py::buffer_info(
            frames[i],
            sizeof(uint8_t),
            py::format_descriptor<uint8_t>::format(),
            3,
            { frameHeight, frameWidth, 3 },
            {
              sizeof(uint8_t) * 3 * frameWidth,
              sizeof(uint8_t) * 3,
              sizeof(uint8_t)
            }
          )));
  }
  std::cout << "arr assigned\n";
  return arr;
}
PYBIND11_MODULE(captoarray, m)
{
  m.def("cap_to_array", &cap_to_array, py::return_value_policy::reference);
}
