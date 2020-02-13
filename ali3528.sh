mkdir -p out-ali3528
pushd out-ali3528
cmake -DCMAKE_TOOLCHAIN_FILE=../ali3528_mtitoolchain.cmake \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DNGL_CHIPSET="ali3528" \
   -DNGL_CA=acs \
   ..
popd
