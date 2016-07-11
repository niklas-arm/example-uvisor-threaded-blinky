/*
 * Copyright (c) 2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LED1_H
#define LED1_H

#include "uvisor-lib/uvisor-lib.h"
#include <stdint.h>

UVISOR_EXTERN int led1_display_secret_async(uvisor_rpc_result_t *, uint32_t a, uint32_t b);
UVISOR_EXTERN int led1_display_secret_sync(uint32_t p0, uint32_t p1);

#endif
