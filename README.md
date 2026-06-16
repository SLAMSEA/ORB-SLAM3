# ORB-SLAM3 with GoPro — SLAMSEA Project

This repository extends ORB-SLAM3 with added functionality to process data captured by a GoPro camera, based on the [Urbste fork](https://github.com/urbste/ORB_SLAM3.git). It is developed and maintained as part of the **SLAMSEA 2026 Capstone Project**, targeting underwater SLAM applications using monocular and monocular-inertial configurations.

---

## Table of Contents

- [Installation on arm64 (UTM VM)](#installation-on-arm64)
  - [Add Dependencies](#1-add-dependencies)
  - [Install OpenCV](#2-install-opencv)
  - [Install Pangolin](#3-install-pangolin)
  - [Install ORB-SLAM3 (SLAMSEA Repository)](#4-install-orb-slam3-slamsea-repository)
- [System Testing](#system-testing)
- [Camera Calibration](#camera-calibration)
- [ORB-SLAM3 Overview](#orb-slam3)
- [Prerequisites](#2-prerequisites)
- [Building ORB-SLAM3](#3-building-orb-slam3-library-and-examples)
- [Running with Your Camera](#4-running-orb-slam3-with-your-camera)
- [EuRoC Examples](#5-euroc-examples)
- [TUM-VI Examples](#6-tum-vi-examples)
- [ROS Examples](#7-ros-examples)
- [Running Time Analysis](#8-running-time-analysis)
- [Calibration](#9-calibration)
- [License](#1-license)
- [Related Publications](#related-publications)

---

## Installation on arm64

This installation procedure has been tested on a **Mac M1** running an **Ubuntu 20.04.6** virtual machine via [UTM](https://mac.getutm.app/gallery/ubuntu-20-04). All software is installed under the `~/SLAMSEA/software/` directory structure used by the SLAMSEA project.

> **Note:** Open a Terminal by pressing `Ctrl + Alt + T`. When prompted during any installation step, type `Y` and press `Enter` to confirm package installation.

---

### 1. Add Dependencies

Before installing ORB-SLAM3 or any of its dependent libraries, update the system package list and install all required build tools and support libraries.

#### 1.1 Update Package List

```bash
sudo apt update
```

#### 1.2 Install Build Essentials

Install the core compilation toolchain required by all subsequent build steps:

```bash
sudo apt-get install build-essential
```

#### 1.3 Install Compilation, Repository, and Media Processing Libraries

```bash
sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
```

#### 1.4 Install Numerical Computing and Image Format Libraries

```bash
sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libdc1394-22-dev
```

> **Note (arm64 / VM only):** On some VM or arm64 configurations, the `libjasper-dev` package may also be required. If dependency resolution fails, add the following repository sources before re-running `apt update`:
>
> ```bash
> # For VM (arm64):
> sudo add-apt-repository "deb [arch=arm64] http://us.ports.ubuntu.com/ubuntu-ports/ xenial-security main multiverse restricted universe"
> sudo add-apt-repository "deb [arch=arm64] http://us.security.ubuntu.com/ubuntu/ xenial-security main multiverse restricted universe"
>
> # For Desktop (x86_64):
> sudo add-apt-repository "deb http://security.ubuntu.com/ubuntu xenial-security main"
> sudo apt update
> sudo apt-get install libjasper-dev
> ```

#### 1.5 Install Graphics and C++ Support Libraries

```bash
sudo apt-get install libglew-dev libboost-all-dev libssl-dev
```

#### 1.6 Install Eigen3

Eigen3 is required by the g2o non-linear optimization library used internally by ORB-SLAM3:

```bash
sudo apt install libeigen3-dev
```

#### 1.7 Install GNU Scientific Library (GSL)

GSL provides numerical computation support required during the build process:

```bash
sudo apt-get install libgsl-dev
```

If all steps complete without error messages, the dependency installation stage is complete and the system is ready for OpenCV installation.

---

### 2. Install OpenCV

OpenCV (Open Source Computer Vision Library) is the primary image processing library used by ORB-SLAM3 for feature extraction, image manipulation, and visual tracking. This project uses **OpenCV 4.6.0** to ensure compatibility with ORB-SLAM3 and all associated support libraries.

#### 2.1 Install OpenCV Build Dependencies

```bash
sudo apt install build-essential cmake git pkg-config libgtk-3-dev \
    libavcodec-dev libavformat-dev libswscale-dev libv4l-dev \
    libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev \
    gfortran openexr libatlas-base-dev python3-dev python3-numpy \
    libtbb2 libtbb-dev libdc1394-22-dev libopenexr-dev \
    libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev
```

#### 2.2 Create Working Directory

```bash
mkdir -p ~/SLAMSEA/software/OpenCV
cd ~/SLAMSEA/software/OpenCV
```

#### 2.3 Clone OpenCV and OpenCV Contrib Repositories

```bash
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git
```

#### 2.4 Checkout OpenCV 4.6.0

```bash
cd ~/SLAMSEA/software/OpenCV/opencv
git checkout 4.6.0

cd ~/SLAMSEA/software/OpenCV/opencv_contrib
git checkout 4.6.0
```

#### 2.5 Configure the Build with CMake

```bash
cd ~/SLAMSEA/software/OpenCV/opencv
mkdir -p build && cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D INSTALL_C_EXAMPLES=ON \
    -D INSTALL_PYTHON_EXAMPLES=ON \
    -D OPENCV_GENERATE_PKGCONFIG=ON \
    -D OPENCV_EXTRA_MODULES_PATH=~/SLAMSEA/software/OpenCV/opencv_contrib/modules \
    -D BUILD_EXAMPLES=ON ..
```

#### 2.6 Compile and Install

```bash
make -j2
sudo make install
```

> **Note:** Compilation time varies depending on the host machine specifications. On a virtual machine, this process typically takes 30 minutes or longer. Using `make -j2` limits parallel jobs to prevent memory exhaustion within the VM environment. Adjust the number of jobs (e.g., `-j4`, `-j8`) according to available system resources.

#### 2.7 Configure the Python Environment

After installation, configure the Python path so that OpenCV is accessible from the Python 3 environment:

```bash
python3 --version
find /usr/local -name "cv2*.so"
export PYTHONPATH=/usr/local/lib/python3.8/site-packages:$PYTHONPATH
echo 'export PYTHONPATH=/usr/local/lib/python3.8/site-packages:$PYTHONPATH' >> ~/.bashrc
source ~/.bashrc
```

#### 2.8 Verify Installation

```bash
pkg-config --modversion opencv4
python3 -c "import cv2; print(cv2.__version__)"
```

If both commands return `4.6.0`, the OpenCV installation has been completed successfully.

---

### 3. Install Pangolin

Pangolin is the visualization library used by ORB-SLAM3 to display real-time tracking, mapping, and camera trajectory during navigation.

#### 3.1 Navigate to the Software Directory

```bash
cd ~/SLAMSEA/software
```

#### 3.2 Clone the Pangolin Repository

```bash
git clone https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
```

#### 3.3 Build and Install Pangolin

```bash
mkdir -p build
cd build
cmake .. -D CMAKE_BUILD_TYPE=Release -D BUILD_PANGOLIN_LIBOPENEXR=OFF
make -j2
sudo make install
```

> **Note:** The `-D BUILD_PANGOLIN_LIBOPENEXR=OFF` flag disables the OpenEXR component of Pangolin, which may cause build failures on certain Ubuntu 20.04 configurations. This flag is recommended for all SLAMSEA installations.

If all steps complete without error, Pangolin has been installed successfully and the system is ready for the final ORB-SLAM3 compilation step.

---

### 4. Install ORB-SLAM3 (SLAMSEA Repository)

This step installs the SLAMSEA-specific fork of ORB-SLAM3, which includes modifications and configurations required for GoPro-based underwater SLAM.

#### 4.1 Navigate to the Software Directory

```bash
cd ~/SLAMSEA/software
```

#### 4.2 Clone the SLAMSEA ORB-SLAM3 Repository

```bash
git clone https://github.com/SLAMSEA/ORB-SLAM3.git
cd ORB-SLAM3
```

> **Note:** This repository is developed within the SLAMSEA project by adapting and extending several open-source ORB-SLAM3 forks to support GoPro camera data, monocular visual navigation, and monocular visual-inertial navigation.

#### 4.3 Build ORB-SLAM3

```bash
chmod +x build.sh
./build.sh
```

This script compiles all third-party libraries (DBoW2, g2o) and the main ORB-SLAM3 library and executables. Compilation time depends on hardware specifications and may take several minutes. Upon successful completion, the shared library `libORB_SLAM3.so` will be created in the `lib/` folder and all executables will be placed in the `Examples/` folder.

---

## System Testing

Once installation is complete, use the following command structure to run ORB-SLAM3:

```
"Executable path" "Vocabulary path" "calibration file.yaml path" "path to video.MP4"
```

### Examples

```bash
cd ~/SLAMSEA/software/ORB-SLAM3

# Monocular — EuRoC dataset
./Examples/Monocular/mono_euroc ./Vocabulary/ORBvoc.txt ./Examples/Monocular/EuRoC.yaml ~/Datasets/EuRoc/MH01 ./Examples/Monocular/EuRoC_TimeStamps/MH01.txt dataset-MH01_mono

# Monocular — GoPro
./Examples/Monocular/mono_gopro ./Vocabulary/ORBvoc.txt ./Examples/Monocular/gopro9_wide_setting.yaml ./Examples/testvideo.MP4

# Monocular-Inertial — GoPro
./Examples/Monocular-Inertial/mono_inertial_gopro_vi ./Vocabulary/ORBvoc.txt ./Examples/Monocular-Inertial/gopro9_wide_setting.yaml ./Examples/testvideo.MP4 ./Examples/testvideo.json

# ROS — GoPro Monocular
rosrun ORB_SLAM3 Mono Vocabulary/ORBvoc.txt Examples/Monocular/gopro.yaml
```

---

## Camera Calibration

The underwater environment presents significant challenges for camera-based SLAM systems. Refraction caused by the transmission of light through different media (air, housing glass, and water) introduces systematic distortions that can lead to miscalculation of depth and object scale if left uncorrected.

Prior to running the ORB-SLAM3 system, it is strongly recommended to correct the distortion caused by refraction using an appropriate underwater camera model. One suitable remapping correction model for underwater cameras is the **Pinhole-Axial (Pinax) camera model**, available at:

> [https://github.com/fickrie67/Pinax-camera-model.git](https://github.com/fickrie67/Pinax-camera-model.git)

For a detailed guide on visual-inertial calibration and the structure of valid configuration files, refer to `Calibration_Tutorial.pdf` included in this repository (see also [Section 9: Calibration](#9-calibration)).

---

# ORB-SLAM3

### V1.0, December 22nd, 2021

**Authors:** Carlos Campos, Richard Elvira, Juan J. Gómez Rodríguez, [José M. M. Montiel](http://webdiis.unizar.es/~josemari/), [Juan D. Tardos](http://webdiis.unizar.es/~jdtardos/).

The [Changelog](https://github.com/UZ-SLAMLab/ORB_SLAM3/blob/master/Changelog.md) describes the features of each version.

ORB-SLAM3 is the first real-time SLAM library able to perform **Visual, Visual-Inertial and Multi-Map SLAM** with **monocular, stereo and RGB-D** cameras, using **pin-hole and fisheye** lens models. In all sensor configurations, ORB-SLAM3 is as robust as the best systems available in the literature, and significantly more accurate.

We provide examples to run ORB-SLAM3 in the [EuRoC dataset](http://projects.asl.ethz.ch/datasets/doku.php?id=kmavvisualinertialdatasets) using stereo or monocular, with or without IMU, and in the [TUM-VI dataset](https://vision.in.tum.de/data/datasets/visual-inertial-dataset) using fisheye stereo or monocular, with or without IMU. Videos of some example executions can be found at [ORB-SLAM3 channel](https://www.youtube.com/channel/UCXVt-kXG6T95Z4tVaYlU80Q).

This software is based on [ORB-SLAM2](https://github.com/raulmur/ORB_SLAM2) developed by [Raul Mur-Artal](http://webdiis.unizar.es/~raulmur/), [Juan D. Tardos](http://webdiis.unizar.es/~jdtardos/), [J. M. M. Montiel](http://webdiis.unizar.es/~josemari/) and [Dorian Galvez-Lopez](http://doriangalvez.com/) ([DBoW2](https://github.com/dorian3d/DBoW2)).

<a href="https://youtu.be/HyLNq-98LRo" target="_blank"><img src="https://img.youtube.com/vi/HyLNq-98LRo/0.jpg" 
alt="ORB-SLAM3" width="240" height="180" border="10" /></a>

---

### Related Publications

[ORB-SLAM3] Carlos Campos, Richard Elvira, Juan J. Gómez Rodríguez, José M. M. Montiel and Juan D. Tardós, **ORB-SLAM3: An Accurate Open-Source Library for Visual, Visual-Inertial and Multi-Map SLAM**, *IEEE Transactions on Robotics 37(6):1874-1890, Dec. 2021*. **[PDF](https://arxiv.org/abs/2007.11898)**.

[IMU-Initialization] Carlos Campos, J. M. M. Montiel and Juan D. Tardós, **Inertial-Only Optimization for Visual-Inertial Initialization**, *ICRA 2020*. **[PDF](https://arxiv.org/pdf/2003.05766.pdf)**

[ORBSLAM-Atlas] Richard Elvira, J. M. M. Montiel and Juan D. Tardós, **ORBSLAM-Atlas: a robust and accurate multi-map system**, *IROS 2019*. **[PDF](https://arxiv.org/pdf/1908.11585.pdf)**.

[ORBSLAM-VI] Raúl Mur-Artal, and Juan D. Tardós, **Visual-inertial monocular SLAM with map reuse**, IEEE Robotics and Automation Letters, vol. 2 no. 2, pp. 796-803, 2017. **[PDF](https://arxiv.org/pdf/1610.05949.pdf)**.

[Stereo and RGB-D] Raúl Mur-Artal and Juan D. Tardós. **ORB-SLAM2: an Open-Source SLAM System for Monocular, Stereo and RGB-D Cameras**. *IEEE Transactions on Robotics,* vol. 33, no. 5, pp. 1255-1262, 2017. **[PDF](https://arxiv.org/pdf/1610.06475.pdf)**.

[Monocular] Raúl Mur-Artal, José M. M. Montiel and Juan D. Tardós. **ORB-SLAM: A Versatile and Accurate Monocular SLAM System**. *IEEE Transactions on Robotics,* vol. 31, no. 5, pp. 1147-1163, 2015. (**2015 IEEE Transactions on Robotics Best Paper Award**). **[PDF](https://arxiv.org/pdf/1502.00956.pdf)**.

[DBoW2 Place Recognition] Dorian Gálvez-López and Juan D. Tardós. **Bags of Binary Words for Fast Place Recognition in Image Sequences**. *IEEE Transactions on Robotics,* vol. 28, no. 5, pp. 1188-1197, 2012. **[PDF](http://doriangalvez.com/php/dl.php?dlp=GalvezTRO12.pdf)**

---

# 1. License

ORB-SLAM3 is released under [GPLv3 license](https://github.com/UZ-SLAMLab/ORB_SLAM3/LICENSE). For a list of all code/library dependencies (and associated licenses), please see [Dependencies.md](https://github.com/UZ-SLAMLab/ORB_SLAM3/blob/master/Dependencies.md).

For a closed-source version of ORB-SLAM3 for commercial purposes, please contact the authors: orbslam (at) unizar (dot) es.

If you use ORB-SLAM3 in an academic work, please cite:

```bibtex
@article{ORBSLAM3_TRO,
  title={{ORB-SLAM3}: An Accurate Open-Source Library for Visual, Visual-Inertial 
           and Multi-Map {SLAM}},
  author={Campos, Carlos AND Elvira, Richard AND G\´omez, Juan J. AND Montiel, 
          Jos\'e M. M. AND Tard\'os, Juan D.},
  journal={IEEE Transactions on Robotics}, 
  volume={37},
  number={6},
  pages={1874-1890},
  year={2021}
}
```

---

# 2. Prerequisites

The library has been tested on **Ubuntu 16.04** and **18.04**, and is also supported on **Ubuntu 20.04** as used in the SLAMSEA project. A powerful computer (e.g., Intel i7 or Apple Silicon via VM) will ensure real-time performance and provide more stable and accurate results.

## C++11 or C++0x Compiler

We use the new thread and chrono functionalities of C++11.

## Pangolin

We use [Pangolin](https://github.com/stevenlovegrove/Pangolin) for visualization and user interface. Download and install instructions can be found at: https://github.com/stevenlovegrove/Pangolin.

## OpenCV

We use [OpenCV](http://opencv.org) to manipulate images and features. Download and install instructions can be found at: http://opencv.org. **Required at least version 3.0. Tested with OpenCV 3.2.0, 4.4.0, and 4.6.0**.

## Eigen3

Required by g2o (see below). Download and install instructions can be found at: http://eigen.tuxfamily.org. **Required at least version 3.1.0**.

## DBoW2 and g2o (Included in Thirdparty folder)

We use modified versions of the [DBoW2](https://github.com/dorian3d/DBoW2) library to perform place recognition and [g2o](https://github.com/RainerKuemmerle/g2o) library to perform non-linear optimizations. Both modified libraries (which are BSD) are included in the *Thirdparty* folder.

## Python

Required to calculate the alignment of the trajectory with the ground truth. **Requires the NumPy module**.

- (win) http://www.python.org/downloads/windows
- (deb) `sudo apt install libpython2.7-dev`
- (mac) preinstalled with macOS

## ROS (optional)

We provide some examples to process input from a monocular, monocular-inertial, stereo, stereo-inertial, or RGB-D camera using ROS. Building these examples is optional. These have been tested with ROS Melodic under Ubuntu 18.04.

---

# 3. Building ORB-SLAM3 Library and Examples

Clone the repository:

```bash
git clone https://github.com/UZ-SLAMLab/ORB_SLAM3.git ORB_SLAM3
```

We provide a script `build.sh` to build the *Thirdparty* libraries and *ORB-SLAM3*. Please make sure you have installed all required dependencies (see section 2). Execute:

```bash
cd ORB_SLAM3
chmod +x build.sh
./build.sh
```

This will create **libORB_SLAM3.so** at the *lib* folder and the executables in the *Examples* folder.

---

# 4. Running ORB-SLAM3 with Your Camera

The `Examples` directory contains several demo programs and calibration files to run ORB-SLAM3 in all sensor configurations with Intel RealSense cameras T265 and D435i. The steps needed to use your own camera are:

1. Calibrate your camera following `Calibration_Tutorial.pdf` and write your calibration file `your_camera.yaml`

2. Modify one of the provided demos to suit your specific camera model, and build it

3. Connect the camera to your computer using USB3 or the appropriate interface

4. Run ORB-SLAM3. For example, for the D435i camera:

```bash
./Examples/Stereo-Inertial/stereo_inertial_realsense_D435i Vocabulary/ORBvoc.txt ./Examples/Stereo-Inertial/RealSense_D435i.yaml
```

---

# 5. EuRoC Examples

The [EuRoC dataset](http://projects.asl.ethz.ch/datasets/doku.php?id=kmavvisualinertialdatasets) was recorded with two pinhole cameras and an inertial sensor. We provide an example script to launch EuRoC sequences in all sensor configurations.

1. Download a sequence (ASL format) from http://projects.asl.ethz.ch/datasets/doku.php?id=kmavvisualinertialdatasets

2. Open the script `euroc_examples.sh` in the root of the project. Change the **pathDatasetEuroc** variable to point to the directory where the dataset has been uncompressed.

3. Execute the following script to process all the sequences with all sensor configurations:

```bash
./euroc_examples
```

## Evaluation

EuRoC provides ground truth for each sequence in the IMU body reference. As pure visual executions report trajectories centered in the left camera, we provide in the `evaluation` folder the transformation of the ground truth to the left camera reference. Visual-inertial trajectories use the ground truth from the dataset.

Execute the following script to process sequences and compute the RMS ATE:

```bash
./euroc_eval_examples
```

---

# 6. TUM-VI Examples

The [TUM-VI dataset](https://vision.in.tum.de/data/datasets/visual-inertial-dataset) was recorded with two fisheye cameras and an inertial sensor.

1. Download a sequence from https://vision.in.tum.de/data/datasets/visual-inertial-dataset and uncompress it.

2. Open the script `tum_vi_examples.sh` in the root of the project. Change the **pathDatasetTUM_VI** variable to point to the directory where the dataset has been uncompressed.

3. Execute the following script to process all the sequences with all sensor configurations:

```bash
./tum_vi_examples
```

## Evaluation

In TUM-VI, ground truth is only available in the room where all sequences start and end. As a result, the error metric measures the drift at the end of the sequence.

Execute the following script to process sequences and compute the RMS ATE:

```bash
./tum_vi_eval_examples
```

---

# 7. ROS Examples

### Building the Nodes for Mono, Mono-Inertial, Stereo, Stereo-Inertial and RGB-D

Tested with ROS Melodic and Ubuntu 18.04.

1. Add the path including `Examples/ROS/ORB_SLAM3` to the `ROS_PACKAGE_PATH` environment variable. Open `.bashrc`:

    ```bash
    gedit ~/.bashrc
    ```

    Add the following line at the end, replacing `PATH` with the folder where you cloned ORB_SLAM3:

    ```bash
    export ROS_PACKAGE_PATH=${ROS_PACKAGE_PATH}:PATH/ORB_SLAM3/Examples/ROS
    ```

2. Execute the `build_ros.sh` script:

    ```bash
    chmod +x build_ros.sh
    ./build_ros.sh
    ```

### Running Monocular Node

For a monocular input from topic `/camera/image_raw`, run node `ORB_SLAM3/Mono`. You will need to provide the vocabulary file and a settings file. See the monocular examples above.

```bash
rosrun ORB_SLAM3 Mono PATH_TO_VOCABULARY PATH_TO_SETTINGS_FILE
```

### Running Monocular-Inertial Node

For a monocular input from topic `/camera/image_raw` and an inertial input from topic `/imu`, run node `ORB_SLAM3/Mono_Inertial`. Setting the optional third argument to `true` will apply CLAHE equalization to images (mainly for the TUM-VI dataset).

```bash
rosrun ORB_SLAM3 Mono_Inertial PATH_TO_VOCABULARY PATH_TO_SETTINGS_FILE [EQUALIZATION]
```

### Running Stereo Node

For a stereo input from topics `/camera/left/image_raw` and `/camera/right/image_raw`, run node `ORB_SLAM3/Stereo`. You will need to provide the vocabulary file and a settings file. For the Pinhole camera model, if you **provide rectification matrices** (see `Examples/Stereo/EuRoC.yaml`), the node will rectify the images online; **otherwise, images must be pre-rectified**. For the FishEye camera model, rectification is not required since the system works with original images:

```bash
rosrun ORB_SLAM3 Stereo PATH_TO_VOCABULARY PATH_TO_SETTINGS_FILE ONLINE_RECTIFICATION
```

### Running Stereo-Inertial Node

For a stereo input from topics `/camera/left/image_raw` and `/camera/right/image_raw`, and an inertial input from topic `/imu`, run node `ORB_SLAM3/Stereo_Inertial`. Provide the vocabulary file and a settings file, including rectification matrices if required, in the same manner as the Stereo case:

```bash
rosrun ORB_SLAM3 Stereo_Inertial PATH_TO_VOCABULARY PATH_TO_SETTINGS_FILE ONLINE_RECTIFICATION [EQUALIZATION]
```

### Running RGB-D Node

For an RGB-D input from topics `/camera/rgb/image_raw` and `/camera/depth_registered/image_raw`, run node `ORB_SLAM3/RGBD`. You will need to provide the vocabulary file and a settings file. See the RGB-D example above.

```bash
rosrun ORB_SLAM3 RGBD PATH_TO_VOCABULARY PATH_TO_SETTINGS_FILE
```

**Running ROS Example:** Download a rosbag (e.g., `V1_02_medium.bag`) from the EuRoC dataset (http://projects.asl.ethz.ch/datasets/doku.php?id=kmavvisualinertialdatasets). Open 3 terminal tabs and run the following commands, one per tab, for a Stereo-Inertial configuration:

```bash
roscore
```

```bash
rosrun ORB_SLAM3 Stereo_Inertial Vocabulary/ORBvoc.txt Examples/Stereo-Inertial/EuRoC.yaml true
```

```bash
rosbag play --pause V1_02_medium.bag /cam0/image_raw:=/camera/left/image_raw /cam1/image_raw:=/camera/right/image_raw /imu0:=/imu
```

Once ORB-SLAM3 has loaded the vocabulary, press **Space** in the rosbag terminal tab to begin playback.

**Remark:** For rosbags from the TUM-VI dataset, playback issues may occur due to large chunk sizes. One solution is to rebag them with the default chunk size:

```bash
rosrun rosbag fastrebag.py dataset-room1_512_16.bag dataset-room1_512_16_small_chunks.bag
```

---

# 8. Running Time Analysis

A flag in `include/Config.h` activates time measurements. Uncomment the line `#define REGISTER_TIMES` to obtain timing statistics for one execution. The results are displayed in the terminal and stored in a text file (`ExecTimeMean.txt`).

---

# 9. Calibration

A tutorial for visual-inertial calibration and a detailed description of valid configuration file contents can be found in `Calibration_Tutorial.pdf`, included in the root of this repository.

For underwater deployments, refer also to the [Camera Calibration](#camera-calibration) section above for guidance on correcting refraction-induced distortion using the Pinax camera model.
