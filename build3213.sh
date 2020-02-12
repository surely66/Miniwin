mkdir -p out-gx3213
pushd out-gx3213
cmake -DCMAKE_TOOLCHAIN_FILE=../national-toolchain.cmake \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DUSE_RFB_GRAPH=ON \
   -DNGL_CHIPSET="gx3213" \
   ..
popd
