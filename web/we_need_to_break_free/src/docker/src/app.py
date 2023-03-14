from flask import Flask, render_template, send_file, abort, request
import os
import subprocess

#COMMAND = "shasum" #For Mac
COMMAND = "sha1sum" #For Ubuntu

app = Flask(__name__)

@app.route('/', defaults={'req_path': ''})
@app.route('/<path:req_path>')
def dir_listing(req_path):
    """"List directoritngs in path for rendering"""

    # Joining the base and the requested path
    abs_path = os.path.join('./data', req_path)

    # Return 404 if path doesn't exist
    if not os.path.exists(abs_path):
        return abort(404)

    # Check if path is a file and serve
    if os.path.isfile(abs_path):
        return send_file(abs_path)

    # Show directory contents
    files = os.listdir(abs_path)

    # just files
    just_files = []
    for i in os.listdir(abs_path):
        if os.path.isfile(abs_path + i):
            just_files.append(i)
    return render_template('files.html', files=files, just_files=just_files)

@app.route('/compute_hash', methods=['POST'])
def compute_hash():
    """"Compute SHA1 hash on selected file using bash command"""
    input_text = request.form['computeHashDropDown']
    try:
        output = subprocess.check_output(
            f"{COMMAND} {'./data'}/{input_text}",
            shell=True, text=True)
    except Exception as exception:
        output = exception
    return f"<p>{output}</p>"
