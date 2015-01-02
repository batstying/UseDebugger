.386
.model flat, stdcall
option casemap:none

include windows.inc
include kernel32.inc
include user32.inc

includelib kernel32.lib
includelib user32.lib

READ_VIOLATION  equ 0
WRITE_VIOLATION equ 1

.data
    g_szDone   db "Done", 0
    g_szBPoint db "断点来了", 0
    g_szRead   db "Read", 0
    g_szWrite  db "Write", 0
    g_szDivZero db "除零错误", 0
    g_szDebuged db "被调戏！", 0
    g_szFmtAccessViolation db "0x%p 指令引用的 0x%p 内存。该内存不能为%s", 0Dh, 0Ah
                           db "要终止程序，请单击“确定”。", 0Dh, 0Ah
                           db "要调试程序，请单击“取消”。", 0
                           
    g_dwKey   dd 0  ;  关键代码执行所依赖，调试情况下不会被初始化
                           
.data?                           
    g_dwReadWrite dd  ?  ;this can be read/written
    g_dwOldProtect dd ?
    g_dwOriCode    db ?  ; original code replaced with 0CCh
                                            
.code

;SEH Handler for Dived Zero
HandleDivZero proc uses esi edi _lpExceptionRecord : DWORD, \ 
                    _lpSEH : DWORD, \
            	    _lpContext : DWORD, \
            	    _lpDispatcherContext : DWORD
            	    
    local @ptrEXCEPTION_RECORD
    local @ptrCONTEXT_RECORD
    local @flgReadWrite
    local @addrInAccess
    local @szErrInfo[128]
       
    mov esi, _lpExceptionRecord
    assume esi: ptr EXCEPTION_RECORD
    
    mov edi, _lpContext
    assume edi: ptr CONTEXT
    
    push [esi].ExceptionInformation
    pop @flgReadWrite
    
    lea eax, [esi].ExceptionInformation
    add eax, 4
    push [eax]
    pop @addrInAccess    
    
    .if [esi].ExceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO
        invoke MessageBox, NULL, offset g_szDivZero, NULL, MB_OK
        
        ;now we change the context, to make it runnable continually
        mov [edi].regEcx, 7FFFFFFFh
        
        mov eax, ExceptionContinueExecution
    .else
        mov eax, ExceptionContinueSearch
    .endif

    assume edi: nothing
    assume esi: nothing
    ret
HandleDivZero endp

HandleAccessViolation proc uses esi edi _lpExceptionRecord : DWORD, \ 
                        _lpSEH : DWORD, \
            	        _lpContext : DWORD, \
            	        _lpDispatcherContext : DWORD
            	        
    local @ptrEXCEPTION_RECORD
    local @ptrCONTEXT_RECORD
    local @flgReadWrite
    local @addrInAccess
    local @szErrInfo[128]
       
    mov esi, _lpExceptionRecord
    assume esi: ptr EXCEPTION_RECORD
    
    mov edi, _lpContext
    assume edi: ptr CONTEXT
    
    push [esi].ExceptionInformation
    pop @flgReadWrite
    
    lea eax, [esi].ExceptionInformation
    add eax, 4
    push [eax]
    pop @addrInAccess    
    
    .if [esi].ExceptionCode == EXCEPTION_ACCESS_VIOLATION 
        .if @flgReadWrite == READ_VIOLATION
            invoke wsprintf, addr @szErrInfo, offset g_szFmtAccessViolation, \
                                    [esi].ExceptionAddress,\
                                    @addrInAccess, \
                                    offset g_szRead
                                    
        .elseif @flgReadWrite == WRITE_VIOLATION
            invoke wsprintf, addr @szErrInfo, offset g_szFmtAccessViolation, \
                                    [esi].ExceptionAddress,\
                                    @addrInAccess, \
                                    offset g_szWrite
        .endif
        
        invoke MessageBox, NULL, addr @szErrInfo, NULL, MB_ICONERROR or MB_OKCANCEL
        
        ;now we change the context, to make it runnable continually
        lea eax, g_dwReadWrite
        mov [edi].regEcx, eax
        
        mov eax, ExceptionContinueExecution
    .else
        mov eax, ExceptionContinueSearch
    .endif
       
    assume edi: nothing
    assume esi: nothing 
    ret

HandleAccessViolation endp

HandleBreakPoint proc uses esi edi _lpExceptionRecord : DWORD, \ 
                        _lpSEH : DWORD, \
            	        _lpContext : DWORD, \
            	        _lpDispatcherContext : DWORD
            	        
    local @ptrEXCEPTION_RECORD
    local @ptrCONTEXT_RECORD
    local @flgReadWrite
    local @addrInAccess
    local @szErrInfo[128]
       
    mov esi, _lpExceptionRecord
    assume esi: ptr EXCEPTION_RECORD
    
    mov edi, _lpContext
    assume edi: ptr CONTEXT
    
    push [esi].ExceptionInformation
    pop @flgReadWrite
    
    lea eax, [esi].ExceptionInformation
    add eax, 4
    push [eax]
    pop @addrInAccess    
    
    .if [esi].ExceptionCode == EXCEPTION_BREAKPOINT
        invoke MessageBox, NULL, offset g_szBPoint, NULL, MB_OK  
        inc [edi].regEip            
        mov eax, ExceptionContinueExecution
    .else
        mov eax, ExceptionContinueSearch
    .endif

    assume edi: nothing
    assume esi: nothing

    ret
HandleBreakPoint endp  

FunD proc

    xor ecx, ecx
    mov DWORD PTR [ecx], ecx
    
    ret

FunD endp

;B4 we uses FS, remember to set nothing
assume FS: nothing   
FunC proc 
    ;在SEH链栈顶插入异常回调函数
    push HandleBreakPoint
    mov eax, FS:[0]
    push eax
    mov FS:[0], esp
        
    invoke FunD
       
    xor ecx, ecx
    div ecx
    
    int 3
    
    ;还原SEH链
    pop FS:[0]
    pop ecx
    ret

FunC endp   

FunB proc 
    ;在SEH链栈顶插入异常回调函数
    push HandleAccessViolation
    mov eax, FS:[0]
    push eax
    mov FS:[0], esp
    
    invoke FunC
    
    int 3
     
    
    ;还原SEH链
    pop FS:[0]
    pop ecx
    ret

FunB endp  

FunA proc 
    ;在SEH链栈顶插入异常回调函数
    push HandleDivZero
    mov eax, FS:[0]
    push eax
    mov FS:[0], esp
    
    invoke FunB
    
    
    ;还原SEH链
    pop FS:[0]
    pop ecx
    ret

FunA endp 

start:
    invoke FunA


; things to illustrate exceptions
BPOINT:
    int 3
        
    xor ecx, ecx
WRITEVIO:
    mov DWORD PTR [ecx], ecx
          
    mov ecx, 87654321h
READVIO:
    mov ecx, [ecx]
    
    xor ecx, ecx
DIVZERO:
    div ecx
  
    
    invoke MessageBox, NULL, offset g_szDone, NULL, MB_OK
    invoke ExitProcess, 0

end start