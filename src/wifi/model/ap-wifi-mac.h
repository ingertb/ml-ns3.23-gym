/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Mirko Banchi <mk.banchi@gmail.com>
 */

#ifndef AP_WIFI_MAC_H
#define AP_WIFI_MAC_H

#include "regular-wifi-mac.h"
#include "ht-capabilities.h"
#include "amsdu-subframe-header.h"
#include "supported-rates.h"
#include "ns3/random-variable-stream.h"
#include "rps.h"
#include "s1g-raw-control.h"
#include "ns3/string.h"
#include <stack>


namespace ns3 {

/**
 * \brief Wi-Fi AP state machine
 * \ingroup wifi
 *
 * Handle association, dis-association and authentication,
 * of STAs within an infrastructure BSS.
 */
class ApWifiMac : public RegularWifiMac
{
public:
  static TypeId GetTypeId (void);

  ApWifiMac ();
  virtual ~ApWifiMac ();

  /**
   * \param stationManager the station manager attached to this MAC.
   */
  virtual void SetWifiRemoteStationManager (Ptr<WifiRemoteStationManager> stationManager);

  /**
   * \param linkUp the callback to invoke when the link becomes up.
   */
  virtual void SetLinkUpCallback (Callback<void> linkUp);

  /**
   * \param packet the packet to send.
   * \param to the address to which the packet should be sent.
   *
   * The packet should be enqueued in a tx queue, and should be
   * dequeued as soon as the channel access function determines that
   * access is granted to this MAC.
   */
  virtual void Enqueue (Ptr<const Packet> packet, Mac48Address to);

  /**
   * \param packet the packet to send.
   * \param to the address to which the packet should be sent.
   * \param from the address from which the packet should be sent.
   *
   * The packet should be enqueued in a tx queue, and should be
   * dequeued as soon as the channel access function determines that
   * access is granted to this MAC.  The extra parameter "from" allows
   * this device to operate in a bridged mode, forwarding received
   * frames without altering the source address.
   */
  virtual void Enqueue (Ptr<const Packet> packet, Mac48Address to, Mac48Address from);

  virtual bool SupportsSendFrom (void) const;

  /**
   * \param address the current address of this MAC layer.
   */
  virtual void SetAddress (Mac48Address address);
  /**
   * \param interval the interval between two beacon transmissions.
   */
  void SetBeaconInterval (Time interval);
  /**
   * \return the interval between two beacon transmissions.
   */
  Time GetBeaconInterval (void) const;
  /**
   * Start beacon transmission immediately.
   */
  void StartBeaconing (void);
  void SetSaturatedAssociated (void);
  void SetNAssociating (uint32_t num);
  void SetAssociatingStasAppear (void);
  void SetSecondWaveAppear (void);
  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   *
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);
  //***********************************
  //HaLow
  void SetAuthenThreshold (uint32_t AuthThr);
  void SetAlgorithm (uint32_t alg);
  void Algorithm_0 (uint32_t &MgtQueueSize);
  void Algorithm_1 (uint32_t &MgtQueueSize);
  void Algorithm_2 (uint32_t &MgtQueueSize);
  void Algorithm_3 (uint32_t &MgtQueueSize);
  void Algorithm_5 (uint32_t &MgtQueueSize);
  void Algorithm_6 (uint32_t &MgtQueueSize);
  void Algorithm_7 (uint32_t &MgtQueueSize);
  void Algorithm_8 (uint32_t &MgtQueueSize);
  void Algorithm_9 (uint32_t &MgtQueueSize);
  void Algorithm_10 (uint32_t &MgtQueueSize);
  Ptr<WifiMacQueue> GetQueueInfo(void);
  Ptr<MacLow> GetMlowInfo(void);
  uint32_t GetAuthResp(void);
  uint32_t GetAssocResp(void);
  uint32_t GetAuthenThreshold (void);
  uint32_t GetAlgorithm (void) const;


private:

  enum CAC_STATES
  {
    WAIT,
    LEARN,
    WORK
  } m_cac_state;

  virtual void Receive (Ptr<Packet> packet, const WifiMacHeader *hdr);
  /**
   * The packet we sent was successfully received by the receiver
   * (i.e. we received an ACK from the receiver).  If the packet
   * was an association response to the receiver, we record that
   * the receiver is now associated with us.
   *
   * \param hdr the header of the packet that we successfully sent
   */
  virtual void TxOk (const WifiMacHeader &hdr);
  /**
   * The packet we sent was successfully received by the receiver
   * (i.e. we did not receive an ACK from the receiver).  If the packet
   * was an association response to the receiver, we record that
   * the receiver is not associated with us yet.
   *
   * \param hdr the header of the packet that we failed to sent
   */
  virtual void TxFailed (const WifiMacHeader &hdr);

