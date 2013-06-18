#ifndef SPEECHPROC_H
#define SPEECHPROC_H

#include <string>

namespace Aquila {
    class SignalSource;
    class Spectrogram;
    class FramesCollection;
}

#define MaxNres           6 /* max quantity of recognition decisions */
#define MaxNcadr          6144
typedef struct {
  int R;         /* distance to pattern */
  int pos[2];    /* phoneme begin & end (in shots) */
  int key;       /* additional information */
} CONTRES;

Aquila::SignalSource * procHighpassFilter(Aquila::SignalSource *);
Aquila::SignalSource * procHamming(Aquila::SignalSource *);
Aquila::Spectrogram  * procSpectrum(Aquila::SignalSource *);
Aquila::SignalSource * procChannels(Aquila::Spectrogram *);
Aquila::SignalSource * procParams(Aquila::FramesCollection *);

Aquila::SignalSource * proc(std::string);
int compare(Aquila::SignalSource *, Aquila::SignalSource *);

bool AdjustBandIndexTab(int * indTab, int size, double resolution);
bool MakeLinIndexTab(double minFreq, double maxFreq, double smpFreq, int fftOrder, int * indTab, int tabSize);
bool DivideIntoZones(double * inBank, int inSize, double * outBank, int bandNum, int * indTab, int overFact);
int TransformVector(const double * pIn, double *pOut);
double FindGrCenter(double *dst,double l,double r);
void FreqNorm(double *pIn, double *pOut);
long GetDist(const double *ptn,const double *spk,int num);
int DynTimeWarping(const double * prlz, int rlzSize, const double * pptn, int ptnSize, CONTRES* pcres);

#endif // SPEECHPROC_H
