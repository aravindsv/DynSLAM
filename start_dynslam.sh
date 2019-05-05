cd build
cmake -DCMAKE_BUILD_TYPE=Debug -GNinja .. && ninja -j$(nproc)
cd ..
./build/DynSLAMGUI --use_dispnet --dataset_root=../mini-seq-06 --dataset_type=kitti-odometry
