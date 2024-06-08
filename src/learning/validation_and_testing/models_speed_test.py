import argparse
import csv
import glob
import os
import statistics
import time
from datetime import datetime
from typing import List, Tuple

import cv2 as cv
import ncnn
import numpy as np
import openvino as ov
import tensorflow as tf
from ultralytics import YOLO


def _parce_arguments():
    parser = argparse.ArgumentParser("Speed test for different evaluation libraries and models")
    parser.add_argument("--model_path", type=str)
    parser.add_argument("--data_path", type=str)
    parser.add_argument("--model_type", type=str, default=None)
    parser.add_argument("--input_size", type=int, default=320)
    parser.add_argument("--save_results", type=str, default="results.csv")
    return parser.parse_args()


def _main():
    args = _parce_arguments()

    model_path: str = args.model_path
    data_path: str = args.data_path
    model_type: str = args.model_type
    size: int = args.input_size
    results_path: str = args.save_results

    with open(results_path, "a", newline="") as csvfile:
        fieldnames = [
            "datetime",
            "name",
            "file_format",
            "evaluation_library",
            "int8_quantization",
            "float16_quantization",
            "simplification",
            "mean_process_time_s",
            "average_process_time_s",
            "mean_FPS",
            "average_FPS",
            "mAP@0.5-0.95"
        ]
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        if os.stat(results_path).st_size == 0:
            writer.writeheader()

        if 'onnx' in model_path or model_type == 'onnx':
            _test_opencv(model_path=model_path,
                         samples=_get_data_samples(data_path),
                         csvwriter=writer,
                         size=(size, size))
        elif 'openvino' in model_path or model_type == 'openvino':
            _test_openvino(model_path=model_path,
                         samples=_get_data_samples(data_path),
                         csvwriter=writer,
                         size=(size, size))
        elif 'ncnn' in model_path or model_type == 'ncnn':
            _test_ncnn(model_path=model_path,
                         samples=_get_data_samples(data_path),
                         csvwriter=writer,
                         size=(size, size))
        elif 'tflite' in model_path or model_type == "tflite":
            _test_tflite(model_path=model_path,
                         samples=_get_data_samples(data_path),
                         csvwriter=writer,
                         size=(size, size))


def _get_data_samples(data_path: str) -> List[str]:
    return glob.glob(os.path.join(data_path, "*.jpg"))


def _test_openvino(model_path: str, samples: List[str], csvwriter: csv.DictWriter, size: Tuple[int, int]):
    core = ov.Core()
    calculation_time_list = []

    model = core.read_model(model=os.path.join(model_path, "best.xml"),
                            weights=os.path.join(model_path, "best.bin"))
    compiled_model = core.compile_model(model=model, device_name="CPU")

    input_layer = compiled_model.input(0)
    output_layer = compiled_model.output(0)

    b, c, w, h = input_layer.shape

    for i, filepath in enumerate(samples):
        start_time = time.time_ns()
        image = cv.imread(filepath)
        resized_image = cv.resize(src=image, dsize=(w, h))
        input = np.expand_dims(np.transpose(resized_image, (2, 0, 1)), 0) # .astype(input_layer.element_type)
        result = compiled_model(input)
        end_time = time.time_ns()
        calculation_time = end_time - start_time
        calculation_time_list.append(calculation_time)
        print(f"Image {i + 1} proceed. Took: {calculation_time / 10e9:.5f} s")

    model = YOLO(model_path)
    metrics = model.val(
        data="data/processed/MIAP_YOLO/dataset.yaml",
        task="detect",
        imgsz=320,
        batch=1
    )

    print("All tests successfully complete")
    _log(csvwriter, model_path, "openvino", "openvino", calculation_time_list, metrics.box.maps[0])


def _test_opencv(model_path: str, samples: List[str], csvwriter: csv.DictWriter, size: Tuple[int, int]):
    model = cv.dnn.readNet(model_path)
    calculation_time_list = []
    for i, filepath in enumerate(samples):
        start_time = time.time_ns()
        input = cv.imread(filepath)
        blob = cv.dnn.blobFromImage(input, scalefactor=1 / 255, size=size)
        model.setInput(blob)
        model.forward()
        end_time = time.time_ns()
        calculation_time = end_time - start_time
        calculation_time_list.append(calculation_time)
        print(f"Image {i + 1} proceed. Took: {calculation_time / 10e9:.5f} s")

    model = YOLO(model_path)
    metrics = model.val(
        data="data/processed/MIAP_YOLO/dataset.yaml",
        task="detect",
        imgsz=320,
        batch=1
    )

    print("All tests successfully complete")
    _log(csvwriter, model_path, "onnx", "opencv", calculation_time_list, metrics.box.maps[0])


