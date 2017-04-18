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

#include "icnet_socket_consumer.h"

namespace icnet {

ConsumerSocket::ConsumerSocket(Name prefix, int protocol)
    : is_running_(false),
      name_prefix_(prefix),
      interest_lifetime_(default_values::interest_lifetime),
      min_window_size_(default_values::min_window_size),
      max_window_size_(default_values::max_window_size),
      current_window_size_(-1),
      max_retransmissions_(default_values::transport_protocol_max_retransmissions),
    /****** RAAQM Parameters ******/
      minimum_drop_probability_(default_values::minimum_drop_probability),
      sample_number_(default_values::sample_number),
      gamma_(default_values::gamma_value),
      beta_(default_values::beta_value),
      drop_factor_(default_values::drop_factor),
    /****** END RAAQM Parameters ******/
      rate_estimation_alpha_(default_values::rate_alpha),
      rate_estimation_observer_(NULL),
      is_async_(false),
      on_interest_output_(VOID_HANDLER),
      on_interest_timeout_(VOID_HANDLER),
      on_interest_satisfied_(VOID_HANDLER),
      on_content_object_input_(VOID_HANDLER),
      on_content_object_verification_(VOID_HANDLER),
      on_content_object_(VOID_HANDLER),
      on_manifest_(VOID_HANDLER),
      on_payload_retrieved_(VOID_HANDLER),
      virtual_download_(false),
      rtt_stats_(false) {

  portal_ = std::make_shared<Portal>();

  switch (protocol) {
    case TransportProtocolAlgorithms::VEGAS: {
      transport_protocol_ = std::make_shared<VegasTransportProtocol>(this);
      break;
    }
    case TransportProtocolAlgorithms::RAAQM: {
      transport_protocol_ = std::make_shared<RaaqmTransportProtocol>(this);
      break;
    }
  }
}

ConsumerSocket::~ConsumerSocket() {
  stop();
  transport_protocol_.reset();
  portal_.reset();
}

int ConsumerSocket::consume(Name suffix) {
  if (is_running_) {
    portal_->getIoService().post(std::bind(&ConsumerSocket::postponedConsume, this, suffix));
    return CONSUMER_BUSY;
  }

  if (is_async_) {
    portal_ = std::make_shared<Portal>();
    transport_protocol_->updatePortal();
  }

  name_suffix_ = suffix;
  is_async_ = false;
  transport_protocol_->start();
  is_running_ = false;
  return CONSUMER_READY;
}

void ConsumerSocket::postponedConsume(Name name_suffix) {
  if (is_async_) {
    portal_ = std::make_shared<Portal>();
    transport_protocol_->updatePortal();
  }

  name_suffix_ = name_suffix;
  is_async_ = false;
  transport_protocol_->start();
}

int ConsumerSocket::asyncConsume(Name suffix) {
  if (transport_protocol_->isRunning()) {
    return CONSUMER_BUSY;
  }

  name_suffix_ = suffix;
  is_async_ = true;
  transport_protocol_->start();
  return CONSUMER_READY;
}

void ConsumerSocket::stop() {
  if (transport_protocol_->isRunning()) {
    transport_protocol_->stop();
  }

  is_running_ = false;
}

int ConsumerSocket::setSocketOption(int socket_option_key, double socket_option_value) {
  switch (socket_option_key) {
    case MIN_WINDOW_SIZE:
      min_window_size_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case MAX_WINDOW_SIZE:
      max_window_size_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case CURRENT_WINDOW_SIZE:
      current_window_size_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case GAMMA_VALUE:
      gamma_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case BETA_VALUE:
      beta_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case DROP_FACTOR:
      drop_factor_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case MINIMUM_DROP_PROBABILITY:
      minimum_drop_probability_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case RATE_ESTIMATION_ALPHA:
      if (socket_option_value >= 0 && socket_option_value < 1) {
        rate_estimation_alpha_ = socket_option_value;
      } else {
        rate_estimation_alpha_ = ALPHA;
      }
      return SOCKET_OPTION_SET;
    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::setSocketOption(int socket_option_key, int socket_option_value) {
  switch (socket_option_key) {

    case GeneralTransportOptions::INPUT_BUFFER_SIZE:
      input_buffer_size_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case GeneralTransportOptions::OUTPUT_BUFFER_SIZE:
      output_buffer_size_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case GeneralTransportOptions::MAX_INTEREST_RETX:
      max_retransmissions_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case GeneralTransportOptions::INTEREST_LIFETIME:
      interest_lifetime_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ConsumerCallbacksOptions::INTEREST_RETRANSMISSION:
      if (socket_option_value == VOID_HANDLER) {
        on_interest_retransmission_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ConsumerCallbacksOptions::INTEREST_EXPIRED:
      if (socket_option_value == VOID_HANDLER) {
        on_interest_timeout_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ConsumerCallbacksOptions::INTEREST_SATISFIED:
      if (socket_option_value == VOID_HANDLER) {
        on_interest_satisfied_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ConsumerCallbacksOptions::INTEREST_OUTPUT:
      if (socket_option_value == VOID_HANDLER) {
        on_interest_output_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ConsumerCallbacksOptions::CONTENT_OBJECT_INPUT:
      if (socket_option_value == VOID_HANDLER) {
        on_content_object_input_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ConsumerCallbacksOptions::CONTENT_OBJECT_TO_VERIFY:
      if (socket_option_value == VOID_HANDLER) {
        on_content_object_verification_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case ConsumerCallbacksOptions::CONTENT_RETRIEVED:
      if (socket_option_value == VOID_HANDLER) {
        on_payload_retrieved_ = VOID_HANDLER;
        return SOCKET_OPTION_SET;
      }

    case RateEstimationOptions::RATE_ESTIMATION_BATCH_PARAMETER:
      if (socket_option_value > 0) {
        rate_estimation_batching_parameter_ = socket_option_value;
      } else {
        rate_estimation_batching_parameter_ = BATCH;
      }
      return SOCKET_OPTION_SET;

    case RateEstimationOptions::RATE_ESTIMATION_CHOICE:
      if (socket_option_value > 0) {
        rate_estimation_choice_ = socket_option_value;
      } else {
        rate_estimation_choice_ = RATE_CHOICE;
      }
      return SOCKET_OPTION_SET;
    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::setSocketOption(int socket_option_key, size_t socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::INPUT_BUFFER_SIZE:
      input_buffer_size_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case GeneralTransportOptions::OUTPUT_BUFFER_SIZE:
      output_buffer_size_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::setSocketOption(int socket_option_key, bool socket_option_value) {
  switch (socket_option_key) {

    case GeneralTransportOptions::RUNNING:
      is_running_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case OtherOptions::VIRTUAL_DOWNLOAD:
      virtual_download_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case RaaqmTransportOptions::RTT_STATS:
      rtt_stats_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::setSocketOption(int socket_option_key, Name socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::NAME_PREFIX:
      name_prefix_ = socket_option_value;;
      return SOCKET_OPTION_SET;

    case GeneralTransportOptions::NAME_SUFFIX:
      name_suffix_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::setSocketOption(int socket_option_key, ConsumerContentObjectCallback socket_option_value) {
  switch (socket_option_key) {
    case ConsumerCallbacksOptions::CONTENT_OBJECT_INPUT:
      on_content_object_input_ = socket_option_value;;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::setSocketOption(int socket_option_key, ProducerContentObjectCallback socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

int ConsumerSocket::setSocketOption(int socket_option_key,
                                    ConsumerContentObjectVerificationCallback socket_option_value) {
  switch (socket_option_key) {
    case ConsumerCallbacksOptions::CONTENT_OBJECT_TO_VERIFY:
      on_content_object_verification_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::setSocketOption(int socket_option_key, ConsumerInterestCallback socket_option_value) {
  switch (socket_option_key) {
    case ConsumerCallbacksOptions::INTEREST_RETRANSMISSION:
      on_interest_retransmission_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ConsumerCallbacksOptions::INTEREST_OUTPUT:
      on_interest_output_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ConsumerCallbacksOptions::INTEREST_EXPIRED:
      on_interest_timeout_ = socket_option_value;
      return SOCKET_OPTION_SET;

    case ConsumerCallbacksOptions::INTEREST_SATISFIED:
      on_interest_satisfied_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::setSocketOption(int socket_option_key, ProducerInterestCallback socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

int ConsumerSocket::setSocketOption(int socket_option_key, ConsumerContentCallback socket_option_value) {
  switch (socket_option_key) {
    case ConsumerCallbacksOptions::CONTENT_RETRIEVED:
      on_payload_retrieved_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::setSocketOption(int socket_option_key, ConsumerManifestCallback socket_option_value) {
  switch (socket_option_key) {
    case ConsumerCallbacksOptions::MANIFEST_INPUT:
      on_manifest_ = socket_option_value;
      return SOCKET_OPTION_SET;

    default:
      return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::setSocketOption(int socket_option_key, KeyLocator socket_option_value) {
  return SOCKET_OPTION_NOT_SET;
}

int ConsumerSocket::setSocketOption(int socket_option_key, IcnObserver *socket_option_value) {
  if (socket_option_key == RateEstimationOptions::RATE_ESTIMATION_OBSERVER) {
    rate_estimation_observer_ = socket_option_value;
    return SOCKET_OPTION_SET;
  } else {
    return SOCKET_OPTION_NOT_SET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, double &socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::MIN_WINDOW_SIZE:
      socket_option_value = min_window_size_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::MAX_WINDOW_SIZE:
      socket_option_value = max_window_size_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::CURRENT_WINDOW_SIZE:
      socket_option_value = current_window_size_;
      return SOCKET_OPTION_GET;

      // RAAQM parameters

    case RaaqmTransportOptions::GAMMA_VALUE:
      socket_option_value = gamma_;
      return SOCKET_OPTION_GET;

    case RaaqmTransportOptions::BETA_VALUE:
      socket_option_value = beta_;
      return SOCKET_OPTION_GET;

    case RaaqmTransportOptions::DROP_FACTOR:
      socket_option_value = drop_factor_;
      return SOCKET_OPTION_GET;

    case RaaqmTransportOptions::MINIMUM_DROP_PROBABILITY:
      socket_option_value = minimum_drop_probability_;
      return SOCKET_OPTION_GET;

    case RateEstimationOptions::RATE_ESTIMATION_ALPHA:
      socket_option_value = rate_estimation_alpha_;
      return SOCKET_OPTION_GET;
    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, int &socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::INPUT_BUFFER_SIZE:
      socket_option_value = input_buffer_size_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::OUTPUT_BUFFER_SIZE:
      socket_option_value = output_buffer_size_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::MAX_INTEREST_RETX:
      socket_option_value = max_retransmissions_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::INTEREST_LIFETIME:
      socket_option_value = interest_lifetime_;
      return SOCKET_OPTION_GET;

    case RaaqmTransportOptions::SAMPLE_NUMBER:
      socket_option_value = sample_number_;
      return SOCKET_OPTION_GET;

    case RateEstimationOptions::RATE_ESTIMATION_BATCH_PARAMETER:
      socket_option_value = rate_estimation_batching_parameter_;
      return SOCKET_OPTION_GET;

    case RateEstimationOptions::RATE_ESTIMATION_CHOICE:
      socket_option_value = rate_estimation_choice_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, size_t &socket_option_value) {
  switch (socket_option_key) {
    case INPUT_BUFFER_SIZE:
      socket_option_value = input_buffer_size_;
      return SOCKET_OPTION_GET;

    case OUTPUT_BUFFER_SIZE:
      socket_option_value = output_buffer_size_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, bool &socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::ASYNC_MODE:
      socket_option_value = is_async_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::RUNNING:
      socket_option_value = is_running_;
      return SOCKET_OPTION_GET;

    case OtherOptions::VIRTUAL_DOWNLOAD:
      socket_option_value = virtual_download_;
      return SOCKET_OPTION_GET;

    case RaaqmTransportOptions::RTT_STATS:
      socket_option_value = rtt_stats_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, Name &socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::NAME_PREFIX:
      socket_option_value = name_prefix_;
      return SOCKET_OPTION_GET;

    case GeneralTransportOptions::NAME_SUFFIX:
      socket_option_value = name_suffix_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, ConsumerContentObjectCallback &socket_option_value) {
  switch (socket_option_key) {
    case ConsumerCallbacksOptions::CONTENT_OBJECT_INPUT:
      socket_option_value = on_content_object_input_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, ProducerContentObjectCallback &socket_option_value) {
  return SOCKET_OPTION_NOT_GET;
}

int ConsumerSocket::getSocketOption(int socket_option_key,
                                    ConsumerContentObjectVerificationCallback &socket_option_value) {
  switch (socket_option_key) {
    case ConsumerCallbacksOptions::CONTENT_OBJECT_TO_VERIFY:
      socket_option_value = on_content_object_verification_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, ConsumerInterestCallback &socket_option_value) {
  switch (socket_option_key) {
    case ConsumerCallbacksOptions::INTEREST_RETRANSMISSION:
      socket_option_value = on_interest_retransmission_;
      return SOCKET_OPTION_GET;

    case ConsumerCallbacksOptions::INTEREST_OUTPUT:
      socket_option_value = on_interest_output_;
      return SOCKET_OPTION_GET;

    case ConsumerCallbacksOptions::INTEREST_EXPIRED:
      socket_option_value = on_interest_timeout_;
      return SOCKET_OPTION_GET;

    case ConsumerCallbacksOptions::INTEREST_SATISFIED:
      socket_option_value = on_interest_satisfied_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, ProducerInterestCallback &socket_option_value) {
  return SOCKET_OPTION_NOT_GET;
}

int ConsumerSocket::getSocketOption(int socket_option_key, ConsumerContentCallback &socket_option_value) {
  switch (socket_option_key) {
    case ConsumerCallbacksOptions::CONTENT_RETRIEVED:
      socket_option_value = on_payload_retrieved_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, ConsumerManifestCallback &socket_option_value) {
  switch (socket_option_key) {
    case ConsumerCallbacksOptions::MANIFEST_INPUT:
      socket_option_value = on_manifest_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, KeyLocator &socket_option_value) {
  switch (socket_option_key) {
    case GeneralTransportOptions::KEY_LOCATOR:
      socket_option_value = key_locator_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, std::shared_ptr<Portal> &socket_option_value) {
  switch (socket_option_key) {
    case PORTAL:
      socket_option_value = portal_;
      return SOCKET_OPTION_GET;

    default:
      return SOCKET_OPTION_NOT_GET;
  }
}

int ConsumerSocket::getSocketOption(int socket_option_key, IcnObserver **socket_option_value) {
  if (socket_option_key == RATE_ESTIMATION_OBSERVER) {
    *socket_option_value = (rate_estimation_observer_);
    return SOCKET_OPTION_GET;
  } else {
    return SOCKET_OPTION_NOT_GET;
  }
}

} // end namespace icnet
