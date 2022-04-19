#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <set>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <memory>

#include <libutils/rasserts.h>
#include <libutils/fast_random.h>


int estimateQuality(cv::Mat mat, int j, int i, int ny, int nx);

int min(int a, int b, int c, int d);

// Эта функция говорит нам правда ли пиксель отмаскирован, т.е. отмечен как "удаленный", т.е. белый
bool isPixelMasked(cv::Mat mask, int j, int i) {
    rassert(j >= 0 && j < mask.rows, 372489347280017);
    rassert(i >= 0 && i < mask.cols, 372489347280018);
    rassert(mask.type() == CV_8UC3, 2348732984792380019);
    if (mask.at<cv::Vec3b>(j, i) == cv::Vec3b(255, 255, 255)) {
        return true;
    } else {
        return false;
    }

    // TODO проверьте белый ли пиксель
}

void run(int caseNumber, std::string caseName) {
    std::cout << "_________Case #" << caseNumber << ": " << caseName << "_________" << std::endl;

    cv::Mat original = cv::imread(
            "lesson18/data/" + std::to_string(caseNumber) + "_" + caseName + "/" + std::to_string(caseNumber) +
            "_original.jpg");
    cv::Mat mask = cv::imread(
            "lesson18/data/" + std::to_string(caseNumber) + "_" + caseName + "/" + std::to_string(caseNumber) +
            "_mask.png");
    rassert(!original.empty(), 324789374290018);
    rassert(!mask.empty(), 378957298420019);

    // TODO напишите rassert сверяющий разрешение картинки и маски
    rassert((original.cols == mask.cols && original.rows == original.rows), 324789374290018);
    // TODO выведите в консоль это разрешение картинки
    std::cout << "Image resolution: " << original.cols << "x" << original.rows << std::endl;

    // создаем папку в которую будем сохранять результаты - lesson18/resultsData/ИМЯ_НАБОРА/
    std::string resultsDir = "lesson18/resultsData/";
    if (!std::filesystem::exists(resultsDir)) { // если папка еще не создана
        std::filesystem::create_directory(resultsDir); // то создаем ее
    }
    resultsDir += std::to_string(caseNumber) + "_" + caseName + "/";
    if (!std::filesystem::exists(resultsDir)) { // если папка еще не создана
        std::filesystem::create_directory(resultsDir); // то создаем ее
    }

    // сохраняем в папку с результатами оригинальную картинку и маску
    cv::imwrite(resultsDir + "0original.png", original);
    cv::imwrite(resultsDir + "1mask.png", mask);
    int k = 0;
    // TODO замените белым цветом все пиксели в оригинальной картинке которые покрыты маской
    for (int j = 0; j < original.rows - 1; ++j) {
        for (int i = 0; i < original.cols - 1; ++i) {
            if (isPixelMasked(mask, j, i)) {
                k++;
                original.at<cv::Vec3b>(j, i)[0] = 255;
                original.at<cv::Vec3b>(j, i)[1] = 255;
                original.at<cv::Vec3b>(j, i)[2] = 255;
            }
        }
    }
    // TODO сохраните в папку с результатами то что получилось под названием "2_original_cleaned.png"
    cv::imwrite(resultsDir + "2_original_cleaned.png", original);
    // TODO посчитайте и выведите число отмаскированных пикселей (числом и в процентах) - в таком формате:
    // Number of masked pixels: 7899/544850 = 1%
    std::cout << "Number of masked pixels: " << k << "/" << original.rows * original.cols << " = "
              << ((double) k / (original.rows * original.cols) * 100) << "%" << std::endl;


    FastRandom random(32542341); // этот объект поможет вам генерировать случайные гипотезы

    // TODO 10 создайте картинку хранящую относительные смещения - откуда брать донора для заплатки, см. подсказки про то как с нею работать на сайте
    // TODO 11 во всех отмаскированных пикселях: заполните эту картинку с относительными смещениями - случайными смещениями (но чтобы они и их окрестность 5х5 не выходила за пределы картинки)
    // TODO 12 во всех отмаскированных пикселях: замените цвет пиксела А на цвет пикселя Б на который указывает относительное смещение пикселя А
    // TODO 13 сохраните получившуюся картинку на диск
    // TODO 14 выполняйте эти шаги 11-13 много раз, например 1000 раз (оберните просто в цикл, сохраняйте картинку на диск только на каждой десятой или сотой итерации)
    // TODO 15 теперь давайте заменять значение относительного смещения на новой только если новая случайная гипотеза - лучше старой, добавьте оценку "насколько смещенный патч 5х5 похож на патч вокруг пикселя если их наложить"
    //
    // Ориентировочный псевдокод-подсказка получившегося алгоритма:
    cv::Mat shifts(original.rows, original.cols, CV_32SC2,
                   cv::Scalar(0, 0)); // матрица хранящая смещения, изначально заполнена парами нулей
    cv::Mat image = original;
    std::cout << "Image resolution: " << original.cols << "x" << original.rows << std::endl;
    std::vector<cv::Mat> pyramid; // здесь будем хранить пронумерованные версии картинки разного разрешения
    // нулевой уровень - самая грубая, последний уровень - самая детальная

    cv::Mat img = original.clone();
    const int PYRAMID_MIN_SIZE = 20; // до какой поры уменьшать картинку? давайте уменьшать пока картинка больше 20 пикселей
    while (img.rows > PYRAMID_MIN_SIZE && img.rows > PYRAMID_MIN_SIZE) { // или пока больше (2 * размер окна для оценки качества)
        pyramid.insert(pyramid.begin(), img); // мы могли бы воспользоваться push_back но мы хотим вставлять картинки в начало вектора
        cv::pyrDown(img, img); // эта функция уменьшает картинку в два раза
    }
    
    for (int p = 0; p < 100; p++) {
        for (int j = 0; j < image.rows - 2; ++j) {
            for (int i = 0; i < image.cols - 2; ++i) {

                if (!isPixelMasked(mask, j, i)) continue; // пропускаем т.к. его менять не надо
                cv::Vec2i dxy = shifts.at<cv::Vec2i>(j,
                                                     i); //смотрим какое сейчас смещение для этого пикселя в матрице смещения
                int nx = i + dxy[1];
                int ny = j + dxy[0];
                rassert(i >= 0 && i < image.cols && j >= 0 && j < image.rows, 10);
                // ЭТО НЕ КОРРЕКТНЫЙ КОД, но он иллюстрирует как рассчитать координаты пикселя-донора из которого мы хотим брать цвет
                int currentQuality = estimateQuality(image, j, i, ny,
                                                     nx); // эта функция (создайте ее) считает насколько похож квадрат 5х5 приложенный центром к (i, j)
                //на квадрат 5х5 приложенный центром к (nx, ny)
                bool q = true;
                int newRandx = 0;
                int newRandy = 0;
                while (q) {
                    newRandx = random.next(2, image.cols - 2);
                    newRandy = random.next(2, image.rows - 2);
                    if (!isPixelMasked(image, newRandy, newRandx)) {
                        q = false;
                    }
                }

                int randomQuality = estimateQuality(image, j, i, newRandy,
                                                    newRandx); // оцениваем насколько похоже будет если мы приложим эту случайную гипотезу которую только что выбрали

                if (randomQuality < currentQuality || currentQuality == 0) {
                    shifts.at<cv::Vec2i>(j, i)[0] = newRandy - j;
                    shifts.at<cv::Vec2i>(j, i)[1] = newRandx - i;
                    image.at<cv::Vec3b>(j, i) = image.at<cv::Vec3b>(newRandy, newRandx);
                }
            }
        }
    }
    cv::imwrite(resultsDir + "3mask.png", image);
}


