#pragma once
#include "stdafx.h"

// 控件间距
#define VIEW_CONTROL_SPACING           4

// 按钮高度
#define VIEW_BUTTON_HEIGHT             30


// 每个频率显示的宽度（像素）
#define VIEW_FREQ_PIXEL_WIDTH          8

// 每个时刻显示的高度（像素）
#define VIEW_TIME_PIXEL_HEIGHT         8


// 要显示的频率数量
#define N_FREQ                         120

// 要显示的时刻数量
#define N_TIME                         60

// 最大频率数量
#define MAX_N_FREQ                     120


// 当前时刻所在位置（相对数量）
#define CUR_TIME_POS                   10


// 时间精度，帧率（每秒帧数）
//   实际上当采样率除以此数值不是整数时，这个数会不精确（偏大）
#define FRAME_RATE                     60


// 将一个圆分割的份数用于求三角函数值。
//   此值越大，计算时用到的三角函数计算越精确。
//   此值越大，占用内存越大，占用的内存为其值乘以 sizeof(double)*2
#define MAX_N_DIV_CIRCLE               32768


// 检测窗口内最少包含的周期数
//   此值越小，低频计算时可能会导致失去信号本来的圆滑样子。
//   此值越大，低频计算时时间刻度方向上不够精确。
//   需要通过实验得出最佳值
#define MIN_WIN_N_VIB                  16


// 检测窗口内最多包含的周期数
//   此值越小，高频计算时频率刻度方向上不够精确。
//   此值越大，高频计算时时间刻度方向上不够精确。
//   需要通过实验得出最佳值
#define MAX_WIN_N_VIB                  32


// 最大采样率
//   此值越大，频谱分析器占用的栈空间越多，过大可能导致栈溢出
#define MAX_SAMPLE_RATE                44100


// 最低、最高频率范围
#define MIN_FREQ_RANGE                 20
#define MAX_FREQ_RANGE                 20000


// 最低频率（计算后得出）
#define MIN_FREQ                       20.60172230705432


// 最大每帧样点数
//   该值由最大采样率和帧率自动算出，影响着所需要的栈空间
#define MAX_FRAME_SAMPLE static_cast<int>(MAX_SAMPLE_RATE / FRAME_RATE)


// 分析器分析时缓冲区最小包含的帧数
#define MIN_FRAME_N_BUFFER (static_cast<int>(static_cast<double>(MAX_SAMPLE_RATE) / MIN_FREQ * MIN_WIN_N_VIB / MAX_FRAME_SAMPLE + 2) * 2)



#define DEBUG_I_FREQ                   65
#define DEBUG_BLOCK                    500



