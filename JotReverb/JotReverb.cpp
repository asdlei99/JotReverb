// JotReverb.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "AudioFile.cpp"
#include <iostream>
#include <vector>
#include <math.h>
using namespace std;
//int JotReverb(vector<double>, vector<double>, JotReverbConfig Jconf);
class JotReverb {
public:
	double fs = 48000;
	int nFrame = 256;
	double Un[4] = { 0.4,0.2,0.6,0.8 };
	double J4[4][4] = { { 0,0,1,0 },{ 0,1,0,0 },{ 1,0,0,0 },{ 0,0,0,1} };

	double bi[4] = { 0 };
	double ci[4] = { 0 };
	double gi[4] = { 0 };
	// lpf coef caculate
	double Tr_pi = 0.3;
	double Tr_0 = 1.757;

	double delay_length[4] = { 2205,6615,3087,8820 };

private:
	double An[4][4] = { 0 };			// An = J4 - 1 / 4 * ( Un * Un^T )
	double delay_net[4][8820] = { 0 };	// 8820 = max(delay_length)
	unsigned int delay_pos[4] = { 0 };	// current position of delay line per channle 每个通道延时线当前的指向位置 
	double lpf_cache[4] = { 0 };
	double out_cache[4] = {0};

public:
	void updateAn();					// compute the matrix An
	double updateLpfCoeff();			// compute the lpf coeff
	int init();
	int delay_by_samples(double in, double after_delay, unsigned int cur_delay_pos, unsigned int N, double *delay_line);
	int process(vector<double> data_in, vector<double>&data_out);


};
int main()
{
	// Load audio data
	AudioFile<double> af;
	af.load("audioCut_2.wav");
	af.printSummary();
	vector<double> data_left((af.samples.at(0)).begin(), (af.samples.at(0)).end());

	unsigned int nFrameCount = 0;
	const unsigned int N = 256;	// number of samples per frame
	vector<double> data_processed;
	JotReverb JR;
	JR.init();
	int ttt = 0;
	// Pack the audio data into Frame
	vector<double> dFramePack(N);
	for (size_t n = 0; n != 1025; n++) {
		if (n%N != 0) {
			dFramePack.at(n%N) = data_left.at(n);
		}
		else {
			vector<double> dFrameOut(N);
			JR.process(dFramePack, dFrameOut);
			for (auto data : dFrameOut) {
				data_processed.push_back(data);
			}
		}

		//vector<double> dFramePack(data_left.begin() + n, data_left.begin() + n + N);
		//double *dFrameOut = new double[nFrame];
		//memcpy(dFramePack, (&data_left) + n, sizeof(double) * nFrame);	// load the data to the Frame pack for processing

		ttt++;
		//data_processed.assign(dFrameOut.begin(), dFrameOut.begin()+n);
	}
	cout << "Processed successful\n";
	af.samples.at(0).assign(data_processed.begin(), data_processed.end());
	af.samples.at(1).assign(data_processed.begin(), data_processed.end());
	af.save("JotReverb_output.wav");
    return 0;
}

void JotReverb::updateAn()
{
	/*
		An = J4 - 2/n *(Un*Un');
	*/
	unsigned int nChannel = 4;
	double sum_temp = 0;
	for (size_t nCount = 0; nCount != nChannel; ++nCount) {
		sum_temp += Un[nCount] * Un[nCount];											// compute the Un*Un'
	}
	for (size_t nCloumn = 0; nCloumn != nChannel; ++nCloumn) {
		for (size_t nRow = 0; nRow != nChannel; ++nRow) {
			An[nRow][nCloumn] = J4[nRow][nCloumn] - 2 / (double)nChannel * sum_temp;	//An = J4 - 2/n *(Un*Un');
		}
	}
}

int JotReverb::process(vector<double> data_in, vector<double>& data_out)
{
	//unsigned int nChannel = 4;

	double An_out[4] = { 0 };
	double sum_an_bx[4] = { 0 };
	double after_delay[4] = { 0 };
	double s[4] = { 0 };
	for (size_t nCount = 0; nCount != nFrame; ++nCount) {

		for (size_t nChannel = 0; nChannel != 4; ++nChannel) {
			sum_an_bx[nChannel] = An_out[nChannel] + bi[nChannel] * data_in[nChannel];
			// delay
			delay_by_samples(sum_an_bx[nChannel], after_delay[nChannel], delay_pos[nChannel], delay_length[nChannel], delay_net[nChannel]);
			// lpf
			s[nChannel] = after_delay[nChannel] * gi[nChannel]*(1 - bi[nChannel]) + bi[nChannel] * lpf_cache[nChannel];
			// out
			out_cache[nChannel] += ci[nChannel] * s[nChannel];		// c*s
		}
		// 计算矩阵
		for (size_t row = 0; row != 4; ++row) {
			// compute the s * An 
			double An_out_temp = 0;
			for (size_t j = 0; j != 4; ++j) {
				An_out_temp += An[row][j] * s[j];
			}
			An_out[row] = An_out_temp;

			data_out.at(nCount) += out_cache[row];			// sum( c*s) and output
			//data_out.push_back()
		}
		
		
	}
	return 0;
}

int JotReverb::delay_by_samples(double in, double after_delay, unsigned int cur_delay_pos, unsigned int N,double *delay_line)
{
	// N : delay length
	// the delay_by_samples functin is based on the circular queue
	after_delay = delay_line[cur_delay_pos];		// pop the dealyed data
	delay_line[cur_delay_pos] = in;					// push data into the delay line
	cur_delay_pos = (cur_delay_pos + 1) % N;	// update the pos
	return 0;
}

double JotReverb::updateLpfCoeff()
{
	double alpha = Tr_pi / Tr_0;
	double T = 1 / fs;

	for (size_t i = 0; i != 4; ++i) {
		// mi = delay_length[i]
		gi[i] =pow( 10 , (-3 * delay_length[i] * T  / Tr_0) );				// % lpf coef	
		bi[i] = 1 - 2.0 / (1 + pow(gi[i], (1 - 1 / alpha)) );			//  % lpf coef
	}
	return 0.0;
}

int JotReverb::init()
{
	updateAn();		// compute the matrix An
	updateLpfCoeff();	// compute the lpf coeff
	return 0;
}
