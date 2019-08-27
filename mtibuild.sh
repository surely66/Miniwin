mkdir -p out-mti-ali3528
pushd out-mti-ali3528
cmake -DCMAKE_TOOLCHAIN_FILE=../ali3528_mtitoolchain.cmake \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DDIRECTFB=ON \
   ..
popd