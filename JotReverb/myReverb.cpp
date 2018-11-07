// JotReverb.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "AudioFile.cpp"
#include "myFreeReverb.hpp"
#include <iostream>
#include <vector>
#include <math.h>
using namespace std;

//#define JR_PROCESS
#define OUTPUT
#define LOAD_DATA




int main()
{
#ifdef LOAD_DATA
	// Load audio data
	AudioFile<double> af;
	af.load("dukou_noReverb.wav");
	af.printSummary();
	vector<double> data_left((af.samples.at(0)).begin(), (af.samples.at(0)).end());	// load the left channel data for this test
#endif // LOAD_DATA
	const unsigned int N = 256;							// the number of samples per frame
	vector<double> data_processed;						// a vector wrap the output data

	auto it = data_left.begin();
	vector<double> dFramePack(N);								// wrap the audio data into Frame
	
	myFreeReverb Fb;
	Fb.init();

	// start to process
	cout << "Start to precess\n";
	for (size_t n = 0; n != (data_left.size() / N) * N; n += N) {
		if (it <= data_left.end()) {
			vector<double> dFramePack(it, it + N);
			vector<double> dFrameOut(N);


			// your processing here
			Fb.run_by_frame(dFramePack, dFrameOut);

			for (size_t nn = 0; nn != N; ++nn) {				// output 
				data_processed.push_back(dFrameOut.at(nn));
			}

		}
		it += N;												// update the iterator	
	}


#ifdef JR_PROCESS
														// Initialize and set the Jot reverberator's parameters 
	JotReverb JR;
	JR.init();

	auto it = data_left.begin();
	vector<double> dFramePack(N);							// wrap the audio data into Frame
	for (size_t n = 0; n != (data_left.size()/N) * N; n+=N) {
		if (it <= data_left.end()) {
			vector<double> dFramePack(it, it + N);
			vector<double> dFrameOut(N);
			JR.process(dFramePack, dFrameOut);				// process the frame data
			for (size_t nn = 0; nn != N;++nn) {				// output 
				data_processed.push_back(dFrameOut.at(nn));
			}

		}
		it += N;											// update the iterator	
	}
	cout<< "Processed successful\n";

#endif
#ifdef OUTPUT
	// find max
	//double max = 0;
	//for (auto mData : data_processed) {
	//	if (mData > max)
	//		max = mData;
	//}
	//// 归一化
	//for (auto &mData : data_processed) {
	//	mData = mData / (max +0.1);
	//}
	cout << "Ready to output\n";
	vector<vector<double>> outputBuffer(1);
	outputBuffer.at(0).assign(data_processed.begin(), data_processed.end());
	//outputBuffer.at(1).assign(data_processed.begin(), data_processed.end());
	af.setAudioBuffer(outputBuffer);
	//af.samples.at(0).assign(data_processed.begin(), data_processed.end());
	//af.samples.at(1).assign(data_processed.begin(), data_processed.end());
	af.save("freeReverb_change.wav");
#endif
    return 0;
}