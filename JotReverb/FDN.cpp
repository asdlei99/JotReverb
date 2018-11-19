#include "stdafx.h"
#include "FDN.h"

////

inline void FDN::setJn() {
	// default Jn initialized method
	// initialize Jn
	Jn = new double *[nChannel] {};
	for (size_t n = 0; n != nChannel; ++n) {
		Jn[n] = new double[nChannel] {};
		Jn[n][nChannel - n] = 1;
	}
}

inline void FDN::setUn() {
	// default Un initialized method
	// initialize Un
	Un = new double[nChannel];
	for (size_t i = 0; i < nChannel; i++)
	{
		Un[i] = 1;
	}
}

inline void FDN::setAn() {
	/*
	An = J4 - 2/n *(Un*Un');
	*/
	// default An initialized method
	// initialize An
	An = new double* [nChannel] {};
	for (size_t n = 0; n < nChannel; n++)
	{
		An[n] = new double[nChannel] {};
	}
	// compute the matix Un*Un'	,note that Un is a column vector
	// assume the Un has 4 rows
	double temp_matrix[4][4] = { 0 };
	for (size_t nRow = 0; nRow != 4; ++nRow) {
		for (size_t nColumn = 0; nColumn != 4; ++nColumn) {
			temp_matrix[nRow][nColumn] = Un[nColumn] * Un[nColumn];
		}
	}
	for (size_t nRow = 0; nRow != nChannel; ++nRow) {
		for (size_t nColumn = 0; nColumn != nChannel; ++nColumn) {
			An[nRow][nColumn] = Jn[nRow][nColumn] - 2 / (double)nChannel * temp_matrix[nRow][nColumn];	// An = J4 - 2/n *(Un*Un');
		}
	}
}

inline void FDN::setBn(double * _Bn) {
	Bn = new double[nChannel] {0};
	std::memcpy(Bn, _Bn, sizeof(double)*nChannel);
}

inline void FDN::setCn(double * _Cn) {
	Cn = new double[nChannel] {0};
	std::memcpy(Cn, _Cn, sizeof(double)*nChannel);
}

inline void FDN::setGn(double * _Gn) {
	Gn = new double[nChannel] {0};
	std::memcpy(Gn, _Gn, sizeof(double)*nChannel);
}

inline void FDN::setDelayLine(unsigned int * _delay_length) {
	// initialize the delay_line
	delay_length = new unsigned int[nChannel];
	std::memcpy(delay_length, _delay_length, sizeof(unsigned int)*nChannel);
	delay_line = new DelayLine[nChannel]{};
	for (size_t n = 0; n < nChannel; n++)
	{
		delay_line[n].init(delay_length[n]);
	}
}

inline int FDN::init_fdn(unsigned int num_of_channels, double * _Bn, double * _Cn, double * _Gn, unsigned int * _delay_length) {
	nChannel = num_of_channels;
	setBn(_Bn);
	setCn(_Cn);
	setGn(_Gn);

	setUn();
	setJn();
	setAn();
	setDelayLine(_delay_length);

	sum_of_an = new double[nChannel] {};
	after_delay = new double[nChannel] {};
	return 0;
}

inline double FDN::run_by_sample(double data_in) {

	for (size_t n = 0; n < nChannel; n++)
	{
		delay_line[n].delay_by_sample(Bn[n] * data_in + Gn[n] * sum_of_an[n], after_delay[n]);
	}
	//	update sum_of_an
	for (size_t nRow = 0; nRow < nChannel; nRow++)
	{
		double sum_temp = 0;
		for (size_t nCloumn = 0; nCloumn < nChannel; nCloumn++)
		{
			//sum_temp = sum_temp +after_delay[nCloumn] * An[nRow][nCloumn];
			sum_temp += after_delay[nCloumn] * An[nRow][nCloumn];
		}
		sum_of_an[nRow] = sum_temp;
	}
	//	output
	double output_temp = 0;
	for (size_t n = 0; n < nChannel; n++)
	{
		output_temp += after_delay[n] * Cn[n];
	}
	return output_temp + fdn_dw *data_in;
}

inline void FDN::run_by_frame(std::vector<double> data_in, std::vector<double>& data_out) {
	for (size_t n = 0; n < data_in.size(); n++)
	{
		data_out.at(n) = run_by_sample(data_in.at(n));
	}
}

inline FDN::FDN() {}

inline FDN::~FDN() {
	delete[] Jn;
	delete[] An;
	delete[] Bn;		// pre gain,b
	delete[] Cn;
	delete[] Gn;
	delete[] Un;
	delete[] sum_of_an;
	delete[] after_delay;
}
