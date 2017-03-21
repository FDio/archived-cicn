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

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "icnet_transport_raaqm.h"

namespace icnet {

RaaqmTransportProtocol::RaaqmTransportProtocol(Socket *icnet_socket)
    : VegasTransportProtocol(icnet_socket), rate_estimator_(NULL) {
  init();
}

RaaqmTransportProtocol::~RaaqmTransportProtocol() {
  if (this->rate_estimator_) {
    delete this->rate_estimator_;
  }
}

void RaaqmTransportProtocol::init() {
  std::ifstream is(RAAQM_CONFIG_PATH);

  std::string line;

  socket_->setSocketOption(RaaqmTransportOptions::BETA_VALUE, default_values::beta_value);
  socket_->setSocketOption(RaaqmTransportOptions::DROP_FACTOR, default_values::drop_factor);
  socket_->setSocketOption(GeneralTransportOptions::INTEREST_LIFETIME, default_values::interest_lifetime);
  socket_->setSocketOption(ConsumerCallbacksOptions::INTEREST_RETRANSMISSION,
                           default_values::transport_protocol_max_retransmissions);
  raaqm_autotune_ = false;
  default_beta_ = default_values::beta_value;
  default_drop_ = default_values::drop_factor;
  beta_wifi_ = default_values::beta_value;
  drop_wifi_ = default_values::drop_factor;
  beta_lte_ = default_values::beta_value;
  drop_lte_ = default_values::drop_factor;
  wifi_delay_ = 1000;
  lte_delay_ = 15000;
  avg_rtt_ = 0.0;

  if (!is) {
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "WARNING: RAAQM parameters not found, set default values\n");
#else
    std::cout << "WARNING: RAAQM parameters not found, set default values" << std::endl;
#endif
    return;
  }

#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "Setting RAAQM parameters:\n");
#else
  std::cout << "Setting RAAQM parameters:" << std::endl;
#endif
  while (getline(is, line)) {
    std::string command;
    std::istringstream line_s(line);

    line_s >> command;

    if (command == ";") {
      continue;
    }

    if (command == "autotune") {
      std::string tmp;
      std::string val;
      line_s >> tmp >> val;
      if (val == "yes") {
        raaqm_autotune_ = true;
      } else {
        raaqm_autotune_ = false;
      }
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  autotune = %s",raaqm_autotune_ == 1 ? "true" : "false");
#else
      std::cout << "params:  autotune = " << raaqm_autotune_ << std::endl;
#endif
      continue;
    }

    if (command == "lifetime") {
      std::string tmp;
      int lifetime;
      line_s >> tmp >> lifetime;
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  lifetime = %d\n", lifetime);
#else
      std::cout << "params:  lifetime = " << lifetime << std::endl;
#endif
      socket_->setSocketOption(GeneralTransportOptions::INTEREST_LIFETIME, lifetime);
      continue;
    }

    if (command == "retransmissions") {
      std::string tmp;
      int rtx;
      line_s >> tmp >> rtx;
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  retransmissions = %d\n", rtx);
#else
      std::cout << "params:  retransmissions = " << rtx << std::endl;
#endif
      socket_->setSocketOption(ConsumerCallbacksOptions::INTEREST_RETRANSMISSION, rtx);
      continue;
    }

    if (command == "beta") {
      std::string tmp;
      line_s >> tmp >> default_beta_;
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  beta = %f\n", default_beta_);
#else
      std::cout << "params:  beta = " << default_beta_ << std::endl;
#endif
      socket_->setSocketOption(RaaqmTransportOptions::BETA_VALUE, default_beta_);
      continue;
    }

    if (command == "drop") {
      std::string tmp;
      line_s >> tmp >> default_drop_;
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  drop = %f\n", default_drop_);
#else
      std::cout << "params:  drop = " << default_drop_ << std::endl;
#endif
      socket_->setSocketOption(RaaqmTransportOptions::DROP_FACTOR, default_drop_);
      continue;
    }

    if (command == "beta_wifi_") {
      std::string tmp;
      line_s >> tmp >> beta_wifi_;
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  beta_wifi_ = %f\n", beta_wifi_);
#else
      std::cout << "params:  beta_wifi_ = " << beta_wifi_ << std::endl;
#endif
      continue;
    }

    if (command == "drop_wifi_") {
      std::string tmp;
      line_s >> tmp >> drop_wifi_;
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  drop_wifi_ = %f\n", drop_wifi_);
#else
      std::cout << "params:  drop_wifi_ = " << drop_wifi_ << std::endl;
#endif
      continue;
    }

    if (command == "beta_lte_") {
      std::string tmp;
      line_s >> tmp >> beta_lte_;
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  beta_lte_ = %f\n", beta_lte_);
#else
      std::cout << "params:  beta_lte_ = " << beta_lte_ << std::endl;
#endif
      continue;
    }

    if (command == "drop_lte_") {
      std::string tmp;
      line_s >> tmp >> drop_lte_;
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  drop_lte_ = %f\n", drop_lte_);
#else
      std::cout << "params:  drop_lte_ = " << drop_lte_ << std::endl;
#endif
      continue;
    }

    if (command == "wifi_delay_") {
      std::string tmp;
      line_s >> tmp >> wifi_delay_;
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  wifi_delay_ = %u\n", wifi_delay_);
#else
      std::cout << "params:  wifi_delay_ = " << wifi_delay_ << std::endl;
#endif
      continue;
    }

    if (command == "lte_delay_") {
      std::string tmp;
      line_s >> tmp >> lte_delay_;
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  lte_delay_ = %u\n", lte_delay_);
#else
      std::cout << "params:  lte_delay_ = " << lte_delay_ << std::endl;
#endif
      continue;
    }
    if (command == "alpha") {
      std::string tmp;
      double rate_alpha = 0.0;
      line_s >> tmp >> rate_alpha;
      socket_->setSocketOption(RateEstimationOptions::RATE_ESTIMATION_ALPHA, rate_alpha);
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  alpha = %f\n", rate_alpha);
#else
      std::cout << "params:  alpha = " << rate_alpha << std::endl;
#endif
      continue;
    }

    if (command == "batching_parameter") {
      std::string tmp;
      int batching_param = 0;
      line_s >> tmp >> batching_param;
      socket_->setSocketOption(RateEstimationOptions::RATE_ESTIMATION_BATCH_PARAMETER, batching_param);
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params:  batching = %d\n", batching_param);
#else
      std::cout << "params:  batching = " << batching_param << std::endl;
#endif
      continue;
    }

    if (command == "rate_estimator") {
      std::string tmp;
      int choice_param = 0;
      line_s >> tmp >> choice_param;
      socket_->setSocketOption(RateEstimationOptions::RATE_ESTIMATION_CHOICE, choice_param);
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "params: choice = %d\n", choice_param);
#else
      std::cout << "params: choice = " << choice_param << std::endl;
#endif
      continue;
    }
  }
  is.close();
