/** @file

Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>

#include <Guid/VariableFormat.h>

#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/ProtectedVariableLib.h>

#include <Library/UnitTestLib.h>

#include <Library/ProtectedVariableLib/ProtectedVariableInternal.h>

#include "StubRpmcLib.h"
#include "StubVariableStore.h"
#include "StubFvbProtocol.h"
#include "StubSmmComm.h"
#include "TestSmmProtectedVariableLib.h"
#include "TestProtectedVariableLib.h"
#include "TestStubLib.h"

#include "var_fv.c"

/**
  Test Case:
    - Add one encryped variable
    - Update it with new data
**/

STATIC EFI_GUID  TestVar1Guid = {
  0x03EB1519,
  0xA9A0,
  0x4C43,
  {0xBD, 0x22, 0xF3, 0xAC, 0x73, 0xF1, 0x54, 0x5B}
};

STATIC EFI_GUID  TestVar2Guid = {
  0xB872A153,
  0xDC9F,
  0x41EB,
  {0x81, 0x8F, 0x00, 0xE6, 0x6D, 0x66, 0xB1, 0x7B}
};

// 04B37FE8-F6AE-480B-BDD5-37D98C5E89AA
STATIC EFI_GUID  VarErrorFlagGuid = {
  0x04b37fe8,
  0xf6ae,
  0x480b,
  {0xbd, 0xd5, 0x37, 0xd9, 0x8c, 0x5e, 0x89, 0xaa}
};

// B872A153-DC9F-41EB-818F-00E66D66B17B
STATIC EFI_GUID  TestVar3Guid = {
  0xb872a153,
  0xdc9f,
  0x41eb,
  {0x81, 0x8f, 0x00, 0xe6, 0x6d, 0x66, 0xb1, 0x7b}
};

// B872A153-DC9F-41EB-818F-00E66D66B17B
#define TestVar4Guid TestVar3Guid

// B872A153-DC9F-41EB-818F-00E66D66B17B
#define TestVar5Guid TestVar3Guid

// E3E890AD-5B67-466E-904F-94CA7E9376BB
STATIC EFI_GUID  MetaDataHmacVarGuid = {
  0xe3e890ad,
  0x5b67,
  0x466e,
  {0x90, 0x4f, 0x94, 0xca, 0x7e, 0x93, 0x76, 0xbb}
};

EFI_STATUS
EFIAPI
GetVariableInfoPei (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN OUT  PROTECTED_VARIABLE_INFO   *VariableInfo
  );

EFI_STATUS
EFIAPI
GetNextVariableInfoPei (
  IN      VARIABLE_STORE_HEADER     *VariableStore,
  IN      VARIABLE_HEADER           *VariableStart OPTIONAL,
  IN      VARIABLE_HEADER           *VariableEnd OPTIONAL,
  IN  OUT PROTECTED_VARIABLE_INFO   *VariableInfo
  );

EFI_STATUS
EFIAPI
InitNvVariableStorePei (
     OUT  EFI_PHYSICAL_ADDRESS              StoreCacheBase OPTIONAL,
  IN OUT  UINT32                            *StoreCacheSize,
     OUT  UINT32                            *IndexTable OPTIONAL,
     OUT  UINT32                            *VariableNumber OPTIONAL,
     OUT  BOOLEAN                           *AuthFlag OPTIONAL
  );

EFI_STATUS
EFIAPI
ProtectedVariableLibInitializePei (
  IN  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn
  );

EFI_STATUS
EFIAPI
ProtectedVariableLibInitializeSmm (
  IN  PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn
  );

EFI_STATUS
EFIAPI
SmmVariableSetVariable (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data
  );

EFI_STATUS
VerifyMetaDataHmac (
  IN      PROTECTED_VARIABLE_CONTEXT_IN   *ContextIn,
  IN OUT  PROTECTED_VARIABLE_GLOBAL       *Global
  );

EFI_STATUS
FindVariableInSmm (
  IN      CHAR16                            *VariableName,
  IN      EFI_GUID                          *VendorGuid,
  OUT     UINT32                            *Attributes OPTIONAL,
  IN OUT  UINTN                             *DataSize,
  OUT     VOID                              *Data OPTIONAL
  );

VOID
EFIAPI
SmmVariableReady (
  IN  EFI_EVENT                             Event,
  IN  VOID                                  *Context
  );

