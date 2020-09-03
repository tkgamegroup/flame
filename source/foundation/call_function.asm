.code
__call proc
    sub rsp, 8h

    mov rax, rcx
    mov r10, rdx
    mov r11, r8

    mov rcx, qword ptr [r10]
    mov rdx, qword ptr [r10 + 8]
    mov r8, qword ptr [r10 + 16]
    mov r9, qword ptr [r10 + 24]

    push qword ptr [r10 + 32]
    push qword ptr [r10 + 40]

    movss xmm0, dword ptr [r11]
	movss xmm1, dword ptr [r11 + 4]
	movss xmm2, dword ptr [r11 + 8]
	movss xmm3, dword ptr [r11 + 12]

    sub rsp, 20h
    call rax
    add rsp, 20h
    
    pop rdx
    pop rcx

    mov qword ptr [rcx], rax
    movss dword ptr [rdx], xmm0

    add rsp, 8h

    ret
__call endp
end