  /**
   * This method is called to de-aggregate an A-MSDU and forward the
   * constituent packets up the stack. We override the WifiMac version
   * here because, as an AP, we also need to think about redistributing
   * to other associated STAs.
   *
   * \param aggregatedPacket the Packet containing the A-MSDU.
   * \param hdr a pointer to the MAC header for \c aggregatedPacket.
   */
  virtual void DeaggregateAmsduAndForward (Ptr<Packet> aggregatedPacket,
                                           const WifiMacHeader *hdr);
  /**
   * Forward the packet down to DCF/EDCAF (enqueue the packet). This method
   * is a wrapper for ForwardDown with traffic id.
   *
   * \param packet the packet we are forwarding to DCF/EDCAF
   * \param from the address to be used for Address 3 field in the header
   * \param to the address to be used for Address 1 field in the header
   */
  void ForwardDown (Ptr<const Packet> packet, Mac48Address from, Mac48Address to);
  /**
   * Forward the packet down to DCF/EDCAF (enqueue the packet).
   *
   * \param packet the packet we are forwarding to DCF/EDCAF
   * \param from the address to be used for Address 3 field in the header
   * \param to the address to be used for Address 1 field in the header
   * \param tid the traffic id for the packet
   */
  void ForwardDown (Ptr<const Packet> packet, Mac48Address from, Mac48Address to, uint8_t tid);
  /**
   * Forward a probe response packet to the DCF. The standard is not clear on the correct
   * queue for management frames if QoS is supported. We always use the DCF.
   *
   * \param to the address of the STA we are sending a probe response to
   */
  void SendProbeResp (Mac48Address to);
  /**
   * Forward an authentication response packet to the DCF. The standard is not clear on the correct
   * queue for management frames if QoS is supported. We always use the DCF.
   *
   * \param to the address of the STA we are sending an authentication response to
   * \param success indicates whether the association was successful or not
   */
  void SendAuthResp (Mac48Address to, bool success);
  /**
   * Forward an association response packet to the DCF. The standard is not clear on the correct
   * queue for management frames if QoS is supported. We always use the DCF.
   *
   * \param to the address of the STA we are sending an association response to
   * \param success indicates whether the association was successful or not
   */
  void SendAssocResp (Mac48Address to, bool success, uint8_t staType);
  /**
   * Forward a beacon packet to the beacon special DCF.
   */
  void SendOneBeacon (void);
  /**
   * Return the HT capability of the current AP.
   *
   * \return the HT capability that we support
   */
  HtCapabilities GetHtCapabilities (void) const;
  /**
   * Return an instance of SupportedRates that contains all rates that we support
   * including HT rates.
   *
   * \return SupportedRates all rates that we support
   */
  SupportedRates GetSupportedRates (void) const;
  /**
   * Enable or disable beacon generation of the AP.
   *
   * \param enable enable or disable beacon generation
   */
  void SetBeaconGeneration (bool enable);
  /**
   * Return whether the AP is generating beacons.
   *
   * \return true if beacons are periodically generated, false otherwise
   */
  bool GetBeaconGeneration (void) const;
  /**
   * Enable the usage of RAW
   *
   * \param enable enable or disable RAW
   */
  void SetRawEnabled (bool enabled);
  /**
   * Return whether the AP uses RAW
   *
   * \return true if the AP uses RAW
   */
  bool GetRawEnabled (void) const;

  void SetRawGroupInterval (uint32_t interval);
  uint32_t GetRawGroupInterval (void) const;
  void SetSlotFormat (uint32_t format);
  void SetSlotCrossBoundary (uint32_t cross);
  void SetSlotDurationCount (uint32_t count);
  void SetSlotNum (uint32_t count);
  void SetProtocol (uint32_t prot);
  void SetAuthSlot (uint32_t slot);
  void SetTiMin (uint32_t timin);
  void SetTiMax (uint32_t timax);  
  void SetNsaturated (uint32_t Nsaturated);
  void SetValue (uint32_t val);
  void SetMinValue (uint32_t minval);
  void SetLearningThreshold (uint32_t t);
  void SetNewGroupThreshold (uint32_t t);
  uint32_t GetSlotFormat (void) const;
  uint32_t GetSlotCrossBoundary (void) const;
  uint32_t GetSlotDurationCount (void) const;
  uint32_t GetSlotNum (void) const;
  uint32_t GetProtocol (void) const;
  uint32_t GetAuthSlot (void) const;
  uint32_t GetTiMin (void) const;
  uint32_t GetTiMax (void) const;  
  uint32_t GetNsaturated (void) const;
  uint32_t GetValue (void) const;
  uint32_t GetMinValue (void) const;
  uint32_t GetNAssociating (void) const;
  uint32_t GetLearningThreshold (void) const;
  uint32_t GetNewGroupThreshold (void) const;

