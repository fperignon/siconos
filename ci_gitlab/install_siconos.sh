#!bin/bash

# Standard install of siconos (all components, without oce).

# Note : this script takes osname (from docker image in gitlab-ci script) as arg.
: ${CI_PROJECT_DIR:?"Please set environment variable CI_PROJECT_DIR with 'siconos' repository (absolute) path."}

mkdir build-siconos
cd build-siconos
#ctest -S ${CI_PROJECT_DIR}/ci_gitlab/ctest_driver_install_siconos.cmake -Dmodel=$CTEST_MODEL -DSICONOS_INSTALL_DIR=${CI_PROJECT_DIR}/install-siconos -DOSNAME=$1 -DNO_SUBMIT=TRUE -V 
cmake .. -DWITH_TESTING=ON
make -j 4
make test
make install
