// EasyHook (File: EasyHookSys\main.c)
//
// Copyright (c) 2009 Christoph Husse & Copyright (c) 2015 Justin Stenning
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Please visit https://easyhook.github.io for more information
// about the project and latest updates.

#include "stdafx.h"

typedef struct _DRIVER_NOTIFICATION_
{
    SLIST_ENTRY		ListEntry;
    ULONG			ProcessId;
}DRIVER_NOTIFICATION, *PDRIVER_NOTIFICATION;

NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT InDriverObject,
    IN PUNICODE_STRING InRegistryPath);

NTSTATUS EasyHookDispatchCreate(
    IN PDEVICE_OBJECT InDeviceObject,
    IN PIRP	InIrp);

NTSTATUS EasyHookDispatchClose(
    IN PDEVICE_OBJECT InDeviceObject,
    IN PIRP InIrp);

NTSTATUS EasyHookDispatchDeviceControl(
    IN PDEVICE_OBJECT InDeviceObject,
    IN PIRP InIrp);

VOID EasyHookUnload(IN PDRIVER_OBJECT DriverObject);

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, EasyHookDispatchCreate)
#pragma alloc_text(PAGE, EasyHookDispatchClose)
#pragma alloc_text(PAGE, EasyHookDispatchDeviceControl)
#pragma alloc_text(PAGE, EasyHookUnload)

#endif

void OnImageLoadNotification(
    IN PUNICODE_STRING  FullImageName,
    IN HANDLE  ProcessId, // where image is mapped
    IN PIMAGE_INFO  ImageInfo)
{
    UNREFERENCED_PARAMETER(FullImageName);
    UNREFERENCED_PARAMETER(ProcessId);
    UNREFERENCED_PARAMETER(ImageInfo);

    LhModuleListChanged = TRUE;
}

