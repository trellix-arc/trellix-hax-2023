from flask import Flask, request
import importlib, os, traceback
from subprocess import check_output

programs = {
    "94e4bf5088964919a2da59aca20c7666afb28d1f9cd899bba825bc1a7237bc22560de5f0": "delete"
}

app = Flask(__name__)
@app.route("/")
def serve():
    try:
        prog = request.args.get("prog", "")
        args = request.args.get("args", [])
        if prog in programs:
            secret = os.environ["flag"]
            code = check_output(["./decode", secret, prog])
            with open("program.py", "wb+") as f:
                f.write(code)

            module = importlib.import_module("program")
            return {"result": module.run(args)}
    except Exception as e:
        return {"error": traceback.format_exc().split("\n")}

    return {"error": "program not in catalogue"}

app.run("0.0.0.0", 8000)
