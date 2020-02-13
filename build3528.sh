mkdir -p out-ali3528
pushd out-ali3528
cmake -DCMAKE_TOOLCHAIN_FILE=../ali3528_toolchain.cmake \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DDIRECTFB=ON \
   ..
popd
