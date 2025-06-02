#include <gtest/gtest.h>
#include <QImage>
#include <QPainter>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "tedgedetector.h"
#include "video_wdg/cv_to_qt_image/cvmatandqimage.h"


// Тестовый класс для TEdgeDetector
class TEdgeDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Инициализация TEdgeDetector с порогами 100 и 200
        detector = std::make_unique<TEdgeDetector>(100, 200);
    }

    std::unique_ptr<TEdgeDetector> detector;
};

//Неиницилиизированный QImage
TEST_F(TEdgeDetectorTest, NullImage) {
    QImage img;
    ASSERT_TRUE(img.isNull());

    detector->processFrame(&img);
    EXPECT_TRUE(img.isNull());
}

// Неподдерживыемый формат
TEST_F(TEdgeDetectorTest, InvalidFormat) {
    QImage img(100, 100, QImage::Format_Mono);
    img.fill(Qt::black);

    detector->processFrame(&img);
    EXPECT_NE(img.format(), QImage::Format_RGB32);
    EXPECT_FALSE(img.isNull());
    EXPECT_EQ(img.size(), QSize(100, 100));
}

// Валидное изображение
TEST_F(TEdgeDetectorTest, ValidRGB32Image) {
    // Создаем изображение в формате RGB32
    QImage img(100, 100, QImage::Format_RGB32);
    img.fill(Qt::white);

    QImage original = img.copy();
    detector->processFrame(&img);

    EXPECT_FALSE(img.isNull());
    EXPECT_TRUE(img.format() == QImage::Format_ARGB32 || QImage::Format_RGB32);
    EXPECT_EQ(img.size(), QSize(100, 100));

    bool imagesEqual = true;
    for (int y = 0; y < img.height() && imagesEqual; ++y) {
        for (int x = 0; x < img.width(); ++x) {
            if (img.pixel(x, y) != original.pixel(x, y)) {
                imagesEqual = false;
                break;
            }
        }
    }
    EXPECT_FALSE(imagesEqual) << "Image should be modified after edge detection";
}
// Изображение с нулевыми размерами
TEST_F(TEdgeDetectorTest, EmptyCvMat) {
    QImage img(0, 0, QImage::Format_RGB32);
    ASSERT_TRUE(img.isNull());

    detector->processFrame(&img);
    EXPECT_TRUE(img.isNull());
}
// Проверка на обнаружение краев
TEST_F(TEdgeDetectorTest, EdgeDetectionOutput) {
    // Создаем изображение с четкими границами
    QImage img(100, 100, QImage::Format_RGB32);
    img.fill(Qt::white);
    // Рисуем черный квадрат в центре для создания границ
    QPainter painter(&img);
    painter.fillRect(40, 40, 20, 20, Qt::black);
    painter.end();

    QImage original = img.copy();
    detector->processFrame(&img);

    // Проверяем, что изображение обработано
    EXPECT_FALSE(img.isNull());
    EXPECT_TRUE(img.format() == QImage::Format_ARGB32 || QImage::Format_RGB32);

    // Проверяем, что некоторые пиксели стали ненулевыми (обнаружены края)
    bool hasEdges = false;
    for (int y = 0; y < img.height() && !hasEdges; ++y) {
        for (int x = 0; x < img.width(); ++x) {
            QRgb pixel = img.pixel(x, y);
            if (qRed(pixel) > 0 || qGreen(pixel) > 0 || qBlue(pixel) > 0) {
                hasEdges = true;
                break;
            }
        }
    }
    EXPECT_TRUE(hasEdges) << "Edges should be detected in the processed image";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
