.data
extern TEST_Trampoline : qword
extern TEST_TrampolineSkip : qword

.code
TestHook proc
xor rax, rax
jz skip
jmp qword ptr [TEST_Trampoline]
skip:
jmp qword ptr [TEST_TrampolineSkip]
TestHook endp

end
