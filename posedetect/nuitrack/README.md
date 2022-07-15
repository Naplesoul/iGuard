# iGuard: Pose Detect using Nuitrack
## Dependencies
```shell
# install libfreenect2
sudo apt-get install build-essential cmake pkg-config
sudo apt-get install libusb-1.0-0-dev
sudo apt-get install libturbojpeg0-dev
sudo apt-get install libglfw3-dev
sudo apt-get install beignet-dev

git clone https://github.com/OpenKinect/libfreenect2.git
cd libfreenect2

mkdir build && cd build
cmake ..
make
sudo make install
sudo cp ../platform/linux/udev/90-kinect2.rules /etc/udev/rules.d/

# install nuitrack
sudo dpkg -i nuitrack.deb
```