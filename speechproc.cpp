#include "speechproc.h"

#include <QVector>

#include "consts.h"

#include "aquila/global.h"
#include "aquila/source/WaveFile.h"
#include "aquila/source/SignalSource.h"
#include "aquila/source/window/HammingWindow.h"
#include "aquila/transform/FftFactory.h"
#include "aquila/source/Frame.h"
#include "aquila/source/FramesCollection.h"
#include "aquila/transform/Spectrogram.h"
#include "aquila/functions.h"

Aquila::SignalSource * procHighpassFilter(Aquila::SignalSource * source)
{
    QVector<Aquila::SampleType> data;
    const int step = 16;
    int size = source->getSamplesCount();
    for (int i=0; i<size; i++)
    {
        double res = 0.0;
        for(int j=0; j<step; j++){
            if(i-j < 0) res += source->sample(0);
            else res += source->sample(i-j);
            if(i+j >= size) res += source->sample(size-1);
            else res += source->sample(i+j);
        }
        Aquila::SampleType val = source->sample(i) - (res / (step * 2));
        if(val > 32767.0)
           val = 32767.0;
        if(val< -32767.0)
           val =-32767.0;
        data.append(val);
    }
    return new Aquila::SignalSource(data.toStdVector(), source->getSampleFrequency());
}

Aquila::SignalSource * procHamming(Aquila::SignalSource * source)
{
//    int SIZE = source->length();
//    Aquila::HammingWindow window(SIZE);
//    auto hamming = (*source) * window;
//    return new Aquila::SignalSource(hamming.toArray(), SIZE, hamming.getSampleFrequency());
    return new Aquila::SignalSource(source->toArray(), source->length(), source->getSampleFrequency());
}

Aquila::Spectrogram * procSpectrum(Aquila::SignalSource * source)
{
    Aquila::FramesCollection frames;
    frames.divideFrames(*source, CSpectrogramFrameSize, CSpectrogramFrameOverlapSize);

    return new Aquila::Spectrogram(frames);
}

Aquila::SignalSource * procChannels(Aquila::Spectrogram * spectrogram)
{
    int * bandIndex = new int[bandNumb+1];
    MakeLinIndexTab(startFreq,endFreq,smplFreq, fftOrder, bandIndex, bandNumb+1);

    int fftSize = spectrogram->getSpectrumSize() / 2;
    Aquila::SampleType * evalVect = new Aquila::SampleType[bandNumb];
    Aquila::SampleType * pFft = new Aquila::SampleType[fftSize];

    int frameCount = spectrogram->getFrameCount();
    int size = frameCount * bandNumb;
    std::vector<Aquila::SampleType> * data = new std::vector<Aquila::SampleType>(size, 0.0);

    for (std::size_t x = 0; x < frameCount; ++x)
    {
        for (std::size_t y = 0; y < fftSize; y++)
            pFft[y] = Aquila::dB(spectrogram->getPoint(x, y));

        DivideIntoZones(pFft,fftSize,evalVect,bandNumb,bandIndex,overFact);

        for (std::size_t y = 0; y < bandNumb; y++)
            (*data)[x*bandNumb + y] = evalVect[y];
    }
    delete evalVect;
    delete pFft;

    return new Aquila::SignalSource(*data);
}

Aquila::SignalSource * procParams(Aquila::FramesCollection *channels)
{
    int count = FrmVectSize;
    int size = channels->count();
    std::vector<Aquila::SampleType> * data = new std::vector<Aquila::SampleType>(size*count, 0.0);

    for (std::size_t x = 1; x < size; x++)
    {
        double byteVector[FrmVectSize*2];
        TransformVector(channels->frame(x).toArray(),byteVector);

        float fv0 = (byteVector[1]-1);
        if(fv0 < 0.0) fv0 = 0.0;
        else if(fv0 > (7-1)) fv0 = 1.0;
        else fv0 /= (7-1);

        float fv1 = (byteVector[2]-4);
        if(fv1 < 0.0) fv1 = 0.0;
        else if(fv1 > (29-4)) fv1 = 1.0;
        else fv1 /= (29-4);

        float fv2 = (byteVector[0]-16);
        if(fv2 < 0.0) fv2 = 0.0;
        else if(fv2 > (31-16)) fv2 = 1.0;
        else fv2 /= (31-16);

        (*data)[x*count+0] = fv0;
        (*data)[x*count+1] = fv1;
        (*data)[x*count+2] = fv2;

        (*data)[x*count+6] = abs(fv0 - (*data)[(x-1)*count+0]);
        (*data)[x*count+7] = abs(fv1 - (*data)[(x-1)*count+1]);
        (*data)[x*count+8] = abs(fv2 - (*data)[(x-1)*count+2]);

        (*data)[x*count+3] = byteVector[3];
        (*data)[x*count+4] = byteVector[4];
        (*data)[x*count+5] = byteVector[5];
    }
    return new Aquila::SignalSource(*data);
}

