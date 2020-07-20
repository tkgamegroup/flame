.code
fun_asm proc
    sub rsp, 20h
    mov rax, rcx
    mov rcx, rdx
    call rax
    add rsp, 20h
    ret
fun_asm endp
end