/**************************************************************

Description:

Initializes the driver and also loads the system specific PatchGuard
information.
*/
NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT		InDriverObject,
    IN PUNICODE_STRING		InRegistryPath)
{
    UNREFERENCED_PARAMETER(InRegistryPath);

    NTSTATUS						Status;
    UNICODE_STRING					NtDeviceName;
    UNICODE_STRING					DosDeviceName;
    PEASYHOOK_DEVICE_EXTENSION		DeviceExtension;
    PDEVICE_OBJECT					DeviceObject = NULL;
    BOOLEAN							SymbolicLink = FALSE;

    /*
    Create device...
    */
    RtlInitUnicodeString(&NtDeviceName, EASYHOOK_DEVICE_NAME);

    Status = IoCreateDevice(
        InDriverObject,
        sizeof(EASYHOOK_DEVICE_EXTENSION),		// DeviceExtensionSize
        &NtDeviceName,					// DeviceName
        FILE_DEVICE_EASYHOOK,			// DeviceType
        0,								// DeviceCharacteristics
        TRUE,							// Exclusive
        &DeviceObject					// [OUT]
        );

    if (!NT_SUCCESS(Status))
        goto ERROR_ABORT;

    /*
    Expose interfaces...
    */
    DeviceExtension = (PEASYHOOK_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    DeviceExtension->MaxVersion = EASYHOOK_INTERFACE_v_1;

// Disable warning C4276: no prototype provided; assumed no parameters
#pragma warning(disable: 4276)
    DeviceExtension->API_v_1.RtlGetLastError = RtlGetLastError;
    DeviceExtension->API_v_1.RtlGetLastErrorString = RtlGetLastErrorString;
    DeviceExtension->API_v_1.LhInstallHook = LhInstallHook;
    DeviceExtension->API_v_1.LhUninstallHook = LhUninstallHook;
    DeviceExtension->API_v_1.LhWaitForPendingRemovals = LhWaitForPendingRemovals;
    DeviceExtension->API_v_1.LhBarrierGetCallback = LhBarrierGetCallback;
    DeviceExtension->API_v_1.LhBarrierGetReturnAddress = LhBarrierGetReturnAddress;
    DeviceExtension->API_v_1.LhBarrierGetAddressOfReturnAddress = LhBarrierGetAddressOfReturnAddress;
    DeviceExtension->API_v_1.LhBarrierBeginStackTrace = LhBarrierBeginStackTrace;
    DeviceExtension->API_v_1.LhBarrierEndStackTrace = LhBarrierEndStackTrace;
    DeviceExtension->API_v_1.LhBarrierPointerToModule = LhBarrierPointerToModule;
    DeviceExtension->API_v_1.LhBarrierGetCallingModule = LhBarrierGetCallingModule;
    DeviceExtension->API_v_1.LhBarrierCallStackTrace = LhBarrierCallStackTrace;
    DeviceExtension->API_v_1.LhSetGlobalExclusiveACL = LhSetGlobalExclusiveACL;
    DeviceExtension->API_v_1.LhSetGlobalInclusiveACL = LhSetGlobalInclusiveACL;
    DeviceExtension->API_v_1.LhSetExclusiveACL = LhSetExclusiveACL;
    DeviceExtension->API_v_1.LhSetInclusiveACL = LhSetInclusiveACL;
    DeviceExtension->API_v_1.LhIsProcessIntercepted = LhIsProcessIntercepted;

    /*
    Register for user-mode accessibility and set major functions...
    */
    RtlInitUnicodeString(&DosDeviceName, EASYHOOK_DOS_DEVICE_NAME);

    if (!NT_SUCCESS(Status = IoCreateSymbolicLink(&DosDeviceName, &NtDeviceName)))
        goto ERROR_ABORT;

    SymbolicLink = TRUE;

    InDriverObject->MajorFunction[IRP_MJ_CREATE] = EasyHookDispatchCreate;
    InDriverObject->MajorFunction[IRP_MJ_CLOSE] = EasyHookDispatchClose;
    InDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = EasyHookDispatchDeviceControl;
    InDriverObject->DriverUnload = EasyHookUnload;

    // initialize EasyHook
    if (!NT_SUCCESS(Status = LhBarrierProcessAttach()))
        goto ERROR_ABORT;

    PsSetLoadImageNotifyRoutine(OnImageLoadNotification);

    LhCriticalInitialize();

    return LhUpdateModuleInformation();

ERROR_ABORT:

    /*
    Rollback in case of errors...
    */
    if (SymbolicLink)
        IoDeleteSymbolicLink(&DosDeviceName);

    if (DeviceObject != NULL)
        IoDeleteDevice(DeviceObject);

    return Status;
}

NTSTATUS EasyHookDispatchCreate(
    IN PDEVICE_OBJECT InDeviceObject,
    IN PIRP InIrp)
{
    UNREFERENCED_PARAMETER(InDeviceObject);

    InIrp->IoStatus.Information = 0;
    InIrp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(InIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS EasyHookDispatchClose(
    IN PDEVICE_OBJECT InDeviceObject,
    IN PIRP InIrp)
{
    UNREFERENCED_PARAMETER(InDeviceObject);

    InIrp->IoStatus.Information = 0;
    InIrp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(InIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

/************************************************************

Description:

Handles all device requests.

*/
NTSTATUS EasyHookDispatchDeviceControl(
    IN PDEVICE_OBJECT InDeviceObject,
    IN PIRP	InIrp)
{
    UNREFERENCED_PARAMETER(InDeviceObject);

    InIrp->IoStatus.Information = 0;
    InIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;

    IoCompleteRequest(InIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

/***************************************************

Description:

Release all resources and remove the driver object.
*/
VOID EasyHookUnload(IN PDRIVER_OBJECT InDriverObject)
{
    UNICODE_STRING			DosDeviceName;

    // remove all hooks and shutdown thread barrier...
    LhCriticalFinalize();

    LhBarrierProcessDetach();

    PsRemoveLoadImageNotifyRoutine(OnImageLoadNotification);

    /*
    Delete the symbolic link
    */
    RtlInitUnicodeString(&DosDeviceName, EASYHOOK_DOS_DEVICE_NAME);

    IoDeleteSymbolicLink(&DosDeviceName);

    /*
    Delete the device object
    */

    IoDeleteDevice(InDriverObject->DeviceObject);
}

