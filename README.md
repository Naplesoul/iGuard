# iGuard



### Build

#### PoseDetect

For cuda devices:

1. Download [CUDA 11.5.2](https://developer.nvidia.com/cuda-11-5-2-download-archive) deb(local) version for x86_64 ubuntu and follow installation instructions provided by NVIDIA.
2. Download [cuDNN 8.3.2](https://developer.nvidia.com/cudnn) tar file and unzip it following [this guide](https://docs.nvidia.com/deeplearning/cudnn/install-guide/index.html), you may need to register for a NVIDIA developer account.

```shell
# build openpose
sudo apt-get install build-essential python-pip
sudo apt-get install cmake-qt-gui
sudo apt-get install libopencv-dev
sudo apt install protobuf-compiler libgoogle-glog-dev
sudo apt install libboost-all-dev libhdf5-dev libatlas-base-dev

git clone https://github.com/CMU-Perceptual-Computing-Lab/openpose
cd openpose/
git checkout v1.7.0
git submodule update --init --recursive --remote

sudo bash ./scripts/ubuntu/install_deps.sh

mkdir build/
cd build/
cmake-gui ..
# press configure and choose right settings for you, e.g. GPU_MODE

make -j `nproc`
sudo make install

# build pose_detect
cd posedetect
cmake -B build
cd build
make
# if you have this error message after running pose_detect: error while loading shared libraries: libcaffe.so.1.0.0: cannot open shared object file: No such file or directory
# you may add the following env to your .bashrc
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

