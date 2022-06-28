#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "opencv2/opencv.hpp"
#include "opencv2/videoio/videoio_c.h"
#include <cstdint>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <vector>
#include <thread>
// #include <chrono>

namespace py = pybind11;

void slice_to_array(char* path, uint8_t** frames, size_t from, size_t to) {
  cv::VideoCapture cap(path);
  int frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
  int frameWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
  int frameHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
  cap.set(CV_CAP_PROP_POS_FRAMES, from);
  size_t to1 = to;
  if (to > frameCount) {
    to1 = frameCount;
  }
  cv::Mat frame;
  for (size_t i = from; i < to1; i++)
  {
    frames[i] = new uint8_t[frameHeight*frameWidth*3];
    bool ret = cap.read(frame);
    frame.copyTo(cv::Mat(frameHeight, frameWidth, CV_8UC3, frames[i]));
    if(!ret) {
      std::cerr << "frame " << i << " missing\n";
    }
    // std::cout << i << " image\n";
  }

  if (from < frameCount) {
    from = frameCount;
  }
  for (size_t i = from; i < to; i++)
  {
    frames[i] = new uint8_t[frameHeight*frameWidth*3]();
    // std::cout << i << " blank\n";
  }
  cap.release();
  
}

std::vector<py::array_t<uint8_t>> cap_to_array(char *path, int target, int nbthreads) {
  using std::chrono::high_resolution_clock;
  using std::chrono::duration_cast;
  using std::chrono::duration;
  using std::chrono::milliseconds;
  
  auto t1 = high_resolution_clock::now();

  uint8_t** frames = new uint8_t *[target];

  size_t work_length = target / nbthreads;
  size_t remaining_work = target % nbthreads;

  size_t a = 0;
  size_t b = work_length;

  std::vector<std::thread> threads;

  for (size_t i = 0; i < nbthreads - 1; i++)
  {
    threads.push_back(std::thread(slice_to_array, path, frames, a, b));
    a += work_length;
    b += work_length;
  }
  b += remaining_work;
  threads.push_back(std::thread(slice_to_array, path, frames, a, b));
  
  for (auto& th : threads) {
    th.join();
  }

  cv::VideoCapture cap(path);
  int frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
  int frameWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
  int frameHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

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
