#include<iostream>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

double EuDis(Point pt1, Point pt2)
{
	return sqrt((pt2.x - pt1.x) * (pt2.x - pt1.x) + (pt2.y - pt1.y) * (pt2.y - pt1.y));
}

int main()
{

	Mat src = imread("test.png");
	if (src.empty())
	{
		cout << "No Image!" << endl;
		system("pause");
		return -1;
	}

	Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	//imshow("gray", gray);

	Mat gaussian;
	GaussianBlur(gray, gaussian, Size(3, 3), 0);
	//imshow("gaussian", gaussian);

	Mat thresh;
	threshold(gaussian, thresh, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
	//imshow("thresh", thresh);


	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat open;
	morphologyEx(thresh, open, MORPH_OPEN, kernel);
	//imshow("open", open);

	Mat kernel1 = getStructuringElement(MORPH_RECT, Size(7, 7));
	Mat close;
	morphologyEx(open, close, MORPH_CLOSE, kernel1);
	//imshow("close", close);
	
	//��ȡ��������
	vector<vector<Point>>contours;
	findContours(close, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	// ��ȡ�����ĸ��ǵ�
	vector<vector<Point>>conPoly(contours.size());
	vector<Point>srcPts;

	for (int i = 0; i < contours.size(); i++)
	{
		double area = contourArea(contours[i]);

		if (area > 10000)
		{
			double peri = arcLength(contours[i], true);

			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
			// ����һ���յ�ͼ����Ϊ���ƶ���εĻ���
			Mat drawing = Mat::zeros(close.size(), CV_8UC3);

			// ���ƶ����
			polylines(drawing, conPoly[i], true, Scalar(0, 255, 0), 2); // ���ƶ���Σ���ɫΪ��ɫ���߿�Ϊ2

			// ��ʾ���ƽ��
			imshow("Approximated Polygon", drawing);

			srcPts = { conPoly[i][0],conPoly[i][1],conPoly[i][2],conPoly[i][3] };

		}

	}

	int width = src.cols / 2;
	int height = src.rows / 2;
	int T_L, T_R, B_R, B_L;

	for (int i = 0; i < srcPts.size(); i++)
	{
		if (srcPts[i].x < width && srcPts[i].y < height)
		{
			T_L = i;
		}
		if (srcPts[i].x > width && srcPts[i].y < height)
		{
			T_R = i;
		}
		if (srcPts[i].x > width && srcPts[i].y > height)
		{
			B_R = i;
		}
		if (srcPts[i].x < width && srcPts[i].y > height)
		{
			B_L = i;
		}

	}

	//circle(src, srcPts[T_L], 10, Scalar(0, 0, 255), -1);
	//circle(src, srcPts[T_R], 10, Scalar(0, 255, 255), -1);
	//circle(src, srcPts[B_R], 10, Scalar(255, 0, 0), -1);
	//circle(src, srcPts[B_L], 10, Scalar(0, 255, 0), -1);

	/*
	�任��ͼ��ĳ��Ϳ�Ӧ�ñ�Ϊ��
	�� = max���任ǰ��߳����任ǰ�ұ߳���
	�� = max���任ǰ�ϱ߳����任ǰ�±߳���
	��任��ͼ������Ͻ�λ��Ϊԭ��λ�á�
	*/

	double LeftHeight = EuDis(srcPts[T_L], srcPts[B_L]);
	double RightHeight = EuDis(srcPts[T_R], srcPts[B_R]);
	double MaxHeight = max(LeftHeight, RightHeight);

	double UpWidth = EuDis(srcPts[T_L], srcPts[T_R]);
	double DownWidth = EuDis(srcPts[B_L], srcPts[B_R]);
	double MaxWidth = max(UpWidth, DownWidth);

	Point2f SrcAffinePts[4] = { Point2f(srcPts[T_L]),Point2f(srcPts[T_R]) ,Point2f(srcPts[B_R]) ,Point2f(srcPts[B_L]) };
	Point2f DstAffinePts[4] = { Point2f(0,0),Point2f(MaxWidth,0),Point2f(MaxWidth,MaxHeight),Point2f(0,MaxHeight) };

	Mat M = getPerspectiveTransform(SrcAffinePts, DstAffinePts);

	Mat DstImg;
	warpPerspective(src, DstImg, M, Point(MaxWidth, MaxHeight));
	imshow("Dst", DstImg);


	imshow("src", src);
	waitKey(0);
	destroyAllWindows();

	system("pause");
	return 0;
}
