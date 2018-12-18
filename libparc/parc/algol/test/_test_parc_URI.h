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

/*
 * _test_parc_URI.h
 * PARC Algol
 */

#ifndef PARC_Algol__test_parc_URI_h
#define PARC_Algol__test_parc_URI_h

#define URI_SCHEME "lci"

#define URI_AUTHORITY_USERINFO   "user:pass"
#define URI_AUTHORITY_USERINFO_2 "user2:pass2"
#define URI_AUTHORITY_HOSTNAME   "parc.com"
#define URI_AUTHORITY_HOSTNAME_2 "xerox.com"
#define URI_AUTHORITY_LITERAL_HOSTNAME "127.0.0.1"
#define URI_AUTHORITY_LITERAL_HOSTNAME6 "[::0]"
#define URI_AUTHORITY_LITERAL_HOSTNAME6_2 "[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]"
#define URI_AUTHORITY_PORT_1 "1234"
#define URI_AUTHORITY_PORT_2 "5678"

#define URI_AUTHORITY                 URI_AUTHORITY_USERINFO "@" URI_AUTHORITY_HOSTNAME ":" URI_AUTHORITY_PORT_1
#define URI_AUTHORITY_DIFFERENT_PORT  URI_AUTHORITY_USERINFO "@" URI_AUTHORITY_HOSTNAME ":" URI_AUTHORITY_PORT_2
#define URI_AUTHORITY_DIFFERENT_HOST  URI_AUTHORITY_USERINFO "@" URI_AUTHORITY_HOSTNAME_2 ":" URI_AUTHORITY_PORT_1
#define URI_AUTHORITY_LITERAL_HOST    URI_AUTHORITY_USERINFO "@" URI_AUTHORITY_LITERAL_HOSTNAME ":" URI_AUTHORITY_PORT_1
#define URI_AUTHORITY_LITERAL_HOST6   URI_AUTHORITY_USERINFO "@" URI_AUTHORITY_LITERAL_HOSTNAME6 ":" URI_AUTHORITY_PORT_1
#define URI_AUTHORITY_LITERAL_HOST6_2 URI_AUTHORITY_USERINFO "@" URI_AUTHORITY_LITERAL_HOSTNAME6_2 ":" URI_AUTHORITY_PORT_1
#define URI_AUTHORITY_DIFFERENT_USER  URI_AUTHORITY_USERINFO_2 "@" URI_AUTHORITY_HOSTNAME ":" URI_AUTHORITY_PORT_1

#define URI_PATH_SEGMENT "%00%01%02-.%03%04_%05%06%07%08%09abcdefghijklmnopqrstuvwxyz"
#define URI_PATH_SEGMENT_WITH_SLASHES URI_PATH_SEGMENT "//////"

#define URI_PATH "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT

#define URI_QUERY "x=1&y=2&z=3"

#define URI_FRAGMENT "alphabetagamma"

#define URI_FULL URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH "?" URI_QUERY "#" URI_FRAGMENT

char *TEST_URI_SCHEME = URI_SCHEME;
char *TEST_URI_AUTHORITY = URI_AUTHORITY;
#endif // PARC_Algol__test_parc_URI_h
