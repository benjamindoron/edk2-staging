/** @file
  Sample ACPI Platform Driver

  Copyright (c) 2008 - 2012, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _ACPI_PLATFORM_H_INCLUDED_
#define _ACPI_PLATFORM_H_INCLUDED_

#include <PiDxe.h>

#include <Protocol/AcpiTable.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/PciIo.h>

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/AcpiTdx.h>

typedef struct {
  EFI_PCI_IO_PROTOCOL *PciIo;
  UINT64              PciAttributes;
} ORIGINAL_ATTRIBUTES;

typedef
EFI_STATUS
(EFIAPI *ACPI_TABLE_INSTALL_TABLES)(
  IN   EFI_ACPI_TABLE_PROTOCOL       *AcpiProtocol
);

EFI_STATUS
EFIAPI
InstallAcpiTablesFromFv (
  IN  EFI_ACPI_TABLE_PROTOCOL     *AcpiProtocol
  );

EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint (
  IN EFI_HANDLE                     ImageHandle,
  IN EFI_SYSTEM_TABLE               *SystemTable,
  IN ACPI_TABLE_INSTALL_TABLES      InstallTablesFunction
  );

EFI_STATUS
EFIAPI
InstallAcpiTable (
  IN   EFI_ACPI_TABLE_PROTOCOL       *AcpiProtocol,
  IN   VOID                          *AcpiTableBuffer,
  IN   UINTN                         AcpiTableBufferSize,
  OUT  UINTN                         *TableKey
  );

VOID
EnablePciDecoding (
  OUT ORIGINAL_ATTRIBUTES **OriginalAttributes,
  OUT UINTN               *Count
  );

VOID
RestorePciDecoding (
  IN ORIGINAL_ATTRIBUTES *OriginalAttributes,
  IN UINTN               Count
  );

#endif
