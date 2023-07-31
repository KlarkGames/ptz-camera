# ptz-camera
Ptz camera experiment

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

## Firmware
See documentation [here](docs/FIRMWARE.md).

## Control software
See documentation [here](docs/CONTROL.md).

## Dependencies
Dependencies are based on Debian stable repository.
- [FFmpeg-kit](https://github.com/arthenica/ffmpeg-kit/)
- [OpenCV 4.5.1](https://github.com/opencv/opencv)
- [gstreamer 1.22](https://github.com/GStreamer/gstreamer)
- [Qt 6.5.2](https://www.qt.io/download)
