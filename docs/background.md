# Harris Corner Detector
In this file, we will introduce Harris corner detector, including detail of algorithm and theory.
## Introduction
Harris corner detector(HCD) is a corner detection operator that is commonly used in computer vision algorithms to extract corners and infer features of an image. It was first introduced by Chris Harris and Mike Stephens in 1988 upon the improvement of Moravec's corner detector.
A corner is a point whose local neighborhood stands in two dominant and different edge directions. In other words, a corner can be interpreted as the junction of two edges, where an edge is a sudden change in image brightness.
There are many applications of Harris corner detector, such as motion detection, object recognition, video tracking, etc.
<img src="https://i.imgur.com/QFoaJ6F.png" width="80%" height="80%"/>
## Theory
For a given patch ![](https://i.imgur.com/kGrLKvk.png) and displacement ![](https://i.imgur.com/sMi1WB2.png)
, the difference function can be written as
<center>
<img src="https://i.imgur.com/dVBM7zf.png" width="60%" height="60%"/>
</center>

![](https://i.imgur.com/PZgK0nI.png)
 is in gray-scaled. If ![](https://i.imgur.com/sUgXU2J.png)
 is large, ![](https://i.imgur.com/kGrLKvk.png)
 is very likely be a corner.
Applying Taylor Expansion to the above equation and using some mathematical steps, we can get the following equation
<center>
<img src="https://i.imgur.com/bjXlrma.png">
</center>

where 
<center>
<img src="https://i.imgur.com/11pZQKG.png">
</center>

![](https://i.imgur.com/giPWgae.png)
 and ![](https://i.imgur.com/5T70a3W.png)
 are image derivatives in x and y directions respectively.
Then, we need a scoring metric to determine a point is corner or not. We call it response ![](https://i.imgur.com/uNq8we8.png).


<center>
<img src="https://i.imgur.com/wxsU4ix.png">
</center>

<center>
<img src="https://i.imgur.com/X1LPXni.png">
</center>

where ![](https://i.imgur.com/CVlgsGa.png)
 and ![](https://i.imgur.com/PTt6x0c.png)
 are the eigenvalues of ![](https://i.imgur.com/hVQ2jRb.png)
 and ![](https://i.imgur.com/esblccN.png) typically is 0.04-0.06.
If both ![](https://i.imgur.com/h0oPwYe.png)
 and ![](https://i.imgur.com/UkM50C3.png)
 are small, the region is flat. If one of ![](https://i.imgur.com/isPTX2j.png)
and ![](https://i.imgur.com/U54jWEb.png)
 is large and the other one is small, the region is edge. If both 
![](https://i.imgur.com/56QuhAl.png) and ![](https://i.imgur.com/QG5DfXV.png)
 are large, the region is corner. The figure below describes such cases:
![](https://i.imgur.com/n1CiRX2.png)

In HCD, we use magnitude of ![](https://i.imgur.com/CVaEJL4.png)
to determine a corner. The larger ![](https://i.imgur.com/JvRQCDQ.png)
 is, the more possible region is a corner.
## Algorithm
The following code example is in src/host/HCD.py.
In our implementation, the alogrithm of HCD are the following steps:
1. We process input: we read input image file, and change RGB image into grayscale.
2. Applying image filter(Gaussian blur) to blur image.
```
smooth = cv2.GaussianBlur(img, (3, 3), 1.5)
```
3. Compute diff: for every pixel, we compute ![](https://i.imgur.com/giPWgae.png), ![](https://i.imgur.com/5T70a3W.png), and their multipliers.
```
kernel_x = np.array([[1, 0, -1]])
kernel_y = np.array([[1.], [0.], [-1.]])
Ix = cv2.filter2D(smooth, -1, kernel_x)
Iy = cv2.filter2D(smooth, -1, kernel_y)
```
```
Ixx = Ix * Ix
Ixy = Ix * Iy
Iyy = Iy * Iy
```
4. We compute Sxx, Sxy, Syy (weighted summation of Ixx, Ixy, Iyy in neighbor pixels) and matrix ![](https://i.imgur.com/9m6iZLe.png). 
```
Sxx = cv2.GaussianBlur(Ixx, (3, 3), 1)
Sxy = cv2.GaussianBlur(Ixy, (3, 3), 1)
Syy = cv2.GaussianBlur(Iyy, (3, 3), 1)
```
```
mt = np.stack((Sxx, Sxy), axis=2)
mb = np.stack((Sxy, Syy), axis=2)
M = np.stack((mt, mb), axis=2)
```
5. We compute ![](https://i.imgur.com/JW0vcAk.png)
, ![](https://i.imgur.com/6mfsOzX.png) and response ![](https://i.imgur.com/gQiDMBT.png).
```
M_det = np.array(np.linalg.det(M[:][:]))
M_trace = np.array(np.trace(M, axis1=2, axis2=3))
```
The response in python code is different from our hardware implementation.
```
response = M_det / (M_trace + 1e-12)
```
6. We use thresholding to filter the pixels with low ![](https://i.imgur.com/aaKCrOH.png).
7. For every 5x5 region, find the local maxima to determine final result.
8. Finally, every pixel is a corner or not is determined. 





