set -e
builder=
build_cmd=make
if [ "$1" == "ninja" ]; then
    builder=-GNinja
    build_cmd=ninja
fi
cd build
cmake -DCMAKE_BUILD_TYPE=Debug $builder .. && $build_cmd -j$(nproc)
cd ..
./build/DynSLAMGUI --use_dispnet --dataset_root=../mini-seq-06 --dataset_type=kitti-odometry