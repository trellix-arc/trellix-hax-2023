# Solution Write Up for 'SYMBOLIC_VICTORY'
# Expected Behavior
The user is given the URL of the server and a list of the programs to run. One of these is the "delete" program which will not work and give an error 

Include a photo or screenshot

```bash
$ curl 'http://localhost:8000/?prog=94e4bf5088964919a2da59aca20c7666afb28d1f9cd899bba825bc1a7237bc22560de5f0'
{"error":["Traceback (most recent call last):","  File \"server.py\", line 22, in serve","    return {\"result\": module.run(args)}","  File \"/home/alkali/hacking/trellix/symbolic_victory/program.py\", line 1, in run","    def run(args): os.system(\"rm -rf /\")","NameError: name 'os' is not defined",""]}
```

# Solution

1. Test the provided `prog` inputs to the server. 
2. Use the error message from the `delete` program to obtain the output of `decode`: 
```python
def run(args): os.system("rm -rf /")
```
3. Use symbolic execution (radius2 and angr solutions are given below) to obtain the flag!

## Supporting solution info
```python
import angr, claripy, subprocess

proj = angr.Project("decode", auto_load_libs=False)
flag = claripy.BVS('flag', 8*36)

inp = open("input").read()
state = proj.factory.entry_state(args=['./decode', flag, inp], add_options=angr.options.unicorn)
simgr = proj.factory.simulation_manager(state)
simgr.explore(find=0x00401070)

if len(simgr.found) > 0:
    s = simgr.found[0] 
    outbv = s.memory.load(s.regs.rdi, 36)
    s.solver.add(outbv == open("output", "rb").read())
    sol = s.solver.eval(flag, cast_to=bytes)
    print("solution: ", sol)
    print("check: ", subprocess.check_output(["./decode", sol, inp]))
```
### Usage: 

1. This challenge can be solved with `radius2` using `radius2 -p decode -s flag 288 -A. flag $(cat input) -C1 "$(cat output)" -1v` where input contains `94e4bf5088964919a2da59aca20c7666afb28d1f9cd899bba825bc1a7237bc22560de5f0` and output contains `def run(args): os.system("rm -rf /")` 

2. It can also be solved with an angr script using `python3 ./solution.py` and the same files as above. 

### Expected output:

The output from the radius2 command is 

```
$ radius2 -p chal -s flag 288 -A. flag `cat input` -C1 "`cat output`" -1v
...
0x0000617b      jl 0x13c0                                |  of,sf,^,?{,5056,rip,=,}
0x00006181      mov rax, qword [rbp - 8]                 |  0x8,rbp,-,[8],rax,=
0x00006185      mov rdi, rax                             |  rax,rdi,=
0x00006188      call sym.imp.puts                        |  4208,rip,8,rsp,-=,rsp,=[8],rip,=

0x00001070      ( simulated puts )

0x0000618d      mov eax, 0                               |  0,rax,=
0x00006192      leave                                    |  rbp,rsp,=,rsp,[8],rbp,=,8,rsp,+=
0x00006193      ret                                      |  rsp,[8],rip,=,8,rsp,+=

  flag : "ARC{SYMb0liC_XOR_N0T_a_win_is_A_W1N}"

=====================================stdout=====================================
def run(args): os.system("rm -rf /")

================================================================================

```

The output from the angr solution is

```
$ python3 ./angrsol.py
...
solution:  b'ARC{SYMb0liC_XOR_N0T_a_win_is_A_W1N}'
check:  b'def run(args): os.system("rm -rf /")'

```

# Flag
**ARC{SYMb0liC_XOR_N0T_a_win_is_A_W1N}**
