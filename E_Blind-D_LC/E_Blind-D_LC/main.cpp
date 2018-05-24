#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <io.h>
#include <time.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

using namespace std;
using namespace cv;

int alpha = 1;
int m;

void getAllFiles(string path, vector<string>& pic)
{
	long long File = 0;
	struct _finddata_t fileinfo;
	string p = path;
	if ((File = _findfirst((p + "\\*").c_str(), &fileinfo)) != -1L)
	{
		do
		{
			if (strstr(fileinfo.name, ".bmp") != NULL
				|| strstr(fileinfo.name, ".BMP") != NULL
				|| strstr(fileinfo.name, ".dib") != NULL
				|| strstr(fileinfo.name, ".jpeg") != NULL
				|| strstr(fileinfo.name, ".jpg") != NULL
				|| strstr(fileinfo.name, ".jpe") != NULL
				|| strstr(fileinfo.name, ".jp2") != NULL
				|| strstr(fileinfo.name, ".png") != NULL
				|| strstr(fileinfo.name, ".pbm") != NULL
				|| strstr(fileinfo.name, ".pgm") != NULL
				|| strstr(fileinfo.name, ".ppm") != NULL
				|| strstr(fileinfo.name, ".sr") != NULL
				|| strstr(fileinfo.name, ".ras") != NULL
				|| strstr(fileinfo.name, ".tiff") != NULL
				|| strstr(fileinfo.name, ".tif") != NULL)		//寻找符合格式的图片文件
				pic.push_back(p + "\\" + fileinfo.name);
		} while (_findnext(File, &fileinfo) == 0);
		_findclose(File);
	}
}

double D_LC(Mat a, Mat b)
{
	double ans = 0;
	for (int i = 0; i < a.rows; i++)
	{
		for (int j = 0; j < a.cols; j++)
		{
			ans += a.at<uchar>(i, j) * b.at<double>(i, j);
		}
	}
	ans /= a.rows * a.cols;
	return ans;
}

int reg(int a)
{
	return a > 255 ? 255 : (a < 0 ? 0 : a);
}

Mat E_Blind(Mat src,Mat watermark)
{
	int height = min(src.rows, watermark.rows);
	int width = min(src.cols, watermark.cols);
	Mat ans(src.rows,src.cols,CV_8UC1);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			ans.at<uchar>(i, j) = reg(src.at<uchar>(i, j) + alpha * (2 * m - 1) * watermark.at<double>(i, j));
		}
	}
	return ans;
}

