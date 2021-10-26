
## Results and Comparisons

### Result table
Copy from [optimization.md](./optimization.md)  
The following table summarizes the different optimize methods used in each implementations.

|  design                         | basic optimizations | sub-function pipelining | compact streaming interface | parallel processing | m_axi interface | latency(ms) | BRAM | DSP | FF | LUT |
| :-----------------------------: | :-----------------: |:-----------------------:|:---------------------------:|:-------------------:|:---------------:| :------------: |:----:|:---:|:----:|:-----:|
| [basic](./../src/kernel_basic/) |         v           |                         |                             |                     |                 | 31.708  |  80  | 2 | 9214 | 10352 |
| [opt1](./../src/kernel_opt1/)   |         v           |            v            |                             |                     |                 | 0.666   |  164 | 2 | 17557| 15446 |
| [opt2](./../src/kernel_opt2/)   |         v           |                         |              v              |                     |                 | 31.708  | 54 | 7  | 6821 | 8644 |
| [opt3](./../src/kernel_opt3/)   |         v           |            v            |              v              |                     |                 |  0.666 | 116 | 38 | 14468 | 12463 |
| [opt4](./../src/kernel_opt4/), N=2   |         v      |            v            |              v              |          v          |                 | 0.382 | 48 | 76  | 15965 | 16721 |
| [opt4](./../src/kernel_opt4/), N=4   |         v      |            v            |              v              |          v          |                 | 0.298 | 56 | 138 | 23465 | 26791 |
| [opt5](./../src/kernel_opt5/)   |         v           |            v            |              v              |          v          |       v         | 0.384 | 112 | 76 | 30157 | 45530 |
| CPU [1]                         |                     |                         |                             |                     |                 | 235 | x | x | x | x |

[1] [Python code](./../src/host/HCD.py) runs on Intel(R) Xeon(R) CPU E5-2650 v2 @ 2.60GHz.

### basic
![](https://i.imgur.com/uX4Ui8P.png)

### opt1
![](https://i.imgur.com/FGn68QL.png)
> Compared to basic implementation, after **pipelining the sub-functions**:
> * achieve II=1
> * latency: 3170881 -> 66611
> * resources usage:
>     * BRAM, FF, LUT: roughly doubled
>     * DSP: not changed


### opt2
![](https://i.imgur.com/qlO3ITu.png)
> Compared to basic implementation, after applying **compact streaming interface**:
> * latency: not changed
> * resources usage:
>     * BRAM, FF, LUT: roughly 0.6x
>     * DSP: 2 -> 7

### opt3
![](https://i.imgur.com/z3P3V5W.png)
> Compared to opt1, the result is similar to opt2 compares to basic  
> Compared to opt2, the result is similar to opt1 compares to basic

### opt4 with N = 2
![](https://i.imgur.com/fktOwD9.png)
> Since we have changed the code structure used in sub-function find_local_maxima, blur_img and blur_diff to legalize the parallel processing, we don't compare it to other implementations.
> * achieve II=2
> * latency: 66622 -> 38246

### opt4 with N = 4
![](https://i.imgur.com/LOUZraa.png)
> Compared to opt4 with N=2, after **doubling N**:
> * achieve II=4
> * latency (cycles): 38246 -> 21316
> * latency (ns): 3.82E5 -> 2.98E5 (cycle time: 10ns -> 14ns)
> * resources usage:
>     * BRAM: 48 -> 56
>     * FF, LUT: roughly 1.6x
>     * DSP: 76 -> 138

### opt5
![](https://i.imgur.com/hQCXHwP.png)
