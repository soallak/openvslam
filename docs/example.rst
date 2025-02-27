.. _chapter-example:

=======
Example
=======

We provided example code snippets for running OpenVSLAM with variety of datasets.

.. _section-example-video:

SLAM with Video Files
=====================

Tracking and Mapping
^^^^^^^^^^^^^^^^^^^^

We provide an example snippet for using video files (e.g. ``.mp4``) for visual SLAM.
The source code is placed at ``./example/run_video_slam.cc``.
The following options are allowed:

.. code-block:: bash

    $ ./run_video_slam -h
    Allowed options:
    -h, --help             produce help message
    -v, --vocab arg        vocabulary file path
    -m, --video arg        video file path
    -c, --config arg       config file path
    --mask arg             mask image path
    --frame-skip arg (=1)  interval of frame skip
    --no-sleep             not wait for next frame in real time
    --auto-term            automatically terminate the viewer
    --debug                debug mode
    --eval-log             store trajectory and tracking times for evaluation
    -p, --map-db arg       store a map database at this path after SLAM

| The camera that captures the video file must be calibrated. Create a config file (``.yaml``) according to the camera parameters.
| We provided a vocabulary file for FBoW at `here <https://github.com/OpenVSLAM-Community/FBoW_orb_vocab/raw/main/orb_vocab.fbow>`__.

Localization
^^^^^^^^^^^^

We provide an example snippet for using video files (e.g. ``.mp4``) for localization based on a prebuilt map.
The source code is placed at ``./example/run_video_localization.cc``.
The following options are allowed:

.. code-block:: bash

    $ ./run_video_localization -h
    Allowed options:
    -h, --help             produce help message
    -v, --vocab arg        vocabulary file path
    -m, --video arg        video file path
    -c, --config arg       config file path
    -p, --map-db arg       path to a prebuilt map database
    --mapping              perform mapping as well as localization
    --mask arg             mask image path
    --frame-skip arg (=1)  interval of frame skip
    --no-sleep             not wait for next frame in real time
    --auto-term            automatically terminate the viewer
    --debug                debug mode

| The camera that captures the video file must be calibrated. Create a config file (``.yaml``) according to the camera parameters.
| We provided a vocabulary file for FBoW at `here <https://github.com/OpenVSLAM-Community/FBoW_orb_vocab/raw/main/orb_vocab.fbow>`__.

You can create a map database file by running one of the ``run_****_slam`` executables with ``--map-db map_file_name.msg`` option.

.. _section-example-image-sequence:

SLAM with Image Sequences
=========================

Tracking and Mapping
^^^^^^^^^^^^^^^^^^^^

We provided an example snippet for using image sequences for visual SLAM.
The source code is placed at ``./example/run_image_slam.cc``.
The following options are allowed:

.. code-block:: bash

    $ ./run_image_slam -h
    Allowed options:
    -h, --help             produce help message
    -v, --vocab arg        vocabulary file path
    -i, --img-dir arg      directory path which contains images
    -c, --config arg       config file path
    --mask arg             mask image path
    --frame-skip arg (=1)  interval of frame skip
    --no-sleep             not wait for next frame in real time
    --auto-term            automatically terminate the viewer
    --debug                debug mode
    --eval-log             store trajectory and tracking times for evaluation
    -p, --map-db arg       store a map database at this path after SLAM

| The camera that captures the video file must be calibrated. Create a config file (``.yaml``) according to the camera parameters.
| We provided a vocabulary file for FBoW at `here <https://github.com/OpenVSLAM-Community/FBoW_orb_vocab/raw/main/orb_vocab.fbow>`__.

Localization
^^^^^^^^^^^^

We provided an example snippet for using image sequences for localization based on a prebuilt map.
The source code is placed at ``./example/run_image_localization.cc``.
The following options are allowed:

.. code-block:: bash

    $ ./run_image_localization -h
    Allowed options:
    -h, --help             produce help message
    -v, --vocab arg        vocabulary file path
    -i, --img-dir arg      directory path which contains images
    -c, --config arg       config file path
    -p, --map-db arg       path to a prebuilt map database
    --mapping              perform mapping as well as localization
    --mask arg             mask image path
    --frame-skip arg (=1)  interval of frame skip
    --no-sleep             not wait for next frame in real time
    --auto-term            automatically terminate the viewer
    --debug                debug mode