int compare(Aquila::SignalSource * source, Aquila::SignalSource * pattern)
{
    CONTRES res;
    res.R = -1;
    res.key = -1;
    res.pos[0] = -1;
    res.pos[1] = -1;
    DynTimeWarping(((*source)*255).toArray(),   source->getSamplesCount()/FrmVectSize
                 , ((*pattern)*255).toArray(),  pattern->getSamplesCount()/FrmVectSize
                 , &res );
    return res.R;
}

Aquila::SignalSource * proc(std::string name)
{
    Aquila::SignalSource * data = new Aquila::WaveFile(name);

    Aquila::SignalSource * _data = procHighpassFilter(data);
    delete data;
    data = procHamming(_data);
    delete _data;
    Aquila::Spectrogram * spectrogram = procSpectrum(data);
    delete data;
    data = procChannels(spectrogram);
    Aquila::FramesCollection channels(*data, bandNumb);
    delete spectrogram;
    _data = procParams(&channels);
    delete data;
    return _data;
}

bool AdjustBandIndexTab(int * indTab, int size, double resolution)
{
    bool repeat = true;
    int i;

    while(repeat)
    {
        for(i=0; i < size-2; i++)
        {
            if((indTab[i+2]-indTab[i+1]) < (indTab[i+1]-indTab[i]))
            {
                indTab[i+1]-=1;
                break;
            }
            if(i == size-3)
                repeat = false;
        }
    }
    return true;
}

bool MakeLinIndexTab(double minFreq, double maxFreq, double smpFreq, int fftOrder, int * indTab, int tabSize)
{
    double bandFreq;
    double resolution = smpFreq / pow(2.0, (double)fftOrder);

    double freqIncr = (maxFreq - minFreq) / double(tabSize-1);

    for(int i=0; i < tabSize; i++)
    {
        bandFreq = minFreq +i*freqIncr;
        indTab[i] =(int)(bandFreq/resolution);
    }

    AdjustBandIndexTab(indTab,tabSize,(double)resolution);

    return true;
}

bool DivideIntoZones(Aquila::SampleType * inBank, int inSize, Aquila::SampleType * outBank, int bandNum, int * indTab, int overFact)
{
    int i,j;

    if(overFact==0)
    {
        for(i=0; i < bandNum ; i++)
        {
            outBank[i] =0.0f;
            for( j=indTab[i]; j < indTab[i+1]; j++ )
                outBank[i] +=inBank[j];
            outBank[i]/= (indTab[i+1]-indTab[i]);
        }
        return true;
    }
    else
    {
        int lftDist,rhtDist,center;
        Aquila::SampleType weight;

        for(i=0; i < bandNum ; i++)
        {
            center  =indTab[i];
            outBank[i] =inBank[center];

            if(i < overFact)
            {
                lftDist =indTab[i+overFact]-indTab[i];
                if(center-lftDist <0)
                    lftDist =center;
            }
            else
                lftDist =indTab[i]-indTab[i-overFact];

            weight =1.0f / lftDist;
            for(j=1;j < (int)lftDist;j++ )
                outBank[i] +=inBank[center-lftDist+j] * weight * j;

            if(i>=bandNum-overFact)
            {
                rhtDist =indTab[i]-indTab[i-overFact];
                if(center+rhtDist >(int)inSize-1)
                    rhtDist =inSize -1 -center;
            }
            else
                rhtDist =indTab[i+overFact]-indTab[i];

            weight =1.0f / rhtDist;
            for(j=1;j < (int)rhtDist;j++ )
                outBank[i] +=inBank[center+rhtDist-j] * weight * j;
        }

        return true;
    }
}

int TransformVector(const double * pIn, double *pOut)
{
    double p = 0.3;
    double min = pIn[0];
    double max = pIn[0];
    double pProc[bandNumb];
    for(int i=1; i<bandNumb; i++){
        if(min > pIn[i]) min = pIn[i];
        if(max < pIn[i]) max = pIn[i];
    }
    for(int i=0; i<bandNumb; i++){
        pProc[i] = (pIn[i]-min)/(max-min);
        if(pProc[i] < p) pProc[i] = 0.0;
    }

    double c, k;
    c = FindGrCenter(pProc,Fmin[0],Fmax[2]);
    k = FindGrCenter(pProc,Fmin[0],c);
    pOut[0] = FindGrCenter(pProc,c,Fmax[2]);
    pOut[1] = FindGrCenter(pProc,0,k);
    pOut[2] = FindGrCenter(pProc,k,c);;

    for(int i=0; i<FrmVectSize/2; i++){
        int n = FrmVectSize/2 + i;
        pOut[n] = 0.0;
        for(int j=Amin[i]; j<Amax[i]; j++)
            pOut[n] += pProc[j];
        pOut[n] /= (Amax[i]-Amin[i]);
    }
}

