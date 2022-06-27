#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "opencv2/opencv.hpp"
#include "opencv2/videoio/videoio_c.h"
#include <cstdint>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <vector>
// #include <chrono>

namespace py = pybind11;

std::vector<py::array_t<uint8_t>> cap_to_array(char *path, int target) {
  // using std::chrono::high_resolution_clock;
  // using std::chrono::duration_cast;
  // using std::chrono::duration;
  // using std::chrono::milliseconds;
  
  // auto t1 = high_resolution_clock::now();

  cv::VideoCapture cap(path);
  int frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
  int frameWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
  int frameHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

  uint8_t** frames = new uint8_t *[target];
  for (size_t i = 0; i < frameCount; i++)
  {
    frames[i] = new uint8_t[frameHeight*frameWidth*3];
    bool ret = cap.read(cv::Mat(frameHeight, frameWidth, CV_8UC3, frames[i]));
    if(!ret) {
      std::cerr << "frame " << i << " missing\n";
    }
  }
  cv::Mat black_image(frameHeight, frameWidth, CV_8UC3, cv::Scalar(0, 0, 0));
  for (size_t i = frameCount; i < target; i++)
  {
    frames[i] = new uint8_t[frameHeight*frameWidth*3];
    memcpy(frames[i], black_image.data, frameHeight*frameWidth*3);
  }
  cap.release();

  // std::cout << "frames populated\n";
  // auto t2 = high_resolution_clock::now();
  // std::cout << duration_cast<milliseconds>(t2 - t1).count() << "ms\n";


  std::vector<py::array_t<uint8_t>> arr;

  for (size_t i = 0; i < target; i++)
  {
    arr.push_back(py::array_t<uint8_t>(
                            { frameHeight, frameWidth, 3 },
                            {
                              sizeof(uint8_t) * 3 * frameWidth,
                              sizeof(uint8_t) * 3,
                              sizeof(uint8_t)
                            },
                            frames[i],
                            py::capsule(frames[i], [](void *f) { free(f); }
                            )));
  }

  // std::cout << "arr assigned\n";
  // auto t3 = high_resolution_clock::now();
  // std::cout << duration_cast<milliseconds>(t3 - t2).count() << "ms\n";
  return arr;
}
PYBIND11_MODULE(captoarray, m)
{
  m.def("cap_to_array", &cap_to_array, py::return_value_policy::take_ownership);
}