| The camera that captures the video file must be calibrated. Create a config file (``.yaml``) according to the camera parameters.
| We provided a vocabulary file for FBoW at `here <https://github.com/OpenVSLAM-Community/FBoW_orb_vocab/raw/main/orb_vocab.fbow>`__.

You can create a map database file by running one of the ``run_****_slam`` executables with ``--map-db map_file_name.msg`` option.

.. _section-example-standard-datasets:

SLAM with Standard Datasets
===========================

.. _subsection-example-kitti:

KITTI Odometry dataset
^^^^^^^^^^^^^^^^^^^^^^

`KITTI Odometry dataset <http://www.cvlibs.net/datasets/kitti/>`_ is a benchmarking dataset for monocular and stereo visual odometry and lidar odometry that is captured from car-mounted devices.
We provided an example source code for running monocular and stereo visual SLAM with this dataset.
The source code is placed at ``./example/run_kitti_slam.cc``.

Start by downloading the dataset from `here <http://www.cvlibs.net/datasets/kitti/eval_odometry.php>`__.
Download the grayscale set (``data_odometry_gray.zip``).

After downloading and uncompressing it, you will find several sequences under the ``sequences/`` directory.

.. code-block:: bash

    $ ls sequences/
    00  01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17  18  19  20  21

In addition, download a vocabulary file for FBoW from `here <https://github.com/OpenVSLAM-Community/FBoW_orb_vocab/raw/main/orb_vocab.fbow>`__.

A configuration file for each sequence is contained under ``./example/kitti/``.

If you built examples with Pangolin Viewer support, a map viewer and frame viewer will be launced right after executing the following command.

.. code-block:: bash

    # at the build directory of OpenVSLAM
    $ ls
    ...
    run_kitti_slam
    ...
    # monocular SLAM with sequence 00
    $ ./run_kitti_slam \
        -v /path/to/orb_vocab/orb_vocab.fbow \
        -d /path/to/KITTI/Odometry/sequences/00/ \
        -c ../example/kitti/KITTI_mono_00-02.yaml
    # stereo SLAM with sequence 05
    $ ./run_kitti_slam \
        -v /path/to/orb_vocab/orb_vocab.fbow \
        -d /path/to/KITTI/Odometry/sequences/05/ \
        -c ../example/kitti/KITTI_stereo_04-12.yaml

The following options are allowed:

.. code-block:: bash

    $ ./run_kitti_slam -h
    Allowed options:
    -h, --help             produce help message
    -v, --vocab arg        vocabulary file path
    -d, --data-dir arg     directory path which contains dataset
    -c, --config arg       config file path
    --frame-skip arg (=1)  interval of frame skip
    --no-sleep             not wait for next frame in real time
    --auto-term            automatically terminate the viewer
    --debug                debug mode
    --eval-log             store trajectory and tracking times for evaluation
    -p, --map-db arg       store a map database at this path after SLAM

.. _subsection-example-euroc:

EuRoC MAV dataset
^^^^^^^^^^^^^^^^^

`EuRoC MAV dataset <https://projects.asl.ethz.ch/datasets/doku.php?id=kmavvisualinertialdatasets>`_ is a benchmarking dataset for monocular and stereo visual odometry that is captured from drone-mounted devices.
We provide an example source code for running monocular and stereo visual SLAM with this dataset.
The source code is placed at ``./example/run_euroc_slam.cc``.

Start by downloading the dataset from `here <http://robotics.ethz.ch/~asl-datasets/ijrr_euroc_mav_dataset/>`__.
Download the ``.zip`` file of a dataset you plan on using.

After downloading and uncompressing it, you will find several directories under the ``mav0/`` directory.

.. code-block:: bash

    $ ls mav0/
    body.yaml  cam0  cam1  imu0  leica0  state_groundtruth_estimate0

In addition, download a vocabulary file for FBoW from `here <https://github.com/OpenVSLAM-Community/FBoW_orb_vocab/raw/main/orb_vocab.fbow>`__.

We provided the two config files for EuRoC, ``./example/euroc/EuRoC_mono.yaml`` for monocular and ``./example/euroc/EuRoC_stereo.yaml`` for stereo.

If you have built examples with Pangolin Viewer support, a map viewer and frame viewer will be launched right after executing the following command.

