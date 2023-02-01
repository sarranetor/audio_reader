#pragma once

enum fft { 
    fftOrder = 11, 
    fftSize = 1 << fftOrder, // 2^fftOrder
    scopeSize = 512 // number of points in the visual representation
};