#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

void capFrame() {
    const std::string winName("Loading");
    auto              cap = cv::VideoCapture(1);

    if (!cap.isOpened()) {
        std::cerr << "Cannot open capture using index 0\n";
        return;
    }

    cv::Mat frame;

    for (;;) {
        cap.read(frame);
        cv::imshow(winName, frame);
        if (cv::waitKey(5) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyWindow(winName);
}

void drawBlocks() {
    int               width  = 400;
    int               height = 400;
    const std::string winName("Draw Blocks");

    cv::Mat image(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

    int blockSize = 20;

    for (int y = 0; y < height; y += blockSize) {
        for (int x = 0; x < width; x += blockSize) {
            if ((y / blockSize + x / blockSize) % 2 == 0) {
                image(cv::Rect(x, y, blockSize, blockSize)).setTo(cv::Scalar(255, 255, 255));
            } else {
                image(cv::Rect(x, y, blockSize, blockSize)).setTo(cv::Scalar(125, 125, 125));
            }
        }
    }

    cv::namedWindow(winName, cv::WINDOW_AUTOSIZE);
    cv::imshow(winName, image);

    cv::imwrite("Block.png", image);

    cv::waitKey(3000);
}

int main() {
    capFrame();
    drawBlocks();
    return 0;
}