// JotReverb.cpp: 定义控制台应用程序的入口点。
//
#pragma once
#include "stdafx.h"
#include "AudioFile.cpp"
#include "FDN.cpp"
#include "myFreeReverb.hpp"
#include <iostream>
#include <vector>
#include <math.h>

#include "FenderReverb.hpp"
#include "myJotReverb.cpp"
#include "DelayLine.hpp"

using namespace std;

#define OUTPUT
#define LOAD_DATA
#define NORMALIZE

int main()
{
#ifdef OUTPUT
	//string output_file_name = "mJotReverb_ch8_v2.wav";
	string output_file_name = "mJr v6.wav";
#endif // OUTPUT

#ifdef LOAD_DATA
	// Load audio data
	AudioFile<double> af;
	//af.load("ideal_impulse_48k.wav");
	//af.load("dukou_noReverb.wav");
	af.load("audioCut_2.wav");
	af.printSummary();
	vector<double> data;			// load mono data for test
	if (af.getNumChannels() > 1)
	{
		for (size_t nCount = 0; nCount != af.getNumSamplesPerChannel(); ++nCount) {
			data.push_back( (af.samples.at(0).at(nCount) + af.samples.at(1).at(nCount)) *0.5 );
		}
	}
	else
		data.assign( (af.samples.at(0)).begin(), (af.samples.at(0)).end() );	
#endif // LOAD_DATA

	// ================  initialize the reverb unit  ================//
	//myFreeReverb Fb;
	//Fb.init();

	//JotReverb Jb;
	//Jb.init();

	mJotReverb mJb;
	mJb.default_init();

	FenderRev FR;
	FR.FR_init(af.getSampleRate());

	for (size_t i = 0; i < 8; i++)
	{
		//cout << "B C G";
		//cout << FR.Bn[i] << " = " <<
		//	FR.Cn[i] << " = " <<
		//	FR.Gn[i] << " = " <<
		//	FR.delay_length[i] << endl;
		for (size_t k = 0; k < 8; k++)
		{
			cout << FR.An[i][k] << " ";
		}
		cout << endl;
	}

	//FDN Fn;
	//size_t num_of_channel = 8;	// num_of_channel
	//Fn.init_fdn(	4,								// number of channels
	//			new double[num_of_channel]{ 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25  },	// b
	//			new double[num_of_channel]{ 1, 1, 1, 1, 1, 1, 1, 1 },	// c
	//			new double[num_of_channel]{ 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25 },	// g
	//			new unsigned int[num_of_channel]{ 920,998,1062,1094, 606,696,779,844  }	// delay line length
	//		);

	//EarlyReverb ER;
	//ER.init(new double[3]{0.014401440346511,0.028802880693022,0.014401440346511},	// lpf'b
	//		new double[3]{1,-1.632993161855452,0.690598923241497},					// lpf'a
	//		2400,																	// total delay length
	//		new double[5]{1901,2011,2113,2203,2333}									// 5 channels		
	//		);	
	////ER.ER_init(new double[3]{0.4566722944641210 * 1.2171,0,0},		// lpf'b
	////		new double[3]{1,-0.444124594801,0},					// lpf'a
	////		2400,																	// total delay length
	////		new double[5]{1901,2011,2113,2203,2333}									// 5 channels		
	////		);

	// ================  initialize the processing   ================//
	const unsigned int N = 256;							// the number of samples per frame
	vector<double> data_processed;						// a vector wrap the output data
	auto it = data.begin();
	vector<double> dFramePack(N);						// wrap the audio data into Frame
	vector<double> dFrameOut(N);						// wrap the processed data

	// start to process
	std::cout<< "Start to precess\n";
	vector<double> temp(N);
	for (size_t n = 0; n != (data.size() / N) * N; n += N) {
		if (it <= data.end()) {
			dFramePack.assign(it, it + N);
			// your processing here

			FR.run_by_frame(dFramePack, dFrameOut);
			//mJb.run_by_frame(dFramePack, dFrameOut);

			// output 
			data_processed.insert(data_processed.end(), dFrameOut.begin(), dFrameOut.end());
			//for (auto per_data:dFrameOut) {				
			//	data_processed.push_back(per_data);
			//}

		}
		it += N;										// update the iterator	to next frame
	}

#ifdef OUTPUT
#ifdef NORMALIZE
	 //find max
	double max = 0;
	for (auto mData : data_processed) {
		if (mData > max)
			max = mData;
	}
	// normalized
	for (auto &mData : data_processed) {
		mData = mData / (max +0.1);
	}
#endif // NORMALIZE

	cout << "Ready to output\n";
	vector<vector<double>> outputBuffer(1);
	outputBuffer.at(0).assign(data_processed.begin(), data_processed.end());
	//outputBuffer.at(1).assign(data_processed.begin(), data_processed.end());
	af.setAudioBuffer(outputBuffer);
	//af.samples.at(0).assign(data_processed.begin(), data_processed.end());
	//af.samples.at(1).assign(data_processed.begin(), data_processed.end());
	af.save(output_file_name);
#endif
    return 0;
}