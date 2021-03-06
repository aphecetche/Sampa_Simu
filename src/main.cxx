#include <iostream>
#include <cstdint>
#include <chrono>
#include <ctime>
#include "Sampa.h"
#include "DualSampa.h"
#include "Receiver.h"
#include "GbtS.h"
#include "GbtR.h"
#include "interface.h"
#include "test.h"
using namespace std;

#define  NBT 100

Sampa sampa_0(0);
DualSampa ds(10,12);
GbtS     gbt_s1;
GbtR     gbt_r1(gbt_s1);
Receiver rec(0,gbt_r1);
DualSampa ds1(22,24);
Receiver rec1(1,gbt_r1);
int dsid =0;
int loop = 500;//00000;

void dsp_handler(int ref)
{
uint16_t data[4] = {7,2,0x1,0x20}; 
  // stops when max number of loops reached
  if (loop-- == 0) return;
  for (int i = 0;i<32;i++)
  {
    ds.SelectChannel(0,i);
    ds.ResetFrames();
    data[2]=i;
    ds.AddData(0,4,data);
    ds.SendFrames();
  }
  ds.RegenerateData();
  return;  
  //cout << "DSP handler "<< std::dec <<ref  << endl; 
 }
 #if 1
void dsp_handler1(int ref)
{
uint16_t data[4] = {7,2,0x1,0x20}; 
  // stops when max number of loops reached
  if (loop-- == 0) return;
  for (int i = 0;i<32;i++)
  {
    ds1.SelectChannel(0,i);
    ds1.ResetFrames();
    data[2]=i;
    ds1.AddData(0,4,data);
    ds1.SendFrames();
  }
  ds1.RegenerateData();
  return;  
  //cout << "DSP handler "<< std::dec <<ref  << endl; 
 }
#endif
void rec_handler(int addr,int ch,int nbsamp,int ts,int len, short *buff)
{
  //cout << "Packet recieved "<< std::dec <<addr << " " << buff[0] << endl; 
  if (buff[0] != ch)
  {
    throw std::runtime_error("horreur"); 
  }
}

void rec_handler1(int addr,int ch,int nbsamp,int ts,int len, short *buff)
{
  //cout << "Packet recieved ++++"<< std::dec <<addr << " " << buff[0] << endl; 
  if (len != 4)
  {
    throw std::runtime_error("horreur"); 
  }
}

void toto (void *ui,int addr,int ch,int nbsamp,int ts,int val)
{
return;
}
int test1()
{
  try {
  ds.SetDataProvider(dsp_handler);
  ds.SetInternalRef(0);
  rec.SetUserHandler(rec_handler);
  gbt_s1.PlugElink(0,&ds);
  rec.Start();
  ds1.SetDataProvider(dsp_handler1); 
  ds1.SetInternalRef(10);
  rec1.SetUserHandler(rec_handler1);
   gbt_s1.PlugElink(1,&ds1);
  rec1.Start();
  rec.Join();
  rec1.Join();
  }
  catch (const std::runtime_error& e)
  {
    cout << "execption catched" << endl;
    throw;
  }
  return 0;
}

void t2 (void *ui,int addr,int ch,int nbsamp,int ts,short *buff)
{
}
Receiver rec11(ds1,NULL,NULL,toto,NULL);

int test2()
{

  for (int i=1;i<2;i++)
  {
    Test2  t1(1,5000);

    t1.start();
    t1.display();
  }
  return 0;
  for (int i=1;i<20;i++)
  {
    Test1  t1(i*10,5000);

    t1.start();
    t1.display();
  }
  return 0;
}
DualSampa dsT1(100,102);
DualSampa dsT2(100,102);
int llop1 = 2;
int llop2 = 5;
void dsp_handlerT1(int ref)
{
cout << "eeee" << endl;
  if (llop1==0) return;
  llop1--;
  if (ref == 0)  dsT1.RegenerateData();
  if (ref == 1)  dsT2.RegenerateData();
}
void rec_handler_s1(void *ui,int addr,int ch,int nbsamp,int ts, short *buff)
{
}

int test3()
{
GbtS     gbt_s2;
GbtR     gbt_r1(gbt_s2);
Receiver rec(0,gbt_r1,NULL,NULL,rec_handler_s1,NULL);
Receiver rec1(1,gbt_r1,NULL,NULL,rec_handler_s1,NULL);

  dsT1.SetDataProvider(dsp_handlerT1);
  dsT1.SetInternalRef(0);
  dsT2.SetDataProvider(dsp_handlerT1);
  dsT2.SetInternalRef(1);
  gbt_s2.PlugElink(0,&dsT1);
  gbt_s2.PlugElink(1,&dsT2);
  rec.SetUserHandler(rec_handler);
 
   rec.Start(); 
   rec1.Start(); 
   rec.Join(); 
   rec1.Join(); 
}
int main()
{
  test2();
  return 0;
}
