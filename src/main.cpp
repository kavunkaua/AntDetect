#include <iostream>
#include "trophallaxis/antdetect.hpp"
#include <torch/script.h> // One-stop header.
#include <unistd.h>

int main(int argc, char **argv)
{

    // testtorch();
    //./detector ../data/modules/traced_resnet_model.pt
    //./detector ../data/modules/AnTroD_resnet50.pt
    // testmodule(argv[1]);

    torch::jit::script::Module module;
    std::string pathmodel;
    std::vector<ALObject> objects;

    pathmodel = argv[1];
    std::cout << "pathmodel - " << pathmodel << std::endl;

    try
    {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        module = torch::jit::load(pathmodel);
    }
    catch (const c10::Error &e)
    {
        std::cerr << "error loading the model\n";
        return -1;
    }

    torch::DeviceType device_type = torch::kCPU;

    if (std::strstr(argv[3], "CUDA") != NULL)
        device_type = torch::kCUDA;

    if (std::strstr(argv[2], ".mp4") != NULL)
    {
        uint16_t start = 460;
        uint16_t nfram = 30;

        std::vector<std::vector<Obj>> objs;
        std::vector<std::pair<uint, idFix>> fixedIds;
        std::vector<cv::Mat> d_images;

        //---TEST---
        d_images = LoadVideo(argv[2], start, nfram);
        cv::VideoWriter writer;
        int codec = cv::VideoWriter::fourcc('M', 'P', '4', 'V');

        double fps = 1.0;
        std::string filename = "ant_detect_demo_" + std::to_string(start) + "_" + std::to_string(nfram) + ".mp4";
        cv::Mat frame;

        // cv::Size sizeFrame(992+extr,992);
        // cv::Mat testimg = std::get<2>(detectORB(d_images.at(0), d_images.at(1), 2.5));

        // cv::Mat testimg = DetectorMotionV2_1(pathmodel, device_type, d_images.at(0), d_images.at(1), objects, 0, /*class_name_color,*/ false);
        cv::Mat testimg = DetectorMotionV2_3(pathmodel, device_type, d_images.at(0), d_images.at(1), objects, 0, /*class_name_color,*/ false);

        cv::Size sizeFrame(testimg.cols, testimg.rows);

        writer.open(filename, codec, fps, sizeFrame, true);
        std::vector<ALObject> objects;

        std::vector<cv::Scalar> class_name_color;
        for (size_t i = 0; i < 366; i++)
            class_name_color.push_back(cv::Scalar(rand() % 255, rand() % 255, rand() % 255));

        for (int i = 0; i < d_images.size() - 1; i++)
        {
            std::cout << "[Frame: " << start + i << "]" << std::endl;
            // writer.write(DetectorMotionV2_1(pathmodel, device_type, d_images.at(i), d_images.at(i + 1), objects, start + i, /*class_name_color,*/ false));
            // DetectorMotionV2_1_artemis(pathmodel, device_type, d_images.at(i), d_images.at(i + 1), objects, start + i, false);
            writer.write(DetectorMotionV2_3(pathmodel, device_type, d_images.at(i), d_images.at(i + 1), objects, start + i, /*class_name_color,*/ false));
            // writer.write(std::get<2>(detectORB(d_images.at(i), d_images.at(i + 1), 2.5)));
        }
        writer.release();
        return 0;
        //---TEST---*/

        d_images = LoadVideo(argv[2], start, nfram);

        std::vector<OBJdetect> obj_detects;
        std::vector<Obj> objbuf;
        for (int i = 0; i < d_images.size(); i++)
        {
            obj_detects.clear();
            obj_detects = detectorV4(pathmodel, d_images.at(i), device_type);
            OBJdetectsToObjs(obj_detects, objbuf);
            objs.push_back(objbuf);
        }

        fixIDs(objs, fixedIds, d_images);
    }
    else
    {
        cv::Mat imageBGR = cv::imread(argv[2], cv::ImreadModes::IMREAD_COLOR);
        detectorV4(pathmodel, imageBGR, device_type);
    }

    return 0;
}