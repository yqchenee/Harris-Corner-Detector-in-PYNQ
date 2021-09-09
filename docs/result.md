# Results

## Origin Design (Opt1)

![](https://i.imgur.com/21pJP09.png)


## Optimize Using Unrolling (Opt2)
![](https://i.imgur.com/p9CAjBd.png)

## Optimize Using Unrolling and Pipeline (Opt3)

![](https://i.imgur.com/znOluvo.png)

## Comparison Table

result on 256*256 size image :
| design   | latency(ms) | BRAM | DSP | FF | LUT|
| -------- | -------- | -------- |  -------- | -------- | ----|
| kernel_opt1 | 10.65   | 112 | 2 | 14384 | 15070 |
| kernel_opt2 | 0.768 | 164  | 4 | 14872 | 15630 |
| kernel_opt3 | 0.66 | 164  | 2| 17557 | 15446 |
| CPU [1] | 235 | x | x | x | x |

[1] We use Intel(R) Xeon(R) CPU E5-2650 v2 @ 2.60GHz.
