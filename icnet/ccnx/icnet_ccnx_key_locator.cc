//
// Created by msardara on 01/11/16.
//

#include "icnet_ccnx_key_locator.h"

namespace icnet {

namespace ccnx {

KeyLocator::KeyLocator() : type_(KeyLocatorType::UNKNOWN) {
}

KeyLocator::KeyLocator(KeyLocatorType type, Name &name) : type_(type), name_(name) {
}

Name &KeyLocator::getName() {
  return name_;
}

void KeyLocator::setName(Name &name) {
  name_ = name;
}

void KeyLocator::setType(KeyLocatorType type) {
  type_ = type;
}

KeyLocatorType KeyLocator::getType() {
  return type_;
}

void KeyLocator::clear() {
  type_ = KeyLocatorType::UNKNOWN;
  name_.clear();
}

} // end namespace ccnx

} // end namespace icnet