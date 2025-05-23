#ifdef __unix__
int main() {}
#else
#include <windows.h>  // 添加Windows API头文件

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

// 全局变量
Mat                              canvas;
Point                            startPoint, endPoint;
bool                             isDrawing = false;
int                              drawMode  = 0;  // 0:矩形, 1:直线, 2:曲线
vector<pair<int, vector<Point>>> shapes;         // 存储所有绘制的形状
vector<Point>                    curvePoints;    // 存储当前曲线的点

// 鼠标回调函数
void mouseCallback(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN) {
        startPoint = Point(x, y);
        if (drawMode == 2) {  // 曲线模式
            curvePoints.clear();
            curvePoints.push_back(startPoint);
        }
        isDrawing = true;
    } else if (event == EVENT_MOUSEMOVE && isDrawing) {
        endPoint = Point(x, y);

        // 创建临时画布以显示实时绘制效果
        Mat temp = canvas.clone();

        if (drawMode == 0) {  // 矩形
            rectangle(temp, startPoint, endPoint, Scalar(0, 255, 0), 2);
        } else if (drawMode == 1) {  // 直线
            line(temp, startPoint, endPoint, Scalar(0, 0, 255), 2);
        } else if (drawMode == 2) {  // 曲线
            curvePoints.push_back(Point(x, y));
            for (size_t i = 1; i < curvePoints.size(); i++) {
                line(temp, curvePoints[i - 1], curvePoints[i], Scalar(255, 0, 0), 2);
            }
        }

        imshow("Drawing Pad", temp);
    } else if (event == EVENT_LBUTTONUP && isDrawing) {
        isDrawing = false;
        endPoint  = Point(x, y);

        if (drawMode == 0) {  // 矩形
            rectangle(canvas, startPoint, endPoint, Scalar(0, 255, 0), 2);
            vector<Point> rectPoints = {startPoint, endPoint};
            shapes.push_back(make_pair(0, rectPoints));
        } else if (drawMode == 1) {  // 直线
            line(canvas, startPoint, endPoint, Scalar(0, 0, 255), 2);
            vector<Point> linePoints = {startPoint, endPoint};
            shapes.push_back(make_pair(1, linePoints));
        } else if (drawMode == 2) {  // 曲线
            for (size_t i = 1; i < curvePoints.size(); i++) {
                line(canvas, curvePoints[i - 1], curvePoints[i], Scalar(255, 0, 0), 2);
            }
            shapes.push_back(make_pair(2, curvePoints));
        }

        imshow("Drawing Pad", canvas);
    }
}

// 创建动画效果的函数
void createAnimation() {
    // 准备一个新画布
    Mat animCanvas = Mat::zeros(canvas.size(), CV_8UC3);

    // 设置目标帧率为60fps
    int fps   = 60;
    int delay = 1000 / fps;  // 每帧延迟的毫秒数

    bool   running = true;
    double angle   = 0.0;
    double scale   = 1.0;
    Point  center(canvas.cols / 2, canvas.rows / 2);

    while (running) {
        animCanvas = Scalar(0, 0, 0);  // 清除画布

        // 计算旋转矩阵
        angle += 0.05;
        scale = 0.5 + 0.5 * sin(angle / 5.0) + 0.5;

        Mat rotMat = getRotationMatrix2D(center, angle * 180 / CV_PI, scale);

        // 对每个形状执行变换并绘制
        for (const auto& shape : shapes) {
            vector<Point> transformedPoints;

            // 对每个点执行变换
            for (const Point& pt : shape.second) {
                Mat ptMat = (Mat_<double>(3, 1) << pt.x, pt.y, 1.0);
                Mat dstPt = rotMat * ptMat;
                transformedPoints.push_back(Point(dstPt.at<double>(0, 0), dstPt.at<double>(1, 0)));
            }

            // 根据形状类型绘制
            if (shape.first == 0 && transformedPoints.size() >= 2) {  // 矩形
                rectangle(animCanvas, transformedPoints[0], transformedPoints[1], Scalar(0, 255, 0), 2);
            } else if (shape.first == 1 && transformedPoints.size() >= 2) {  // 直线
                line(animCanvas, transformedPoints[0], transformedPoints[1], Scalar(0, 0, 255), 2);
            } else if (shape.first == 2 && transformedPoints.size() >= 2) {  // 曲线
                for (size_t i = 1; i < transformedPoints.size(); i++) {
                    line(animCanvas, transformedPoints[i - 1], transformedPoints[i], Scalar(255, 0, 0), 2);
                }
            }
        }

        // 显示动画帧
        imshow("Animation", animCanvas);

        // 检查键盘输入，按ESC退出
        char key = waitKey(delay);
        if (key == 27) {  // ESC键
            running = false;
        }
    }

    destroyWindow("Animation");
}

int main() {
// 设置控制台编码为UTF-8
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    try {
        // 创建画布
        canvas = Mat::zeros(600, 800, CV_8UC3);

        // 创建窗口并设置鼠标回调
        namedWindow("Drawing Pad");
        setMouseCallback("Drawing Pad", mouseCallback);

        cout << "绘图模式: 0-矩形, 1-直线, 2-曲线" << endl;
        cout << "按[m]键切换绘图模式, [c]键清除画布, [a]键播放动画, [ESC]键退出" << endl;

        while (true) {
            imshow("Drawing Pad", canvas);
            char key = waitKey(20);  // 等待键盘输入

            if (key == 27) {  // ESC键
                break;
            } else if (key == -1) {
                // 检查窗口是否关闭
                if (!getWindowProperty("Drawing Pad", WND_PROP_VISIBLE)) {
                    break;
                }
            } else if (key == 'm' || key == 'M') {  // 切换绘图模式
                drawMode = (drawMode + 1) % 3;
                string modeName;
                switch (drawMode) {
                    case 0: modeName = "矩形"; break;
                    case 1: modeName = "直线"; break;
                    case 2: modeName = "曲线"; break;
                }
                cout << "当前绘图模式: " << modeName << endl;
            } else if (key == 'c' || key == 'C') {  // 清除画布
                canvas = Mat::zeros(canvas.size(), CV_8UC3);
                shapes.clear();
            } else if (key == 'a' || key == 'A') {  // 播放动画
                if (!shapes.empty()) {
                    createAnimation();
                } else {
                    cout << "请先绘制一些图形再播放动画" << endl;
                }
            }
        }
    } catch (const cv::Exception& e) {
        cerr << "OpenCV错误: " << e.what() << endl;
        return -1;
    } catch (const std::exception& e) {
        cerr << "标准错误: " << e.what() << endl;
        return -1;
    } catch (...) {
        cerr << "未知错误" << endl;
        return -1;
    }

    return 0;
}
#endif