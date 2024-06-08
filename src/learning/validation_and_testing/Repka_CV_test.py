import cv2 as cv
import argparse
import glob
import time
import statistics


def main():
    parser = argparse.ArgumentParser(description="OpenCV YOLO5 Python evaluation test")
    parser.add_argument('image_path', help="The path where images to process are stored.")
    parser.add_argument('model_path', help="The path where selected model ONNX file is stored. "
                                           "Only .jpg allowed.")
    parser.add_argument('-n', '--number_of_images',
                        help="The number of processed images. By default equals number of all images in data path.",
                        type=int, dest="n_of_images")

    args = vars(parser.parse_args())

    image_path = args["image_path"]
    model_path = args["model_path"]
    n_of_images = args["n_of_images"]

    calculation_time_list = []

    print("Searching images in directory")
    images_filenames = glob.glob(image_path + "/*.jpg")
    image_limit = len(images_filenames) if n_of_images is None else min(n_of_images, len(images_filenames))
    print(f"Test will proceed {image_limit}")
    print("Loading ONNX model")
    model = cv.dnn.readNet(model_path)

    print("Starting tests")
    for n, image_filename in enumerate(images_filenames):
        start_time = time.time_ns()
        input = cv.imread(image_filename)
        blob = cv.dnn.blobFromImage(input, scalefactor=1 / 255, size=(640, 640))
        model.setInput(blob)
        model.forward()
        end_time = time.time_ns()
        calculation_time = end_time - start_time
        calculation_time_list.append(calculation_time)
        print(f"Image {n + 1} proceed. Took: {calculation_time / 10e9:.5f} s")
        if n + 1 == image_limit:
            break

    avg_time = statistics.mean(calculation_time_list)
    median_time = statistics.median(calculation_time_list)
    print("All tests successfully complete")
    print("Calculation time:\n"
          f"    Average: {avg_time / 10e9:.5f} s\n"
          f"    Median:  {median_time / 10e9:.5f} s")
    print("FPS:\n"
          f"    Average:  {10e9 / avg_time:.3f}\n"
          f"    Median:   {10e9 / median_time:.3f}")


if __name__ == "__main__":
    main()
