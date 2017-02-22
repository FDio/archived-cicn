//
// Created by msardara on 16/02/2017.
//

#ifndef ICNET_CCNX_KEY_LOCATOR_TYPE_H_
#define ICNET_CCNX_KEY_LOCATOR_TYPE_H_

namespace icnet {

namespace ccnx {

enum Type {
  NAME = 0, KEY_DIGEST = 1, UNKNOWN = 255
};

typedef enum Type KeyLocatorType;

} // end namespace ccnx

} // end namespace icnet

#endif // ICNET_CCNX_KEY_LOCATOR_TYPE_H_
