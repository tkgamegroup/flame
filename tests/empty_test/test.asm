.code
fun_asm proc
    sub rsp, 20h
    call rcx
    add rsp, 20h
    ret
fun_asm endp
end
