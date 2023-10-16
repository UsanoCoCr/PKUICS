0000000000001a02 <phase_6>:
    1a02:	f3 0f 1e fa          	endbr64 
    1a06:	41 54                	push   %r12
    1a08:	55                   	push   %rbp
    1a09:	53                   	push   %rbx
    1a0a:	48 83 ec 60          	sub    $0x60,%rsp
    1a0e:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
    1a15:	00 00 
    1a17:	48 89 44 24 58       	mov    %rax,0x58(%rsp)
    1a1c:	31 c0                	xor    %eax,%eax
    1a1e:	48 89 e6             	mov    %rsp,%rsi

    1a21:	e8 ea 06 00 00       	call   2110 <read_six_numbers> # 读入6个数字
    1a26:	bd 00 00 00 00       	mov    $0x0,%ebp # ebp = 0
    1a2b:	eb 27                	jmp    1a54 <phase_6+0x52>
    1a2d:	e8 58 06 00 00       	call   208a <explode_bomb>
    1a32:	eb 33                	jmp    1a67 <phase_6+0x65>
    1a34:	83 c3 01             	add    $0x1,%ebx # ebx++
    1a37:	83 fb 05             	cmp    $0x5,%ebx
    1a3a:	7f 15                	jg     1a51 <phase_6+0x4f> # if ebx > 5 goto 1a51
    1a3c:	48 63 c5             	movslq %ebp,%rax # else
    1a3f:	48 63 d3             	movslq %ebx,%rdx 
    1a42:	8b 3c 94             	mov    (%rsp,%rdx,4),%edi # edi = rsp开头第ebx个数
    1a45:	39 3c 84             	cmp    %edi,(%rsp,%rax,4) 
    1a48:	75 ea                	jne    1a34 <phase_6+0x32> # 必须有rsp开头第ebx个数不等于rsp开头第ebp个数
    1a4a:	e8 3b 06 00 00       	call   208a <explode_bomb>
    1a4f:	eb e3                	jmp    1a34 <phase_6+0x32>
    1a51:	44 89 e5             	mov    %r12d,%ebp # ebp = r12d
    1a54:	83 fd 05             	cmp    $0x5,%ebp 
    1a57:	7f 17                	jg     1a70 <phase_6+0x6e> # if ebp > 5 goto 1a70
    1a59:	48 63 c5             	movslq %ebp,%rax # if ebp <= 5
    1a5c:	8b 04 84             	mov    (%rsp,%rax,4),%eax # eax = (%rsp,%rax,4)
    1a5f:	83 e8 01             	sub    $0x1,%eax # eax--
    1a62:	83 f8 05             	cmp    $0x5,%eax 
    1a65:	77 c6                	ja     1a2d <phase_6+0x2b> # if eax > 5 , explode_bomb
    1a67:	44 8d 65 01          	lea    0x1(%rbp),%r12d # r12d = ebp + 1
    1a6b:	44 89 e3             	mov    %r12d,%ebx # ebx = r12d
    1a6e:	eb c7                	jmp    1a37 <phase_6+0x35>

# 得到的线索： 输入必须是1~6的数字，且不能重复

    1a70:	be 00 00 00 00       	mov    $0x0,%esi # esi = 0
    1a75:	eb 08                	jmp    1a7f <phase_6+0x7d>
    1a77:	48 89 54 cc 20       	mov    %rdx,0x20(%rsp,%rcx,8)
    1a7c:	83 c6 01             	add    $0x1,%esi
    1a7f:	83 fe 05             	cmp    $0x5,%esi 
    1a82:	7f 1d                	jg     1aa1 <phase_6+0x9f> # if esi > 5 goto 1aa1
    1a84:	b8 01 00 00 00       	mov    $0x1,%eax # if esi <= 5 , eax = 1
    1a89:	48 8d 15 c0 65 00 00 	lea    0x65c0(%rip),%rdx        # 8050 <node1>
    1a90:	48 63 ce             	movslq %esi,%rcx # rcx = esi
    1a93:	39 04 8c             	cmp    %eax,(%rsp,%rcx,4)
    1a96:	7e df                	jle    1a77 <phase_6+0x75> # if eax <= (%rsp,%rcx,4) goto 1a77
    1a98:	48 8b 52 08          	mov    0x8(%rdx),%rdx # else , rdx = 0x8(%rdx)
    1a9c:	83 c0 01             	add    $0x1,%eax # eax++
    1a9f:	eb ef                	jmp    1a90 <phase_6+0x8e> 

    1aa1:	48 8b 5c 24 20       	mov    0x20(%rsp),%rbx # rbx = 0x20(%rsp)
    1aa6:	48 89 d9             	mov    %rbx,%rcx # rcx = rbx
    1aa9:	b8 01 00 00 00       	mov    $0x1,%eax # eax = 1
    1aae:	eb 12                	jmp    1ac2 <phase_6+0xc0>
    1ab0:	48 63 d0             	movslq %eax,%rdx # rdx = eax
    1ab3:	48 8b 54 d4 20       	mov    0x20(%rsp,%rdx,8),%rdx # rdx = 0x20(%rsp,%rdx,8)
    1ab8:	48 89 51 08          	mov    %rdx,0x8(%rcx) # 0x8(%rcx) = rdx
    1abc:	83 c0 01             	add    $0x1,%eax # eax++
    1abf:	48 89 d1             	mov    %rdx,%rcx # rcx = rdx
    1ac2:	83 f8 05             	cmp    $0x5,%eax
    1ac5:	7e e9                	jle    1ab0 <phase_6+0xae> # if eax <= 5 goto 1ab0
    1ac7:	48 c7 41 08 00 00 00 	movq   $0x0,0x8(%rcx) # 0x8(%rcx) = 0
    1ace:	00 
    1acf:	bd 00 00 00 00       	mov    $0x0,%ebp # ebp = 0
    1ad4:	eb 07                	jmp    1add <phase_6+0xdb>
    1ad6:	48 8b 5b 08          	mov    0x8(%rbx),%rbx
    1ada:	83 c5 01             	add    $0x1,%ebp
    1add:	83 fd 04             	cmp    $0x4,%ebp
    1ae0:	7f 11                	jg     1af3 <phase_6+0xf1> # if ebp > 4 goto 1af3
    1ae2:	48 8b 43 08          	mov    0x8(%rbx),%rax # else , rax = 0x8(%rbx)
    1ae6:	8b 00                	mov    (%rax),%eax # eax = (%rax)
    1ae8:	39 03                	cmp    %eax,(%rbx) 
    1aea:	7d ea                	jge    1ad6 <phase_6+0xd4> # eax >= (%rbx) goto 1ad6, else bomb
    1aec:	e8 99 05 00 00       	call   208a <explode_bomb>

    
    1af1:	eb e3                	jmp    1ad6 <phase_6+0xd4>
    1af3:	48 8b 44 24 58       	mov    0x58(%rsp),%rax # rax = 0x58(%rsp)
    1af8:	64 48 33 04 25 28 00 	xor    %fs:0x28,%rax 
    1aff:	00 00 
    1b01:	75 09                	jne    1b0c <phase_6+0x10a> # if rax != 0 goto 1b0c
    1b03:	48 83 c4 60          	add    $0x60,%rsp
    1b07:	5b                   	pop    %rbx
    1b08:	5d                   	pop    %rbp
    1b09:	41 5c                	pop    %r12
    1b0b:	c3                   	ret    
    1b0c:	e8 6f f7 ff ff       	call   1280 <__stack_chk_fail@plt>