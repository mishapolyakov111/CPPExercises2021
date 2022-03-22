#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <libutils/rasserts.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/types.hpp>


struct Edge {
    int u, v; // номера вершин которые это ребро соединяет
    int w; // длина ребра (т.е. насколько длинный путь предстоит преодолеть переходя по этому ребру между вершинами)

    Edge(int u, int v, int w) : u(u), v(v), w(w)
    {}
};

// Эта биективная функция по координате пикселя (строчка и столбик) + размерам картинки = выдает номер вершины
int encodeVertex(int row, int column, int nrows, int ncolumns) {
    rassert(row < nrows, 348723894723980017);
    rassert(column < ncolumns, 347823974239870018);
    int vertexId = row * ncolumns + column;
    return vertexId;
}

// Эта биективная функция по номеру вершины говорит какой пиксель этой вершине соовтетствует (эта функция должна быть симметрична предыдущей!)
cv::Point2i decodeVertex(int vertexId, int nrows, int ncolumns) {

    // TODO: придумайте как найти номер строки и столбика пикселю по номеру вершины (просто поймите предыдущую функцию и эта функция не будет казаться сложной)
    int row = vertexId / ncolumns;
    int column = vertexId % ncolumns;

    // сверим что функция симметрично сработала:
    rassert(encodeVertex(row, column, nrows, ncolumns) == vertexId, 34782974923035);

    rassert(row < nrows, 34723894720027);
    rassert(column < ncolumns, 3824598237592030);
    return cv::Point2i(column, row);
}

void run(int mazeNumber) {
    //cv::Mat maze = cv::imread("lesson15/data/mazesImages/maze" + std::to_string(mazeNumber) + ".png");
    cv::Mat maze = cv::imread("lesson15/data/mazesImages/maze1.png");
    rassert(!maze.empty(), 324783479230019);
    rassert(maze.type() == CV_8UC3, 3447928472389020);
    std::cout << "Maze resolution: " << maze.cols << "x" << maze.rows << std::endl;

    int nvertices = maze.cols * maze.rows; // TODO

    std::vector<std::vector<Edge>> edges_by_vertex(nvertices);
    for (int j = 1; j < maze.rows-1; ++j) {
        for (int i = 1; i < maze.cols-1; ++i) {
            cv::Vec3b color = maze.at<cv::Vec3b>(j, i);

            unsigned char blue = color[0];
            unsigned char green = color[1];
            unsigned char red = color[2];

            int v = encodeVertex(j, i, maze.rows, maze.cols);
            int u1 =  encodeVertex(j+1, i, maze.rows, maze.cols);
            int u3 =  encodeVertex(j-1, i, maze.rows, maze.cols);
            int u2 =  encodeVertex(j, i+1, maze.rows, maze.cols);
            int u4 =  encodeVertex(j, i-1, maze.rows, maze.cols);
            int w = blue + green + red +1;
            Edge edge1 = Edge(v, u1, w);
            Edge edge2 = Edge(v, u2, w);
            Edge edge3 = Edge(v, u3, w);
            Edge edge4 = Edge(v, u4, w);
            edges_by_vertex[v].push_back(edge1);
            edges_by_vertex[v].push_back(edge2);
            edges_by_vertex[v].push_back(edge3);
            edges_by_vertex[v].push_back(edge4);

            // TODO добавьте соотвтетсвующие этому пикселю ребра
        }
    }

    int start, finish;
    if (mazeNumber >= 1 && mazeNumber <= 3) { // Первые три лабиринта очень похожи но кое чем отличаются...
        start = encodeVertex(300, 300, maze.rows, maze.cols);
        finish = encodeVertex(0, 305, maze.rows, maze.cols);
    } else if (mazeNumber == 4) {
        start = encodeVertex(154, 312, maze.rows, maze.cols);
        finish = encodeVertex(477, 312, maze.rows, maze.cols);
    } else if (mazeNumber == 5) { // Лабиринт в большом разрешении, добровольный (на случай если вы реализовали быструю Дейкстру с приоритетной очередью)
        start = encodeVertex(1200, 1200, maze.rows, maze.cols);
        finish = encodeVertex(1200, 1200, maze.rows, maze.cols);
    } else {
        rassert(false, 324289347238920081);
    }

    const int INF = std::numeric_limits<int>::max();

    cv::Mat window = maze.clone(); // на этой картинке будем визуализировать до куда сейчас дошла прокладка маршрута

    std::vector<int> distances(nvertices, INF);
    // TODO СКОПИРУЙТЕ СЮДА ДЕЙКСТРУ ИЗ ПРЕДЫДУЩЕГО ИСХОДНИКА

    distances[start] = 0;
    int mindistance = 0;
    std::vector<bool> used(nvertices, false);
    std::vector<int> father(nvertices, -1);

    while (!used[finish]) {
        int n = -1;

        for(int i = 0; i < nvertices; i++){
            if(!used[i] && (distances[i] < INF) && (distances[i] == mindistance )) n = i;
        }

        if(n == -1) break;

        used[n] = true;
        int y = n / maze.rows;
        int x = n % maze.cols;
        window.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 255, 0);
        cv::imshow("Maze", window);
        cv::waitKey(1);

        for(auto edge : edges_by_vertex[n]){
            if (n == encodeVertex(350, 300, maze.rows, maze.cols)){
                std::cout << edge.w << " ";
            }
            if (distances[edge.v] > distances[n] + edge.w){
                distances[edge.v] = distances[n] + edge.w;
                father[edge.v] = n;
            }
        }
        mindistance = INF;
        for(int i = 0; i < nvertices; i++){
            if(!used[i] && (distances[i] < mindistance)) mindistance = distances[i];
        }
    }

    if (distances[finish] != INF) {
        std::vector<int> path;
        int place = finish;
        while(place != -1){
            path.push_back(place);
            place = father[place];
        }
        for (int i = path.size()-1; i >= 0; i--) {
            int row = path[i] / maze.rows;
            int column = path[i] % maze.cols;
            window.at<cv::Vec3b>(row, column) = cv::Vec3b(255, 0, 0);
        }
    }

    // TODO в момент когда вершина становится обработанной - красьте ее на картинке window в зеленый цвет и показывайте картинку:
    //    cv::Point2i p = decodeVertex(the_chosen_one, maze.rows, maze.cols);
    //    window.at<cv::Vec3b>(p.y, p.x) = cv::Vec3b(0, 255, 0);
    //    cv::imshow("Maze", window);
    //    cv::waitKey(1);
    // TODO это может тормозить, в таком случае показывайте window только после обработки каждой сотой вершины

    // TODO обозначьте найденный маршрут красными пикселями

    // TODO сохраните картинку window на диск

    std::cout << "Finished!" << std::endl;

    // Показываем результат пока пользователь не насладиться до конца и не нажмет Escape
    while (cv::waitKey(10) != 27) {
        cv::imshow("Maze", window);
    }

}



int main() {
    try {
        int mazeNumber = 1;
        run(mazeNumber);

        return 0;
    } catch (const std::exception &e) {
        std::cout << "Exception! " << e.what() << std::endl;
        return 1;
    }
}
