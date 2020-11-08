section .data
color_red: db 1Bh, '[31;1m', 0
.len equ $ - color_red
color_default: db 1Bh,'[37;1m',0
.len: equ $ - color_default
section .bss

global asmPrint
section .text
asmPrint:
    ;get parameters
    ;the address of string in eax
    ;the mode in ebx
    mov eax, [esp+4]
    mov ebx, [esp+8]
    
    cmp ebx,0
    jne set_red
    call set_color_default
    jmp color_print
    
set_red:
    call set_color_red

color_print:
    call sprint
    ret


;functions

set_color_red:
    push edx
    push ecx
    push ebx
    push eax
    
    mov eax,4
    mov ebx,1
    mov ecx,color_red
    mov edx,color_red.len
    int 80h
    
    pop eax
    pop ebx
    pop ecx
    pop edx
    ret
    
set_color_default:
    push edx
    push ecx
    push ebx
    push eax
    
    mov eax,4
    mov ebx,1
    mov ecx,color_default
    mov edx,color_default.len
    int 80h
    
    pop eax
    pop ebx
    pop ecx
    pop edx
    ret
    
common_functions:

;the quit function
quit:
    mov ebx, 0
    mov eax, 1
    int 80h
    ret

;the length of a string end with 0
;eax as the start address
;eax as return
str_len:
    push ebx
    mov ebx, eax
 
sl_loop1:
    cmp byte[eax], 0
    jz sl_end
    inc eax
    jmp sl_loop1

sl_end:
    sub eax, ebx
    pop ebx
    ret 
    
;print a string
;eax as the start address
sprint:
    push edx
    push ecx
    push ebx
    push eax
    call str_len
    
    mov edx, eax
    pop eax
    
    mov ecx, eax
    mov ebx, 1
    mov eax, 4
    int 80h
    
    pop ebx
    pop ecx
    pop edx
    ret