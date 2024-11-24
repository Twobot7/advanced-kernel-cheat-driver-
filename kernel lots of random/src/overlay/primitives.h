#pragma once
#include "../common/types.h"

// Drawing functions
NTSTATUS DrawLine(PPOINT2D Start, PPOINT2D End, COLOR_RGBA Color);
NTSTATUS DrawRect(PRECT2D Rect, COLOR_RGBA Color, BOOLEAN Filled);
NTSTATUS DrawCircle(PPOINT2D Center, ULONG Radius, COLOR_RGBA Color, BOOLEAN Filled);
NTSTATUS DrawTriangle(PPOINT2D P1, PPOINT2D P2, PPOINT2D P3, COLOR_RGBA Color, BOOLEAN Filled);
