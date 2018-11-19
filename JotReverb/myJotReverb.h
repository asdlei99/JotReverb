// Jot's reverb based FDN
#pragma once
#include "stdafx.h"
#include <math.h>
#include "FDN.h"
class mJotReverb:public FDN
{
public:
	// lpf coef 
	double *after_lpf;
	double *lpf_cache;
	double *bi ;
	double *gi ;
	double Tr_pi = 0.3;
	double Tr_0 = 1.757;

	int updateLpfCoeff();
	int default_init();
	virtual double run_by_sample(double data_in);

	mJotReverb();;
	virtual ~mJotReverb();;
private:

};

