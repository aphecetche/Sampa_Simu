#include <iostream>
#include <cstdint>
#include "GbtR.h"  
#include "RecInterface.h"  

using namespace std;

/*!
*  \brief Constructeur
*
*  \param provider : reference of the 'remote' GbtS
*/

GbtR::GbtR(gbtlink &provider) :mDataProvider(provider),
			       mCurSample(-1),
			       mNbLinks(0),
			       mNbSampleReaders(0), 
			       //mCurWord(Bits128()),
			       mOffset(0),
			       mPOffset(0),
			       mDataAvailable(false),
			       mRecNumber(0)
{
  mElinkMap.clear();
}

/*!
 *  \brief Allocates a new virtual Elink port on the GbtR
 *
 *  \return the allocated Elink 
 */

Elink &GbtR::GetElink(RecInterface *rec,int const port)
{
  //
  // Check ports availability
  //
  if (mElinkMap.find(port) != mElinkMap.end()) throw;
  //if (mRec.find(port) != mRec.end()) throw;
  //
  // add the new ports
  //
  mElinkMap[port] = new GbtElink(port,*this);
  mRec[mRecNumber++] = new RecRecord(port,rec);
  //
  // update internal variables
  mNbLinks++;
  mNbSampleReaders = mNbLinks;
  //
  // return allocated Elink
  //
  return *mElinkMap[port];
}
/*!
 *  \brief Start the receiver processing loop in a thread
 *
 */

void GbtR::Start()
{
  try {
  TheThread = new std::thread(&GbtR::Process,this);
  }
  catch (const std::exception& e) {
    throw;
  }
}

/*!
 *  \brief Waits for the receiver processing loop to terminate
 *
 */
void GbtR::Join()
{
  TheThread->join();
}

/*!
 *  \brief Check the thread still exist
 *
 */
bool GbtR::Joinable()
{
  return TheThread->joinable();
}

void GbtR::Process()
{
  while (mDataProvider.GbtWordAvailable()) {
    Push(mDataProvider.GetWord());
  }
  
}

void GbtR::Push(Bits128 word)
{
#ifdef STL
  for (std::map<int,RecInterface *>::iterator it = mRec.begin();
                                              it != mRec.end();
					      it++) {
					      
    it->second->Push((word.Get((it->first *2) +1)));
    it->second->Push((word .Get(it->first *2)));					      
  }
#else
  for (int i=0;i<mRecNumber;i++) 
  {
    mRec[i]->mRec->Push((word.Get((mRec[i]->mPort *2) +1)));
    mRec[i]->mRec->Push((word.Get((mRec[i]->mPort *2))));
  }
#endif 					      
}
/*!
 *  \brief Get a new Gbt 128 bit word from the remote GBT
 *
 *  This routine must be called by a receiver to get sure that
 *  a word is available. \n
 *  All receivers will be suspended as long as some receiver is
 *  no synchronised (i.e. fetching the same GBT word).
 *
 *  \return true if a new word is available, false otherwise 
 */

bool GbtR::Fetch(int const port,int const sample) 
{
  if ((sample/2) < (mCurSample+mOffset)) {
    return true;
  }
  mMutex.lock();
  // we run out of memory, wait for all other process to reach the
  // same point
  //cout << "GbtR::Fetch " << mCurSample <<" "<<mOffset<<  " " << sample << endl;
  mNbSampleReaders--;
  if (mNbSampleReaders != 0) {
    // 
    // some readers are out of synchronisation, let' wait for them
    // suspend task
    //
    mMutex.unlock();
    mElinkMap[port]->lock() ;
    //
    // returns data availability when all readers are synchronised
    //
    return mDataAvailable;
  }
  else {
    //
    // The last readers is now synchronised, let's get the GBT word
    // from the distant party and release all pending readers
    //
    // fecth a new page
    //
    mCurSample = 0;
    while (mDataProvider.GbtWordAvailable()){
      if (mCurSample == mWindowsize) 
      { 
        break;
      }
      mCurWord[mCurSample] = mDataProvider.GetWord();
      mCurSample++;
    }
    mOffset = mPOffset;
    mPOffset += mCurSample;
    mNbSampleReaders = mNbLinks;
   // mDataAvailable= mDataProvider.GbtWordAvailable();
    mDataAvailable = (mCurSample != 0);
    
    //
    // free all waiting thread (but ourself)
    //
    for (std::map<int,GbtElink *>::iterator it = mElinkMap.begin();it !=mElinkMap.end();it++) {
      if (it->first != port) 
        it->second->unlock() ; 
    }
    mMutex.unlock(); // end on critical section
    if (mDataAvailable == false)
    {
      mNbLinks--;
    }
    return mDataAvailable;
  }
}
/*!
 *  \brief Read a bit on a given port
 *
 *  Return the next bit of the GBT stream. \n
 *  Receiver is assumed to have check the availibilty of the
 *  date through the Fetch method, otherwise returned information
 *  validity is not guarantied.
 *
 *  \return bit information 
 */
#if 0
uint8_t GbtR::Read(int const port,int const sample)
{
//  return mCurWord[(sample>>1)-mOffset].Get((port<<1)+(1-(sample & 1)));
  return mCurWord[(sample>>1)-mOffset].Get((port<<1)+((~sample) & 1));
}
#endif
