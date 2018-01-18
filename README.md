# Unscented Kalman Filter Project
*Self-Driving Car Engineer Nanodegree Program*

## Summary
For this project C++ was used to write an implementation of an Unscented Kalman Filter that takes in radar and lidar data to track position. This is a more advanced and accurate method than the Extended Kalman filter in the [previous](https://github.com/cpdis/P6_CarND_Extended_Kalman_Filter) project.

The `src` folder contains all of the code for this project. 

## Build Instructions
The main program can be built and ran by doing the following from the project top directory.

1. mkdir build
2. cd build
3. cmake ..
4. make
5. ./UnscentedKF

## Results
Based on the provided data, the Unscented Kalman Filter produces the following results. The x position is shown as `px`, the y position as `py`, the velocity in the x direction is `vx`, and velocity in the y direction is `vy`.

| Input |  RMSE   |
| ----- | ------- |
|  px   | 0.0786  |
|  py   | 0.0846  |
|  vx   | 0.3243  |
|  vy   | 0.2997  |