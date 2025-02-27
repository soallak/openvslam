#!/bin/sh

TOPDIR="$(dirname $(realpath "$0"))"/../../ #openvslam directory

(
    cd $TOPDIR

    # Building Docker Image
    docker build -t openvslam-desktop -f Dockerfile.desktop . --build-arg NUM_THREADS=`expr $(nproc) - 1`

    # Starting Docker Container
    # before launching the container, allow display access from local users
    xhost +local:
    # launch the container
    GPU_OPTION=
    if command -v nvidia-container-toolkit &> /dev/null ; then
	GPU_OPTION="--gpus all"
    fi
    docker run -it --rm ${GPU_OPTION} -e DISPLAY=$DISPLAY -v /tmp/.X11-unix/:/tmp/.X11-unix:ro --volume ${TOPDIR}/dataset:/dataset:ro --volume ${TOPDIR}/vocab:/vocab:ro openvslam-desktop

)

# reference
## https://openvslam-community.readthedocs.io/en/latest/installation.html#chapter-installation
## https://openvslam-community.readthedocs.io/en/latest/docker.html
