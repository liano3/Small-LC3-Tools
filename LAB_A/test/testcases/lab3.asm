.ORIG x3000
LDI R0, NUM		;存储字符串长度
LD R1, DATA		;指向下一个要扫描的字符的地址
;初始化
ADD R2, R2, #1		;R2=1
LDR R3, R1, #0		;R3=M[R1]
ADD R4, R4, #1		;R4=1
;循环
AGAIN ADD R0, R0, #-1		;剩余字符个数--
BRnz SKIP	;循环出口
ADD R1, R1, #1	;指针后移
LDR R5, R1, #0	;获取当前字符
;比较当前字符与上一个字符
NOT R6, R5
ADD R6, R6, #1	;R6=-R5
ADD R6, R6, R3
BRz SKIP1	;跳过
;判断R4与R2大小
NOT R6, R4
ADD R6, R6, #1	;R6=-R4
ADD R6, R6, R2
BRzp SKIP2
;更新最长长度
AND R2, R2, #0
ADD R2, R2, R4	;R2=R4
;更新当前叠子串字符
SKIP2 AND R3, R3, #0
ADD R3, R3, R5	;R3=R5
AND R4, R4, #0	;当前长度清零
SKIP1 ADD R4, R4, #1		;当前长度++
BRnzp AGAIN		;回到循环起点
;循环结束，判断是否更新R2
SKIP  NOT R6, R4
ADD R6, R6, #1	;R6=-R4
ADD R6, R6, R2
BRzp THEND
;更新最长长度
AND R2, R2, #0	;R2=0
ADD R2, R2, R4	;R2=R4
;存储结果
THEND STI R2, RESULT
HALT
RESULT .FILL x3050
NUM .FILL x3100
DATA .FILL x3101
.END