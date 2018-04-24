// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include <stdio.h>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>
#include <functional>
#include "net.h"
#include <time.h>
//#include "image_processing/frontal_face_detector.h"
//#include "image_io.h"

#define VIDEO_MODEL 0

using namespace std;
using namespace cv;

ncnn::Net squeezenet;
float mean_vals[3] = { 128.f, 158.f, 158.f };

int main(int argc, char** argv)
{
	//��ʼ��ģ�ͣ��Լ������ǩ
	squeezenet.load_param("../Landmarks/landmark.params");
	squeezenet.load_model("../Landmarks/landmark.bin");

	//�������ͼƬ
	const char* imagepath = "model/3.jpg";
	cv::Mat img = cv::imread(imagepath, CV_LOAD_IMAGE_COLOR);

	cv::Mat img3;
	cvtColor(img, img3, CV_RGB2GRAY);

	cv::Mat img2;
	img.convertTo(img2, CV_32FC1);

	cv::Mat tmp_m, tmp_sd;
	float m = 0, sd = 0;
	cv::meanStdDev(img2, tmp_m, tmp_sd);
	m = tmp_m.at<double>(0, 0);
	sd = tmp_sd.at<double>(0, 0);


	ncnn::Mat in = ncnn::Mat::from_pixels_resize(img3.data, ncnn::Mat::PIXEL_GRAY, img3.cols, img3.rows, 60, 60);
	mean_vals[0] = m;
	
	in.substract_mean_normalize(mean_vals, 0);
	for (int i = 0; i < in.h; i++)
	{
		float* p = in.row(i);
		for (int j = 0; j < in.w; j++)
		{
			*p = (*p) / (0.000001 + sd);
			p++;
		}
	}
	
	ncnn::Extractor ex = squeezenet.create_extractor();
	ex.set_light_mode(true);

	ex.input("data", in);
	ncnn::Mat out;

	clock_t start, finish;
	start = clock();
	int a = ex.extract("Dense3", out);
	finish = clock();
	double totaltime;
	totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("run time: %f\n", totaltime);

	std::vector<float> feat;

	for (int i = 0; i < out.w; i++)
	{
		const float* prob = static_cast<float*>(out.data) + i;
		feat.push_back(prob[0]);

	}
	for (int i = 0; i < out.w / 2; i++)
	{
		Point x = Point(int(feat[2 * i] * 278), int(feat[2 * i + 1] * 289));//�����278��289��ͼ��ķֱ��ʣ���͵����
		cv::circle(img, x, 0.1, Scalar(0, 0, 255), 4, 8, 0);
	}


	imshow("m", img);
	imwrite("result.jpg", img);
	waitKey(0);


	return 0;
}
