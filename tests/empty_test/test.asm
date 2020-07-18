extrn printf:proc

.data
hello db 'Hello 64-bit world!',0ah,0

.code
main proc
        mov     rcx,offset hello
        sub     rsp,20h
        call    printf
        add     rsp,20h

        mov     rcx,0
main endp
end