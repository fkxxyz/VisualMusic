#pragma once
#include "stdafx.h"

// 控件间距
#define VIEW_CONTROL_SPACING           4

// 按钮高度
#define VIEW_BUTTON_HEIGHT             30


// 每个频率显示的宽度（像素）
#define VIEW_FREQ_PIXEL_WIDTH          4

// 每个时刻显示的高度（像素）
#define VIEW_TIME_PIXEL_HEIGHT         4


// 要显示的频率数量
#define N_FREQ                         120

// 要显示的时刻数量
#define N_TIME                         60


// 当前时刻所在位置（相对数量）
#define CUR_TIME_POS                   10


// 时间精度，帧率（每秒帧数）
#define FRAME_RATE                     60


// 将一个圆分割的份数用于求三角函数值。
//   此值越大，计算时用到的三角函数计算越精确。
//   此值越大，占用内存越大，占用的内存为其值乘以 sizeof(double)*2
#define MAX_N_DIV_CIRCLE               65536


// 检测窗口内最少包含的周期数
//   此值越小，低频计算时可能会导致失去信号本来的圆滑样子。
//   此值越大，低频计算时时间刻度方向上不够精确。
//   需要通过实验得出最佳值
#define MIN_WIN_N_VIB                  4


// 检测窗口内最多包含的周期数
//   此值越小，高频计算时频率刻度方向上不够精确。
//   此值越大，高频计算时时间刻度方向上不够精确。
//   需要通过实验得出最佳值
#define MAX_WIN_N_VIB                  128


// 将一个周期最大分割的次数的指数
// 即：以2为底其值作为指数得到的结果为最大振动次数将一个周期最大分割的次数
//   此值越大，低频计算得越精确
//   此值越大，计算要用的时间越长，每增加 1 计算时间翻倍
#define MAX_T_SEG_P                    7