.. code-block:: bash

    # at the build directory of OpenVSLAM
    $ ls
    ...
    run_euroc_slam
    ...
    # monocular SLAM with any EuRoC sequence
    $ ./run_euroc_slam \
        -v /path/to/orb_vocab/orb_vocab.fbow \
        -d /path/to/EuRoC/MAV/mav0/ \
        -c ../example/euroc/EuRoC_mono.yaml
    # stereo SLAM with any EuRoC sequence
    $ ./run_euroc_slam \
        -v /path/to/orb_vocab/orb_vocab.fbow \
        -d /path/to/EuRoC/MAV/mav0/ \
        -c ../example/euroc/EuRoC_stereo.yaml

The following options are allowed:

.. code-block:: bash

    $ ./run_euroc_slam -h
    Allowed options:
    -h, --help             produce help message
    -v, --vocab arg        vocabulary file path
    -d, --data-dir arg     directory path which contains dataset
    -c, --config arg       config file path
    --frame-skip arg (=1)  interval of frame skip
    --no-sleep             not wait for next frame in real time
    --auto-term            automatically terminate the viewer
    --debug                debug mode
    --eval-log             store trajectory and tracking times for evaluation
    -p, --map-db arg       store a map database at this path after SLAM

.. _subsection-example-tum-rgbd:

TUM RGBD dataset
^^^^^^^^^^^^^^^^

`TUM RGBD dataset <https://vision.in.tum.de/data/datasets/rgbd-dataset>`_ is a benchmarking dataset fcontaining RGB-D data and ground-truth data with the goal to establish a novel benchmark for the evaluation of visual odometry and visual SLAM systems.
The source code is placed at ``./example/run_tum_rgbd_localization.cc``.

Start by downloading the various dataset from `here <https://vision.in.tum.de/data/datasets/rgbd-dataset/download>`__.
One of many example datasets can be found from  `here <https://vision.in.tum.de/rgbd/dataset/freiburg3/rgbd_dataset_freiburg3_calibration_rgb_depth.tgz>`__.
Download the ``.tgz`` file of a dataset you plan on using.

After downloading and uncompressing it, you will find two directories and few text files under the ``rgbd_dataset_freiburg3_calibration_rgb_depth/`` directory.

.. code-block:: bash

    $ ls rgbd_dataset_freiburg3_calibration_rgb_depth
    accelerometer.txt  depth  depth.txt  groundtruth.txt  rgb  rgb.txt

If you would like to preprocess dataset then you can usee tool from `here <https://vision.in.tum.de/data/datasets/rgbd-dataset/tools>`__.

In addition, download a vocabulary file for FBoW from `here <https://github.com/OpenVSLAM-Community/FBoW_orb_vocab/raw/main/orb_vocab.fbow>`__.

We provided the config files for RGBD dataset at, ``./example/tum_rgbd``.

For above specific example we shall use two config files, ``./example/tum_rgbd/TUM_RGBD_mono_3.yaml`` for monocular and ``./example/tum_rgbd/TUM_RGBD_rgbd_3.yaml`` for RGBD.

Tracking and Mapping
^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

    # at the build directory of OpenVSLAM
    $ ls
    ...
    run_tum_rgbd_slam
    ...
    # monocular SLAM with rgbd_dataset_freiburg3_calibration_rgb_depth
    $ ./run_tum_rgbd_slam \
        -v /path/to/orb_vocab/orb_vocab.fbow \
        -d /path/to/rgbd_dataset_freiburg3_calibration_rgb_depth/ \
        -c ../example/tum_rgbd/TUM_RGBD_mono_3.yaml \
        --no-sleep \
        --auto-term \
        --map-db fr3_slam_mono.msg

    # RGBD SLAM with rgbd_dataset_freiburg3_calibration_rgb_depth
    $ ./run_tum_rgbd_slam \
        -v /path/to/orb_vocab/orb_vocab.fbow \
        -d /path/to/rgbd_dataset_freiburg3_calibration_rgb_depth/ \
        -c ../example/tum_rgbd/TUM_RGBD_rgbd_3.yaml \
        --no-sleep \
        --auto-term \
        --map-db fr3_slam_rgbd.msg

The following options are allowed:

