import os
import argparse
import json
from typing import Dict

import torch
from torch.utils.data import DataLoader
from torchvision.transforms.v2 import (Compose,
                                       Resize,
                                       RandomHorizontalFlip,
                                       Grayscale,
                                       ToDtype,
                                       ToPureTensor)
from torchvision.models.detection.ssdlite import ssdlite320_mobilenet_v3_large, SSDLite320_MobileNet_V3_Large_Weights, \
    SSDLiteClassificationHead

from src.data.utils.MIAPDetection import MIAPDetection
from src.learning.utils.engine import train_one_epoch, evaluate


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Script for MobileNet training")
    parser.add_argument("--train_data_path", type=str)
    parser.add_argument("--val_data_path", type=str)
    parser.add_argument("--save_path", type=str, default="models/Debug")
    parser.add_argument("--width", type=int, default=600)
    parser.add_argument("--height", type=int, default=600)
    parser.add_argument("--device", type=str, default="cuda")
    parser.add_argument("--epochs", type=int, default=100)
    parser.add_argument("--tag", type=str, default="Debug")
    parser.add_argument("--grayscale_channels", type=int, default=3)
    parser.add_argument("--log_every", type=int, default=10)
    parser.add_argument("--batch_size", type=int, default=64)
    parser.add_argument("--pretrained", type=bool, default=True)
    return parser.parse_args()


def _collate_fn(batch):
    return tuple(zip(*batch))


def _main():
    args = _parse_args()

    train_data_path = args.train_data_path
    val_data_path = args.val_data_path
    save_path = args.save_path
    if not os.path.exists(save_path):
        os.makedirs(save_path)

    device = torch.device(args.device)

    transforms = Compose([
        Resize(size=(args.width, args.height)),
        RandomHorizontalFlip(),
        Grayscale(3),
        ToDtype(torch.float32),
        ToPureTensor()
    ])

    dataset_train = MIAPDetection(root=os.path.join(train_data_path, "data"),
                            annFile=os.path.join(train_data_path, "new_labels.json"),
                            transforms=transforms)

    dataset_val = MIAPDetection(root=os.path.join(val_data_path, "data"),
                                  annFile=os.path.join(val_data_path, "new_labels.json"),
                                  transforms=transforms)

    dataloader_train = DataLoader(dataset_train, batch_size=args.batch_size, collate_fn=_collate_fn)
    dataloader_val = DataLoader(dataset_val, batch_size=args.batch_size, collate_fn=_collate_fn)

    model = ssdlite320_mobilenet_v3_large(num_classes=2)

    # model.head.classification_head = SSDLiteClassificationHead(num_classes=2)
    model.train()

    # move model to the right device
    model.to(device)

    # construct an optimizer
    params = [p for p in model.parameters() if p.requires_grad]
    optimizer = torch.optim.SGD(
        params,
        lr=0.05,
        momentum=0.9,
        weight_decay=0.0005
    )

    # and a learning rate scheduler
    lr_scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(
        optimizer,
        patience=3,
        factor=0.7
    )

    num_epochs = args.epochs

    logs = {
        "train": {
            "losses": [],
            "detection_losses": [],
            "classification_losses": []
        },
        "val": {
            "losses": [],
            "detection_losses": [],
            "classification_losses": []
        },
        "val_metrics": {
            "AP": {
                "IoU 0.5-0.95 All maxDets 100": [],
                "IoU 0.5 All maxDets 100": [],
                "IoU 0.75 All maxDets 100": [],
                "IoU 0.5-0.95 Small maxDets 100": [],
                "IoU 0.5-0.95 Medium maxDets 100": [],
                "IoU 0.5-0.95 Large maxDets 100": []
            },
            "AR": {
                "IoU 0.5-0.95 All maxDets 1": [],
                "IoU 0.5-0.95 All maxDets 10": [],
                "IoU 0.5-0.95 All maxDets 100": [],
                "IoU 0.5-0.95 Small maxDets 100": [],
                "IoU 0.5-0.95 Medium maxDets 100": [],
                "IoU 0.5-0.95 Large maxDets 100": [],
            }
        }
    }

    for epoch in range(num_epochs):
        # train for one epoch, printing every 10 iterations
        train_loss_logger = train_one_epoch(model, optimizer, dataloader_train, device, epoch, print_freq=10)
        # evaluate on the test dataset
        val_metric_logger, val_loss_logger = evaluate(model, dataloader_val, device=device)
        # update the learning rate
        lr_scheduler.step(val_loss_logger.meters["loss"].median)

        logs = _update_logs(logs, train_loss_logger, val_loss_logger, val_metric_logger)

        if (epoch + 1) % args.log_every == 0:
            torch_input = torch.randn(1, args.grayscale_channels, args.width, args.height)
            _save_model(model, torch_input, save_path)

            logs["epoch"] = epoch
            _save_logs(logs, args, save_path)


def _update_logs(logs, train_loss_logger, val_loss_logger, val_metric_logger) -> Dict:

    for (label, logger) in [["train", train_loss_logger], ["val", val_loss_logger]]:
        logs[label]["losses"].append(logger.meters["loss"].median)
        logs[label]["detection_losses"].append(logger.meters["bbox_regression"].median)
        logs[label]["classification_losses"].append(logger.meters["classification"].median)

    metrics = val_metric_logger.coco_eval["bbox"].stats
    for (label, number) in zip(logs["val_metrics"]["AP"].keys(), range(0, 6)):
        logs["val_metrics"]["AP"][label].append(metrics[number])
    for (label, number) in zip(logs["val_metrics"]["AR"].keys(), range(6, 12)):
        logs["val_metrics"]["AR"][label].append(metrics[number])

    return logs


def _save_logs(logs, args, save_path):
    logs["parameters"] = vars(args)
    json_string = json.dumps(logs, indent=4)
    with open(os.path.join(save_path, "logs.json"), "w", encoding="UTF_8") as json_file:
        json_file.write(json_string)
    print(f"Training logs saved in {os.path.join(save_path, 'logs.json')}")


def _save_model(model, torch_input, save_path):
    model.to("cpu")
    # onnx_program = torch.onnx.dynamo_export(model, torch_input)
    # onnx_program.save(os.path.join(save_path, "model.onnx"))
    torch.onnx.export(model, torch_input,
                      os.path.join(save_path, "MobileNet.onnx"),
                      export_params=True,
                      input_names=["input"],
                      output_names=["output"]
                      )
    print(f"PyTorch Model saved as ONNX file in {os.path.join(save_path, 'model.onnx')}")

    torch.save(model.state_dict(), os.path.join(save_path, "checkpoint.pt"))
    print(f"PyTorch Model checkpoint saved as .pt file in {os.path.join(save_path, 'checkpoint.pt')}")


if __name__ == "__main__":
    _main()

