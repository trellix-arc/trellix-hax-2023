import angr, claripy, subprocess

proj = angr.Project("../challenge/decode", auto_load_libs=False)
flag = claripy.BVS('flag', 8*36)

inp = open("input").read()
state = proj.factory.entry_state(args=['../challenge/decode', flag, inp], add_options=angr.options.unicorn)
simgr = proj.factory.simulation_manager(state)
simgr.explore(find=0x00401070)

if len(simgr.found) > 0:
    s = simgr.found[0] 
    outbv = s.memory.load(s.regs.rdi, 36)
    s.solver.add(outbv == open("output", "rb").read())
    sol = s.solver.eval(flag, cast_to=bytes)
    print("solution: ", sol)
    print("check: ", subprocess.check_output(["../challenge/decode", sol, inp]))