int estimateQuality(cv::Mat image, int j, int i, int ny, int nx) {
    int sd0 = 0;
    int sd1 = 0;
    int sd2 = 0;
    for (int k = -2; k < 3; ++k) {
        for (int l = -2; l < 3; ++l) {
            //rassert(i + l >= 0 && i + l < image.cols && j + k >= 0 && j +k < image.rows, 11);
            if (nx + l >= 0 && nx + l < image.cols && ny + k >= 0 && ny + k < image.rows) {
                //rassert(nx + l >= 0 && nx + l < image.cols && ny + k >= 0 && ny + k < image.rows, 12);
                sd0 += abs(image.at<cv::Vec3b>(j + k, i + l)[0] - image.at<cv::Vec3b>(ny + k, nx + l)[0]);
                sd1 += abs(image.at<cv::Vec3b>(j + k, i + l)[1] - image.at<cv::Vec3b>(ny + k, nx + l)[1]);
                sd2 += abs(image.at<cv::Vec3b>(j + k, i + l)[2] - image.at<cv::Vec3b>(ny + k, nx + l)[2]);
            }else{
                return 100000000;
            }
        }
    }
    return sd0 + sd1 + sd2;
}


int main() {
    try {
        run(1, "mic");
        // TODO протестируйте остальные случаи:
//        run(2, "flowers");
//        run(3, "baloons");
//        run(4, "brickwall");
//        run(5, "old_photo");
//        run(6, "your_data"); // TODO придумайте свой случай для тестирования (рекомендуется не очень большое разрешение, например 300х300)

        return 0;
    } catch (const std::exception &e) {
        std::cout << "Exception! " << e.what() << std::endl;
        return 1;
    }
}
