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

#ifndef Libccnx_config_CryptoCache_h
#define Libccnx_config_CryptoCache_h

#include <ccnx/transport/common/ccnx_TransportConfig.h>

/**
 * Generates:
 *
 * { "KEYS" :
 *   [
 *     { "KEYID" : base64{ keyhash },
 *       "KEY" : base64{ derEncodedKey },
 *       "PREFIXES" : [ uripath, uripath, ... uripath ]
 *     },
 *     { "KEYID" : base64{ keyhash },
 *       "KEY" : base64{ derEncodedKey },
 *       "PREFIXES" : [ uripath, uripath, ... uripath ]
 *     },
 *     ...
 *   ]
 * }
 */
CCNxConnectionConfig *cryptoCache_ConnectionConfig(CCNxConnectionConfig *connConfig, const char *filename, const char *password);

const char *cryptoCache_GetName(void);
#endif // Libccnx_config_CryptoCache_h
