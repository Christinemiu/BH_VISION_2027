#include "rclcpp/rclcpp.hpp"
#include "video_publisher/video_publisher.hpp"
#include "opencv4/opencv.hpp"
#include <chrono>
#include <thread>

//定义视频发布节点类
//继承自rclcpp::Node类
//“：：”作用域解析运算符，包括访问命名空间(std::count),访问类的成员函数(MyClass::myFunction()）等
class VideoPublisherNode : public rclcpp::Node
{
public:
    VideoPublisherNode() : Node("video_publisher_node"){


        //第一步：通过this->declare_parameter声明参数，注册到参数系统
        //这里修改视频路径。默认值为video.mp4
        this->declare_parameter<std::string>("video_path", "video.mp4"); 
        //发送帧率，默认30fps
        this->declare_parameter<int>("fps",30);
        //是否循环播放，默认false
        this->declare_parameter<bool>("loop",false);

        //step2:通过this->get_parameter获取参数值
        //这方式保证调试过程中可以通过ros2 param set命令修改参数值
        this->get_parameter("video_path", video_path_);
        this->get_parameter("fps", fps_);
        this->get_parameter("loop", loop_);


        //创建发布者
        
    }