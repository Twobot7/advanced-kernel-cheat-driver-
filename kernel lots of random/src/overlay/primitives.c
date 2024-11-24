#include "primitives.h"
#include "renderer.h"
#include "../common/utils.h"

NTSTATUS DrawLine(PPOINT2D Start, PPOINT2D End, COLOR_RGBA Color) {
    if (!Start || !End) {
        return STATUS_INVALID_PARAMETER;
    }

    VERTEX vertices[2];
    
    // Start point
    vertices[0].x = (FLOAT)Start->x;
    vertices[0].y = (FLOAT)Start->y;
    vertices[0].u = 0.0f;
    vertices[0].v = 0.0f;
    vertices[0].color = Color;

    // End point
    vertices[1].x = (FLOAT)End->x;
    vertices[1].y = (FLOAT)End->y;
    vertices[1].u = 1.0f;
    vertices[1].v = 1.0f;
    vertices[1].color = Color;

    return DrawVertices(vertices, 2);
}

NTSTATUS DrawRect(PRECT2D Rect, COLOR_RGBA Color, BOOLEAN Filled) {
    if (!Rect) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Filled) {
        // For filled rectangles, we use two triangles
        VERTEX vertices[6];
        
        // First triangle
        vertices[0].x = (FLOAT)Rect->x;
        vertices[0].y = (FLOAT)Rect->y;
        vertices[1].x = (FLOAT)(Rect->x + Rect->width);
        vertices[1].y = (FLOAT)Rect->y;
        vertices[2].x = (FLOAT)Rect->x;
        vertices[2].y = (FLOAT)(Rect->y + Rect->height);

        // Second triangle
        vertices[3].x = (FLOAT)(Rect->x + Rect->width);
        vertices[3].y = (FLOAT)Rect->y;
        vertices[4].x = (FLOAT)(Rect->x + Rect->width);
        vertices[4].y = (FLOAT)(Rect->y + Rect->height);
        vertices[5].x = (FLOAT)Rect->x;
        vertices[5].y = (FLOAT)(Rect->y + Rect->height);

        // Set color and UV coordinates for all vertices
        for (int i = 0; i < 6; i++) {
            vertices[i].color = Color;
            vertices[i].u = (i % 3) == 0 ? 0.0f : 1.0f;
            vertices[i].v = i < 3 ? 0.0f : 1.0f;
        }

        return DrawVertices(vertices, 6);
    } else {
        // For outlined rectangles, we use 4 lines
        POINT2D points[5];
        points[0].x = Rect->x;
        points[0].y = Rect->y;
        points[1].x = Rect->x + Rect->width;
        points[1].y = Rect->y;
        points[2].x = Rect->x + Rect->width;
        points[2].y = Rect->y + Rect->height;
        points[3].x = Rect->x;
        points[3].y = Rect->y + Rect->height;
        points[4].x = Rect->x;
        points[4].y = Rect->y;

        NTSTATUS status;
        for (int i = 0; i < 4; i++) {
            status = DrawLine(&points[i], &points[i + 1], Color);
            if (!NT_SUCCESS(status)) {
                return status;
            }
        }
        return STATUS_SUCCESS;
    }
}

NTSTATUS DrawCircle(PPOINT2D Center, ULONG Radius, COLOR_RGBA Color, BOOLEAN Filled) {
    if (!Center || Radius == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    const INT segments = 32; // Number of segments to approximate the circle
    VERTEX* vertices;
    ULONG vertexCount;

    if (Filled) {
        vertexCount = (segments + 1) * 3;
        vertices = ExAllocatePool2(POOL_FLAG_NON_PAGED, vertexCount * sizeof(VERTEX), 'CIRC');
    } else {
        vertexCount = segments + 1;
        vertices = ExAllocatePool2(POOL_FLAG_NON_PAGED, vertexCount * sizeof(VERTEX), 'CIRC');
    }

    if (!vertices) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    FLOAT angleStep = 2.0f * 3.14159f / segments;
    
    if (Filled) {
        for (INT i = 0; i < segments; i++) {
            FLOAT angle = i * angleStep;
            FLOAT nextAngle = (i + 1) * angleStep;

            // Center vertex
            vertices[i * 3].x = (FLOAT)Center->x;
            vertices[i * 3].y = (FLOAT)Center->y;
            vertices[i * 3].color = Color;

            // Current point
            vertices[i * 3 + 1].x = (FLOAT)Center->x + cosf(angle) * Radius;
            vertices[i * 3 + 1].y = (FLOAT)Center->y + sinf(angle) * Radius;
            vertices[i * 3 + 1].color = Color;

            // Next point
            vertices[i * 3 + 2].x = (FLOAT)Center->x + cosf(nextAngle) * Radius;
            vertices[i * 3 + 2].y = (FLOAT)Center->y + sinf(nextAngle) * Radius;
            vertices[i * 3 + 2].color = Color;
        }
    } else {
        for (INT i = 0; i <= segments; i++) {
            FLOAT angle = i * angleStep;
            vertices[i].x = (FLOAT)Center->x + cosf(angle) * Radius;
            vertices[i].y = (FLOAT)Center->y + sinf(angle) * Radius;
            vertices[i].color = Color;
        }
    }

    NTSTATUS status = DrawVertices(vertices, vertexCount);
    ExFreePoolWithTag(vertices, 'CIRC');
    return status;
}

NTSTATUS DrawTriangle(PPOINT2D P1, PPOINT2D P2, PPOINT2D P3, COLOR_RGBA Color, BOOLEAN Filled) {
    if (!P1 || !P2 || !P3) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Filled) {
        VERTEX vertices[3];

        vertices[0].x = (FLOAT)P1->x;
        vertices[0].y = (FLOAT)P1->y;
        vertices[1].x = (FLOAT)P2->x;
        vertices[1].y = (FLOAT)P2->y;
        vertices[2].x = (FLOAT)P3->x;
        vertices[2].y = (FLOAT)P3->y;

        for (int i = 0; i < 3; i++) {
            vertices[i].color = Color;
            vertices[i].u = (i == 0) ? 0.0f : (i == 1) ? 1.0f : 0.5f;
            vertices[i].v = (i == 0) ? 0.0f : (i == 1) ? 0.0f : 1.0f;
        }

        return DrawVertices(vertices, 3);
    } else {
        NTSTATUS status;
        
        status = DrawLine(P1, P2, Color);
        if (!NT_SUCCESS(status)) return status;
        
        status = DrawLine(P2, P3, Color);
        if (!NT_SUCCESS(status)) return status;
        
        return DrawLine(P3, P1, Color);
    }
}
