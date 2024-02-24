/*
 * Generic syscall call.
 * Upon entry:
 *	%eax: system call number  - caller save
 *	%ebx: arg0 to system call -   callee save
 *	%ecx: arg1                - caller save
 *	%edx: arg2                - caller save
 *	%esi: arg3                -   callee save
 *	%edi: arg4                -   callee save
 *	%ebp: arg5                -   callee save
 */

#include "private/bionic_asm.h"

ENTRY(syscall)
    # Push the callee save registers.
    push    %ebx
    .cfi_adjust_cfa_offset 4
    .cfi_rel_offset ebx, 0
    push    %esi
    .cfi_adjust_cfa_offset 4
    .cfi_rel_offset esi, 0
    push    %edi
    .cfi_adjust_cfa_offset 4
    .cfi_rel_offset edi, 0
    push    %ebp
    .cfi_adjust_cfa_offset 4
    .cfi_rel_offset ebp, 0

    # Get and save the system call entry address.
    call    __kernel_syscall
    push    %eax
    .cfi_adjust_cfa_offset 4
    .cfi_rel_offset eax, 0

    # Load all the arguments from the calling frame.
    # (Not all will be valid, depending on the syscall.)
    mov     24(%esp),%eax
    mov     28(%esp),%ebx
    mov     32(%esp),%ecx
    mov     36(%esp),%edx
    mov     40(%esp),%esi
    mov     44(%esp),%edi
    mov     48(%esp),%ebp

    # Make the system call.
    call    *(%esp)
    addl    $4, %esp

    # Error?
    cmpl    $-MAX_ERRNO, %eax
    jb      1f
    # Yes, so set errno.
    negl    %eax
    pushl   %eax
    call    __set_errno_internal
    addl    $4, %esp
1:
    # Restore the callee save registers.
    pop    %ebp
    .cfi_adjust_cfa_offset -4
    .cfi_restore ebp
    pop    %edi
    .cfi_adjust_cfa_offset -4
    .cfi_restore edi
    pop    %esi
    .cfi_adjust_cfa_offset -4
    .cfi_restore esi
    pop    %ebx
    .cfi_adjust_cfa_offset -4
    .cfi_restore ebx
    ret
END(syscall)
