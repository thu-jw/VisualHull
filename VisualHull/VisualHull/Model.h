#pragma once
#pragma warning(disable:4244)

#include <vector>
#include <algorithm>
#include <map>
#include <opencv2/opencv.hpp>
#include <Eigen/Eigen>
#include <limits>
#include <queue>
#include <list>
#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace std;

//class Point
//{
//public:
//	Point(int indexX = 0, int indexY = 0, int indexZ = 0):x(indexX), y(indexY), z(indexZ){}
//	const Point& operator+(const Point &p)
//	{ return Point(x+p.x, y+p.y, z+p.z); }
//	friend bool operator<(const Point &p, const Point & q)
//	{
//		return p.x < q.x || p.y < q.y || p.z < q.z;
//	}
//	bool operator==(const Point &p)
//	{
//		return x == p.x && y == p.y &&z == p.z;
//	}
//	void show()
//	{
//		cout << x << " " << y << " " << z << endl;
//	}
//	int x, y, z;
//};
typedef cv::Vec3i Point;

// �����ж�ͶӰ�Ƿ���visual hull�ڲ�
struct Projection
{
	Eigen::Matrix<float, 3, 4> m_projMat;
	cv::Mat m_image;
	cv::Mat m_image_color;
	const uint m_threshold = 125;

	bool outOfRange(int x, int max)
	{
		return x < 0 || x >= max;
	}

	bool checkRange(double x, double y, double z)
	{
		Eigen::Vector3f vec3 = m_projMat * Eigen::Vector4f(x, y, z, 1);
		int indX = vec3[1] / vec3[2];
		int indY = vec3[0] / vec3[2];

		if (outOfRange(indX, m_image.size().height) || outOfRange(indY, m_image.size().width))
			return false;
		//�ж��Ƿ�Ϊ��ɫ�㣬��ɫ��ʾ��VisualHull���ڲ�
		return m_image.at<uchar>((uint)(vec3[1] / vec3[2]), (uint)(vec3[0] / vec3[2])) > m_threshold;
	}

	cv::Vec3b getColor(double x, double y, double z)
	{
		cv::Vec3b ans;
		Eigen::Vector3f vec3 = m_projMat * Eigen::Vector4f(x, y, z, 1);
		int indX = vec3[1] / vec3[2];
		int indY = vec3[0] / vec3[2];

		return m_image_color.at<cv::Vec3b>((uint)(vec3[1] / vec3[2]), (uint)(vec3[0] / vec3[2]));
	}
};

// ����index��ʵ������֮���ת��
struct CoordinateInfo
{
	int m_resolution;
	double m_min;
	double m_max;

	double index2coor(int index)
	{
		return m_min + index * (m_max - m_min) / m_resolution;
	}

	CoordinateInfo(int resolution = 10, double min = 0.0, double max = 100.0)
		: m_resolution(resolution)
		, m_min(min)
		, m_max(max)
	{
	}
};

class Model
{
public:
	typedef std::vector<std::vector<bool>> Pixel;
	typedef std::vector<Pixel> Voxel;

	Model(int resX = 100, int resY = 100, int resZ = 100);
	~Model();

	void saveModel(const char* pFileName);
	void saveModelWithNormal(const char* pFileName);
	void savePly(const char * pFileName);

	void loadMatrix(const char* pFileName);
	void loadImage(const char* pDir1,const char* pPrefix1, const char* pSuffix1, const char* pDir2, const char* pPrefix2, const char* pSuffix2);
	void getModel();
	void getSurface();
	void getColor();

	Eigen::Vector3f getNormal(int indX, int indY, int indZ);


private:
	//ƫ��                 ǰ���������� 0-5			�˸��ǿ�6-13				12�����14-25								
	const int dx[26] = { -1, 0, 0, 0, 0, 1,   1, 1, 1, 1,-1,-1,-1,-1,    1, 1,-1,-1, 0, 0, 0, 0, 1, 1,-1,-1 };
	const int dy[26] = { 0, 1, -1, 0, 0, 0,   1, 1,-1,-1,-1,-1, 1, 1,    0, 0, 0, 0, 1, 1, 1, 1, 1,-1,-1, 1 };
	const int dz[26] = { 0, 0, 0, 1, -1, 0,   1,-1, 1,-1, 1,-1, 1,-1,    1,-1, 1,-1, 1,-1, 1,-1, 0, 0, 0, 0 };
	Point dp[26];


	bool outOfRange(int indexX, int indexY, int indexZ);
	bool outOfRange(const Point &p) 
	{ return outOfRange(p[0], p[1], p[2]); }
	bool insideHull(int indexX, int indexY, int indexZ);
	bool insideHull(const Point &p) 
	{ return insideHull(p[0], p[1], p[2]); }
	void getColor(const Point &p);

	bool totalInside(const Point &p);


	void BFS(Point p);

	bool voxel(const Point &p) 
	{ return m_voxel[p[0]][p[1]][p[2]]; }
	bool surface(const Point &p) 
	{ return m_surface[p[0]][p[1]][p[2]]; }
	bool visited(const Point &p) 
	{ return m_visited[p[0]][p[1]][p[2]]; }
	bool enqueued(const Point &p) 
	{ return m_enqueued[p[0]][p[1]][p[2]]; }


	void setVoxel(const Point &p, bool v = true) 
	{ m_voxel[p[0]][p[1]][p[2]] = v; }
	void setSurface(const Point &p, bool v = true) 
	{ m_surface[p[0]][p[1]][p[2]] = v; }
	void setVisited(const Point &p, bool v = true) 
	{ m_visited[p[0]][p[1]][p[2]] = v; }
	void setEnqueued(const Point &p, bool v = true) 
	{ m_enqueued[p[0]][p[1]][p[2]] = v; }

	CoordinateInfo m_corrX;
	CoordinateInfo m_corrY;
	CoordinateInfo m_corrZ;

	int m_neiborSize;

	std::vector<Projection> m_projectionList;
	vector<Point> surfacePoints;
	vector<cv::Vec3b> m_colorList;
	vector<Eigen::Vector3f> m_normal;

	Voxel m_voxel;
	Voxel m_surface;
	Voxel m_visited;//�Ƿ���ʹ�
	Voxel m_enqueued;//�Ƿ���ӹ�
};