VOID
EFIAPI
SmmVariableWriteReady (
  IN  EFI_EVENT                             Event,
  IN  VOID                                  *Context
  );

EFI_STATUS
EFIAPI
GetProtectedVariableContextSmm (
  OUT PROTECTED_VARIABLE_CONTEXT_IN   **ContextIn OPTIONAL,
  OUT PROTECTED_VARIABLE_GLOBAL       **Global OPTIONAL
  );

extern VARIABLE_MODULE_GLOBAL       *mVariableModuleGlobalSmm;
extern EFI_LOCK                      mVariableServicesLock;

EFI_HANDLE    mHandle;

EFI_STATUS
Stub_CreateEvent (
  IN  UINT32                       Type,
  IN  EFI_TPL                      NotifyTpl,
  IN  EFI_EVENT_NOTIFY             NotifyFunction,
  IN  VOID                         *NotifyContext,
  OUT EFI_EVENT                    *Event
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
Stub_CreateEventEx (
  IN       UINT32                 Type,
  IN       EFI_TPL                NotifyTpl,
  IN       EFI_EVENT_NOTIFY       NotifyFunction OPTIONAL,
  IN CONST VOID                   *NotifyContext OPTIONAL,
  IN CONST EFI_GUID               *EventGroup    OPTIONAL,
  OUT      EFI_EVENT              *Event
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
Stub_CloseEvent (
  IN EFI_EVENT                Event
  )
{
  return EFI_SUCCESS;
}

STATIC STUB_INFO   mStub1 = {
  (void *)GetVariableStore,
  (void *)Stub_GetVariableStore,
  {0xcc}
};

STATIC STUB_INFO   mStub2 = {
  (void *)GetNvVariableStore,
  (void *)Stub_GetNvVariableStore,
  {0xcc}
};

UNIT_TEST_STATUS
EFIAPI
TC26_Setup (
  UNIT_TEST_CONTEXT           Context
  )
{
  PROTECTED_VARIABLE_CONTEXT_IN   ContextIn;
  EFI_STATUS                      Status;
  PROTECTED_VARIABLE_GLOBAL       *Global;
  EFI_FIRMWARE_VOLUME_HEADER      *VarFv;
  EFI_PHYSICAL_ADDRESS            Addr;

  mCounter = tc26_rpmc;
  gIvec = tc26_ivec;
  mHandle = NULL;

  mVariableFv = AllocatePool ((UINTN)((EFI_FIRMWARE_VOLUME_HEADER *)tc26_var_fv)->FvLength);
  if (mVariableFv == NULL) {
    return UNIT_TEST_ERROR_PREREQUISITE_NOT_MET;
  }
  CopyMem (mVariableFv, tc26_var_fv, sizeof (tc26_var_fv));

  STUB_FUNC (&mStub1);
  STUB_FUNC (&mStub2);

  //
  // Initialze PEI environment
  //
  ContextIn.StructSize = sizeof (ContextIn);
  ContextIn.StructVersion = 1;

  ContextIn.FindVariableSmm     = NULL;
  ContextIn.GetVariableInfo     = NULL;
  ContextIn.IsUserVariable      = NULL;
  ContextIn.UpdateVariableStore = NULL;

  ContextIn.VariableServiceUser = FromPeiModule;
  ContextIn.GetNextVariableInfo = GetNextVariableInfoPei;
  ContextIn.InitVariableStore   = InitNvVariableStorePei;
  ContextIn.GetVariableInfo     = GetVariableInfoPei;

  //
  // Use PEI code to initialize encrytped variable HOB
  //
  Status = ProtectedVariableLibInitializePei (&ContextIn);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  //
  // Initialze SMM environment
  //
  Stub_FvbInitialize ((EFI_FIRMWARE_VOLUME_HEADER *)mVariableFv);
  Stub_MmInitialize ();
  BuildCpuHob (48, 20);

  MmVariableServiceInitialize ();

  Status = GetProtectedVariableContextSmm (NULL, &Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);
  
  Stub_FvbInitialize ((EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)Global->ProtectedVariableCache);

  VariableCommonInitialize ();
  UT_ASSERT_NOT_EQUAL (mVariableModuleGlobalSmm, NULL);
  mVariableModuleGlobalSmm->FvbInstance = &gStubFvb;
  gStubFvb.GetPhysicalAddress(&gStubFvb, &Addr);
  VarFv = (EFI_FIRMWARE_VOLUME_HEADER *)(UINTN)Addr;
  mVariableModuleGlobalSmm->VariableGlobal.NonVolatileVariableBase = Addr + VarFv->HeaderLength;

  //
  // Initialze SMM Runtime environment
  //
  EfiInitializeLock (&mVariableServicesLock, TPL_NOTIFY);

  Status = gBS->InstallProtocolInterface (
                  &mHandle,
                  &gEfiSmmVariableProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->InstallProtocolInterface (
                  &mHandle,
                  &gEfiSmmCommunicationProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &gSmmComm
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->InstallProtocolInterface (
                  &mHandle,
                  &gSmmVariableWriteGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  gBS->CreateEventEx = Stub_CreateEventEx;
  gBS->CreateEvent = Stub_CreateEvent;
  gBS->CloseEvent  = Stub_CloseEvent;
  SmmVariableReady (NULL, NULL);
  SmmVariableWriteReady (NULL, NULL);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TC26_TearDown (
  UNIT_TEST_CONTEXT           Context
  )
{
  mCounter = 0x77;

  if (mVariableFv != NULL) {
    FreePool (mVariableFv);
    mVariableFv = NULL;
  }

  RESET_FUNC (&mStub1);
  RESET_FUNC (&mStub2);

  return UNIT_TEST_PASSED;
}

STATIC
UNIT_TEST_STATUS
VerifyVariableStorages (
  IN UNIT_TEST_CONTEXT              Context,
  IN  PROTECTED_VARIABLE_CONTEXT_IN *ContextIn,
  IN  VARIABLE_STORE_HEADER         **VariableStore,
  IN  UINTN                         StoreNumber
  )
{
  EFI_STATUS                    Status;
  UINTN                         Index;
  PROTECTED_VARIABLE_INFO       VariableN0;

  for (Index = 1; Index < StoreNumber; ++Index) {
    UT_ASSERT_EQUAL (VariableStore[Index]->Size, VariableStore[0]->Size);
  }

  VariableN0.Address = NULL;
  VariableN0.Offset  = 0;
  while (TRUE) {
    Status = ContextIn->GetNextVariableInfo (VariableStore[0], NULL, NULL, &VariableN0);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (VariableN0.Address->State != VAR_ADDED) {
      continue;
    }

    UT_ASSERT_NOT_EQUAL (VariableN0.Offset, 0);

    for (Index = 1; Index < StoreNumber; ++Index) {
      DEBUG ((DEBUG_INFO, "             Checking store=%d, offset=%04x\r\n", Index, VariableN0.Offset));
      UT_ASSERT_MEM_EQUAL (
        (VOID *)VariableN0.Address,
        (VOID *)((UINTN)VariableStore[Index] + VariableN0.Offset),
        VARIABLE_SIZE (&VariableN0)
        );
    }
  }

  DEBUG ((DEBUG_INFO, "            -------------------------------\r\n", Index, VariableN0.Offset));

  return UNIT_TEST_PASSED;
}

STATIC UINT8  mBuffer[0x400];
STATIC UINT8  Variable[0x400];                             

UNIT_TEST_STATUS
EFIAPI
TC26_ProtectedVariableLibInitialize (
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS                          Status;
  PROTECTED_VARIABLE_CONTEXT_IN       ContextIn;
  PROTECTED_VARIABLE_GLOBAL           *Global;

  ContextIn.StructSize = sizeof (ContextIn);
  ContextIn.StructVersion = 1;

  ContextIn.FindVariableSmm     = FindVariableInSmm;
  ContextIn.GetVariableInfo     = GetVariableInfo;
  ContextIn.GetNextVariableInfo = GetNextVariableInfo;
  ContextIn.VariableServiceUser = FromRuntimeModule;
  ContextIn.IsUserVariable      = NULL;
  ContextIn.UpdateVariableStore = NULL;
  ContextIn.InitVariableStore   = NULL;

  Status = ProtectedVariableLibInitialize (&ContextIn);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  Status = GetProtectedVariableContext (NULL, &Global);
  UT_ASSERT_EQUAL (Status, EFI_SUCCESS);

  return UNIT_TEST_PASSED;
}
