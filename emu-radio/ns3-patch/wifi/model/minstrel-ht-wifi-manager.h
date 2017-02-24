/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Ghada Badawy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *Author: Ghada Badawy <gbadawy@gmail.com>
 * 
 * modified by zeng <zengxuan123456@gmail.com>
 * NOTE: currently the spatial stream used are hard coded to 1*1, because only 1 spatial 
 * stream is supported for 802.11n
 */



#ifndef MINSTREL_HT_WIFI_MANAGER_H
#define MINSTREL_HT_WIFI_MANAGER_H

#include "wifi-remote-station-manager.h"
#include "wifi-mode.h"
#include "ns3/nstime.h"
#include <vector>
#include <map>
#include <deque>


namespace ns3 {

struct MinstrelHtWifiRemoteStation;

/**
 * A struct to contain all information related to a data rate
 */
struct HtRateInfo
{
  /**
   * Perfect transmission time calculation, or frame calculation
   * Given a bit rate and a packet length n bytes
   */
  Time perfectTxTime;


  uint32_t retryCount;  ///< retry limit
  uint32_t adjustedRetryCount;  ///< adjust the retry limit for this rate
  uint32_t numRateAttempt;  ///< how many number of attempts so far
  uint32_t numRateSuccess;    ///< number of successful pkts
  uint32_t prob;  ///< (# pkts success )/(# total pkts)

  /**
   * EWMA calculation
   * ewma_prob =[prob *(100 - ewma_level) + (ewma_prob_old * ewma_level)]/100
   */
  uint32_t ewmaProb;

  uint32_t prevNumRateAttempt;  ///< from last rate
  uint32_t prevNumRateSuccess;  ///< from last rate
  uint64_t successHist;  ///< aggregate of all successes
  uint64_t attemptHist;  ///< aggregate of all attempts
  uint32_t throughput;  ///< throughput of a rate
};

/**
 * Data structure for a Minstrel Rate table
 * A vector of a struct RateInfo
 */
typedef std::vector<struct HtRateInfo> HtMinstrelRate;

struct GroupInfo
{
  /**
   * MCS rates are divided into groups based on the number of streams and flags that they use
   */
  uint8_t m_col; ///< sample table column
  uint8_t m_index;  ///<sample table index
  uint32_t m_maxTpRate; ///< Rate the has max throughput for this group
  uint32_t m_maxTpRate2;///< Rate the has second max throughput for this group
  uint32_t m_maxProbRate;///< Rate the has highest success probability for this group
  HtMinstrelRate m_minstrelTable; ///< Information about rates member in this group
  
};

/**
 * Data structure for a MCS group table
 * A vector of a Minstrel Rate Table
 */
typedef std::vector<struct GroupInfo> McsGroup;

/**
 * Data structure for a Sample Rate table
 * A vector of a vector uint32_t
 */
typedef std::vector<std::vector<uint32_t> > HtSampleRate;


/**
 * \author Ghada Badawy
 * \brief Implementation of Minstrel HT Rate Control Algorithm
 * \ingroup wifi
 *
 * http://lwn.net/Articles/376765/
 */
class MinstrelHtWifiManager : public WifiRemoteStationManager
{

public:
  static TypeId GetTypeId (void);
  MinstrelHtWifiManager ();
  virtual ~MinstrelHtWifiManager ();

  virtual void SetupPhy (Ptr<WifiPhy> phy);

  typedef void (*RateChangeTracedCallback)(const uint64_t rate, const Mac48Address remoteAddress);

private:
  // overriden from base class
  virtual WifiRemoteStation * DoCreateStation (void) const;
  virtual void DoReportRxOk (WifiRemoteStation *station,
                             double rxSnr, WifiMode txMode);
  virtual void DoReportRtsFailed (WifiRemoteStation *station);
  virtual void DoReportDataFailed (WifiRemoteStation *station);
  virtual void DoReportRtsOk (WifiRemoteStation *station,
                              double ctsSnr, WifiMode ctsMode, double rtsSnr);
  virtual void DoReportDataOk (WifiRemoteStation *station,
                               double ackSnr, WifiMode ackMode, double dataSnr);
  virtual void DoReportFinalRtsFailed (WifiRemoteStation *station);
  virtual void DoReportFinalDataFailed (WifiRemoteStation *station);
  virtual WifiTxVector DoGetDataTxVector (WifiRemoteStation *station, uint32_t size);
  virtual WifiTxVector DoGetRtsTxVector (WifiRemoteStation *station);

  virtual bool DoNeedDataRetransmission (WifiRemoteStation *st, Ptr<const Packet> packet, bool normally); 

  
  virtual bool IsLowLatency (void) const;


  /// for estimating the TxTime of a packet with a given mode
  Time GetCalcTxTime (WifiMode mode) const;
  void AddCalcTxTime (WifiMode mode, Time t);

  /// update the number of retries and reset accordingly
  void UpdateRetry (MinstrelHtWifiRemoteStation *station);

  /// getting the next sample from Sample Table
  uint32_t GetNextSample (MinstrelHtWifiRemoteStation *station);

  /// find a rate to use from Minstrel Table
  uint32_t FindRate (MinstrelHtWifiRemoteStation *station);

  /// updating the Minstrel Table every 1/10 seconds
  void UpdateStats (MinstrelHtWifiRemoteStation *station);

  /// initialize Minstrel Table
  void RateInit (MinstrelHtWifiRemoteStation *station);

  /// initialize Sample Table
  void InitSampleTable (MinstrelHtWifiRemoteStation *station);

  /// printing Sample Table
  void PrintSampleTable (MinstrelHtWifiRemoteStation *station);

  /// printing Minstrel Table
  void PrintTable (MinstrelHtWifiRemoteStation *station);

  void CheckInit (MinstrelHtWifiRemoteStation *station);  ///< check for initializations

  uint32_t  GetRateId(uint32_t rate);

  uint32_t  GetGroupId(uint32_t rate, WifiRemoteStation *st, uint8_t txstreams);
  uint32_t GetTxRate(uint32_t groupid, uint32_t index);
  uint8_t GetStreams (uint32_t groupId, MinstrelHtWifiRemoteStation *station);

  typedef std::vector<std::pair<Time,WifiMode> > TxTime;


  TxTime m_calcTxTime;  ///< to hold all the calculated TxTime for all modes
  Time m_updateStats;  ///< how frequent do we calculate the stats(1/10 seconds)
  double m_lookAroundRate;  ///< the % to try other rates than our current rate
  double m_ewmaLevel;  ///< exponential weighted moving average
  uint32_t m_segmentSize;  ///< largest allowable segment size
  uint32_t m_sampleCol;  ///< number of sample columns
  uint32_t m_pktLen;  ///< packet length used  for calculate mode TxTime
  uint32_t m_nsupported;  ///< modes supported
  uint8_t m_nGroups;///<hold the number of different MCS groups that the STA has if the STA supports 40MHz then it only uses 40MHz rates no switching between 20 and 40MHz rates

  TracedCallback<uint64_t, Mac48Address> m_rateChange;
};

} // namespace ns3

#endif /* MINSTREL_HT_WIFI_MANAGER_H */