  RPSVector m_rpsset;
  void SetTotalStaNum (uint32_t num);
  uint32_t GetTotalStaNum (void) const;

  typedef std::vector<ns3::RPS *>::iterator RPSlistCI;

  virtual void DoDispose (void);
  virtual void DoInitialize (void);

  uint16_t AuthenThreshold;
  uint32_t m_totalStaNum;
  uint32_t m_rawGroupInterval;
  uint32_t m_SlotFormat;
  uint32_t m_slotCrossBoundary;
  uint32_t m_slotDurationCount;
  uint32_t m_slotNum;
  uint32_t m_protocol;
  uint32_t algorithm;
  uint32_t Nsaturated;
  uint32_t minvalue;
  uint32_t value;
  uint32_t m_tiMin;
  uint32_t m_tiMax;
  int m_authSlot;
  std::vector<uint16_t> m_sensorList; //stations allowed to transmit in last beacon
  std::vector<uint16_t> m_OffloadList;
  std::vector<uint16_t> m_receivedAid;

  S1gRawCtr m_S1gRawCtr;
  Ptr<DcaTxop> m_beaconDca;                  //!< Dedicated DcaTxop for beacons
  Time m_beaconInterval;                     //!< Interval between beacons
  bool m_enableBeaconGeneration;             //!< Flag if beacons are being generated
  EventId m_beaconEvent;                     //!< Event to generate one beacon
  Ptr<UniformRandomVariable> m_beaconJitter; //!< UniformRandomVariable used to randomize the time of the first beacon
  bool m_enableBeaconJitter;                 //!< Flag if the first beacon should be generated at random time
  std::string  m_outputpath;
  bool m_rawEnabled;                         //!< Flag if the Access Point uses RAW and includes an RPS element in beacons
  bool m_saturatedAssociated;
  bool m_associatingStasAppear;
  bool m_secondWaveAppear;
  uint32_t m_nAssociating;

  int m_cac_wait_counter;                   //number of beacon intervals before move to waiting mode after working mode with threshold=1023
  int m_cac_improve_counter;                //number of beacon intervals with empty queue before improving delta
  int m_cac_learn_counter;                  //number of beacon intervals with non-empty queue before transit to study mode again
  bool m_cac_improve;

  Time m_tau;
  Time m_lastBeacon;

  int m_delta, m_delta_temp, m_delta_reduced;
  int m_queueLast;


  int m_counterAssocSuccess;
  int m_counterAuthSuccess;
  int m_totalAssocSuccess;
  int m_learningThreshold;
  int m_newGroupThreshold;
  uint64_t t;
  uint64_t y;

  int a1;                                   // the number of successful AuthReps
  int a2;                                   // the number of successful AReps
  int r1;                                   // the number of successful AuthReq
  int r2;                                   // the number of successful AReqs
  int q1 = 0;                                   // the number of AuthReps remaining in the queue
  int q2 = 0;                                   // the number of AReps remaining in the queue

  int T1 = 1023;                                 // threshold to memorize in algorithm oracle for twogroups scenario
  uint32_t firstWave = 0;
  uint32_t secondWave = 0;

  int sent_ass = 0;
  int sent_auth = 0;



  uint32_t contAuthResp; 
  uint32_t contAssocResp;


  struct Triplet {
    int delta;
    int threshold;
    Triplet(int t = 0, int d = 0) : threshold(t), delta(d) {}
  };
  std::stack<Triplet> m_cac_history;
  //********************************************
  //HaLow
  //TracedCallback<Mac48Address> m_beaconS1G;
  TracedCallback<Ptr<ApWifiMac>> m_beaconS1G;
  TracedCallback<Ptr<ApWifiMac>> m_setAlg;

};

} //namespace ns3

#endif /* AP_WIFI_MAC_H */
