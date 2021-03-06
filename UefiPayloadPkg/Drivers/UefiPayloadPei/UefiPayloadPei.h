/** @file
  This PEIM processes platform info handed from previous stage firmware.

  Copyright (c) 2014 - 2018, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#ifndef __PEI_COREBOOT_SUPPORT_H__
#define __PEI_COREBOOT_SUPPORT_H__

#include <PiPei.h>

#include <Library/PeimEntryPoint.h>
#include <Library/PeiServicesLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PlatformInfoParseLib.h>
#include <Library/MtrrLib.h>
#include <Library/IoLib.h>

#include <Guid/SmramMemoryReserve.h>
#include <Guid/MemoryTypeInformation.h>
#include <Guid/FirmwareFileSystem2.h>
#include <Guid/FrameBufferInfoGuid.h>
#include <Guid/SystemTableInfoGuid.h>
#include <Guid/AcpiBoardInfoGuid.h>

#include <Ppi/MasterBootMode.h>
#include <Ppi/VtdInfo.h>
#include "Coreboot.h"

typedef struct {
  UINT32  UsableLowMemMinSize;
  UINT32  UsableLowMemTop;
  UINT32  SystemLowMemTop;
} PAYLOAD_MEM_INFO;

#endif
