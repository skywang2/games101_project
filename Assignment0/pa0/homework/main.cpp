#include <cmath>
#include <iostream>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
using namespace std;

int main() {
    //double pi = acos(-1);
    Eigen::Vector3f v1;
    v1 << 2.0, 0.0, 1.0;
    //v1 << 2.0, 0.0, 1.0;
    cout << "v1:\n" << v1 << endl;

    Eigen::Matrix3f roateM, transM;
    roateM << cos(45.0/180.0*M_PIl), -sin(45.0/180.0*M_PIl), 0, sin(45.0/180.0*M_PIl), cos(45.0/180.0*M_PIl), 0, 0, 0, 1.0;
    transM << 1.0, 0, 1.0, 0, 1.0, 2.0, 0, 0, 1.0;
    //roateM << cos(90.0/180.0*M_PIl), -sin(90.0*M_PIl/180.0), 0.0, sin(90.0*M_PIl/180.0), cos(90.0*M_PIl/180.0), 0.0, 0.0, 0.0, 1.0;
    //printf("cos0:%lf\n", cos(0.0*M_PI/180.0));

    //cout << "roateM:\n" << roateM << endl;
    cout << "transM * roateM * v1:\n" << transM * roateM * v1 << endl;//from right to left

    return 0;
}