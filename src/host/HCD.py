import numpy as np
import cv2
import matplotlib.pyplot as plt


class Harris_corner_detector(object):
    def __init__(self, threshold):
        self.threshold = threshold

    def detect_harris_corners(self, img):
        ### TODO ####
        # Step 1: Smooth the image by Gaussian kernel
        # - Function: cv2.GaussianBlur (kernel = 3, sigma = 1.5)
        smooth = cv2.GaussianBlur(img, (3, 3), 1.5)

        # Step 2: Calculate Ix, Iy (1st derivative of image along x and y axis)
        # - Function: cv2.filter2D (kernel = [[1.,0.,-1.]] for Ix or [[1.],[0.],[-1.]] for Iy)
        kernel_x = np.array([[1, 0, -1]])
        kernel_y = np.array([[1.], [0.], [-1.]])
        Ix = cv2.filter2D(smooth, -1, kernel_x)
        Iy = cv2.filter2D(smooth, -1, kernel_y)
        #print(img.shape)
        #print(smooth.shape)
        #print(Ix.shape)
        
        # Step 3: Compute Ixx, Ixy, Iyy (Ixx = Ix*Ix, ...)
        Ixx = Ix * Ix
        Ixy = Ix * Iy
        Iyy = Iy * Iy
        
        # Step 4: Compute Sxx, Sxy, Syy (weighted summation of Ixx, Ixy, Iyy in neighbor pixels)
        # - Function: cv2.GaussianBlur (kernel = 3, sigma = 1.)
        Sxx = cv2.GaussianBlur(Ixx, (3, 3), 1)
        Sxy = cv2.GaussianBlur(Ixy, (3, 3), 1)
        Syy = cv2.GaussianBlur(Iyy, (3, 3), 1)

        # Step 5: Compute the det and trace of matrix M (M = [[Sxx, Sxy], [Sxy, Syy]])
        mt = np.stack((Sxx, Sxy), axis=2)
        mb = np.stack((Sxy, Syy), axis=2)
        M = np.stack((mt, mb), axis=2)

        M_det = np.array(np.linalg.det(M[:][:]))
        M_trace = np.array(np.trace(M, axis1=2, axis2=3))

        # Step 6: Compute the response of the detector by det/(trace+1e-12)
        response = M_det / (M_trace + 1e-12)

        return response
    
    def post_processing(self, response):
        # Step 1: Thresholding
        response[response < self.threshold] = 0

        # Step 2: Non-Maximum Suppression
        response = np.pad(response, (2, 2), 'constant')
        len_x, len_y = response.shape
        #print(len_x, len_y)

        local_max = []
        for i in range(2, len_x-2):
            for j in range(2, len_y-2):
                ismax = True
                if response[i][j]:
                    for count in range(25):
                        ri = count / 5
                        rj = count % 5
                        if response[int(i + ri - 2)][int(j + rj - 2)] > response[i][j]:
                            ismax = False
                            break
                    if ismax:
                        local_max.append([i-2, j-2])
        return local_max

