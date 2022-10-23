.data
extern ProcAddr : qword

.code
JMPToProc proc
jmp qword ptr [ProcAddr]
JMPToProc endp

end