#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "init done\n");
#else
  std::cout << "init done" << std::endl;
#endif
}

void RaaqmTransportProtocol::reset() {
  is_final_block_number_discovered_ = false;
  final_block_number_ = std::numeric_limits<uint64_t>::max();
  segment_number_ = 0;
  interests_in_flight_ = 0;
  last_reassembled_segment_ = 0;
  content_buffer_size_ = 0;
  content_buffer_.clear();
  interest_retransmissions_.clear();
  receive_buffer_.clear();
  unverified_segments_.clear();
  verified_manifests_.clear();
}

void RaaqmTransportProtocol::start() {
  if (this->rate_estimator_) {
    this->rate_estimator_->onStart();
  }

  if (!cur_path_) {

    double drop_factor;
    double minimum_drop_probability;
    int sample_number;
    int interest_lifetime;
    double beta;

    socket_->getSocketOption(DROP_FACTOR, drop_factor);
    socket_->getSocketOption(MINIMUM_DROP_PROBABILITY, minimum_drop_probability);
    socket_->getSocketOption(SAMPLE_NUMBER, sample_number);
    socket_->getSocketOption(INTEREST_LIFETIME, interest_lifetime);
    socket_->getSocketOption(BETA_VALUE, beta);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "Drop Factor: %f\n", drop_factor);
    __android_log_print(ANDROID_LOG_DEBUG, "Minimum drop prob: %f\n", minimum_drop_probability);
    __android_log_print(ANDROID_LOG_DEBUG, "Sample number: %d\n", sample_number);
    __android_log_print(ANDROID_LOG_DEBUG, "lifetime: %d\n", interest_lifetime);
    __android_log_print(ANDROID_LOG_DEBUG, "beta: %f\n", beta);
#else
    std::cout << "Drop Factor: " << drop_factor << std::endl;
    std::cout << "Minimum drop prob: " << minimum_drop_probability << std::endl;
    std::cout << "Sample number: " << sample_number << std::endl;
    std::cout << "lifetime: " << interest_lifetime << std::endl;
    std::cout << "beta: " << beta << std::endl;
#endif

    double alpha = 0.0;
    int batching_param = 0;
    int choice_param = 0;
    socket_->getSocketOption(RATE_ESTIMATION_ALPHA, alpha);
    socket_->getSocketOption(RATE_ESTIMATION_BATCH_PARAMETER, batching_param);
    socket_->getSocketOption(RATE_ESTIMATION_CHOICE, choice_param);
    if (choice_param == 1) {
      this->rate_estimator_ = new ALaTcpEstimator();
    } else {
      this->rate_estimator_ = new SimpleEstimator(alpha, batching_param);
    }

    socket_->getSocketOption(RATE_ESTIMATION_OBSERVER, &(this->rate_estimator_->observer_));

    cur_path_ =
        std::make_shared<RaaqmDataPath>(drop_factor, minimum_drop_probability, interest_lifetime * 1000, sample_number);
    path_table_[default_values::path_id] = cur_path_;
  }

  VegasTransportProtocol::start();
}

