#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    float angle = rotation_angle * MY_PI / 180;
    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    Eigen::Matrix4f rotate;
    //Z-axis
    /*rotate <<   cos(angle), -sin(angle),    0,  0,
                sin(angle), cos(angle),     0,  0,
                0,          0,              1,  0,
                0,          0,              0,  1;*/
    //X-axis
    /*rotate <<   1,          0,              0,  0,
                0,          cos(angle), -sin(angle),      0,
                0,          sin(angle), cos(angle),       0,
                0,          0,              0,  1;*/
    //Y-axis
    rotate <<   cos(angle), 0,  sin(angle), 0,
                0,          1,  0,          0,
                -sin(angle),0,  cos(angle), 0,
                0,          0,  0,  1;
    model = rotate * model;
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this functionfov);
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    float FOVY_2 = (eye_fov / 2) * MY_PI / 180;
    float t = tan(FOVY_2) * fabs(zNear);
    float b = -t;
    float r = t * aspect_ratio;
    float l = -r;
    float n = zNear, f = zFar;
    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    Eigen::Matrix4f perspective;
    Eigen::Matrix4f translate, rotate;
    //Eigen::Matrix4f orthographic = Eigen::Matrix4f::Identity();

    perspective <<  n,  0,  0,  0,
                    0,  n,  0,  0,
                    0,  0,  n+f, -n*f,
                    0,  0,  1,  0;

    rotate <<   2/(r-l),  0,  0,  0,
                0,  2/(t-b),  0,  0,
                0,  0,  2/(n-f),  0,
                0,  0,  0,  1;

    translate <<    1,  0,  0,  -(r+l)/2,
                    0,  1,  0,  -(t+b)/2,
                    0,  0,  1,  -(n+f)/2,
                    0,  0,  0,  1;
    //projection = orthographic * (perspective to orthographic)
    projection = (translate * rotate) * (perspective * projection);
    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 10};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));
        time_t a = time(NULL);
        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        time_t b = time(NULL);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(1);

        std::cout << "frame count: " << frame_count++ << ", time:" << b-a << std::endl;

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