.. code-block:: bash

    $ ./run_tum_rgbd_slam -h
    Allowed options:
    -h, --help             produce help message
    -v, --vocab arg        vocabulary file path
    -d, --data-dir arg     directory path which contains dataset
    -c, --config arg       config file path
    --frame-skip arg (=1)  interval of frame skip
    --no-sleep             not wait for next frame in real time
    --auto-term            automatically terminate the viewer
    --debug                debug mode
    --eval-log             store trajectory and tracking times for evaluation
    -p, --map-db arg       store a map database at this path after SLAM

Localization
^^^^^^^^^^^^

.. code-block:: bash

    # at the build directory of OpenVSLAM
    $ ls
    ...
    run_tum_rgbd_localization
    ...
    # monocular localization with rgbd_dataset_freiburg3_calibration_rgb_depth
    $ ./run_tum_rgbd_localization \
        -v /path/to/orb_vocab/orb_vocab.fbow \
        -d /path/to/rgbd_dataset_freiburg3_calibration_rgb_depth/ \
        -c ../example/tum_rgbd/TUM_RGBD_mono_3.yaml \
        --no-sleep \
        --auto-term \
        --map-db fr3_slam_mono.msg

    # RGBD SLAM with rgbd_dataset_freiburg3_calibration_rgb_depth
    $ ./run_tum_rgbd_localization \
        -v /path/to/orb_vocab/orb_vocab.fbow \
        -d /path/to/rgbd_dataset_freiburg3_calibration_rgb_depth/ \
        -c ../example/tum_rgbd/TUM_RGBD_rgbd_3.yaml \
        --no-sleep \
        --auto-term \
        --map-db fr3_slam_rgbd.msg

The following options are allowed:

.. code-block:: bash

    $ ./run_tum_rgbd_localization -h
    Allowed options:
    -h, --help             produce help message
    -v, --vocab arg        vocabulary file path
    -d, --data-dir arg     directory path which contains dataset
    -c, --config arg       config file path
    --frame-skip arg (=1)  interval of frame skip
    --no-sleep             not wait for next frame in real time
    --auto-term            automatically terminate the viewer
    --debug                debug mode
    --mapping              perform mapping as well as localization
    -p, --map-db arg       store a map database at this path after SLAM

.. _section-example-uvc-camera:

SLAM with UVC camera
=========================

Tracking and Mapping
^^^^^^^^^^^^^^^^^^^^

We provided an example snippet for using a UVC camera, which is often called a webcam, for visual SLAM.
The source code is placed at ``./example/run_camera_slam.cc``.
The following options are allowed:

.. code-block:: bash

    $ ./run_camera_slam  -h
    Allowed options:
    -h, --help            produce help message
    -v, --vocab arg       vocabulary file path
    -n, --number arg      camera number
    -c, --config arg      config file path
    --mask arg            mask image path
    -s, --scale arg (=1)  scaling ratio of images
    -p, --map-db arg      store a map database at this path after SLAM
    --debug               debug mode

| Please specify the camera number you want to use by ``-n`` option.
| The camera must be calibrated. Create a config file (``.yaml``) according to the camera parameters.
| You can scale input images to the performance of your machine by ``-s`` option. Please modify the config accordingly.
| We provided a vocabulary file for FBoW at `here <https://github.com/OpenVSLAM-Community/FBoW_orb_vocab/raw/main/orb_vocab.fbow>`__.

Localization
^^^^^^^^^^^^

We provided an example snippet for using a UVC camera for localization based on a prebuilt map.
The source code is placed at ``./example/run_camera_localization.cc``.
The following options are allowed:

.. code-block:: bash

    $ ./run_camera_localization -h
    Allowed options:
    -h, --help            produce help message
    -v, --vocab arg       vocabulary file path
    -n, --number arg      camera number
    -c, --config arg      config file path
    --mask arg            mask image path
    -s, --scale arg (=1)  scaling ratio of images
    -p, --map-db arg      path to a prebuilt map database
    --mapping             perform mapping as well as localization
    --debug               debug mode

| Please specify the camera number you want to use by ``-n`` option.
| The camera must be calibrated. Create a config file (``.yaml``) according to the camera parameters.
| You can scale input images to the performance of your machine by ``-s`` option. Please modify the config accordingly.
| We provided a vocabulary file for FBoW at `here <https://github.com/OpenVSLAM-Community/FBoW_orb_vocab/raw/main/orb_vocab.fbow>`__.
