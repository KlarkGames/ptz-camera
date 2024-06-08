from os.path import join
import glob
import numpy as np
from PIL import Image
from tqdm import tqdm
from torchvision.transforms.v2 import PILToTensor, Normalize, Compose, ToDtype, Resize, Grayscale
import torch

path = "/home/alex/Projects/Basilevs/ptz-camera/data/processed/MIAP_COCO/data/*.jpg"
imgs_path = glob.glob(path)

transforms = Compose([
    PILToTensor(),
    Resize(size=(600, 600)),
    # Grayscale(1),
    ToDtype(torch.float32),
    # Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
])

values = []
for img in tqdm(imgs_path[:500]):
    image = transforms(Image.open(img))
    if image.shape[0] == 3:
        values.append(image.numpy())
    # print(image.shape)

rgb_values = np.asarray(values)
mu_rgb = std_rgb = rgb_values
# rgb_values.shape == (n, 3),
# where n is the total number of pixels in all images,
# and 3 are the 3 channels: R, G, B.

# Each value is in the interval [0; 1]

for axis in [0, 1, 1]:
    mu_rgb = np.mean(mu_rgb, axis=axis)  # mu_rgb.shape == (3,)
    std_rgb = np.std(std_rgb, axis=axis)  # std_rgb.shape == (3,)

print(f"Mean: {mu_rgb}")
print(mu_rgb.shape)
print(f"Std: {std_rgb}")
print(std_rgb.shape)