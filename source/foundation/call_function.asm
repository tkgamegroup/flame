.code
__call proc
    push rdx

    mov rax, rcx
    mov r10, r8
    mov r11, r9

    mov rcx, qword ptr [r10]
    mov rdx, qword ptr [r10 + 8]
    mov r8, qword ptr [r10 + 16]
    mov r9, qword ptr [r10 + 24]

    movss xmm0, dword ptr [r11]
	movss xmm1, dword ptr [r11 + 4]
	movss xmm2, dword ptr [r11 + 8]
	movss xmm3, dword ptr [r11 + 12]

    sub rsp, 20h
    call rax
    add rsp, 20h

    pop rdx
    and rdx, rdx
    jz e
    mov qword ptr [rdx], rax
e:
    ret
__call endp
end
