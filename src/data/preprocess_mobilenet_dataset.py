import argparse
import copy
import csv
import json
from os.path import join

import fiftyone as fo
import pandas as pd
from tqdm import tqdm
from fiftyone import ViewField as F


def _parce_arguments():
    parser = argparse.ArgumentParser("Preprocessing MIAP dataset"
                                     "for MobileNet training")
    parser.add_argument("--dataset_name", type=str, default="MIAP")
    parser.add_argument("--boxes_path", type=str)
    parser.add_argument("--classes_path", type=str)
    parser.add_argument("--images_path", type=str)
    parser.add_argument("--save_path", type=str)
    parser.add_argument("--without_occluded", type=bool, default=False)
    parser.add_argument("--without_truncated", type=bool, default=False)
    parser.add_argument("--without_group", type=bool, default=False)
    parser.add_argument("--without_depiction", type=bool, default=False)
    args = parser.parse_args()
    return args

def _main():
    args = _parce_arguments()

    dataset_name = args.dataset_name
    images_path = args.images_path
    boxes_path = args.boxes_path
    classes_path = args.classes_path
    save_path = args.save_path

    without_occluded = args.without_occluded
    without_truncated = args.without_truncated
    without_group = args.without_group
    without_depiction = args.without_depiction

    with open(classes_path) as f:
        reader = csv.reader(f, delimiter=",")
        classes = {row[0]: row[1] for row in reader}

    df = pd.read_csv(boxes_path)
    df["LabelName"] = df["LabelName"].apply(lambda x: classes[x])

    df = df[(df["XMin"] != df["XMax"]) &
            (df["YMin"] != df["YMax"])]
    if without_occluded:
        df = df[df["IsOccluded"] == 0]
    if without_truncated:
        df = df[df["IsTruncated"] == 0]
    if without_group:
        df = df[df["IsGroupOf"] == 0]
    if without_depiction:
        df = df[df["IsDepictionOf"] == 0]

    detections = df.groupby("ImageID", group_keys=True).apply(lambda x: _to_detections(x))
    dataset = fo.Dataset(name=dataset_name, persistent=True, overwrite=True)
    for label, detection in detections:
        sample = fo.Sample(filepath=join(images_path, f"{label}.jpg"))
        sample["detections"] = detection
        dataset.add_sample(sample)

    dataset.compute_metadata()
    three_channels_view = dataset.match(F("metadata.num_channels") == 3)
    three_channels_view.export(
        export_dir=join(save_path),
        labels_path=join("labels.json"),
        dataset_type=fo.types.COCODetectionDataset,
        label_field="detections",
        export_media="symlink"
    )

    with open(join(save_path, "labels.json")) as labels:
        data = json.load(labels)
        new_data = copy.deepcopy(data)
        new_data["categories"] = [{'id': 0, 'name': 'Background', 'supercategory': None},
                                  {'id': 1, 'name': 'Person', 'supercategory': None}]
        for annotation in tqdm(new_data['annotations']):
            image_id = annotation['image_id']
            image = new_data['images'][image_id - 1]
            # annotation['bbox'] = [annotation['bbox'][0] / image['width'],
            #                       annotation['bbox'][1] / image['height'],
            #                       annotation['bbox'][2] / image['width'],
            #                       annotation['bbox'][3] / image['height']]
            annotation['category_id'] = 1

    with open(join(save_path, "new_labels.json"), 'w') as output_labels:
        json.dump(new_data, output_labels)


def _to_detections(sample: pd.DataFrame):
    detections = []

    label = sample["ImageID"].iloc[0]

    for _, row in sample.iterrows():
        bbox = [row["XMin"], row["YMin"], row["XMax"] - row["XMin"], row["YMax"] - row["YMin"]]
        detection = fo.Detection(bounding_box=bbox, label=row["LabelName"])

        detection["IsOccluded"] = bool(row["IsOccluded"])
        detection["IsTruncated"] = bool(row["IsTruncated"])
        detection["IsGroupOf"] = bool(row["IsGroupOf"])
        detection["IsDepictionOf"] = bool(row["IsDepictionOf"])
        detection["IsInsideOf"] = bool(row["IsInsideOf"])

        detections.append(detection)

    return label, fo.Detections(detections=detections)


if __name__ == "__main__":
    _main()
