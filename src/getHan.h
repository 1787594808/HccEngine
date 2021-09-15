#include <tensorflow/c/c_api.h>
#include "tf_utils.hpp"
#include "scope_guard.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <memory>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
using namespace std;
using namespace cv;
using namespace rapidjson;
//using namespace tensorflow;
//using namespace ::tensorflow::ops;
vector<int> gethan(Mat image);
vector<int> getEnglish(Mat image);

TF_Session* getSession();
TF_Session* getSessionE();

Document &getDocument();
TF_Graph* getGraph();
TF_Graph* getGraphE();
