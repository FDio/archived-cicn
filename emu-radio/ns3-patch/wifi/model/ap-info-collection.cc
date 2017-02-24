#include "ap-info-collection.h"


namespace ns3 {

ApInfo::ApInfo (Mac48Address bssid, Time delayFromProbResp, double rssi, SupportedRates supportedRates)
:m_bssid(bssid)
,m_delayFromProbResp(delayFromProbResp)
,m_supportedRates(supportedRates)
{
  m_rssiSamples.push_back(rssi);
}

ApInfo::ApInfo()
{
  
}

void 
ApInfo::addRssi (double rssi)
{
  m_rssiSamples.push_back(rssi);
}

void 
ApInfo::removeRssi (double rssi)
{
  std::vector<double>::iterator it;
  for(it=m_rssiSamples.begin();it!=m_rssiSamples.end();it++)
  {
    if(*it == rssi)
      break;
  }
  m_rssiSamples.erase(it);
}


Mac48Address 
ApInfo::getBssid() const
{
  return m_bssid;
}

Time 
ApInfo::getDelayFromProbResp() const
{
  return m_delayFromProbResp;
}

double 
ApInfo::getAverageRssi () const
{
  if(m_rssiSamples.empty())
    return -1.0;
  
  double sum=0;
  std::vector<double>::const_iterator it;
  for(it=m_rssiSamples.begin();it!=m_rssiSamples.end();it++)
  {
    sum+=*it;
  }
  
  return sum/m_rssiSamples.size();
}

SupportedRates 
ApInfo::getSupportedRates() const
{
  return m_supportedRates;
}

} //namespace ns3