int main()
{
	string path;
	vector <string> pic;
	vector <Mat> watermark;
	vector <Mat> pictures;

	while (pic.size() == 0)
	{
		cout << "请输入图片所在路径：\n";
		cin >> path;
		getAllFiles(path, pic);
	}
	cout << pic.size() << endl;
	for (int i = 0; i < pic.size(); i++)
		cout << pic[i] << endl;

	for (int i = 0; i < pic.size(); i++)
	{
		Mat p = imread(pic[i]);
		pictures.push_back(p);
	}

	int width, height;
	for (int i = 0; i < pictures.size(); i++)
	{
		width = max(width, pictures[i].cols);
		height = max(height, pictures[i].rows);
	}

	//随机生成40张水印
	for (int k = 0; k < 40; k++)
	{
		Mat_<double>w(height, width);
		randn(w, 0, 1);
		watermark.push_back(w);
	}

	Mat dst;

	//part1:1张水印和所有图片
	fstream out("part1.m",ios::out);
	
	out << "x1 = [";
	for (int i = pictures.size() - 1; i >= 0; i--)
	{
		out << D_LC(pictures[i], watermark[0]);
		if (i > 0)
			out << ", ";
		else
			out << "];\n";
	}
	out << "x1 = sort(x1);\n";
	out << "[mu1,sigma1] = normfit(x1);\n";
	out << "y1 = normpdf(x1,mu1,sigma1);\n";
	
	out << "x2 = [";
	m = 1;
	for (int i = pictures.size() - 1; i >= 0; i--)
	{
		dst = E_Blind(pictures[i], watermark[0]);
		out << D_LC(dst, watermark[0]);
		if (i > 0)
			out << ", ";
		else
			out << "];\n";
	}
	out << "x2 = sort(x2);\n";
	out << "[mu2,sigma2] = normfit(x2);\n";
	out << "y2 = normpdf(x2,mu2,sigma2);\n";

	out << "x3 = [";
	m = 0;
	for (int i = pictures.size() - 1; i >= 0; i--)
	{
		dst = E_Blind(pictures[i], watermark[0]);
		out << D_LC(dst, watermark[0]);
		if (i > 0)
			out << ", ";
		else
			out << "];\n";
	}
	out << "x3 = sort(x3);\n";
	out << "[mu3,sigma3] = normfit(x3);\n";
	out << "y3 = normpdf(x3,mu3,sigma3);\n";
	out << "plot(x1, y1, 'r', x2, y2, 'g', x3, y3, 'b');\n";
	out.close();

	//part2:40张水印和1张图片,选用第25张:lena512.bmp
	out.open("part2.m", ios::out);

	out << "x1 = [";
	for (int i = 0; i < 40; i++)
	{
		out << D_LC(pictures[25], watermark[i]);
		if (i < 39)
			out << ", ";
		else
			out << "];\n";
	}
	out << "x1 = sort(x1);\n";
	out << "[mu1,sigma1] = normfit(x1);\n";
	out << "y1 = normpdf(x1,mu1,sigma1);\n";

	out << "x2 = [";
	m = 1;
	for (int i = 0; i < 40; i++)
	{
		dst = E_Blind(pictures[25], watermark[i]);
		out << D_LC(dst, watermark[i]);
		if (i < 39)
			out << ", ";
		else
			out << "];\n";
	}
	out << "x2 = sort(x2);\n";
	out << "[mu2,sigma2] = normfit(x2);\n";
	out << "y2 = normpdf(x2,mu2,sigma2);\n";

	out << "x3 = [";
	m = 0;
	for (int i = 0; i < 40; i++)
	{
		dst = E_Blind(pictures[25], watermark[i]);
		out << D_LC(dst, watermark[i]);
		if (i < 39)
			out << ", ";
		else
			out << "];\n";
	}
	out << "x3 = sort(x3);\n";
	out << "[mu3,sigma3] = normfit(x3);\n";
	out << "y3 = normpdf(x3,mu3,sigma3);\n";
	out << "plot(x1, y1, 'r', x2, y2, 'g', x3, y3, 'b');\n";
	out.close();

	//part3:40张水印和1张图片,选用第32张:rec.bmp
	out.open("part3.m", ios::out);

	out << "x1 = [";
	for (int i = 0; i < 40; i++)
	{
		out << D_LC(pictures[32], watermark[i]);
		if (i < 39)
			out << ", ";
		else
			out << "];\n";
	}
	out << "x1 = sort(x1);\n";
	out << "[mu1,sigma1] = normfit(x1);\n";
	out << "y1 = normpdf(x1,mu1,sigma1);\n";

	out << "x2 = [";
	m = 1;
	for (int i = 0; i < 40; i++)
	{
		dst = E_Blind(pictures[32], watermark[i]);
		out << D_LC(dst, watermark[i]);
		if (i < 39)
			out << ", ";
		else
			out << "];\n";
	}
	out << "x2 = sort(x2);\n";
	out << "[mu2,sigma2] = normfit(x2);\n";
	out << "y2 = normpdf(x2,mu2,sigma2);\n";

	out << "x3 = [";
	m = 0;
	for (int i = 0; i < 40; i++)
	{
		dst = E_Blind(pictures[32], watermark[i]);
		out << D_LC(dst, watermark[i]);
		if (i < 39)
			out << ", ";
		else
			out << "];\n";
	}
	out << "x3 = sort(x3);\n";
	out << "[mu3,sigma3] = normfit(x3);\n";
	out << "y3 = normpdf(x3,mu3,sigma3);\n";
	out << "plot(x1, y1, 'r', x2, y2, 'g', x3, y3, 'b');\n";
	out.close();
}