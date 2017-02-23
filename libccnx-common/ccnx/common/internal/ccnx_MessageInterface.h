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

/**
 */

#ifndef CCNx_Common_ccnx_MessageInterface_h
#define CCNx_Common_ccnx_MessageInterface_h

/**
 * A type to represent the ContentObject/Interest/Control Interface pointer. The CCNxTlvDictionary
 * maintains one that points to the appropriate interface implementation to reference the data
 * contained in the CCNxTlvDictionary. For example, it might point to a CCNxContentObjectInterface,
 * which would enable accessing the CCNxTlvDictionary as a ContentObject.
 *
 *
 * @see CCNxContentObjectInterface
 * @see CCNxInterestInterface
 */
typedef void CCNxMessageInterface;
#endif
