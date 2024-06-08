import pandas as pd
import seaborn as sns
from ultralytics import YOLO
import os
import argparse
from tqdm import tqdm


def _parce_arguments():
    parser = argparse.ArgumentParser("Script for validating models after speed_test")
    parser.add_argument("--data_csv", type=str, default="results.csv")
    parser.add_argument("--dataset_info", type=str, default="data/processed/MIAP_YOLO/dataset.yaml")
    parser.add_argument("--save_path", type=str, default="processed_results.csv")
    return parser.parse_args()


def _validate(row, dataset_path):
    model_path = row["name"]
    if os.path.exists(model_path):
        model = YOLO(model_path)
        metrics = model.val(
            data=dataset_path,
            task="detect",
            imgsz=320,
            batch=1
        )
        return metrics.box.maps[0]
    else:
        return None


def _main():
    args = _parce_arguments()

    df = pd.read_csv(args.data_csv)
    for index, row in tqdm(df.iterrows()):
        mAP = _validate(row, args.dataset_info)
        df.loc[index, "mAP@0.5-0.95"] = mAP
        df.to_csv(args.save_path)


if __name__ == "__main__":
    _main()