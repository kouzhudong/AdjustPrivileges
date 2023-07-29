#include "pch.h"


int g_TokenOffset;
PVOID g_SystemTokenObject;


int GetTokenOffsetInProcess()
/*
���ܣ���ȡToken��Process�����е�ƫ������

ע�⣺�˺���ֻ��������������ڡ�

�ο���Ϣ��
0: kd> ?? sizeof(nt!_eprocess)
unsigned int64 0xa40
0: kd> dt nt!_eprocess token 
   +0x4b8 Token : _EX_FAST_REF
*/
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    CLIENT_ID   ClientId = {0};
    OBJECT_ATTRIBUTES ob;
    HANDLE  ProcessHandle;
    HANDLE TokenHandle = 0;
    int TokenOffset = 0;

    ClientId.UniqueProcess = PsGetCurrentProcessId();
    InitializeObjectAttributes(&ob, 0, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
    Status = ZwOpenProcess(&ProcessHandle, GENERIC_ALL, &ob, &ClientId);
    ASSERT(NT_SUCCESS(Status));

    Status = ZwOpenProcessTokenEx(ProcessHandle, TOKEN_ALL_ACCESS, OBJ_KERNEL_HANDLE, &TokenHandle);
    ASSERT(NT_SUCCESS(Status));

    PVOID ProcessObject = nullptr;
    Status = ObReferenceObjectByHandle(ProcessHandle, GENERIC_ALL, *PsProcessType, KernelMode, &ProcessObject, 0);
    ASSERT(NT_SUCCESS(Status));

    PVOID TokenObject = nullptr;
    Status = ObReferenceObjectByHandle(TokenHandle, TOKEN_ALL_ACCESS, *SeTokenObjectType, KernelMode, &TokenObject, 0);
    ASSERT(NT_SUCCESS(Status));

    g_SystemTokenObject = TokenObject;//��һ���Զ��Ч�����õ������ü�����ά�����⡣

    size_t * ProcessObjectAddress = (size_t *)ProcessObject;
    size_t TokenObjectAddress = (size_t)TokenObject;

    for (int i = 0; i < 0xa00 / sizeof(size_t); i++) {
        size_t tmp = ProcessObjectAddress[i];
        tmp = tmp & ~0xf;
        if (tmp == TokenObjectAddress) {
            TokenOffset = i * sizeof(size_t);
            break;
        }
    }

    ObDereferenceObject(ProcessObject);
    ObDereferenceObject(TokenObject);
    Status = ZwClose(TokenHandle);
    Status = ZwClose(ProcessHandle);
    return TokenOffset;
}
