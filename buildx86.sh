mkdir -p out-x86
pushd out-x86
cmake -DNGL_CHIPSET=x86  -DUSE_RFB_GRAPH=ON  \
    -DCMAKE_INSTALL_PREFIX=./ \
    ..
popd
