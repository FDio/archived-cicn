
#ifndef AP_INFO_COLLECTION_H
#define AP_INFO_COLLECTION_H

#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include <vector>

#include "ns3/supported-rates.h"

//for wifi handover triggers
#define MAX_NUM_RSSI_SAMPMES 4
	
namespace ns3 {
class RssiMeasureInfo
{
public:
  RssiMeasureInfo(Mac48Address bssid)
  :m_index(0)
  ,m_numberOfSamples(0)
  ,m_bssid(bssid)
  {
    for(int i=0;i<MAX_NUM_RSSI_SAMPMES;i++)
      m_rssiSamples[i]=0;
  }
  
  RssiMeasureInfo()
  :m_index(0)
  ,m_numberOfSamples(0)
  {
    for(int i=0;i<MAX_NUM_RSSI_SAMPMES;i++)
      m_rssiSamples[i]=0;
  }
  
  Mac48Address getBssid() const
  {
    return m_bssid;
  }
  
  void setBssid(Mac48Address bssid)
  {
    m_bssid=bssid;
  }
  
  bool isFull() const
  {
    return m_numberOfSamples>=MAX_NUM_RSSI_SAMPMES;
  }
  
  void addRssi(double rssi)
  {
    m_rssiSamples[m_index]=rssi;
    m_index=(m_index+1)%MAX_NUM_RSSI_SAMPMES;
    if(m_numberOfSamples<MAX_NUM_RSSI_SAMPMES)
      m_numberOfSamples++;
  }
  
  double getAverageRssi()
  {
    if(!isFull())
      return -1;
    else
    {
      double sum=0;
      for(int i=0;i<MAX_NUM_RSSI_SAMPMES;i++)
	sum+=m_rssiSamples[i];
      return sum/m_numberOfSamples;
    }
  }
  
  void clear()
  {
    m_index=0;
    m_numberOfSamples=0;
    for(int i=0;i<MAX_NUM_RSSI_SAMPMES;i++)
      m_rssiSamples[i]=0;
  }
  
  
private:
  double m_rssiSamples[MAX_NUM_RSSI_SAMPMES];
  int m_index;
  int m_numberOfSamples;
  Mac48Address m_bssid;
};

typedef std::vector<RssiMeasureInfo> RssiMeasureInfoCollection;

/**
 * \ingroup wifi
 *
 * this class is used to store information about one AP, so as to facilitate the AP selection process
 */
class ApInfo
{
public:
  /**
   * Create an entry containning the info about an AP
   *  \param bssid the bssid of an AP
   *  \param delayFromProbResp computed delay from response according to info in proberesp
   *  \param rssi a sample rssi associated with the AP obtained from a proberesp 
   *  \param supportedRates supported rates of the AP 
   */
  ApInfo (Mac48Address bssid, Time delayFromProbResp, double rssi, SupportedRates supportedRates);
  
 /** 
  *  create an empty entry for the AP info
  */
  ApInfo();
  /**
   * add a rssi sample to this Ap info
   * \param rssi sample rssi obtained from proberesp
   */
  void addRssi (double rssi);
  /**
   * remove a rssi sample from this AP info
   * \param rssi
   */
  void removeRssi (double rssi);

  /**
   * get Bssid of thIS AP from AP info
   * \return bssid of AP
   */
  Mac48Address getBssid() const;
  
  /**
   * get DelayFromProbResp of thIS AP from AP info
   * \return computed DelayFromProbResp(time to wait to get into beacon missed state) of AP
   */
  Time getDelayFromProbResp() const;

  /**
   * get mearsured average rssi for this AP 
   * \return computed average rssi for this AP
   *
   */
  double getAverageRssi () const;
  
  SupportedRates getSupportedRates() const;

private:
  Mac48Address m_bssid;     //!< bssid of AP
  Time m_delayFromProbResp; //!< time to wait to switch into beacon missed state
  std::vector<double> m_rssiSamples; //!< contains all rssi sapmples obtained from probe response
  SupportedRates m_supportedRates;
};



typedef std::vector<ApInfo> ApInfoCollection;

} //namespace ns3

#endif /* AP_INFO_COLLECTION_H*/
