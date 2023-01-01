.ORIG x3000
LD R2, x00ff		;x3000 (R2 = p)
LD R1, x00ff		;x3001 (R1 = q)
LD R0, x00ff		;x3002 (R0 = N)
ADD R2, R2, #-1		;x3003 (R2 = p-1)
NOT R6, R1			;x3004
ADD R6, R6, #1		;x3005 (R6 = -q)
ADD R3, R3, #1		;x3006 (R3 = F(N-1))
ADD R4, R4, #1		;x3007 (R4 = F(N-2))
ADD R0, R0, #-1		;x3008 (R0 = N-1)
BRnz #8				;x3009 循环结束条件：R0 <= 0
ADD R5, R3, #0		;x300a (R5 = R3)
ADD R5, R6, R5		;x300b (R5 = R5 - q)
BRzp #-2			;x300c 循环结束条件：R5 < 0
ADD R5, R5, R1		;x300d 得到模 q 余数
AND R7, R4, R2		;x300e 得到模 p 余数
ADD R4, R3, #0		;x300f F(N-2) = F(N-1)
ADD R3, R5, R7		;x3010 F(N-1) = F(N-2) % p + F(N-1) % q
BRnzp #-10			;x3011 
ST R3, x00f0		;x3012 存储结果
TRAP x25
.END