def _test_ncnn(model_path: str, samples: List[str], csvwriter: csv.DictWriter, size: Tuple[int, int]):
    model = ncnn.Net()
    model.load_param(os.path.join(model_path, "model.ncnn.param"))
    model.load_model(os.path.join(model_path, "model.ncnn.bin"))
    calculation_time_list = []

    with model.create_extractor() as ex:
        for i, filepath in enumerate(samples):
            start_time = time.time_ns()
            image = cv.imread(filepath)
            image = cv.cvtColor(image, cv.COLOR_BGR2RGB)
            resized_image = cv.resize(src=image, dsize=size)
            input = np.transpose(resized_image, (2, 0, 1)).astype(float)
            ex.input("in0", ncnn.Mat(input).clone())

            _, out0 = ex.extract("out0")

            end_time = time.time_ns()
            calculation_time = end_time - start_time
            calculation_time_list.append(calculation_time)
            print(f"Image {i + 1} proceed. Took: {calculation_time / 10e9:.5f} s")

    model = YOLO(model_path)
    metrics = model.val(
        data="data/processed/MIAP_YOLO/dataset.yaml",
        task="detect",
        imgsz=320,
        batch=1
    )

    print("All tests successfully complete")
    _log(csvwriter, model_path, "ncnn", "ncnn", calculation_time_list, metrics.box.maps[0])


def _test_tflite(model_path: str, samples: List[str], csvwriter: csv.DictWriter, size: Tuple[int, int]):
    model = tf.lite.Interpreter(model_path)
    pass
    b, w, h, c = model.get_input_details()[0]["shape"]
    runner = model.get_signature_runner()
    calculation_time_list = []
    for i, filepath in enumerate(samples):
        start_time = time.time_ns()
        image = cv.imread(filepath)
        resized_image = cv.resize(src=image, dsize=(w, h))
        input = np.expand_dims(resized_image, 0) # .astype(input_layer.element_type)
        result = runner(images=input.astype(np.float32))
        end_time = time.time_ns()
        calculation_time = end_time - start_time
        calculation_time_list.append(calculation_time)
        print(f"Image {i + 1} proceed. Took: {calculation_time / 10e9:.5f} s")

    model = YOLO(model_path)
    metrics = model.val(
        data="data/processed/MIAP_YOLO/dataset.yaml",
        task="detect",
        imgsz=320,
        batch=1
    )

    print("All tests successfully complete")
    _log(csvwriter, model_path, "tflite", "tflite", calculation_time_list, metrics.box.maps[0])

def _log(writer: csv.DictWriter,
         model_path: str,
         file_format: str,
         evaluation_library: str,
         calculation_time_list: List[float],
         mAP: float):
    avg_time = statistics.mean(calculation_time_list)
    median_time = statistics.median(calculation_time_list)
    print("Calculation time:\n"
          f"    Average: {avg_time / 10e9:.5f} s\n"
          f"    Median:  {median_time / 10e9:.5f} s")
    print("FPS:\n"
          f"    Average:  {10e9 / avg_time:.3f}\n"
          f"    Median:   {10e9 / median_time:.3f}")

    info = {
        "datetime": datetime.now().strftime("%d.%m.%Y %H:%M:%S"),
        "name": model_path,
        "file_format": file_format,
        "evaluation_library": evaluation_library,
        "int8_quantization": "int8-true" in model_path,
        "float16_quantization": "half-true" in model_path,
        "simplification": "simplify-true" in model_path,
        "mean_process_time_s": median_time / 10e9,
        "average_process_time_s": avg_time / 10e9,
        "mean_FPS": 10e9 / median_time,
        "average_FPS": 10e9 / avg_time,
        "mAP@0.5-0.95": mAP
    }
    writer.writerow(info)
    print("Experiment results successfully saved")


if __name__ == "__main__":
    _main()