double FindGrCenter(double *dst,double l,double r)
{
    int i;
    double sum1,sum2;

    for(i=l,sum1=0.0f,sum2=0.0f; i <= r; i++)
    {
      sum1+=dst[i];
      sum2+=(dst[i]*(i+1));
    }

    if(sum1 == 0)
      return(double((r+l)/2));

    sum2/=sum1;

    return (sum2-1);
}

void FreqNorm(double *pIn, double *pOut)
{
    int j;
    int Fmin[FrmVectSize/2]={2, 7,6};
    int Fmax[FrmVectSize/2]={4,15,14};
    int Fdif[FrmVectSize/2]={2, 8,8};

    // frequency normalization
    for(j=0; j < FrmVectSize/2; j++)
    {
        if(pIn[j] <= Fmin[j])
        {
            pOut[j]=0;
            continue;
        }
        if(pIn[j] >= Fmax[j])
        {
            pOut[j]=255;
            continue;
        }
        pOut[j]=(unsigned char)((pIn[j]-Fmin[j])*255/Fdif[j]);
    }
}

long GetDist(const double *ptn,const double *spk,int num)
{
  int k;
  long val;

  for(k=0,val=0; k < num; k++)
    val+=abs((int)*(spk+k)-(*(ptn+k)));

  return(val);
}

int DynTimeWarping(const double * prlz, int rlzSize, const double * pptn, int ptnSize, CONTRES* pcres)
{
    long** M = new long*[MaxNcadr];
    memset(M, 0, sizeof(long*) * MaxNcadr);
    for (int i = 0; i < MaxNcadr; i++)
        M[i] = new long[MaxNcadr];
    double* buf_d = new double[MaxNcadr];
    double* buf_t = new double[MaxNcadr];

    int num = FrmVectSize;
    int i,j;
    long val[3];
    int del;
    int res;
    int T[2][MaxNcadr];
    double koff;

    koff=256.0/ptnSize;
    del=num/3;
    // Word Spotting

    M[0][0]=GetDist(pptn,prlz,num);

    T[0][0]=0;
    for(j=1; j < ptnSize; j++)
    {
        M[0][j]=GetDist(pptn+num*j,prlz,num);
        M[0][j]+=(M[0][j-1]+(long)koff*(j-1));
        T[0][j]=0;
    }
    if(M[0][ptnSize-1]/((long)ptnSize*del) > 255)
        *buf_d=255;
    else
        *buf_d=M[0][ptnSize-1]/((long)ptnSize*del);
    *buf_t=T[0][ptnSize-1];
    res=0;

    for(i=1; i < rlzSize; i++)
    {
        M[i][0]=GetDist(pptn,prlz+num*i,num);
        T[1][0]=0;
        for(j=1; j < ptnSize; j++)
        {
            M[i][j]=GetDist(pptn+num*j,prlz+num*i,num);
            val[0]=M[i][j-1];
            val[1]=M[i-1][j-1];
            val[2]=M[i-1][j];
            if(val[1] <= val[0] && val[1] <= val[2])
            {
                M[i][j]+=(val[1]+(long)koff*abs(T[0][j-1]-j+1));
                T[1][j]=T[0][j-1]+1;
            }
            else if(val[0] <= val[1] && val[0] <= val[2])
            {
                M[i][j]+=(val[0]+(long)koff*abs(T[1][j-1]-j+1));
                T[1][j]=T[1][j-1];
            }
            else
            {
                M[i][j]+=(val[2]+(long)koff*abs(T[0][j]-j));
                T[1][j]=T[0][j]+1;
            }
        }
        for(j=0; j < ptnSize; j++)
            T[0][j]=T[1][j];
        if(M[i][ptnSize-1]/((long)ptnSize*del) > 255)
            *(buf_d+i)=255;
        else
            *(buf_d+i)=M[i][ptnSize-1]/((long)ptnSize*del);
        *(buf_t+i)=T[0][ptnSize-1];
        if(*(buf_d+i) <= *(buf_d+res))
            res=i;
    }

    if(*(buf_d+res) <= 250 && res > 0)
    {
        pcres->R=*(buf_d+res);
        pcres->pos[1]=res;                 // end of word
        pcres->pos[0]=res-(*(buf_t+res));  // beg of word
        pcres->key=1;
    }
    else
        pcres->key=0;

    return(*(buf_d+res));
}
