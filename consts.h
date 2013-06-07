#ifndef CONSTS_H
#define CONSTS_H

const double CHighpassFilterConst = 0.5;

const int CSpectrogramFrameSize = 512;
const int CSpectrogramFrameOverlapSize = CSpectrogramFrameSize/4*3;

const int SpkVectSize = 32;
const int FrmVectSize = 6;

const int startFreq = 0;
const int endFreq = 8000;
const int smplFreq = 8000;
const int fftOrder = 8;
const int bandNumb = 32;
const int overFact = 2;

const double Fmin[FrmVectSize/2]={1,4,16};
const double Fmax[FrmVectSize/2]={7,29,31};

const double Amin[FrmVectSize/2]={0, 8,  24};
const double Amax[FrmVectSize/2]={8, 24, 31};

#endif // CONSTS_H