void RaaqmTransportProtocol::copyContent(ContentObject &content_object) {
  if ((content_object.getName().get(-1).toSegment() == final_block_number_) || !(is_running_)) {
    this->rate_estimator_->onDownloadFinished();
  }
  VegasTransportProtocol::copyContent(content_object);
}

void RaaqmTransportProtocol::updatePathTable(const ContentObject &content_object) {
  unsigned char path_id = content_object.getPathLabel();

  if (path_table_.find(path_id) == path_table_.end()) {
    if (cur_path_) {
      // Create a new path with some default param
      if (path_table_.empty()) {
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "No path initialized for path table, error could be in default path initialization.\n");
#else
        std::cerr << "No path initialized for path table, error could be in default path initialization." << std::endl;
#endif
        exit(EXIT_FAILURE);
      } else {
        // Initiate the new path default param
        std::shared_ptr<RaaqmDataPath>
            new_path = std::make_shared<RaaqmDataPath>(*(path_table_.at(default_values::path_id)));
        // Insert the new path into hash table
        path_table_[path_id] = new_path;
      }
    } else {
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "UNEXPECTED ERROR: when running,current path not found.\n");
#else
      std::cerr << "UNEXPECTED ERROR: when running,current path not found." << std::endl;
#endif
      exit(EXIT_FAILURE);
    }
  }

  cur_path_ = path_table_[path_id];

  size_t packet_size = content_object.getPacketSize();
  size_t data_size = content_object.getContent().size();

  // Update measurements for path
  cur_path_->updateReceivedStats(packet_size, data_size);
}

void RaaqmTransportProtocol::updateRtt(uint64_t segment) {
  if (!cur_path_) {
    throw std::runtime_error("ERROR: no current path found, exit");
  } else {
    std::chrono::microseconds rtt;

    std::chrono::steady_clock::duration duration =
        std::chrono::steady_clock::now() - interest_timepoints_[segment % default_values::default_buffer_size];
    rtt = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    if (this->rate_estimator_) {
      this->rate_estimator_->onRttUpdate(rtt.count());
    }
    cur_path_->insertNewRtt(rtt.count());
    cur_path_->smoothTimer();

    avg_rtt_ = (avg_rtt_ * 0.99) + ((double) rtt.count() * 0.01);
    if (cur_path_->newPropagationDelayAvailable()) {
      check_drop_probability();
    }
  }
}

void RaaqmTransportProtocol::changeInterestLifetime(uint64_t segment) {
  return;
}

void RaaqmTransportProtocol::check_drop_probability() {
  if (!raaqm_autotune_) {
    return;
  }

  unsigned int max_pd = 0;
  std::unordered_map<unsigned char, std::shared_ptr<RaaqmDataPath >>::iterator it;
  for (it = path_table_.begin(); it != path_table_.end(); ++it) {
    if (it->second->getPropagationDelay() > max_pd && it->second->getPropagationDelay() != UINT_MAX
        && !it->second->isStale()) {
      max_pd = it->second->getPropagationDelay();
    }
  }

  double drop_prob = 0;
  double beta = 0;
  if (max_pd < wifi_delay_) {       //only ethernet paths
    drop_prob = default_drop_;
    beta = default_beta_;
  } else if (max_pd < lte_delay_) { //at least one wifi path
    drop_prob = drop_wifi_;
    beta = beta_wifi_;
  } else {         //at least one lte path
    drop_prob = drop_lte_;
    beta = beta_lte_;
  }

  double old_drop_prob = 0;
  double old_beta = 0;
  socket_->getSocketOption(BETA_VALUE, old_beta);
  socket_->getSocketOption(DROP_FACTOR, old_drop_prob);

  if (drop_prob == old_drop_prob && beta == old_beta) {
    return;
  }
#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "*************[RAAQM TUNING] new beta = %f new drop = %f max pd = %u\n", beta, drop_prob, max_pd);
#else
  std::cout << "*************[RAAQM TUNING] new beta = " << beta << " new drop = " << drop_prob << " max pd = "
            << max_pd << std::endl;
#endif
  socket_->setSocketOption(BETA_VALUE, beta);
  socket_->setSocketOption(DROP_FACTOR, drop_prob);

  for (it = path_table_.begin(); it != path_table_.end(); it++) {
    it->second->setDropProb(drop_prob);
  }
}

