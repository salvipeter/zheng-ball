# Zheng-Ball
An implementation of the [Zheng-Ball multi-sided surface](https://doi.org/10.1016/S0167-8396(97)00007-1).

Prerequisites:
- My [geometry library](https://github.com/salvipeter/libgeom/)
- My [derivative-free optimization library](https://github.com/salvipeter/dfo/)

There are 4 programs included, as detailed below.

## `3sided` and `nsided`

These utilities generate the parameterization for 3, 5 or 6-sided patches with a given resolution.
The output is like a Wavefront Object file containing vertices (of dimensions 3, 5 and 6).

Usage:
```
3sided <resolution> [filename]
nsided <# of sides> <resolution> [filename]
```

## `gbp2zhb`

This is a converter between my [Generalized Bézier Patch (GBP) format](https://github.com/salvipeter/transfinite)
and the Zheng-Ball (ZHB) format used by this package. The latter has a very simple structure:

```
<# of sides> <degree>
<1st CP index> <1st CP position>
<2nd CP index> <2nd CP position>
...
```

For example:

```
5 3                                                                                                 
0 3 3 3 0 0.019329 11.8409 8.48475
0 2 3 3 1 -1.51948 45.793 26.2797
0 1 3 3 2 -5.44776 86.7114 28.1235
0 0 3 3 3 -7.25685 116.377 5.19098
1 0 2 3 3 -39.1466 114.269 1.88092
2 0 1 3 3 -70.2844 119.283 -3.11562
3 0 0 3 3 -100 130.84 -10.1656
3 1 0 2 3 -100.631 70.647 15.4956
3 2 0 1 3 -103.893 -26.4528 2.59484
3 3 0 0 3 -114.857 -96.0908 -21.5742
3 3 1 0 2 -98.3582 -96.8019 -2.99034
3 3 2 0 1 -75.7665 -98.4993 10.577
3 3 3 0 0 -52.7903 -100 8.47485
2 3 3 1 0 -58.4356 -53.8188 9.18527
1 3 3 2 0 -47.7592 11.2259 8.50383
1 2 2 2 1 -39.6258 34.2415 25.6299
1 1 2 2 2 -41.7351 77.7256 25.2927
2 1 1 2 2 -72.1417 72.7119 22.8152
2 2 1 1 2 -89.4007 -35.899 13.1435
2 2 2 1 1 -72.503 -43.197 15.6582
```

The converter just needs the names of the input and output files:

```
gbp2zhb <input.gbp> <output.zhb>
```

## `zheng-ball`

This program generates the actual patch and writes the tessellation into a Wavefront Object file.
By default it looks for a file called `3sided.obj`, `5sided.obj` or `6sided.obj` as parameterization,
but different files can also be supplied.

Usage:
```
zheng-ball <input.zhb> <output.obj> [parameters.obj]
```
