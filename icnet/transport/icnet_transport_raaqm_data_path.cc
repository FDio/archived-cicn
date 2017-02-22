/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "icnet_transport_raaqm_data_path.h"

namespace icnet {

RaaqmDataPath::RaaqmDataPath(double drop_factor,
                             double minimum_drop_probability,
                             unsigned new_timer,
                             unsigned int samples,
                             uint64_t new_rtt,
                             uint64_t new_rtt_min,
                             uint64_t new_rtt_max,
                             unsigned new_pd)

    : drop_factor_(drop_factor),
      minimum_drop_probability_(minimum_drop_probability),
      timer_(new_timer),
      samples_(samples),
      rtt_(new_rtt),
      rtt_min_(new_rtt_min),
      rtt_max_(new_rtt_max),
      prop_delay_(new_pd),
      new_prop_delay_(false),
      drop_prob_(0),
      packets_received_(0),
      last_packets_received_(0),
      m_packets_bytes_received_(0),
      last_packets_bytes_received_(0),
      raw_data_bytes_received_(0),
      last_raw_data_bytes_received_(0),
      average_rtt_(0),
      alpha_(ALPHA) {
  gettimeofday(&previous_call_of_path_reporter_, 0);
  gettimeofday(&m_last_received_pkt_, 0);
}

RaaqmDataPath &RaaqmDataPath::pathReporter() {
  struct timeval now;
  gettimeofday(&now, 0);

  double rate, delta_t;

  delta_t = getMicroSeconds(now) - getMicroSeconds(previous_call_of_path_reporter_);
  rate = (m_packets_bytes_received_ - last_packets_bytes_received_) * 8 / delta_t; // MB/s
  std::cout << "RaaqmDataPath status report: " << "at time " << (long) now.tv_sec << "." << (unsigned) now.tv_usec
            << " sec:\n" << (void *) this << " path\n" << "Packets Received: "
            << (packets_received_ - last_packets_received_) << "\n" << "delta_t " << delta_t << " [us]\n" << "rate "
            << rate << " [Mbps]\n" << "Last RTT " << rtt_ << " [us]\n" << "Max RTT " << rtt_max_ << " [us]\n"
            << "Min RTT " << rtt_min_ << " [us]\n" << "Prop delay " << prop_delay_ << " [us]\n";
  last_packets_received_ = packets_received_;
  last_packets_bytes_received_ = m_packets_bytes_received_;
  gettimeofday(&previous_call_of_path_reporter_, 0);

  return *this;
}

RaaqmDataPath &RaaqmDataPath::insertNewRtt(uint64_t new_rtt) {
  rtt_ = new_rtt;
  rtt_samples_.get<byArrival>().push_back(new_rtt);

  if (rtt_samples_.get<byArrival>().size() > samples_) {
    rtt_samples_.get<byArrival>().pop_front();
  }

  rtt_max_ = *(rtt_samples_.get<byOrder>().rbegin());
  rtt_min_ = *(rtt_samples_.get<byOrder>().begin());

  if (rtt_min_ < prop_delay_) {
    new_prop_delay_ = true;
    prop_delay_ = rtt_min_;
  }

  gettimeofday(&m_last_received_pkt_, 0);

  return *this;
}

RaaqmDataPath &RaaqmDataPath::updateReceivedStats(std::size_t packet_size, std::size_t data_size) {
  packets_received_++;
  m_packets_bytes_received_ += packet_size;
  raw_data_bytes_received_ += data_size;

  return *this;
}

double RaaqmDataPath::getDropFactor() {
  return drop_factor_;
}

double RaaqmDataPath::getDropProb() {
  return drop_prob_;
}

RaaqmDataPath &RaaqmDataPath::setDropProb(double dropProb) {
  drop_prob_ = dropProb;

  return *this;
}

double RaaqmDataPath::getMinimumDropProbability() {
  return minimum_drop_probability_;
}

double RaaqmDataPath::getTimer() {
  return timer_;
}

RaaqmDataPath &RaaqmDataPath::smoothTimer() {
  timer_ = (1 - TIMEOUT_SMOOTHER) * timer_ + (TIMEOUT_SMOOTHER) * rtt_ * (TIMEOUT_RATIO);

  return *this;
}

double RaaqmDataPath::getRtt() {
  return rtt_;
}

double RaaqmDataPath::getAverageRtt() {
  return average_rtt_;
}

double RaaqmDataPath::getRttMax() {
  return rtt_max_;
}

double RaaqmDataPath::getRttMin() {
  return rtt_min_;
}

unsigned RaaqmDataPath::getSampleValue() {
  return samples_;
}

unsigned RaaqmDataPath::getRttQueueSize() {
  return rtt_samples_.get<byArrival>().size();
}

RaaqmDataPath &RaaqmDataPath::updateDropProb() {

  drop_prob_ = 0.0;

  if (getSampleValue() == getRttQueueSize()) {
    if (rtt_max_ == rtt_min_) {
      drop_prob_ = minimum_drop_probability_;
    } else {
      drop_prob_ = minimum_drop_probability_ + drop_factor_ * (rtt_ - rtt_min_) / (rtt_max_ - rtt_min_);
    }
  }

  return *this;
}

double RaaqmDataPath::getMicroSeconds(struct timeval &time) {
  return (double) (time.tv_sec) * 1000000 + (double) (time.tv_usec);
}

void RaaqmDataPath::setAlpha(double alpha) {
  if (alpha >= 0 && alpha <= 1) {
    alpha_ = alpha;
  }
}

bool RaaqmDataPath::newPropagationDelayAvailable() {
  bool r = new_prop_delay_;
  new_prop_delay_ = false;
  return r;
}

unsigned int RaaqmDataPath::getPropagationDelay() {
  return prop_delay_;
}

bool RaaqmDataPath::isStale() {
  struct timeval now;
  gettimeofday(&now, 0);
  double time = getMicroSeconds(now) - getMicroSeconds(m_last_received_pkt_);
  if (time > 2000000) {
    return true;
  }
  return false;
}

} // end namespace icnet
