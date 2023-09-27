# Machine-Level Programming II: Control

### Why use lea?
- CPU designers intended to use: calculate a pointer to an object
- Compiler author like to use it for arithmetic
   
### Control Flow
```c++
extern void op1(void);
extern void op2(void);

void decision(int x){
    if (x){
        op1();
    }
    else{
        op2();
    }
}
```
```assembly
decision:
    tsubl $8, %esp
    je .L2 // jump if x == 0
    call op1
    jmp .L1
.L2:
    call op2
.L1:
    addl $8, %esp
    ret
```

### Processor State
- information about currently executing program
    - Temporary data: %rax, %rcx, %rdx, %rsi, %rdi, %r8, %r9, %r10, %r11
    - Location of runtime stack: %rsp
    - Location of current code control point: %rip
    - Status of current test: CF, ZF, SF, OF

### Condition Codes
- CF: carry flag
- ZF: zero flag
- SF: sign flag
- OF: overflow flag

- e.g. : addq src, dest $\equalscolon$ t = a + b
    - CF set if carry/borrow out of most significant bit (unsigned overflow)
    - ZF set if result is zero (t == 0)
    - SF set if result is negative (t < 0)  
    - OF set if two's complement overflow (signed overflow)
        - (a>0 && b>0 && t<0) || (a<0 && b<0 && t>0)

**Not set by leaq instruction**
lea不会改变条件码

CF: 如果是无符号数，那么借位、补位就意味着溢出，CF和二者等价

### Condition Codes: Compare
- cmpq src1, src2 $\equalscolon$ t = src2 - src1
    - CF set if unsigned src2 < src1
    - ZF set if src2 == src1
    - SF set if src2 < src1 (signed)
    - OF set if two's complement overflow (signed)

### Condition Codes: Test
- testq src1, src2 $\equalscolon$ t = src1 & src2
    - ZF set if t == 0
    - SF set if t < 0
**very often: testq %rax, %rax**

### Reading Condition Codes
- setX instructions
    - set low-order byte of destination to 0 or 1 based on combinations of condition codes
    
    | setX | Condition | Description |
    | --- | --- | --- |
    | sete | ZF | Equal |
    | setne | ~ZF | Not equal |
    | sets | SF | Negative |
    | setns | ~SF | Nonnegative |
    | setg | \~(SF^OF)&~ZF | Greater than (signed) |
    | setge | ~(SF^OF) | Greater than or equal (signed) |
    | setl | (SF^OF) | Less than (signed) |
    | setle | (SF^OF)| Less than or equal (signed) |
    | seta | \~CF&~ZF | Above (unsigned) |
    | setb | CF | Below (unsigned) |

- e.g. setl(signed <)

    | SF | OF | SF^OF | Implication |
    | --- | --- | --- | --- |
    | 0 | 0 | 0 | a-b >= 0 & no overflow |
    | 0 | 1 | 1 | (a-b >= 0 & overflow) == a-b<0| 
    | 1 | 0 | 1 | a-b < 0 & no overflow|
    | 1 | 1 | 0 | (a-b < 0 & overflow) == a-b>=0|

### Jumping
- jump to different part of code based on condition codes

    | jX | Condition | Description |
    | --- | --- | --- |
    | jmp | 1 | Unconditional |
    | je | ZF | Equal |
    | jne | ~ZF | Not equal |
    | js | SF | Negative |
    | jns | ~SF | Nonnegative |
    | jg | \~(SF^OF)&~ZF | Greater than (signed) |
    | jge | ~(SF^OF) | Greater than or equal (signed) |
    | jl | (SF^OF) | Less than (signed) |
    | jle | (SF^OF)| Less than or equal (signed) |
    | ja | \~CF&~ZF | Above (unsigned) |
    | jb | CF | Below (unsigned) |


### General Conditional Expression Translation
C Code:
```c++
    val = Test ? Then_Expr : Else_Expr;
```
Goto Version:
```c++
    ntest = !Test;
    if (ntest) goto Else;
    val = Then_Expr;
    goto Done;
Else:
    val = Else_Expr;
Done:
```
### Using Conditional Moves
- Conditional Move Instructions
    - Instruction supports: if (Test) Dest = Src;
    - GCC tried to use them when known to be safe

- Why
    - Branches are very disruptive to instruction flow through pipeline
    - Conditional moves do not require control transfer

- Bad Cases
    - Expensive Computation
        - Both then and else value get computed
        - only make sense when computations are cheap
    - Risky Computation
        - May have undesirable effects
    - Computation with side effects
        - Must be side-effect free

### Loops
- General Do-While Translation
    ```c++
    do
        Body
    while (Test);
    ```
    ```c++
    //goto version
    loop:
        Body
        if (test) goto loop;
    ```

    - Do-While Loop Compilation
        ```assembly
            movl $1, %eax // set result to 0
        .L2:
            imulq %rdi, %rax // result *= x
            subq $1, %rdi // n--
            cmpq $1, %rdi // n == 1?
            jg .L2 // if n > 1, goto .L2
            rep; ret // return result
        ```

- While Loop Example
- For Loop Form

### Switch Statements

    