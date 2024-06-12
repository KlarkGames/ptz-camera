# ptz-camera
## Description
This is the open-source pet-project of creating the PTZ-camera with DeepSORT algorithm implemented for person detection.
This repository consists of 3 parts:
1. ML part, training of YOLO and MobileNet detectors on MIAP dataset (Root directory)
2. Qt/C++ Server where DeepSORT implemented. Streams video to client and controls the tripod position (`src/control`)
3. Qt/C++ Client for remote server management (`src/client`)

# Training Detection model

## Installing dependencies 

### Poetry environment configuration
```bash
pipx install poetry
poetry config virtualenvs.in-project true
poetry install
source .venv/bin/activate
```

### Downloading MIAP dataset
To download MIAP dataset (32GB) run these commands:
```bash
python -m src.data.utils.downloader data/raw/MIAP_TEST/metadata/open_images_extended_miap_images_test.lst --download_folder data/raw/MIAP_TEST/data
python -m src.data.utils.downloader data/raw/MIAP_TRAIN/metadata/open_images_extended_miap_images_train.lst --download_folder data/raw/MIAP_TRAIN/data
python -m src.data.utils.downloader data/raw/MIAP_VAL/metadata/open_images_extended_miap_images_val.lst --download_folder data/raw/MIAP_VAL/data
```

## DVC pipeline

Initialize DVC.
```bash
dvc init
```

Before running DVC pipeline - configurate Datasets path, training settings in `params.yaml` and `dvc.yaml`
**It's highly recommended** to read through the DVC pipeline to understand the steps and created artefacts.

Start the pipeline
```bash
dvc repro
```

## Result of the pipeline
In the end you'll se the `models` directory containing all your trained models.
For YOLO you'll see also different versions of models in `models/YOLO/[MODEL NAME]/exported`. 
These models will be optimized as ONNX, NCNN, OpenVino models using Ultralytics functionality.

# Using the client-server Qt/C++ application
Server is located at `src/control` directory.
Client is located at `src/client` directory.

You can build the applications using CMake. 

## Dependencies
Dependencies are based on Debian stable repository.
- [FFmpeg-kit](https://github.com/arthenica/ffmpeg-kit/)
- [OpenCV 4.5.1](https://github.com/opencv/opencv)
- [gstreamer 1.22](https://github.com/GStreamer/gstreamer)
- [Qt 6.5.2](https://www.qt.io/download)

## Testing
To test the builded application move to the test folder:
```bash
cd ./build-control-...-Debug/tests
```

In forder you'll see several test binary files (**hungarian** for example).
Execute this files to run tests
```bash
./hungarian
```

To check command line options visit this [site](https://doc.qt.io/qt-6/qtest-overview.html#options).
