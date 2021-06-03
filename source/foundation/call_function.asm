.code
__call proc
    sub rsp, 8h

    mov rax, rcx
    mov r10, rdx
    mov r11, r8

    mov rcx, qword ptr [r10 + 0]  ; first int-parameter
    mov rdx, qword ptr [r10 + 8]  ; second int-parameter
    mov r8,  qword ptr [r10 + 16] ; third int-parameter
    mov r9,  qword ptr [r10 + 24] ; fourth int-parameter

    push qword ptr [r10 + 32] ; store int-return address
    push qword ptr [r10 + 40] ; store float-return address

    movss xmm0, dword ptr [r11 + 0]  ; first float-parameter
	movss xmm1, dword ptr [r11 + 4]  ; second float-parameter
	movss xmm2, dword ptr [r11 + 8]  ; third float-parameter
	movss xmm3, dword ptr [r11 + 12] ; fourth float-parameter

    sub rsp, 20h
    call rax
    add rsp, 20h
    
    pop rdx ; restore int-return address
    pop rcx ; restore float-return address

    mov   qword ptr [rcx], rax  ; get int-return from rax (for small return)
    movss dword ptr [rdx], xmm0 ; get float-return from xmm0 (for small return)
    ; big return is returned through first parameter

    add rsp, 8h

    ret
__call endp
end
