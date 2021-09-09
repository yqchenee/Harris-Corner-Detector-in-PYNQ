# Harris Corner Detector
In this file, we will introduce Harris corner detector, including detail of algorithm and theory.
## Introduction
Harris corner detector(HCD) is a corner detection operator that is commonly used in computer vision algorithms to extract corners and infer features of an image. It was first introduced by Chris Harris and Mike Stephens in 1988 upon the improvement of Moravec's corner detector.
A corner is a point whose local neighborhood stands in two dominant and different edge directions. In other words, a corner can be interpreted as the junction of two edges, where an edge is a sudden change in image brightness.
<img src="https://i.imgur.com/QFoaJ6F.png" width="80%" height="80%"/>
## Theory
For a given patch $(x,y)$ and displacement $(u,v)$, the difference function can be written as
<center>
<img src="https://i.imgur.com/dVBM7zf.png" width="60%" height="60%"/>
</center>

$I(x,y)$ is in gray-scaled. If $E(u,v)$ is large, $(x,y)$ is very likely be a corner.
Applying Taylor Expansion to the above equation and using some mathematical steps, we can get the following equation
<center>
<img src="https://i.imgur.com/bjXlrma.png">
</center>

where 
<center>
<img src="https://i.imgur.com/11pZQKG.png">
</center>

$I_x$ and $I_y$ are image derivatives in x and y directions respectively.
Then, we need a scoring metric to determine a point is corner or not. We call it response $R$.
<center>
<img src="https://i.imgur.com/wxsU4ix.png">
</center>

<center>
<img src="https://i.imgur.com/X1LPXni.png">
</center>

where $\lambda_1$ and $\lambda_2$ are the eigenvalues of $M$ and $k$ typically is 0.04-0.06.
If both $\lambda_1$ and $\lambda_2$ are small, the region is flat. If one of $\lambda_1$ and $\lambda_2$ is large and the other one is small, the region is edge. If both $\lambda_1$ and $\lambda_2$ are large, the region is corner. The figure below describes such cases:
![](https://i.imgur.com/n1CiRX2.png)
In HCD, we use magnitude of $R$ to determine a corner. The larger $|R|$ is, the more possible region is a corner.
## Algorithm
In our implementation, the alogrithm of HCD are the following steps:
1. We read input image file.
2. We change RGB image into grayscale.
3. Applying image filter(Gaussian blur) to blur image.
4. For every pixel, we compute $I_x$, $I_y$ and matrix $M$.
5. We compute $det(M)$, $trace(M)$ and response $R$.
6. We use thresholding to filter the pixels with low $|R|$.
7. For every 5x5 region, find the local maxima to determine final result.
8. Finally, every pixel is a corner or not is determined. 





