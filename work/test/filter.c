#include <stdio.h>

void ft(int *data)
{
	data[1] -= (5*data[0] + 15*data[2] - 5*data[4] + data[6]) >> 4;
	data[3] -= (-1*data[0] + 9*data[2] + 9*data[4] - data[6]) >> 4;
	data[5] -= (1*data[0] - 5*data[2] + 15*data[4] + 5*data[6]) >> 4;
	data[7] -= (-5*data[0] + 21*data[2] - 35*data[4] + 35*data[6]) >> 4;

	data[0] -= (35*data[1] - 35*data[3] + 21*data[5] - 5*data[7]) >> 4;
	data[2] -= (5*data[1] + 15*data[3] - 5*data[5] + 1*data[7]) >> 4;
	data[4] -= (-1*data[1] + 9*data[3] + 9*data[5] - 1*data[7]) >> 4;
	data[6] -= (1*data[1] - 5*data[3] + 15*data[5] + 5*data[7]) >> 4;
}

void it(int *data)
{
	data[0] += (35*data[1] - 35*data[3] + 21*data[5] - 5*data[7]) >> 4;
	data[2] += (5*data[1] + 15*data[3] - 5*data[5] + 1*data[7]) >> 4;
	data[4] += (-1*data[1] + 9*data[3] + 9*data[5] - 1*data[7]) >> 4;
	data[6] += (1*data[1] - 5*data[3] + 15*data[5] + 5*data[7]) >> 4;

	data[1] += (5*data[0] + 15*data[2] - 5*data[4] + data[6]) >> 4;
	data[3] += (-1*data[0] + 9*data[2] + 9*data[4] - data[6]) >> 4;
	data[5] += (1*data[0] - 5*data[2] + 15*data[4] + 5*data[6]) >> 4;
	data[7] += (-5*data[0] + 21*data[2] - 35*data[4] + 35*data[6]) >> 4;
}

void pack(int *data)
{
	int temp[8], x;
	for (x = 0; x < 4; x++) {
		temp[x] = data[x+x+0];
		temp[x+4] = data[x+x+1];
	}
	for (x = 0; x < 8; x++)
		data[x] = temp[x];
}

void depack(int *data)
{
	int temp[8], x;
	for (x = 0; x < 4; x++) {
		temp[x+x+0] = data[x];
		temp[x+x+1] = data[x+4];
	}
	for (x = 0; x < 8; x++)
		data[x] = temp[x];
}

main()
{
	int buf[8], x;

/*	clrscr(); */
	for (x = 0; x < 8; x++)
		buf[x] = x;
	for (x = 0; x < 8; x++) printf("%d, ", buf[x]); printf("\n");
	ft(buf); pack(buf);
	for (x = 0; x < 8; x++) printf("%d, ", buf[x]); printf("\n");
	depack(buf); it(buf);
	for (x = 0; x < 8; x++) printf("%d, ", buf[x]); printf("\n");
}