void RaaqmTransportProtocol::check_for_stale_paths() {
  if (!raaqm_autotune_) {
    return;
  }

  bool stale = false;
  std::unordered_map<unsigned char, std::shared_ptr<RaaqmDataPath >>::iterator it;
  for (it = path_table_.begin(); it != path_table_.end(); ++it) {
    if (it->second->isStale()) {
      stale = true;
      break;
    }
  }
  if (stale) {
    check_drop_probability();
  }
}

void RaaqmTransportProtocol::onTimeout(const Interest &interest) {
  check_for_stale_paths();
  VegasTransportProtocol::onTimeout(interest);
}

void RaaqmTransportProtocol::increaseWindow() {
  double max_window_size = -1;
  socket_->getSocketOption(MAX_WINDOW_SIZE, max_window_size);
  if (current_window_size_ < max_window_size) // don't expand window above max level
  {
    double gamma = -1;
    socket_->getSocketOption(GAMMA_VALUE, gamma);

    current_window_size_ += gamma / current_window_size_;
    socket_->setSocketOption(CURRENT_WINDOW_SIZE, current_window_size_);
  }
  this->rate_estimator_->onWindowIncrease(current_window_size_);
}

void RaaqmTransportProtocol::decreaseWindow() {
  double min_window_size = -1;
  socket_->getSocketOption(MIN_WINDOW_SIZE, min_window_size);
  if (current_window_size_ > min_window_size) // don't shrink window below minimum level
  {
    double beta = -1;
    socket_->getSocketOption(BETA_VALUE, beta);

    current_window_size_ = current_window_size_ * beta;
    if (current_window_size_ < min_window_size) {
      current_window_size_ = min_window_size;
    }

    socket_->setSocketOption(CURRENT_WINDOW_SIZE, current_window_size_);
  }
  this->rate_estimator_->onWindowDecrease(current_window_size_);
}

void RaaqmTransportProtocol::RAAQM() {
  if (!cur_path_) {
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "ERROR: no current path found, exit\n");
#else
    std::cerr << "ERROR: no current path found, exit" << std::endl;
#endif
    exit(EXIT_FAILURE);
  } else {
    // Change drop probability according to RTT statistics
    cur_path_->updateDropProb();

    if (rand() % 10000 <= cur_path_->getDropProb() * 10000) {
      decreaseWindow();
    }
  }
}

void RaaqmTransportProtocol::afterDataUnsatisfied(uint64_t segment) {
  // Decrease the window because the timeout happened
  decreaseWindow();
}

void RaaqmTransportProtocol::afterContentReception(const Interest &interest, const ContentObject &content_object) {
  updatePathTable(content_object);
  increaseWindow();
  updateRtt(interest.getName().get(-1).toSegment());
  this->rate_estimator_->onDataReceived((int) content_object.getPacketSize());
  // Set drop probablility and window size accordingly
  RAAQM();
}

void RaaqmTransportProtocol::checkForFastRetransmission(const Interest &interest) {
}

#if 0
void
RaaqmTransportProtocol::onInterest(const Interest &interest)
{
  bool mobility = interest.get_MobilityLossFlag();

  if(mobility){
    const Name &name = interest.getName();
    uint64_t segment = name[-1].toSegment();
    timeval now;
    gettimeofday(&now, 0);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "%ld.%u RAAQM: M-Interest %ld %s\n", (long) now.tv_sec, (unsigned) now.tv_usec, segment, interest.getName());
#else
    std::cout << (long) now.tv_sec << "." << (unsigned) now.tv_usec << " RAAQM: M-Interest " <<
    segment << " " << interest.getName() << std::endl;
#endif
    NackSet::iterator it = m_nackSet.find(segment);
    if(it == m_nackSet.end()){
      m_nackSet.insert(segment);
    }
  }
}
#endif

} // end namespace icnet
