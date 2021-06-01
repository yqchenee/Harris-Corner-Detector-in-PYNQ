import numpy as np
import cv2
import argparse
from HCD import Harris_corner_detector


def main():
    parser = argparse.ArgumentParser(description='main function of Harris corner detector')
    parser.add_argument('--threshold', default=100., type=float, help='threshold value to determine corner')
    parser.add_argument('--image_path', default='./testdata/1.png', help='path to input image')
    args = parser.parse_args()

    print('Processing %s ...'%args.image_path)
    img = cv2.imread(args.image_path)
    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY).astype(np.float64)

    ### TODO ###
    HCD = Harris_corner_detector(args.threshold)

    response = HCD.detect_harris_corners(img_gray)
    result = HCD.post_processing(response)

    for res in result:
        if res[0] < img.shape[0] - 1:
            if res[1] < img.shape[1] - 1:
                img[res[0]+1, res[1]+1] = [0, 0, 255]
            if res[1] > 0:
                img[res[0]+1, res[1]-1] = [0, 0, 255]
            img[res[0]+1, res[1]] = [0, 0, 255]
        if res[0] > 0:
            if res[1] < img.shape[1] - 1:
                img[res[0]-1, res[1]+1] = [0, 0, 255]
            if res[1] > 0:
                img[res[0]-1, res[1]-1] = [0, 0, 255]
            img[res[0]-1, res[1]] = [0, 0, 255]
        if res[1] < img.shape[1] - 1:
            img[res[0], res[1]+1] = [0, 0, 255]
        if res[1] > 0:
            img[res[0], res[1]-1] = [0, 0, 255]
        img[res[0], res[1]] = [0, 0, 255]

    cv2.imwrite(f"visualize_{args.image_path[11:-4]}.png", img)


if __name__ == '__main__':
    main()