#ifndef _LIBRARY_H_
#define _LIBRARY_H_
#include <Windows.h>
#include <vector>
#ifndef loop
#define loop(v, m) for(UINT (v) = 0; (v) < (m); (v)++)
#endif

void CalcHist(UINT* pixels, UINT stride, UINT x_max, UINT y_max, std::vector<UINT>* red, std::vector<UINT>* green, std::vector<UINT>* blue)
{
	loop(y, y_max)
	{
		UINT ys = y * stride;
		
		loop(x, x_max)
		{
			UINT color = pixels[ys + x];
			red[0][(color & 0xFF0000) >> 16]++;
			green[0][(color & 0xFF00) >> 8]++;
			blue[0][color & 0xFF]++;
		}
	}

	UINT max = 0;
	loop(i, red[0].size()) if (red[0][i] > max) max = red[0][i];
	loop(i, red[0].size()) red[0][i] = red[0][i] * 255 / max;

	max = 0;
	loop(i, green[0].size()) if (green[0][i] > max) max = green[0][i];
	loop(i, green[0].size()) green[0][i] = green[0][i] * 255 / max;

	max = 0;
	loop(i, blue[0].size()) if (blue[0][i] > max) max = blue[0][i];
	loop(i, blue[0].size()) blue[0][i] = blue[0][i] * 255 / max;
}

#endif