import os.path
from typing import Any, Callable, List, Optional, Tuple, Dict

# from PIL import Image

import torch
from torchvision.datasets import VisionDataset
from torchvision.transforms import PILToTensor
from torchvision.io import read_image
from torchvision.tv_tensors import BoundingBoxes, Image
from pycocotools.coco import COCO


class MIAPDetection(VisionDataset):
    """`MS Coco Detection <https://cocodataset.org/#detection-2016>`_ Dataset.

    It requires the `COCO API to be installed <https://github.com/pdollar/coco/tree/master/PythonAPI>`_.

    Args:
        root (string): Root directory where images are downloaded to.
        annFile (string): Path to json annotation file.
        transform (callable, optional): A function/transform that  takes in an PIL image
            and returns a transformed version. E.g, ``transforms.PILToTensor``
        target_transform (callable, optional): A function/transform that takes in the
            target and transforms it.
        transforms (callable, optional): A function/transform that takes input sample and its target as entry
            and returns a transformed version.
    """

    def __init__(
            self,
            root: str,
            annFile: str,
            transforms: Optional[Callable] = None,
    ) -> None:
        self.root = root
        super().__init__(root, transforms)

        self.coco = COCO(annFile)
        self.ids = list(sorted(self.coco.imgs.keys()))

    def _load_image_and_target(self, id: int) -> Tuple[torch.Tensor, Dict]:
        path = os.path.join(self.root, self.coco.loadImgs(id)[0]["file_name"])
        tensor_image = Image(read_image(path))

        target = self.coco.loadAnns(self.coco.getAnnIds(id))
        target = {
            "boxes": BoundingBoxes([[x["bbox"][0], x["bbox"][1],
                                    (x["bbox"][0] + x["bbox"][2]),
                                    (x["bbox"][1] + x["bbox"][3])] for x in target],
                                   format="XYXY", canvas_size=tensor_image.shape[-2:]),
            "labels": torch.tensor([x["category_id"] for x in target]),
            "image_id": id,
            "area": torch.tensor([x["area"] for x in target]),
            "iscrowd": torch.tensor([x["iscrowd"] for x in target])
        }

        return tensor_image, target

    def __getitem__(self, index: int) -> Tuple[Any, Any]:
        id = self.ids[index]
        image, target = self._load_image_and_target(id)
        image, target = self.transforms(image, target)
        return image, target

    def __len__(self) -> int:
        return len(self.ids)
